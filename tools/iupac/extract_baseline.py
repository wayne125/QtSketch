#!/usr/bin/env python3
"""
Extract baseline golden data from iupac_namer_test.cpp and its execution output.

Parses:
1. baseline_raw.txt (from iupac_namer_test.exe output) - for all data
2. tests/iupac_namer_test.cpp - for extracting pinned_name/pinned_success

Outputs: tests/data/iupac/golden/baseline.csv

CSV columns: id,smiles,pinned_name,pinned_success,actual_name,actual_success,expected_status,notes

Usage:
    python tools/iupac/extract_baseline.py [--cpp path/to/iupac_namer_test.cpp] [--raw path/to/baseline_raw.txt] [--output path/to/baseline.csv]
"""

import re
import csv
import sys
import os
import argparse
import codecs


def unescape_cpp_string(s):
    """Unescape a C++ string literal."""
    if not s or len(s) < 2 or s[0] != '"' or s[-1] != '"':
        return s
    content = s[1:-1]
    try:
        return codecs.decode(content, 'unicode_escape')
    except:
        return content


def parse_raw_output_for_results(raw_path):
    """
    Parse the baseline_raw.txt output to extract test results.
    Returns a dict: test_id -> (actual_name, actual_success)
    """
    results = {}
    
    with open(raw_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Pattern for PASS lines with name:
    # [PASS] Test N (SMILES) -> name
    # Note: SMILES can contain parentheses, so use non-greedy match
    pass_pattern_name = re.compile(
        r'\[PASS\] Test (\d+) \((.*?)\) -> (.+)$'
    )
    
    # Pattern for PASS lines with rejection:
    # [PASS] Test N (SMILES) correctly rejected: error message
    pass_pattern_reject = re.compile(
        r'\[PASS\] Test (\d+) \((.*?)\) correctly rejected: (.+)$'
    )
    
    # Pattern for FAIL lines:
    # [FAIL] Test N (SMILES) expected '...', got success=... name='...' err='...'
    fail_pattern = re.compile(
        r'\[FAIL\] Test (\d+) \((.*?)\).*'
    )
    
    for line in content.split('\n'):
        line = line.strip()
        if not line:
            continue
        
        # Try PASS with name
        match = pass_pattern_name.match(line)
        if match:
            test_id = int(match.group(1))
            name = match.group(3).strip()  # Group 3 is the name after ->
            results[test_id] = (name, True)
            continue
        
        # Try PASS with rejection
        match = pass_pattern_reject.match(line)
        if match:
            test_id = int(match.group(1))
            error_msg = match.group(3).strip()  # Group 3 is the error message after :
            results[test_id] = (error_msg, False)
            continue
        
        # Try FAIL
        match = fail_pattern.match(line)
        if match:
            test_id = int(match.group(1))
            # Extract details
            success_match = re.search(r'success=(\w+)', line)
            name_match = re.search(r"name='([^']*)'", line)
            err_match = re.search(r"err='([^']*)'", line)
            
            actual_success = success_match.group(1) == 'true' if success_match else False
            actual_name = name_match.group(1) if name_match else ''
            error_msg = err_match.group(1) if err_match else ''
            
            # For FAIL lines, use error message as name
            results[test_id] = (actual_name if actual_name else error_msg, actual_success)
            continue
    
    return results


def parse_raw_output_for_smiles(raw_path):
    """
    Parse the baseline_raw.txt to extract SMILES strings for all tests.
    Returns a dict: test_id -> smiles
    """
    smiles_dict = {}
    
    with open(raw_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Pattern for PASS lines with name: [PASS] Test N (SMILES) -> name
    # Pattern for PASS lines with rejection: [PASS] Test N (SMILES) correctly rejected: error
    # Pattern for FAIL lines: [FAIL] Test N (SMILES) expected ...
    # Note: SMILES can contain parentheses, use non-greedy match
    pass_pattern_name = re.compile(
        r'\[PASS\] Test (\d+) \((.*?)\) ->'
    )
    pass_pattern_reject = re.compile(
        r'\[PASS\] Test (\d+) \((.*?)\) correctly rejected:'
    )
    fail_pattern = re.compile(
        r'\[FAIL\] Test (\d+) \((.*?)\)'
    )
    
    for line in content.split('\n'):
        line = line.strip()
        if not line:
            continue
        
        # Try PASS with name
        match = pass_pattern_name.match(line)
        if match:
            test_id = int(match.group(1))
            smiles = match.group(2)
            if smiles and test_id not in smiles_dict:
                smiles_dict[test_id] = smiles
            continue
        
        # Try PASS with rejection
        match = pass_pattern_reject.match(line)
        if match:
            test_id = int(match.group(1))
            smiles = match.group(2)
            if smiles and test_id not in smiles_dict:
                smiles_dict[test_id] = smiles
            continue
        
        # Try FAIL
        match = fail_pattern.match(line)
        if match:
            test_id = int(match.group(1))
            smiles = match.group(2)
            if smiles and test_id not in smiles_dict:
                smiles_dict[test_id] = smiles
            continue
    
    return smiles_dict


def extract_test_cases_from_cpp(cpp_path):
    """
    Extract test cases from the C++ source file.
    Returns a dict: test_id -> (smiles, pinned_name, pinned_success, error_substring)
    """
    test_data = {}
    
    with open(cpp_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find the tests vector
    start_marker = 'std::vector<TestCase> tests = {'
    start_idx = content.find(start_marker)
    if start_idx == -1:
        print("ERROR: Could not find tests vector start")
        return test_data
    
    # Find the end of the tests vector (the matching })
    brace_count = 0
    end_idx = start_idx
    found_open = False
    for i in range(start_idx, len(content)):
        if content[i] == '{':
            brace_count += 1
            found_open = True
        elif content[i] == '}':
            brace_count -= 1
            if found_open and brace_count == 0:
                end_idx = i + 1
                break
    
    tests_str = content[start_idx + len(start_marker):end_idx].strip()
    
    # Now parse individual test cases
    # Split by lines and look for test entries
    test_id = 1
    lines = tests_str.split('\n')
    
    for line in lines:
        line = line.strip()
        if not line or line.startswith('//'):
            continue
        
        # Remove trailing comma or semicolon
        if line.endswith(',') or line.endswith(';'):
            line = line[:-1].strip()
        
        # Remove leading/trailing braces
        if line.startswith('{'):
            line = line[1:].strip()
        if line.endswith('}'):
            line = line[:-1].strip()
        
        if not line:
            continue
        
        # Parse: "SMILES", "expectedName"
        # or: "SMILES", "expectedName", true, "error"
        # or: "SMILES", "expectedName", false
        # Use a simpler approach: tokenize properly
        tokens = []
        i = 0
        n = len(line)
        while i < n:
            if line[i] in ' \t':
                i += 1
                continue
            if line[i] == '"':
                # Quoted string
                j = i + 1
                while j < n and line[j] != '"':
                    if line[j] == '\\':
                        j += 2
                    else:
                        j += 1
                if j < n:
                    tokens.append(line[i:j+1])
                    i = j + 1
                else:
                    break
            elif line[i] == ',':
                i += 1
            else:
                # Unquoted token
                j = i
                while j < n and line[j] not in ', \t':
                    j += 1
                tokens.append(line[i:j])
                i = j
        
        if len(tokens) < 2:
            continue
        
        # Unescape quoted tokens
        unescaped = []
        for t in tokens:
            if t and t[0] == '"' and t[-1] == '"':
                unescaped.append(unescape_cpp_string(t))
            else:
                unescaped.append(t)
        
        smiles = unescaped[0]
        expected_name = unescaped[1]
        should_fail = False
        error_substring = ''
        
        if len(unescaped) >= 3:
            if unescaped[2] == 'true':
                should_fail = True
            elif unescaped[2] == 'false':
                should_fail = False
        
        if len(unescaped) >= 4:
            error_substring = unescaped[3]
        
        pinned_name = expected_name if not should_fail else ''
        pinned_success = not should_fail
        
        test_data[test_id] = (smiles, pinned_name, pinned_success, error_substring)
        test_id += 1
    
    return test_data


def main():
    parser = argparse.ArgumentParser(description='Extract baseline golden data from IUPAC namer tests')
    parser.add_argument('--cpp', default='tests/iupac_namer_test.cpp', help='Path to C++ test file')
    parser.add_argument('--raw', default='baseline_raw.txt', help='Path to raw test output')
    parser.add_argument('--output', default='tests/data/iupac/golden/baseline.csv', help='Path to output CSV')
    args = parser.parse_args()
    
    # Parse raw output
    print(f"Parsing raw output: {args.raw}")
    results = parse_raw_output_for_results(args.raw)
    smiles_from_output = parse_raw_output_for_smiles(args.raw)
    max_test_id = max(results.keys()) if results else 0
    print(f"Found {len(results)} test results in output, max ID: {max_test_id}")
    
    # Parse C++ test file
    print(f"Parsing C++ test file: {args.cpp}")
    cpp_data = extract_test_cases_from_cpp(args.cpp)
    print(f"Found {len(cpp_data)} test cases in C++ source")
    
    # Create output directory
    output_dir = os.path.dirname(args.output)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
    
    # Build CSV rows
    csv_rows = []
    fieldnames = ['id', 'smiles', 'pinned_name', 'pinned_success', 'actual_name', 'actual_success', 'expected_status', 'notes']
    
    for test_id in range(1, max_test_id + 1):
        # Get pinned data from C++ source
        if test_id in cpp_data:
            smiles, pinned_name, pinned_success, error_substring = cpp_data[test_id]
        else:
            smiles = smiles_from_output.get(test_id, '')
            pinned_name = ''
            pinned_success = True
            error_substring = ''
        
        # Get actual results
        if test_id in results:
            actual_name, actual_success = results[test_id]
            notes = ''
        else:
            actual_name = ''
            actual_success = False
            notes = 'Result not found in output'
        
        # If SMILES is empty but we have it from output, use that
        if not smiles and test_id in smiles_from_output:
            smiles = smiles_from_output[test_id]
        
        # Mark missing SMILES
        if not smiles:
            notes = 'smiles not extracted, needs follow-up' if not notes else notes + '; smiles not extracted'
        
        csv_rows.append({
            'id': test_id,
            'smiles': smiles,
            'pinned_name': pinned_name,
            'pinned_success': str(pinned_success).lower(),
            'actual_name': actual_name,
            'actual_success': str(actual_success).lower(),
            'expected_status': '',
            'notes': notes
        })
    
    # Parse additional formats from raw output
    # Phase 31, Phase 48, stereo anchor, etc.
    extra_results = parse_additional_formats(args.raw, max_test_id)
    print(f"Found {len(extra_results)} additional results")
    
    # Add extra results to csv_rows
    for test_id, smiles, actual_name, actual_success in extra_results:
        # For these, we don't have C++ data, so pinned_name is empty
        csv_rows.append({
            'id': test_id,
            'smiles': smiles,
            'pinned_name': '',
            'pinned_success': 'true',
            'actual_name': actual_name,
            'actual_success': str(actual_success).lower(),
            'expected_status': '',
            'notes': ''
        })
    
    # Write to CSV
    with open(args.output, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for row in csv_rows:
            writer.writerow(row)
    
    print(f"Wrote {len(csv_rows)} rows to {args.output}")
    
    # Verify counts
    expected_count = 1115
    if len(csv_rows) != expected_count:
        print(f"WARNING: Row count {len(csv_rows)} != expected {expected_count}")
    else:
        print(f"Row count matches expected {expected_count}")
    
    # Check for missing SMILES
    missing_smiles = [row for row in csv_rows if not row['smiles']]
    if missing_smiles:
        print(f"WARNING: {len(missing_smiles)} rows missing SMILES")
        print(f"First few missing: {missing_smiles[:3]}")
    else:
        print("All rows have SMILES")


def parse_additional_formats(raw_path, start_id):
    """Parse Phase 31, Phase 48, stereo anchor, and other formats."""
    extra = []
    next_id = start_id + 1
    
    with open(raw_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    for line in content.split('\n'):
        line = line.strip()
        
        # Skip Test N format (already parsed)
        if line.startswith('[PASS] Test ') or line.startswith('[FAIL] Test '):
            continue
        
        # Skip non-PASS lines
        if not line.startswith('[PASS]'):
            continue
        
        # Try to extract SMILES and name from various formats
        # Format: [PASS] Phase N name (SMILES) -> result
        # Format: [PASS] Phase N description -> result (no SMILES in output)
        # Format: [PASS] Stereo anchor SMILES1 vs SMILES2 -> name1 vs name2
        # Format: [PASS] Some description -> name
        
        try:
            # Check for Stereo anchor format FIRST (before generic handlers)
            if 'Stereo anchor' in line and ' vs ' in line and ' -> ' in line:
                parts = line.split(' -> ', 1)
                stereo_part = parts[0].replace('[PASS] Stereo anchor ', '')
                names = parts[1]
                if ' vs ' in stereo_part and ' vs ' in names:
                    s1, s2 = stereo_part.split(' vs ')
                    n1, n2 = names.split(' vs ')
                    extra.append((next_id, s1, n1, True))
                    next_id += 1
                    extra.append((next_id, s2, n2, True))
                    next_id += 1
                    continue
            
            # Check if it has (SMILES) pattern
            if '(' in line and ')' in line and ' -> ' in line:
                # Try to extract SMILES from parentheses
                # Look for a pattern like "(SMILES)" where SMILES doesn't contain spaces
                match = re.search(r'\(([^)\s]+)\)', line)
                if match:
                    smiles = match.group(1)
                    # Extract name after ->
                    name = line.split(' -> ', 1)[1]
                    extra.append((next_id, smiles, name, True))
                    next_id += 1
                    continue
            
            # Check for SMILES before -> without parentheses
            # Pattern: [PASS] Phase 61 description: SMILES -> name
            # or: [PASS] description SMILES -> name
            if ' -> ' in line:
                before_arrow = line.split(' -> ', 1)[0]
                name = line.split(' -> ', 1)[1]
                # Try to find a SMILES at the end of before_arrow
                # SMILES typically don't have spaces
                tokens = before_arrow.split()
                if tokens:
                    last_token = tokens[-1]
                    # Simple heuristic: if it has no spaces and looks like a chemical identifier
                    # Just use it as SMILES
                    # This is a rough approximation
                    if ':' in before_arrow:
                        # Pattern: "description: SMILES"
                        after_colon = before_arrow.split(':', 1)[1]
                        last_token = after_colon.strip()
                    # Use the last token as SMILES
                    if last_token and not last_token.startswith('['):
                        extra.append((next_id, last_token, name, True))
                        next_id += 1
                        continue
                    else:
                        # No SMILES found
                        extra.append((next_id, '', name, True))
                        next_id += 1
                        continue
                else:
                    extra.append((next_id, '', name, True))
                    next_id += 1
                    continue
            
            # Check for Stereo anchor format
            if 'Stereo anchor' in line and ' vs ' in line and ' -> ' in line:
                parts = line.split(' -> ', 1)
                stereo_part = parts[0].replace('[PASS] Stereo anchor ', '')
                names = parts[1]
                if ' vs ' in stereo_part and ' vs ' in names:
                    s1, s2 = stereo_part.split(' vs ')
                    n1, n2 = names.split(' vs ')
                    extra.append((next_id, s1, n1, True))
                    next_id += 1
                    extra.append((next_id, s2, n2, True))
                    next_id += 1
                    continue
            
            # Check for stereo format: [PASS] Stereo description -> name1 vs name2
            if line.startswith('[PASS] Stereo ') and ' -> ' in line:
                parts = line.split(' -> ', 1)
                names = parts[1]
                if ' vs ' in names:
                    # Skip - no SMILES in output
                    pass
                else:
                    # Single stereo result
                    extra.append((next_id, '', names, True))
                    next_id += 1
                continue
            
            # Generic format: [PASS] description -> name
            # These don't have SMILES in the output
            if ' -> ' in line:
                parts = line.split(' -> ', 1)
                name = parts[1]
                extra.append((next_id, '', name, True))
                next_id += 1
                continue
            
        except Exception as e:
            # Silently skip parsing errors
            pass
    
    return extra


if __name__ == '__main__':
    main()

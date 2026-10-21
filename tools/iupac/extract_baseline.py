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
        parts = [p.strip().strip('"') for p in line.split(',')]
        
        if len(parts) < 2:
            continue
        
        smiles = parts[0]
        expected_name = parts[1]
        should_fail = False
        error_substring = ''
        
        if len(parts) >= 3:
            if parts[2] == 'true':
                should_fail = True
            elif parts[2] == 'false':
                should_fail = False
        
        if len(parts) >= 4:
            error_substring = parts[3]
        
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


if __name__ == '__main__':
    main()

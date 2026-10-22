// tests/iupac_golden_test.cpp
// Golden-data regression test harness for IUPAC naming.
// Reads tests/data/iupac/golden/baseline.csv and verifies that
// generateName() produces identical output to the baseline.

#include "IupacNamer.h"
#include "AtomOrderRandomizer.h"
#include "indigo.h"
#include "indigo-inchi.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <map>
#include <set>

// Helper to convert NamingStatus enum to string (Task C)
static std::string namingStatusToString(NamingStatus status) {
    switch (status) {
        case NamingStatus::Failed: return "Failed";
        case NamingStatus::KnownGapFallback: return "KnownGapFallback";
        case NamingStatus::SystematicIupacName: return "SystematicIupacName";
        case NamingStatus::PreferredIupacName: return "PreferredIupacName";
        default: return "Unknown";
    }
}

struct GoldenRow {
    int id;
    std::string smiles;
    std::string pinned_name;
    bool pinned_success;
    std::string actual_name;
    bool actual_success;
    std::string expected_status;
    std::string notes;
};

std::vector<GoldenRow> read_baseline_csv(const std::string& path, bool& hadMalformedRow) {
    std::vector<GoldenRow> rows;
    hadMalformedRow = false;
    std::ifstream file(path);
    
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open baseline CSV: " << path << std::endl;
        return rows;
    }
    
    std::string line;
    // Read and discard header
    if (!std::getline(file, line)) {
        std::cerr << "ERROR: Empty baseline CSV" << std::endl;
        return rows;
    }
    
    int line_num = 1;
    while (std::getline(file, line)) {
        line_num++;
        if (line.empty()) continue;
        
        std::istringstream ss(line);
        std::string token;
        GoldenRow row;
        
        // Parse CSV fields (comma-separated, values may be quoted)
        std::vector<std::string> fields;
        std::string field;
        bool in_quotes = false;
        char prev = '\0';
        
        for (char ch : line) {
            if (ch == '"') {
                if (in_quotes && prev == '"') {
                    // Escaped quote
                    field += ch;
                } else {
                    in_quotes = !in_quotes;
                }
            } else if (ch == ',' && !in_quotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field += ch;
            }
            prev = ch;
        }
        fields.push_back(field);
        
        if (fields.size() < 7) {
            std::cerr << "ERROR: Line " << line_num << " has only " << fields.size() << " fields, expected 8 -- treating baseline.csv as corrupt" << std::endl;
            hadMalformedRow = true;
            continue;
        }
        
        // Remove quotes from fields
        auto unquote = [](std::string& s) {
            if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
                s = s.substr(1, s.size() - 2);
            }
        };
        
        try {
            row.id = std::stoi(fields[0]);
        } catch (...) {
            std::cerr << "ERROR: Invalid id on line " << line_num << " -- treating baseline.csv as corrupt" << std::endl;
            hadMalformedRow = true;
            continue;
        }
        
        unquote(fields[1]);
        unquote(fields[2]);
        unquote(fields[3]);
        unquote(fields[4]);
        unquote(fields[5]);
        unquote(fields[6]);
        if (fields.size() > 7) unquote(fields[7]);
        
        row.smiles = fields[1];
        row.pinned_name = fields[2];
        row.pinned_success = (fields[3] == "true" || fields[3] == "TRUE" || fields[3] == "1");
        row.actual_name = fields[4];
        row.actual_success = (fields[5] == "true" || fields[5] == "TRUE" || fields[5] == "1");
        row.expected_status = fields[6];
        row.notes = (fields.size() > 7) ? fields[7] : "";
        
        rows.push_back(row);
    }
    
    return rows;
}

std::set<int> read_known_determinism_failures(const std::string& path) {
    std::set<int> known;
    std::ifstream file(path);
    if (!file.is_open()) return known;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        try {
            known.insert(std::stoi(line));
        } catch (...) {
            // Not a row id, ignore.
        }
    }
    return known;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    
    // Determine the baseline CSV path using SKETCH_SOURCE_DIR compile definition
    std::string csv_path = std::string(SKETCH_SOURCE_DIR) + "/tests/data/iupac/golden/baseline.csv";
    
    // Read baseline
    bool hadMalformedRow = false;
    std::vector<GoldenRow> rows = read_baseline_csv(csv_path, hadMalformedRow);
    if (rows.empty()) {
        std::cerr << "ERROR: No rows read from baseline CSV" << std::endl;
        return 1;
    }
    if (hadMalformedRow) {
        std::cerr << "FATAL: baseline.csv contains malformed row(s) (see ERROR lines above) -- "
                      "a corrupted golden file must never silently pass." << std::endl;
        return 1;
    }

    std::cout << "Loaded " << rows.size() << " golden rows from " << csv_path << std::endl;
    
    // Check for determinism mode
    bool determinism_mode = false;
    const char* det_env = getenv("IUPAC_GOLDEN_DETERMINISM");
    if (det_env && (std::string(det_env) == "1" || std::string(det_env) == "true")) {
        determinism_mode = true;
        std::cout << "Determinism check mode enabled" << std::endl;
    }

    std::set<int> known_determinism_failures;
    if (determinism_mode) {
        std::string known_path = std::string(SKETCH_SOURCE_DIR) +
            "/tests/data/iupac/golden/known_determinism_failures.txt";
        known_determinism_failures = read_known_determinism_failures(known_path);
        std::cout << "Loaded " << known_determinism_failures.size()
                  << " known pre-existing determinism failures from " << known_path << std::endl;
    }
    
    qword sid = indigoAllocSessionId();
    indigoSetSessionId(sid);
    indigoInchiInit(sid);
    
    int total_passed = 0;
    int total_failed = 0;
    int total_pin_mismatches = 0;
    int total_known_determinism_failures = 0;
    int total_determinism_verified = 0;
    int total_determinism_skipped = 0;
    
    for (const auto& row : rows) {
        // Skip rows with empty SMILES (no way to test)
        if (row.smiles.empty()) {
            std::cout << "[SKIP] Row " << row.id << ": " << row.notes << std::endl;
            continue;
        }
        
        int mol = indigoLoadMoleculeFromString(row.smiles.c_str());
        if (mol < 0) {
            std::cout << "[FAIL] Row " << row.id << " (" << row.smiles << "): Indigo failed to parse SMILES." << std::endl;
            total_failed++;
            continue;
        }
        
        IupacResult res = IupacNamer::generateName(mol);
        indigoFree(mol);
        
        bool fresh_success = res.success;
        std::string fresh_name = res.name.toStdString();
        std::string fresh_error = res.error.toStdString();
        // For determinism check and status comparison (Task C)
        std::string fresh_status = namingStatusToString(res.status);
        
        // For failing results, use error as name
        if (!fresh_success) {
            fresh_name = fresh_error;
        }
        
        // Check 0: Status comparison (Task C)
        bool status_match = true;
        if (!row.expected_status.empty()) {
            status_match = (fresh_status == row.expected_status);
        }
        
        // Check 1: Compare against pinned expectation
        bool pin_match = false;
        if (row.pinned_success) {
            // Expect success and name to match
            pin_match = (fresh_success && fresh_name == row.pinned_name);
        } else {
            // Expect failure (name should be empty, error should match)
            pin_match = (!fresh_success && !row.pinned_name.empty() ? 
                        (fresh_name == row.pinned_name) : 
                        !fresh_success);
        }
        
        // Check 2: Compare against actual baseline (behavior fingerprint)
        bool fingerprint_match = (fresh_success == row.actual_success && 
                                  fresh_name == row.actual_name);
        
        // Check 3: Status must match if expected_status is specified (Task C)
        bool all_checks_pass = fingerprint_match && status_match;
        
        if (all_checks_pass) {
            // All checks pass - this is the critical check
            // Log pin mismatch if present, but still PASS
            if (!pin_match) {
                std::cout << "[PASS (pin mismatch)] Row " << row.id << " (" << row.smiles << ") -> "
                          << (fresh_success ? fresh_name : "REJECTED: " + fresh_error) << std::endl;
                std::cout << "  Note: Pinned expectation differs from actual" << std::endl;
                total_pin_mismatches++;
            } else {
                std::cout << "[PASS] Row " << row.id << " (" << row.smiles << ") -> " 
                          << (fresh_success ? fresh_name : "REJECTED: " + fresh_error) << std::endl;
            }
            total_passed++;
        } else {
            // At least one check failed
            std::cout << "[FAIL] Row " << row.id << " (" << row.smiles << "):" << std::endl;
            if (!fingerprint_match) {
                std::cout << "  Fingerprint check: FAIL (expected success=" << row.actual_success
                          << ", name='" << row.actual_name << "')" << std::endl;
                std::cout << "  Actual: success=" << fresh_success
                          << ", name='" << fresh_name << "'" << std::endl;
            }
            if (!status_match) {
                std::cout << "  Status check: FAIL (expected '" << row.expected_status
                          << "', got '" << fresh_status << "')" << std::endl;
            }
            if (!pin_match) {
                std::cout << "  (Pinned check also FAIL)" << std::endl;
            }
            total_failed++;
        }
        
        // Determinism check: for each seed, build a REAL seeded atom-order
        // permutation of this row's SMILES (not the same string re-parsed)
        // and confirm generateName produces the identical result. A
        // molecule the randomizer cannot verify a reordering for (stereo-
        // sensitive, or the equivalence self-check failed) is skipped for
        // that seed rather than silently counted as a pass.
        if (determinism_mode) {
            bool row_is_known_failure = known_determinism_failures.count(row.id) > 0;
            for (int s = 1; s <= 5; s++) {
                std::string reordered_smiles;
                bool reorder_ok = randomizeAtomOrder(row.smiles, s, reordered_smiles);
                if (!reorder_ok) {
                    std::cout << "  [DETERMINISM SKIP] Row " << row.id << ": no verified reordering for seed " << s << std::endl;
                    total_determinism_skipped++;
                    continue;
                }
                total_determinism_verified++;

                int mol_reorder = indigoLoadMoleculeFromString(reordered_smiles.c_str());
                if (mol_reorder < 0) {
                    std::cout << "  [DETERMINISM FAIL] Row " << row.id << ": reordered SMILES failed to parse for seed " << s << std::endl;
                    total_failed++;
                    break;
                }
                IupacResult res_reorder = IupacNamer::generateName(mol_reorder);
                indigoFree(mol_reorder);

                // Normalize the result for comparison
                bool reorder_success = res_reorder.success;
                std::string reorder_name = res_reorder.name.toStdString();
                std::string reorder_error = res_reorder.error.toStdString();
                if (!reorder_success) {
                    reorder_name = reorder_error;
                }
                std::string reorder_status = namingStatusToString(res_reorder.status);

                // Check both name AND status for stability (Task C - Step 5a)
                if (!(reorder_success == fresh_success &&
                      reorder_name == fresh_name &&
                      reorder_status == fresh_status)) {
                    bool status_diverged = (reorder_status != fresh_status);
                    
                    if (row_is_known_failure) {
                        std::cout << "  [DETERMINISM KNOWN FAIL] Row " << row.id << ": seed " << s
                                  << " produced a different result (pre-existing, see known_determinism_failures.txt)" << std::endl;
                        if (status_diverged) {
                            std::cout << "    [Status also diverged: expected '" << fresh_status
                                      << "', got '" << reorder_status << "']" << std::endl;
                        }
                        total_known_determinism_failures++;
                    } else {
                        std::cout << "  [DETERMINISM FAIL] Row " << row.id << ": seed " << s << " produced a different result" << std::endl;
                        std::cout << "    Expected: success=" << fresh_success
                                  << " name='" << fresh_name << "' status='" << fresh_status << "'" << std::endl;
                        std::cout << "    Got: success=" << reorder_success
                                  << " name='" << reorder_name << "' status='" << reorder_status << "'" << std::endl;
                        total_failed++;
                    }
                    break;
                }
            }
        }
    }
    
    // Cleanup
    indigoInchiDispose(sid);
    indigoReleaseSessionId(sid);
    
    std::cout << "\nSummary: " << total_passed << " passed, " << total_failed << " failed, "
              << total_pin_mismatches << " pin mismatches." << std::endl;
    if (determinism_mode) {
        std::cout << "Determinism: " << total_determinism_verified << " seeds verified, "
                  << total_determinism_skipped << " seeds skipped, "
                  << total_known_determinism_failures << " known pre-existing failures." << std::endl;
        // If the randomizer never produced a single verified reordering,
        // the mechanism itself is broken (e.g. regressed back to a
        // no-op) -- that must be a hard failure, not a quiet all-skip
        // that looks identical to "nothing to test."
        if (total_determinism_verified == 0) {
            std::cerr << "FATAL: determinism check verified zero seeds across all rows -- "
                         "the atom-order randomizer itself appears broken." << std::endl;
            return 1;
        }
    }

    return (total_failed == 0) ? 0 : 1;
}

#!/bin/bash
set -u
cd "C:/Users/jp18b/Desktop/sketch/.claude_scratch_fdafull"
DRIVER="C:/Users/jp18b/Desktop/sketch/.claude_scratch_fdafull/smiles_driver_v43.exe"
OUT="namer_results_desalted_v43.tsv"
> "$OUT"

total=$(wc -l < desalted_drug_list.tsv)
count=0
hangs=0
crashes=0
while IFS=$'\t' read -r chembl_id name smiles; do
    count=$((count+1))
    result=$(timeout 8 "$DRIVER" "$smiles" < /dev/null 2>&1)
    code=$?
    if [ $code -eq 124 ]; then
        printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$chembl_id" "$name" "$smiles" "HANG" "0" "HANG (>8s)" >> "$OUT"
        hangs=$((hangs+1))
    elif [ $code -ne 0 ] && [ $code -ne 1 ]; then
        oneline=$(printf '%s' "$result" | tr '\n' ' ' | tr '\t' ' ')
        printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$chembl_id" "$name" "$smiles" "CRASH" "0" "exit=${code}: ${oneline}" >> "$OUT"
        crashes=$((crashes+1))
    else
        # $result is STATUS\theavyAtoms\tmessage already (from the driver, tab-separated)
        printf '%s\t%s\t%s\t%s\n' "$chembl_id" "$name" "$smiles" "$result" >> "$OUT"
    fi
    if [ $((count % 300)) -eq 0 ]; then
        echo "progress: $count / $total (hangs: $hangs, crashes: $crashes)"
    fi
done < desalted_drug_list.tsv

echo "DONE: $count processed, $hangs hangs, $crashes crashes"

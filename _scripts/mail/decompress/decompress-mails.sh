#!/usr/bin/env bash

MAILDIR="./"

# Gebruik find om alle gewone bestanden (.type f) te zoeken, 
# en sla de resultaten op in de array 'gevonden_bestanden'.
# De '-print0' en het null-teken zijn HIER NIET nodig,
# want Bash verdeelt de output op basis van nieuwe regels (standaard IFS).
files=($(find "$MAILDIR" -type f))
nfiles="${#files[@]}"   # aantal
c=0
e=0


for (( i = 1; i <= $nfiles; i++ )); do
    f="${files[i]}"
    magic=$(head -c 4 "$f" | xxd -p) 
    
    # controleer of bestand een zstd-bestand is
    if [[ "$magic" == "28b52ffd" ]]; then
        tmp="${f}.tmp"
        if zstd -d -f -o "$tmp" "$f"; then
            ((c++))
            mv -f "$tmp" "$f"
        else
            ((e++))
            echo "[FOUT] Kon niet uitgepakt worden:  $f" 
            rm -f "$tmp"
        fi
    else
        echo "[SKIP] Geen zstd-bestand: $f"
    fi
done
echo -e "\n\n\n   ${c} files processed successfully.\n   ${e} skipped due to errors."

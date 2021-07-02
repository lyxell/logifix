#!/bin/bash
regex="(S[0-9]+),.*/blob/[a-z0-9-]+/(.*),(.*)$"
set -e
set -o pipefail

csv="$1"
logifix_cli="$2"

tail +2 < "$csv" | while IFS= read -r f; do
    if [[ $f =~ $regex ]]; then
        rule_number="${BASH_REMATCH[1]}"
        file="${BASH_REMATCH[2]}"
        diff_flags="--color=always --unified ${BASH_REMATCH[3]}"
        input="${rule_number}/${file}"
        t1=$(mktemp)
        t2=$(mktemp)
        echo "$rule_number"
        cp "$input" "$t1"
        # run the logifix tool on t1
        "$logifix_cli" --in-place --accept="$rule_number" "$t1"
        # run the patch tool and place the result in t2
        patch -p0 --input="$input.diff" --output="$t2"
        # compare the diffs
        diff $diff_flags "$t1" "$t2"
        echo "$input passed"
    else
        echo "$f doesn't match"
        exit 1
    fi
done

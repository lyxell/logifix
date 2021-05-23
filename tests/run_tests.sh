#!/bin/bash
regex="([0-9]{4})([a-z0-9_]+),.*/blob/[a-z0-9-]+/(.*),(.*)$"
set -e
set -o pipefail

tail +2 < tests.csv | while IFS= read -r f; do
    if [[ $f =~ $regex ]]; then
        rule_number="${BASH_REMATCH[1]}"
        suffix="${BASH_REMATCH[2]}"
        file="${BASH_REMATCH[3]}"
        diff_flags="--color=always --unified ${BASH_REMATCH[4]}"
        input="${rule_number}${suffix}/${file}"
        t1=$(mktemp)
        t2=$(mktemp)
        echo "$rule_number"
        # run the logifix tool and place the result in t1
        "$1" "$input" "$rule_number" > "$t1"
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

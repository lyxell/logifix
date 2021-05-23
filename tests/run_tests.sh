#!/bin/bash
set -e
regex="([0-9]{4})([a-z0-9_]+)/.*"
pushd tests
for f in $(find . -type f -iname "*.java"); do
    echo "$f"
    if [[ "$f" =~ $regex ]]
    then
        rule_number="${BASH_REMATCH[1]}"
        suffix="${BASH_REMATCH[2]}"
        t1=$(mktemp)
        t2=$(mktemp)
        # run the logifix tool and place the result in t1
        "../$1" "$f" "$rule_number" > "$t1"
        # run the patch tool and place the result in t2
        patch -p0 --input="$f.diff" --output="$t2"
        # compare the diffs
        diff "$t1" "$t2"
        echo "$f passed"
    fi
done
popd

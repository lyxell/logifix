#!/bin/bash
set -e
regex=".*/([0-9]{4})[a-z0-9_]+/.*"
for f in $(find . -type f -iname "*.java"); do
    echo "$f"
    if [[ "$f" =~ $regex ]]
    then
        rule_number="${BASH_REMATCH[1]}"
        t1=$(mktemp)
        t2=$(mktemp)
        "$1" "$f" "$rule_number" > "$t1"
        set +e
        diff --unified --label "" --label "" "$f" "$t1" > "$t2"
        set -e
        diff <(tail -n +3 "$t2") <(tail -n +3 "$f.diff")
        echo "$f passed"
    fi
done


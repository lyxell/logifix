#!/bin/bash
files=$(tail +2 < tests.csv)
regex="([a-z0-9_]+),https://github.com/([A-Za-z0-9_-]+)/([\.A-Za-z0-9_-]+)/blob/([a-z0-9-]+)/(.*)"
set -e
set -o pipefail

if ! [ -x "$(command -v filterdiff)" ]; then
    echo "run sudo apt install -y patchutils"
    exit 1
fi

for f in $files; do
    if [[ $f =~ $regex ]]; then
        dir="${BASH_REMATCH[1]}"
        user="${BASH_REMATCH[2]}"
        repo="${BASH_REMATCH[3]}"
        hash="${BASH_REMATCH[4]}"
        path="${BASH_REMATCH[5]}"
        filename=$(basename "$path")
        if [[ ! -f "$dir/$filename" ]]; then
            mkdir -p "$dir"
            pushd "$dir"
            raw="https://raw.githubusercontent.com/$user/$repo/$hash/$path"
            diff="https://github.com/$user/$repo/commit/$hash.diff"
            curl "$raw" > "$filename"
            curl "$diff" | filterdiff --include="$path" --clean --strip-match=1 | tail +3 > "$filename.diff"
            patch --reverse < "$filename.diff"
            popd
        fi
    else
        echo "$f doesn't match" >&2
    fi
done

#!/bin/bash
csv="$1"
regex="([a-z0-9_]+),https://github.com/([A-Za-z0-9_-]+)/([\.A-Za-z0-9_-]+)/blob/([a-z0-9-]+)/(.*),"
set -e
set -o pipefail

if ! [ -x "$(command -v filterdiff)" ]; then
    echo "run sudo apt install -y patchutils"
    exit 1
fi

tail +2 < "$csv" | while IFS= read -r f; do
    if [[ $f =~ $regex ]]; then
        dir="${BASH_REMATCH[1]}"
        user="${BASH_REMATCH[2]}"
        repo="${BASH_REMATCH[3]}"
        hash="${BASH_REMATCH[4]}"
        location="${BASH_REMATCH[5]}"
        path=$(dirname "$location")
        filename=$(basename "$location")
        if [[ ! -f "$dir/$path/$filename" ]]; then
            mkdir -p "$dir/$path"
            pushd "$dir/$path"
            raw="https://raw.githubusercontent.com/$user/$repo/$hash/$location"
            diff="https://github.com/$user/$repo/commit/$hash.diff"
            curl "$raw" > "$filename"
            curl "$diff" | filterdiff --include="$location" --addprefix="$dir/" --clean --strip-match=1 --strip=1 | tail +3 > "$filename.diff"
            patch --reverse < "$filename.diff"
            # assert that the diff is not empty
            if ! [ -s "$filename.diff" ];then
                exit 1
            fi
            popd
        fi
    else
        echo "$f doesn't match" >&2
        exit 1
    fi
done

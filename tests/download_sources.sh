#!/bin/bash
files=$(cat sources.csv | tail +2)
regex="([a-z0-9_]+),https://github.com/([A-Za-z0-9-]+)/([\.A-Za-z0-9-]+)/blob/([a-z0-9-]+)/(.*)"

set -e

for f in $files
do
    if [[ $f =~ $regex ]]
    then
        folder="${BASH_REMATCH[1]}"
        user="${BASH_REMATCH[2]}"
        repo="${BASH_REMATCH[3]}"
        hash="${BASH_REMATCH[4]}"
        path="${BASH_REMATCH[5]}"
        filename=$(basename $path)
        if [ ! -f "$folder/$filename" ]; then
            mkdir -p $folder
            pushd $folder
            raw="https://raw.githubusercontent.com/${user}/${repo}/${hash}/${path}"
            diff="https://github.com/${user}/${repo}/commit/${hash}.diff"
            curl "$raw" > "$filename"
            curl "$diff" | filterdiff --include="${path}" --clean --strip-match=1 | tail +3 > "$filename.diff"
            patch --reverse < "$filename.diff"
            popd
        fi
    else
        echo "$f doesn't match" >&2
    fi
done

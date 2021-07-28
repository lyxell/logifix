#!/bin/bash
set -e
set -o pipefail

regex="([A-Za-z0-9_]+),.*/blob/[a-z0-9-]+/(.*),(.*)$"
test_regex="([A-Za-z0-9_]+),https://github.com/([A-Za-z0-9_-]+)/([\.A-Za-z0-9_-]+)/blob/([a-z0-9-]+)/(.*),"

logifix_cli="$1"
data="$2"

if ! [ -x "$(command -v filterdiff)" ]; then
    echo "run sudo apt install -y patchutils"
    exit 1
fi

# download
if [[ $data =~ $test_regex ]]; then
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

# run
if [[ $data =~ $regex ]]; then
    rule_number="${BASH_REMATCH[1]}"
    file="${BASH_REMATCH[2]}"
    diff_flags="--color=always --unified ${BASH_REMATCH[3]}"
    echo "$diff_flags"
    input="${rule_number}/${file}"
    t1=$(mktemp)
    t2=$(mktemp)
    echo "$rule_number"
    echo "$input"
    cp "$input" "$t1"
    # run the logifix tool on t1
    "$logifix_cli" --in-place --accept="$rule_number" "$t1"
    # run the patch tool and place the result in t2
    patch -p0 --input="$input.diff" --output="$t2"
    # compare the diffs
    diff $diff_flags "$t1" "$t2"
    echo "$input passed"
else
    echo "$f doesn't match!"
    exit 1
fi

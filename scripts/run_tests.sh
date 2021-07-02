#!/bin/bash

BASEDIR=$(dirname "$0")
set -e
set -o pipefail

logifix_cli="$1"

for f in $(find $BASEDIR/../src -iname '*.java'); do
    echo "$f"
    diff <("$1" --patch --accept-all "$f" | tail -n +2) <(tail -n +2 $f.diff)
done

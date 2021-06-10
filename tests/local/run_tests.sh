#!/bin/bash

BASEDIR=$(dirname "$0")
set -e
set -o pipefail

logifix_cli="$1"

for f in $(find $BASEDIR -iname '*.java'); do
    echo "$f"
    diff <("$1" --rules=1116,1125,1153,1155,1217,2095,2121,2159,2204,2225,4087,4973 --no-apply --no-interactive --no-color "$f" | tail -n +2) <(tail -n +2 $f.diff)
done

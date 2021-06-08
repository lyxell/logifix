#!/bin/bash

BASEDIR=$(dirname "$0")
set -e

logifix_cli="$1"

for f in $(find $BASEDIR -iname '*.java'); do
    echo "$f"
    diff <("$1" --rules=1125,1155,4973 --no-apply --no-interactive --no-color "$f" | tail -n +2) <(tail -n +2 $f.diff)
done

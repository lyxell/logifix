#!/bin/bash

BASEDIR=$(dirname "$0")
set -e
set -o pipefail

logifix_cli="$1"

for f in $(find $BASEDIR/../src -iname '*.java'); do
    echo "$f"
    cd $(dirname $f)
    diff <($logifix_cli --patch --accept-all "$(basename $f)") "$(basename $f).diff"
    cd -
done

#!/bin/bash

BASEDIR=$(dirname "$0")

set -e
set -o pipefail

logifix_cli="$1"
test_file="$2"
diff_file="$3"

echo $test_file
cd $(dirname $test_file)
diff <($logifix_cli --patch --accept-all --enable-all "$(basename $test_file)") "$(basename $diff_file)"

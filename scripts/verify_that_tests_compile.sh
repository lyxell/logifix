#!/bin/bash

set -e

for f in $(find ../src/rules/ -iname '*.java'); do
    echo $f
    cd $(dirname "$f")
    javac $(basename $f)
    patch < $(basename $f).diff
    javac $(basename $f)
    patch --reverse < $(basename $f).diff
    cd -
done

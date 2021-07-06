#!/bin/bash

for f in ../src/rules/*; do
    cd $f
    name=$(basename $(pwd))
    from="rewrite(:[a], :[b], :[c], :[d])"
    to="rewrite(\"$name\", :[b], :[c], :[d])"
    comby "$from" "$to"
    comby -i "$from" "$to" 
    cd -
done

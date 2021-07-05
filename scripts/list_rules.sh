#!/bin/bash

function generate {
    for f in ../src/rules/*; do
        json_file="$f/data.json"
        if [ -f "$json_file" ]; then
            echo "* $(jq -j -c '.description' $json_file) ($(jq -j -c '.sonar.id' $json_file), $(jq -j -c '.pmd.id' $json_file))"
        fi
    done
}

generate | sort

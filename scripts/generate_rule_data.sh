#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

function generate {
    echo "#include <vector>"
    echo "#include <tuple>"
    echo "#include <unordered_map>"
    echo "#include <string>"
    echo ""
    echo "std::unordered_map<std::string, std::tuple<std::string, std::string, std::string, bool>> rule_data = {"

    for f in $SCRIPT_DIR/../src/rules/*; do
        json_file="$f/data.json"
        name=$(basename "$f")
        if [ -f "$json_file" ]; then

            printf "  {\"$name\",{"

            jq -c '.sonar.id, .pmd.id, .description, .disabled_by_default // false' $json_file | tr '\n' ','

            echo "}},"
            
        fi
    done

    echo "};"
}

generate > $1

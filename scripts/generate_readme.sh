#!/bin/bash

cat header.md

for f in ../src/rules/*; do
    json_file="$f/data.json"
    if [ -f "$json_file" ]; then

        echo ""
        echo "### $(jq --raw-output .description $json_file)"
        echo ""
        if [ "$(jq --raw-output .pmd.id $json_file)" = "N/A" ]; then
            echo "* PMD ID: N/A"
        else
            echo "* PMD ID: [$(jq --raw-output .pmd.id $json_file)]($(jq --raw-output .pmd.url $json_file))"
        fi
        if [ "$(jq --raw-output .sonar.id $json_file)" = "N/A" ]; then
            echo "* SonarSource ID: N/A"
        else
            echo "* SonarSource ID: [$(jq --raw-output .sonar.id $json_file)]($(jq --raw-output .sonar.url $json_file))"
        fi
        echo ""
        echo "<ul> </ul>"
        echo ""
        
    fi
done

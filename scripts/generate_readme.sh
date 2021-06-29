#!/bin/bash

cat header.md

for f in ../src/rules/*; do
    json_file="$f/data.json"
    examples_dir="$f/examples"
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
        if [ -d "$examples_dir" ]; then
            echo "#### Examples"
            for ex in "$examples_dir"/*; do
                echo "\`\`\`diff"
                cat "$ex"
                echo "\`\`\`"
            done
        fi
        echo ""
        echo "<ul> </ul>"
        echo ""
        
    else
        >&2 echo "$f not found"
    fi
done

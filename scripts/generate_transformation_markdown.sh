#!/bin/bash

echo "# Predefined transformations"

for f in ../src/rules/*; do
    json_file="$f/data.json"
    examples_dir="$f/examples"
    if [ -f "$json_file" ]; then
        if [ "$(jq --raw-output .disabled_by_default $json_file)" != "true" ]; then
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
            if [ "$(jq --raw-output .spotbugs.id $json_file)" = "N/A" ] || [ "$(jq --raw-output .spotbugs.id $json_file)" = "null" ]; then
                echo "* SpotBugs ID: N/A"
            else
                echo "* SpotBugs ID: [$(jq --raw-output .spotbugs.id $json_file)]($(jq --raw-output .spotbugs.url $json_file))"
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
        fi
    else
        >&2 echo "$f not found"
    fi
done


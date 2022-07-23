#!/bin/python3
import re
import subprocess

txt = subprocess.run(["mcpp", "-C", "-P", "../src/datalog/ast.dl"], stdout=subprocess.PIPE).stdout.decode('utf-8')
sections = re.findall("([A-Za-z -]+) \{(.*?)\}", txt, re.MULTILINE | re.DOTALL)
print("# AST API Reference")

for (name, section) in sections:
    print(f"## {name}")
    for (doc, decl) in re.findall("/\*\*\n(.*?)\*/\n\n.decl ([^)]*\))", section, re.DOTALL):
        print(f"### {decl.split('(')[0]}")
        print()
        print("```erlang")
        print(decl)
        print("```")
        print()
        print(doc)

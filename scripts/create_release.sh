#!/bin/bash

set -ex

pattern='^v([0-9]+)\.([0-9]+)\.([0-9]+)'

current_version=$(git describe --tags --abbrev=0)

[[ $current_version =~ $pattern ]]

if [[ "$1" = "major" ]]; then
    next_version="v$((BASH_REMATCH[1]+1)).${BASH_REMATCH[2]}.${BASH_REMATCH[3]}"
elif [[ "$1" = "minor" ]]; then
    next_version="v${BASH_REMATCH[1]}.$((BASH_REMATCH[2]+1)).${BASH_REMATCH[3]}"
elif [[ "$1" = "patch" ]]; then
    next_version="v${BASH_REMATCH[1]}.${BASH_REMATCH[2]}.$((BASH_REMATCH[3]+1))"
fi

if [[ -z $next_version ]]; then
    echo "specify major, minor or patch"
    exit 1
fi

echo "Creating version $next_version"

read -p "Looks ok? Answer y/n" -n 1 -r
[[ $REPLY =~ ^[Yy]$ ]]

git tag $next_version
rm -rf GithubRelease
mkdir GithubRelease
cd GithubRelease
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
# Make sure that the tests pass
make test
# test that binary outputs correct version
[[ "$(./logifix --version)" = "$next_version" ]]
# check linked libraries
ldd ./logifix
read -p "Looks ok? Answer y/n" -n 1 -r
[[ $REPLY =~ ^[Yy]$ ]]
# create tarball
tarball="logifix-$next_version-x86_64-linux-gnu.tar.gz"
tar -cvzf $tarball logifix
gh release create --title $next_version --draft $next_version $tarball

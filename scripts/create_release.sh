#!/bin/bash

set -ex

pattern='^v([0-9]+)\.([0-9]+)\.([0-9]+)'

current_version=$(git describe --tags --abbrev=0)

if [[ $current_version =~ $pattern ]]; then
    next_version="v${BASH_REMATCH[1]}.$((BASH_REMATCH[2]+1)).${BASH_REMATCH[3]}"
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
    read -p "Looks ok? " -n 1 -r
    echo    # (optional) move to a new line
    [[ $REPLY =~ ^[Yy]$ ]]
    # create tarball
    tarball="logifix-$next_version-x86_64-linux-gnu.tar.gz"
    tar -cvzf $tarball logifix
    gh release create --title $next_version --draft $next_version $tarball
fi

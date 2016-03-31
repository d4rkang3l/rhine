#!/bin/bash

if [ $TRAVIS_OS_NAME == linux ] || [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  wget https://github.com/github/git-lfs/releases/download/v1.1.2/git-lfs-linux-amd64-1.1.2.tar.gz &&
  tar xf git-lfs-linux-amd64-1.1.2.tar.gz &&
  export PATH=`pwd`/git-lfs-1.1.2:$PATH &&
  git lfs fetch &&
  git checkout -f # old git errors on un-executable smudge/clean filters
fi

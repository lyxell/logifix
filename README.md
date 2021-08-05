<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix-logo-beam.svg" alt="Logifix">
</h1>

Logifix is a fast static analysis tool for Java that fixes violations
automatically.

## Demo

https://user-images.githubusercontent.com/4975941/126644571-3215f01b-40f6-4278-9752-fe988c5e0367.mp4

<ul> </ul>

## Features

### Intelligent equational reasoning

Logifix is more than a search-replace system. It achieves intelligent equational reasoning
through the use of articulation points in the rewrite graph.

### Speed

Logifix is implemented in [a high-performance Datalog
dialect](https://github.com/souffle-lang/souffle) that is
synthesized into multi-threaded C++ code. It is heavily parallelized even
when working on a single file and usually analyzes
large projects of thousands of files in a few seconds on modern hardware.
If your project is slow to analyze it is considered a bug and you should
[file a bug report](https://github.com/lyxell/logifix/issues/new).

### Mergeability

Logifix is engineered to produce human-like patches that are ready-to-merge by design without
requiring manual modifications.

<ul> </ul>

## Getting started

1. Watch [the demo](https://github.com/lyxell/logifix#demo)

2. Download and unpack [the latest version](https://github.com/lyxell/logifix/releases/latest) (Currently only GNU/Linux)

3. Put the `logifix` binary in `/usr/local/bin` or somewhere else
   in your `$PATH`

4. Run `logifix path/to/your/java/project`

<ul> </ul>

## How does it work?

Logifix uses static analysis and rewriting strategies to
detect and automatically fix bugs and bad patterns in Java source
code. It fixes problems using multiple rewrite steps.

Logifix starts by finding a set of problems in each source code
file. Each problem is then analyzed in parallel to find an
appropriate patch that will fix the problem. There are three
categories of problems: code that contains bugs, code that can be
simplified and code that can be removed.

After patches have been found they are categorized by problem type
and then presented to the user. The user may choose which patches
to apply in each file. If the user chooses multiple patches in the
same file the result is produced by using a merge-algorithm similar
to that of Git.

<ul> </ul>

## Building

### Ubuntu

* `sudo apt install -y bison cmake re2c mcpp`
* `git clone https://github.com/lyxell/logifix`
* `cd logifix`
* `git submodule update --init --recursive`
* `mkdir build`
* `cmake -S . -B build`
* `cmake --build build`

The logifix binary is now found under build.

### macOS

* `brew install bison cmake re2c mcpp`
* `git clone https://github.com/lyxell/logifix`
* `cd logifix`
* `git submodule update --init --recursive`
* `mkdir build`
* `cmake -S . -B build`
* `cmake --build build`

The logifix binary is now found under build.

<ul> </ul>

## Available rules

See [rules](./rules.md).


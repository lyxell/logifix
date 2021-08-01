<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix-logo-beam.svg" alt="Logifix">
</h1>

Logifix is a fast static analyzer for Java that fixes violations
automatically.

## Demo

https://user-images.githubusercontent.com/4975941/126644571-3215f01b-40f6-4278-9752-fe988c5e0367.mp4

<ul> </ul>

## Getting started

1. Watch [the demo](https://github.com/lyxell/logifix#demo)

2. Download and unpack [the latest version](https://github.com/lyxell/logifix/releases/latest) (Currently only GNU/Linux)

3. Put the `logifix` binary in `/usr/local/bin` or somewhere else
   in your `$PATH`

4. Run `logifix path/to/your/java/project`

<ul> </ul>

## How does it work?

Logifix uses static analysis and deep rewriting strategies to
detect and automatically fix bugs and bad patterns in Java source
code. 

Logifix is implemented in [a high-performance Datalog
dialect](https://github.com/souffle-lang/souffle) that is
synthesized into multi-threaded C++ code.

Logifix starts by finding a set of problems in each source code
file. Each problem is then analyzed in parallel to find an
appropriate patch that will fix the problem. There are three
categories of problems: code that contains bugs, code that can be
simplified and code that can be removed.

After patches have been found they are categorized by problem type
and then presented to the user. The user may choose which patches
to apply in each file. If the user chooses multiple patches in the
same file the result is produced using [an n-way merging
algorithm](https://github.com/lyxell/nway).

<ul> </ul>

## Building

To build from source you will need [CMake](https://cmake.org/), [GNU Bison](https://www.gnu.org/software/bison/) and [re2c](https://re2c.org/).

To download and build the project, perform the following steps:

* `git clone https://github.com/lyxell/logifix`
* `cd logifix && git submodule update --init`
* `mkdir build && cd build && cmake .. && cmake --build .`

<ul> </ul>

## Available transformations

See [transformations.md](./transformations.md).


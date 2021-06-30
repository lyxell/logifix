<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logo.svg" alt="Logifix">
</h1>

Logifix statically detects and automatically fixes bugs and code
smells in Java source code. Logifix is implemented in a
high-performance Datalog dialect that is synthesized into fast
multi-threaded C++ code.

https://user-images.githubusercontent.com/4975941/120327017-9c470280-c2e9-11eb-88d1-163943d550ce.mp4

<ul> </ul>

## Downloading

There are pre-built binaries available for GNU/Linux without any
runtime dependencies, see
[Releases](https://github.com/lyxell/logifix/releases).

<ul> </ul>

## Building

To build from source you will need [CMake](https://cmake.org/), [GNU Bison](https://www.gnu.org/software/bison/) and [re2c](https://re2c.org/).

To download and build the project, perform the following steps:

* `git clone https://github.com/lyxell/logifix`
* `cd logifix && git submodule update --init`
* `mkdir build && cd build && cmake .. && cmake --build .`

<ul> </ul>

## Available fixes

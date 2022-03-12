# Building

### Ubuntu

* `sudo apt install -y bison cmake re2c mcpp jq ccache`
* `git clone https://github.com/lyxell/logifix`
* `cd logifix`
* `git submodule update --init --recursive`
* `mkdir build`
* `cmake -S . -B build`
* `cmake --build build`

The logifix binary is now found under build.

### macOS

* `brew install bison cmake re2c mcpp jq ccache`
* `git clone https://github.com/lyxell/logifix`
* `cd logifix`
* `git submodule update --init --recursive`
* `mkdir build`
* `cmake -S . -B build`
* `cmake --build build`

The logifix binary is now found under build.

<ul> </ul>

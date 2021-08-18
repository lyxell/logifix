<p align="center">
  <img width="260px" src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix-logo-beam.svg" alt="Logifix">
</p>

# Logifix

Logifix is a fast analysis-guided rewrite system for Java source code. It automatically rewrites bad Java code into good Java code and can be used
to fix static analysis violations for static analyzers such as SonarQube, PMD or SpotBugs.

<ul> </ul>

## Demo

https://user-images.githubusercontent.com/4975941/126644571-3215f01b-40f6-4278-9752-fe988c5e0367.mp4

<ul> </ul>

## Getting started

#### 1. Install

Prebuilt and dependency-free binaries are provided for macOS and GNU-based Linux systems.

##### Ubuntu/Debian

```bash
curl -L https://github.com/lyxell/logifix/releases/latest/download/logifix-x86_64-linux-gnu.gz | gunzip -c - > /tmp/logifix
chmod +x /tmp/logifix
sudo mv /tmp/logifix /usr/local/bin
```

##### macOS

```bash
curl -L https://github.com/lyxell/logifix/releases/latest/download/logifix-x86_64-macos.gz | gunzip -c - > /tmp/logifix
chmod +x /tmp/logifix
sudo mv /tmp/logifix /usr/local/bin
```

#### 2. Run

* Run `logifix path/to/your/project` in your terminal, run
  `logifix --help` to get help

<ul> </ul>

## What is Logifix?

Logifix is an analysis-guided rewrite system for Java source
code. This means that you define (or use predefined) analyses and
transformations that all work together to improve your code.
Transformations are chained and combined automatically by the
rewrite engine. The analyses and transformations are written in
the highly declarative logic-based language Datalog.

<ul> </ul>

## Features

### Intelligent equational reasoning

Logifix is more than a search-and-replace system. It performs
rewrites in multiple steps and can achieve intelligent equational
reasoning by building articulation points in the rewrite graph.

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

## FAQ

### Where can I find the predefined transformations?

See [docs/predefined-transformations.md](docs/predefined-transformations.md) or the source code [src/rules](src/rules).

### Can I create my own transformations?

Yes! See [docs/creating-your-own-transformations.md](docs/creating-your-own-transformations.md).

### How do I build from source?

See [docs/building.md](docs/building.md).

<ul> </ul>

## Related projects

If you find this project interesting, be sure to check out these
as well:

* [souffle-lang/souffle](https://github.com/souffle-lang/souffle) - The Datalog compiler used by Logifix
* [egraphs-good/egg](https://github.com/egraphs-good/egg) - A nice library for efficient equality saturation of rewrite systems
* [comby-tools/comby](https://github.com/comby-tools/comby) - A lightweight AST based search-and-replace tool

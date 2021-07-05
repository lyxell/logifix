<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix_logo.svg" alt="Logifix">
</h1>

Logifix uses static analysis to detect and automatically fix bugs
and suspicious patterns in Java source code. Logifix is
implemented in a high-performance Datalog dialect that is
synthesized into fast multi-threaded C++ code.

## Demo

https://user-images.githubusercontent.com/4975941/124370679-9478d600-dc7a-11eb-83ae-68d7d818c547.mp4

<ul> </ul>

## Getting started

1. Watch [the demo](https://github.com/lyxell/logifix#demo)

2. Download and unpack [the latest version](https://github.com/lyxell/logifix/releases/latest) (Currently only GNU/Linux)

3. Put the `logifix` binary in `/usr/local/bin` or somewhere else
   in your `$PATH`

4. Run `logifix path/to/your/java/project`

<ul> </ul>

## How does it work?

Logifix performs deep search and rewrite of code. Given the code below:

```java
  private static Pattern getPattern(String groupRegexp) {
    Pattern groupPattern = PATTERN_CACHE.get(groupRegexp);
    if (groupPattern == null) {
      groupPattern = Pattern.compile(groupRegexp);
      PATTERN_CACHE.put(groupRegexp, groupPattern);
    }
    return groupPattern;
  }
```

Logifix finds and performs a rewrite in four steps, where each version of the code gets incrementally better:

```diff
   private static Pattern getPattern(String groupRegexp) {
-    Pattern groupPattern = PATTERN_CACHE.get(groupRegexp);
-    if (groupPattern == null) {
-      groupPattern = Pattern.compile(groupRegexp);
-      PATTERN_CACHE.put(groupRegexp, groupPattern);
-    }
-    return groupPattern;
+    return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> {
+      Pattern groupPattern = Pattern.compile(k);
+      return groupPattern;
+    });
   }
 }
```

```diff
   private static Pattern getPattern(String groupRegexp) {
     return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> {
-      Pattern groupPattern = Pattern.compile(k);
-      return groupPattern;
+      return Pattern.compile(k);
     });
   }
 }
```

```diff
   private static Pattern getPattern(String groupRegexp) {
-    return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> {
-      return Pattern.compile(k);
-    });
+    return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> Pattern.compile(k));
   }
```

```diff
   private static Pattern getPattern(String groupRegexp) {
-    return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> Pattern.compile(k));
+    return PATTERN_CACHE.computeIfAbsent(groupRegexp, Pattern::compile);
   }
 }
```

The diff that is presented by Logifix is the following:

```diff
   private static Pattern getPattern(String groupRegexp) {
-    Pattern groupPattern = PATTERN_CACHE.get(groupRegexp);
-    if (groupPattern == null) {
-      groupPattern = Pattern.compile(groupRegexp);
-      PATTERN_CACHE.put(groupRegexp, groupPattern);
-    }
-    return groupPattern;
+    return PATTERN_CACHE.computeIfAbsent(groupRegexp, Pattern::compile);
   }
 }
```

Thanks to the [fast Datalog engine Soufflé](https://github.com/souffle-lang/souffle) and [fast diff/merge algorithms](https://github.com/lyxell/nway) this rewrite is found in a fraction of a second.

This rewrite was incorporated in the pull request https://github.com/cbeust/testng/pull/2610

<ul> </ul>

## Building

To build from source you will need [CMake](https://cmake.org/), [GNU Bison](https://www.gnu.org/software/bison/) and [re2c](https://re2c.org/).

To download and build the project, perform the following steps:

* `git clone https://github.com/lyxell/logifix`
* `cd logifix && git submodule update --init`
* `mkdir build && cd build && cmake .. && cmake --build .`

<ul> </ul>

## Available fixes

<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix-logo-beam.svg" alt="Logifix">
</h1>

Logifix is a fast static analysis tool for Java that fixes
violations automatically. Logifix is implemented in [a
high-performance Datalog
dialect](https://github.com/souffle-lang/souffle) that is
synthesized into multi-threaded C++ code.

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

Logifix starts by finding a set of problems in each source code
file. Each problem is then analyzed in parallel to find an
appropriate patch that will fix the problem. There are three
categories of problems: code that contains bugs, code that can be
simplified and code that can be removed.

After fixes have been found they are categorized by problem type
and then presented to the user. The user may choose which fixes
to apply in each file. If the user chooses multiple fixes in the
same file the result is produced using [an n-way merging
algorithm](https://github.com/lyxell/nway).

### Example

Given the code below:

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

Logifix finds a patch in four steps:

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
-    Pattern groupPattern = PATTERN_CACHE.get(groupRegexp);
-    if (groupPattern == null) {
-      groupPattern = Pattern.compile(groupRegexp);
-      PATTERN_CACHE.put(groupRegexp, groupPattern);
-    }
-    return groupPattern;
+    return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> {
+      return Pattern.compile(k);
+    });
   }
 }
```

```diff
   private static Pattern getPattern(String groupRegexp) {
-    Pattern groupPattern = PATTERN_CACHE.get(groupRegexp);
-    if (groupPattern == null) {
-      groupPattern = Pattern.compile(groupRegexp);
-      PATTERN_CACHE.put(groupRegexp, groupPattern);
-    }
-    return groupPattern;
+    return PATTERN_CACHE.computeIfAbsent(groupRegexp, k -> Pattern.compile(k));
   }
```

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

In parallel, it finds the following patch in the same file:

```diff
   private static Boolean isMatch(Pattern pattern, String group) {
     Pair<String, String> cacheKey = Pair.create(pattern.pattern(), group);
-    Boolean match = MATCH_CACHE.get(cacheKey);
-    if (match == null) {
-      match = pattern.matcher(group).matches();
-      MATCH_CACHE.put(cacheKey, match);
-    }
-    return match;
+    return MATCH_CACHE.computeIfAbsent(cacheKey, k -> pattern.matcher(group).matches());
   }
```

These patches were incorporated in pull request
https://github.com/cbeust/testng/pull/2610

Thanks to the [fast Datalog engine
Soufflé](https://github.com/souffle-lang/souffle) and [fast
diff/merge algorithms](https://github.com/lyxell/nway) these
patches are found in a fraction of a second.

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


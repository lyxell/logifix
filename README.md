<img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix-logo-beam.svg" alt="Logifix" width="250" align="right">

# Logifix

Logifix is a Datalog-based tool for automatically fixing static
analysis violations in Java source code. Logifix can be used to
fix static analysis violations for static analyzers such as
SonarQube, PMD or SpotBugs, but also to modernize legacy code or
even to automatically enforce custom rules specific to your code
base.

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
The analyses and transformations are written in
the highly declarative logic-based language Datalog and
are combined and chained automatically by the rewrite engine.

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

## Examples

Here is a few examples of patches generated by Logifix.

### Fix inefficient iteration over Map#keySet

```diff
  public static void sign(String token, Map<String, String> params) {
      List<String> list = new ArrayList();
      String tokenClientSlat = "";
-     for (String key : params.keySet()) {
+     for (Map.Entry<String, String> entry : params.entrySet()) {
+         String key = entry.getKey();
          if (key.equals("token_client_salt")) {
-             tokenClientSlat = params.get(key);
+             tokenClientSlat = entry.getValue();
          }
-         String paramString = key + "=" + params.get(key);
+         String paramString = key + "=" + entry.getValue();
          list.add(paramString);
      }
      Collections.sort(list);
```

### Simplify code using try-with-resources

```diff
          continue;
      } 
-     try {
-         JarFile nextJarFile = new JarFile(absNextFile);
-         try {
-             Attributes attrs = getMainAttrs(nextJarFile);
-             Set<Extension> newExtensions = getReferencedExtensions(attrs);
-             result.addAll(newExtensions);
-             filesToProcess.addAll(extensionsToFiles(newExtensions));
-         } finally {
-             nextJarFile.close();
-         }
+     try (JarFile nextJarFile = new JarFile(absNextFile)) {
+         Attributes attrs = getMainAttrs(nextJarFile);
+         Set<Extension> newExtensions = getReferencedExtensions(attrs);
+         result.addAll(newExtensions);
+         filesToProcess.addAll(extensionsToFiles(newExtensions));
      } catch (Exception e) {
          invalidLibPaths.append(nextFile.getPath()).append(" ");
      }
```

### Simplify code using Map::computeIfAbsent

```diff
  private final Map<Class<?>, String> requestQueueNameCache = new ConcurrentHashMap<>();
     
  public String getRequestQueueName(Class<?> remoteInterface) {
-     String str = requestQueueNameCache.get(remoteInterface);
-     if (str == null) {
-         str = "{" + name + ":" + remoteInterface.getName() + "}";
-         requestQueueNameCache.put(remoteInterface, str);
-     }
-     return str;
+     return requestQueueNameCache.computeIfAbsent(remoteInterface, k -> "{" + name + ":" + k.getName() + "}");
  }
```

### Simplify code using streams

```diff
  @Override
  public List<SpoonFile> getAllJavaFiles() {
-     List<SpoonFile> result = new ArrayList<>();
-
-     for (SpoonFile f : getAllFiles()) {
-         if (f.isJava()) {
-             result.add(f);
-         }
-     }
-
-     return result;
+     return getAllFiles().stream().filter(SpoonFile::isJava).collect(Collectors.toList());
  } 
```

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

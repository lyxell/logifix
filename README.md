<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logifix_logo.svg" alt="Logifix">
</h1>

Logifix uses static analysis to detect and automatically fix bugs
and suspicious patterns in Java source code. Logifix is
implemented in a high-performance Datalog dialect that is
synthesized into fast multi-threaded C++ code.

## Demo

https://user-images.githubusercontent.com/4975941/124673993-2f132800-deba-11eb-9b09-22a8c6f0149d.mp4

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

### Fix broken null checks

* PMD ID: [BrokenNullCheck](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#brokennullcheck)
* SonarSource ID: [S2259](https://rules.sonarsource.com/java/RSPEC-2259)


<ul> </ul>


### Fix calls to Thread.run

* PMD ID: [DontCallThreadRun](https://pmd.github.io/latest/pmd_rules_java_multithreading.html#dontcallthreadrun)
* SonarSource ID: [S1217](https://rules.sonarsource.com/java/RSPEC-1217)

#### Examples
```diff
                 Thread.currentThread().interrupt();
             }

-            thread.run();
+            thread.start();
         } catch (Throwable ex) {
             dispatchUncaughtException(thread, ex);
         } finally {
```

<ul> </ul>


### Fix comparisons of atomic classes

* PMD ID: N/A
* SonarSource ID: [S2204](https://rules.sonarsource.com/java/RSPEC-2204)


<ul> </ul>


### Fix comparisons of objects with null

* PMD ID: [EqualsNull](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#equalsnull)
* SonarSource ID: [S2159](https://rules.sonarsource.com/java/RSPEC-2159)


<ul> </ul>


### Fix null pointer exceptions by inverting string comparison

* PMD ID: [LiteralsFirstInComparison](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#literalsfirstincomparisons)
* SonarSource ID: [S1132](https://rules.sonarsource.com/java/RSPEC-1132)

#### Examples
```diff
 	 * @return boolean true is input wildcard, false otherwise
 	 */
 	private boolean isWildCard(String name) {
-		return name.equals("?");
+		return "?".equals(name);
 	}
 }
```

<ul> </ul>


### Fix null returns in toString methods

* PMD ID: N/A
* SonarSource ID: [S2225](https://rules.sonarsource.com/java/RSPEC-2225)

#### Examples
```diff
         case PROVIDER_KEY:
           return "providerKey[" + key + "]";
       }
-      return null;
+      return "";
     }
   }
```

<ul> </ul>


### Fix references to Collections.EMPTY_LIST, EMPTY_MAP and EMPTY_SET

* PMD ID: N/A
* SonarSource ID: [S1596](https://rules.sonarsource.com/java/RSPEC-1596)

#### Examples
```diff
 	@Override
 	public Set<String> getMetadataKeys() {
 		if (metadata == null) {
-			return Collections.EMPTY_SET;
+			return Collections.emptySet();
 		}
 		return metadata.keySet();
 	}
```

<ul> </ul>


### Fix resource leaks

* PMD ID: [CloseResource](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#closeresource)
* SonarSource ID: [S2095](https://rules.sonarsource.com/java/RSPEC-4087)

#### Examples
```diff
     public static Object unwrapResp(URL url, TripleWrapper.TripleResponseWrapper wrap,
                                     MultipleSerialization serialization) {
         String serializeType = convertHessianFromWrapper(wrap.getSerializeType());
-        try {
-            final ByteArrayInputStream bais = new ByteArrayInputStream(wrap.getData().toByteArray());
-            final Object ret = serialization.deserialize(url, serializeType, wrap.getType(), bais);
-            bais.close();
-            return ret;
+        try (final ByteArrayInputStream bais = new ByteArrayInputStream(wrap.getData().toByteArray())) {
+            return serialization.deserialize(url, serializeType, wrap.getType(), bais);
         } catch (Exception e) {
             throw new RuntimeException("Failed to unwrap resp", e);
         }
```

<ul> </ul>


### Improve precision of calls to BigDecimal

* PMD ID: [AvoidDecimalLiteralsInBigDecimalConstructor](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#avoiddecimalliteralsinbigdecimalconstructor)
* SonarSource ID: [S2111](https://rules.sonarsource.com/java/RSPEC-2111)

#### Examples
```diff
     private int taskNotificationThreads = 5;
     private int taskYieldThreads = 3;

-    private BigDecimal levelTimeMultiplier = new BigDecimal(2.0);
+    private BigDecimal levelTimeMultiplier = BigDecimal.valueOf(2.0);

     private boolean legacyLifespanCompletionCondition;
     private TaskPriorityTracking taskPriorityTracking = TaskPriorityTracking.TASK_FAIR;
```

<ul> </ul>


### Provide parameterized type for generic

* PMD ID: N/A
* SonarSource ID: [S3740](https://rules.sonarsource.com/java/RSPEC-3740)


<ul> </ul>


### Remove empty declarations

* PMD ID: N/A
* SonarSource ID: [S1116](https://rules.sonarsource.com/java/RSPEC-1116)


<ul> </ul>


### Remove empty finally blocks

* PMD ID: [EmptyFinallyBlock](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptyfinallyblock)
* SonarSource ID: N/A

#### Examples
```diff
             } finally {
                 bootstrap.stop();
             }
-        } finally {
         }
     }
```

<ul> </ul>


### Remove empty if statements

* PMD ID: [EmptyIfStmt](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptyifstmt)
* SonarSource ID: N/A


<ul> </ul>


### Remove empty nested blocks

* PMD ID: [EmptyStatementBlock](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptystatementblock)
* SonarSource ID: [S108](https://rules.sonarsource.com/java/RSPEC-108)

#### Examples
```diff
 public class SnippetCommentResource {

     public void modifiedMethod() {
-        {
-        }
         return;
     }
```

<ul> </ul>


### Remove empty statements

* PMD ID: [EmptyStatementNotInLoop](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptystatementnotinloop)
* SonarSource ID: [S1116](https://rules.sonarsource.com/java/RSPEC-1116)

#### Examples
```diff
 				Collections.singletonList(compilationUnit.getOriginalSourceFragment()),
 				new ChangeResolver(getChangeCollector(), compilationUnit)),
 		() -> {
-			super.calculate(sourceCompilationUnit, types);;
+			super.calculate(sourceCompilationUnit, types);
 		});
 	}

```

<ul> </ul>


### Remove empty try blocks

* PMD ID: [EmptyTryBlock](https://pmd.github.io/pmd-6.36.0/pmd_rules_java_errorprone.html#emptytryblock)
* SonarSource ID: N/A

#### Examples
```diff
         master.getDispatcherList().addFirst(new CommitLogDispatcher() {
             @Override
             public void dispatch(DispatchRequest request) {
-                try {
-                } catch (Throwable e) {
-                    e.printStackTrace();
-                }
             }
         });
         master.getDispatcherList().addFirst(new CommitLogDispatcherCalcBitMap(brokerConfig, filterManager));
```

<ul> </ul>


### Remove redundant calls to close

* PMD ID: N/A
* SonarSource ID: [S4087](https://rules.sonarsource.com/java/RSPEC-4087)

#### Examples
```diff
                     return;
             }

-            dataLdr.close();
         }
     }
```

<ul> </ul>


### Remove repeated unary operators

* PMD ID: [AvoidMultipleUnaryOperators](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#avoidmultipleunaryoperators)
* SonarSource ID: [S2761](https://rules.sonarsource.com/java/RSPEC-2761)


<ul> </ul>


### Remove unnecessary calls to String.valueOf

* PMD ID: [UselessStringValueOf](https://pmd.github.io/latest/pmd_rules_java_performance.html#uselessstringvalueof)
* SonarSource ID: [S1153](https://rules.sonarsource.com/java/RSPEC-1153)

#### Examples
```diff
         if (size == 0) {
             return String.valueOf(r.nextFloat());
         }
-        String format = "%" + String.valueOf(size) + "f";
+        String format = "%" + size + "f";

         return String.format(format, _getFloatValue(size));
     }
```

<ul> </ul>


### Remove unnecessary return statements

* PMD ID: [UnnecessaryReturn](https://pmd.github.io/latest/pmd_rules_java_codestyle.html#unnecessaryreturn)
* SonarSource ID: [S3626](https://rules.sonarsource.com/java/RSPEC-3626)

#### Examples
```diff
             into.addError(error);
         }
-        return;
     }

     public static void main(String[] argv) throws IOException {
```

<ul> </ul>


### Remove unnecessary variable declarations before return statements

* PMD ID: [UnnecessaryLocalBeforeReturn](https://pmd.github.io/latest/pmd_rules_java_codestyle.html#unnecessarylocalbeforereturn)
* SonarSource ID: [S1488](https://rules.sonarsource.com/java/RSPEC-1488)

#### Examples
```diff
     @Override
     public TypeDefinition build(ProcessingEnvironment processingEnv, DeclaredType type, Map<String, TypeDefinition> typeCache) {
-        TypeDefinition td = new TypeDefinition(type.toString());
-        return td;
+        return new TypeDefinition(type.toString());
     }

     @Override
```

<ul> </ul>


### Remove unused local variables

* PMD ID: [UnusedLocalVariable](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#unusedlocalvariable)
* SonarSource ID: [S1481](https://rules.sonarsource.com/java/RSPEC-1481)

#### Examples
```diff
 			}
 		}
 		if (comment instanceof CtJavaDoc) {
-			List<CtJavaDocTag> tags = null;
 			Collection<CtJavaDocTag> javaDocTags = ((CtJavaDoc) comment).getTags();
 			if (javaDocTags != null && javaDocTags.isEmpty() == false) {
 				printer.write(transfo.apply("")).writeln();
```

<ul> </ul>


### Replace lambda with method reference

* PMD ID: N/A
* SonarSource ID: [S1612](https://rules.sonarsource.com/java/RSPEC-1612)


<ul> </ul>


### Simplify boolean expressions

* PMD ID: [SimplifyBooleanExpressions](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifybooleanexpressions)
* SonarSource ID: [S1125](https://rules.sonarsource.com/java/RSPEC-1125)

#### Examples
```diff

    @Override
 	public void setAutoImports(boolean autoImports) {
-		if (autoImports == true) {
+		if (autoImports) {
 			prettyPrintingMode = PRETTY_PRINTING_MODE.AUTOIMPORT;
 		} else {
 			prettyPrintingMode = PRETTY_PRINTING_MODE.FULLYQUALIFIED;
```

<ul> </ul>


### Simplify calls to Collection.removeAll

* PMD ID: N/A
* SonarSource ID: [S2114](https://rules.sonarsource.com/java/RSPEC-2114)


<ul> </ul>


### Simplify calls to constructor for string conversion

* PMD ID: N/A
* SonarSource ID: [S2131](https://rules.sonarsource.com/java/RSPEC-2131)


<ul> </ul>


### Simplify calls to String.substring

* PMD ID: N/A
* SonarSource ID: [S2121](https://rules.sonarsource.com/java/RSPEC-2121)

#### Examples
```diff
                 for (String headerValue : headerFieldValue) {
                     String[] fields = headerValue.split(";");
                     sessionCookieValue = fields[0];
-                    sesId = sessionCookieValue.substring(sessionCookieValue.indexOf("=") + 1,
-                            sessionCookieValue.length());
+                    sesId = sessionCookieValue.substring(sessionCookieValue.indexOf("=") + 1);
                 }
             }
         }
```

<ul> </ul>


### Simplify calls to String.substring and String.startsWith

* PMD ID: N/A
* SonarSource ID: [S4635](https://rules.sonarsource.com/java/RSPEC-4635)

#### Examples
```diff
                     String path = entry.getPath();
                     int index = path.lastIndexOf('/') + 1;

-                    if (path.substring(index).startsWith(name + '.')) {
+                    if (path.startsWith(name + '.', index)) {
                         // Select the correct root type
                         mainType = currentType;
                     }
```

<ul> </ul>


### Simplify code using Collection.isEmpty

* PMD ID: [UseCollectionIsEmpty](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#usecollectionisempty)
* SonarSource ID: [S1155](https://rules.sonarsource.com/java/RSPEC-1155)

#### Examples
```diff
 		return false;
 	}
 	private boolean checkIdentifierChars(String simplename) {
-		if (simplename.length() == 0) {
+		if (simplename.isEmpty()) {
 			return false;
 		}
 		return (!Character.isJavaIdentifierStart(simplename.charAt(0)))
```
```diff
 		//move end after the last char
 		end++;
-		while (start < end && explicitModifiersByName.size() > 0) {
+		while (start < end && !explicitModifiersByName.isEmpty()) {
 			int o1 = findNextNonWhitespace(contents, end - 1, start);
 			if (o1 == -1) {
 				break;
```

<ul> </ul>


### Simplify code using Map.computeIfAbsent

* PMD ID: N/A
* SonarSource ID: [S3824](https://rules.sonarsource.com/java/RSPEC-3824)

#### Examples
```diff
      * @return the associated list of {@link HttpData} for the request
      */
     private List<HttpData> getList(HttpRequest request) {
-        List<HttpData> list = requestFileDeleteMap.get(request);
-        if (list == null) {
-            list = new ArrayList<HttpData>();
-            requestFileDeleteMap.put(request, list);
-        }
-        return list;
+        return requestFileDeleteMap.computeIfAbsent(request, k -> new ArrayList<HttpData>());
     }

     @Override
```
```diff
      * @param initializer is called immediately after the value is added to the map
      */
     static <K, V> V getOrCreate(Map<K, V> map, K key, Supplier<V> valueCreator, Consumer<V> initializer) {
-        V value = map.get(key);
-        if (value == null) {
-            value = valueCreator.get();
-            map.put(key, value);
+        return map.computeIfAbsent(key, k -> {
+            V value = valueCreator.get();
             if (initializer != null) {
                 initializer.accept(value);
             }
-        }
-        return value;
+            return value;
+        });
     }
     static <T> boolean addUniqueObject(Collection<T> col, T o) {
         if (containsObject(col, o)) {
```

<ul> </ul>


### Simplify inverted boolean expressions

* PMD ID: [LogicInversion](https://pmd.github.io/latest/pmd_rules_java_design.html#logicinversion)
* SonarSource ID: [S1940](https://rules.sonarsource.com/java/RSPEC-1940)

#### Examples
```diff
             }
             catch (BlobStorageException e) {
                 // If the blob already exists, ignore
-                if (!(e.getStatusCode() == 409)) {
+                if (e.getStatusCode() != 409) {
                     throw new IgniteSpiException("Failed to upload blob with exception " +
                             e.getMessage());
                 }
```

<ul> </ul>


### Simplify lambdas containing a block with only one statement

* PMD ID: N/A
* SonarSource ID: [S1602](https://rules.sonarsource.com/java/RSPEC-1602)

#### Examples
```diff
 	 * @param conflictMode
 	 */
 	void setNodeOfElement(CtElement element, RootNode node, ConflictResolutionMode conflictMode) {
-		modifyNodeOfElement(element, conflictMode, oldNode -> {
-			return node;
-		});
+		modifyNodeOfElement(element, conflictMode, oldNode -> node);
 	}

 	/**
```

<ul> </ul>


### Simplify return of boolean expressions

* PMD ID: [SimplifyBooleanReturns](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifybooleanreturns)
* SonarSource ID: [S1126](https://rules.sonarsource.com/java/RSPEC-1126)

#### Examples
```diff
 	private static boolean containsOnlyWhiteSpace(CtElement element) {
 		char[] snippet = (element.toString() + '\n').toCharArray();
 		int next = PositionBuilder.findNextNonWhitespace(snippet, snippet.length - 1, 0);
-		if (next == -1) {
-			return true;
-		} else {
-			return false;
-		}
+		return next == -1;
 	}

 	private static void replaceComments(CtStatement element) {
```

<ul> </ul>


### Simplify ternary conditional expressions

* PMD ID: [SimplifiedTernary](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifiedternary)
* SonarSource ID: [S1125](https://rules.sonarsource.com/java/RSPEC-1125)

#### Examples
```diff
 				return true;
 			}
 			Boolean value = generator.generateSingleTarget(vrOfExpression, parameters, Boolean.class);
-			return value == null ? false : value.booleanValue();
+			return value != null && value.booleanValue();
 		}

 		@Override
```

<ul> </ul>

## Related projects

* [nway](https://github.com/lyxell/nway)

# Predefined transformations

### Fix broken null checks

* PMD ID: [BrokenNullCheck](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#brokennullcheck)
* SonarSource ID: [S2259](https://rules.sonarsource.com/java/RSPEC-2259)
* SpotBugs ID: N/A


<ul> </ul>


### Fix calls to Thread.run

* PMD ID: [DontCallThreadRun](https://pmd.github.io/latest/pmd_rules_java_multithreading.html#dontcallthreadrun)
* SonarSource ID: [S1217](https://rules.sonarsource.com/java/RSPEC-1217)
* SpotBugs ID: N/A

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


### Fix imprecise calls to BigDecimal

* PMD ID: [AvoidDecimalLiteralsInBigDecimalConstructor](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#avoiddecimalliteralsinbigdecimalconstructor)
* SonarSource ID: [S2111](https://rules.sonarsource.com/java/RSPEC-2111)
* SpotBugs ID: [DMI_BIGDECIMAL_CONSTRUCTED_FROM_DOUBLE](https://spotbugs.readthedocs.io/en/latest/bugDescriptions.html#dmi-bigdecimal-constructed-from-double-that-isn-t-represented-precisely-dmi-bigdecimal-constructed-from-double)

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


### Fix inefficient calls to forEach(list::add)

* PMD ID: N/A
* SonarSource ID: [S2203](https://rules.sonarsource.com/java/RSPEC-2203)
* SpotBugs ID: N/A


<ul> </ul>


### Fix inefficient map access

* PMD ID: N/A
* SonarSource ID: [S2864](https://rules.sonarsource.com/java/RSPEC-2864)
* SpotBugs ID: [WMI_WRONG_MAP_ITERATOR](https://spotbugs.readthedocs.io/en/latest/bugDescriptions.html#wmi-inefficient-use-of-keyset-iterator-instead-of-entryset-iterator-wmi-wrong-map-iterator)


<ul> </ul>


### Fix null returns in toString methods

* PMD ID: N/A
* SonarSource ID: [S2225](https://rules.sonarsource.com/java/RSPEC-2225)
* SpotBugs ID: N/A

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


### Fix potential resource leaks

* PMD ID: [CloseResource](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#closeresource)
* SonarSource ID: [S2095](https://rules.sonarsource.com/java/RSPEC-4087)
* SpotBugs ID: N/A

#### Examples
```diff
         checkSize(newsize);
         RandomAccessFile accessFile = new RandomAccessFile(file, "r");
         ByteBuffer byteBuffer;
-        try {
-            FileChannel fileChannel = accessFile.getChannel();
-            try {
-                byte[] array = new byte[(int) newsize];
-                byteBuffer = ByteBuffer.wrap(array);
-                int read = 0;
-                while (read < newsize) {
-                    read += fileChannel.read(byteBuffer);
-                }
-            } finally {
-                fileChannel.close();
+        try (FileChannel fileChannel = accessFile.getChannel()) {
+            byte[] array = new byte[(int) newsize];
+            byteBuffer = ByteBuffer.wrap(array);
+            int read = 0;
+            while (read < newsize) {
+                read += fileChannel.read(byteBuffer);
             }
         } finally {
             accessFile.close();
```

<ul> </ul>


### Fix raw use of empty collections

* PMD ID: N/A
* SonarSource ID: [S1596](https://rules.sonarsource.com/java/RSPEC-1596)
* SpotBugs ID: N/A

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


### Fix raw use of generic class

* PMD ID: N/A
* SonarSource ID: [S3740](https://rules.sonarsource.com/java/RSPEC-3740)
* SpotBugs ID: N/A


<ul> </ul>


### Remove empty declarations

* PMD ID: N/A
* SonarSource ID: [S1116](https://rules.sonarsource.com/java/RSPEC-1116)
* SpotBugs ID: N/A


<ul> </ul>


### Remove empty finally blocks

* PMD ID: [EmptyFinallyBlock](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptyfinallyblock)
* SonarSource ID: N/A
* SpotBugs ID: N/A

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
* SpotBugs ID: N/A


<ul> </ul>


### Remove empty nested blocks

* PMD ID: [EmptyStatementBlock](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptystatementblock)
* SonarSource ID: [S108](https://rules.sonarsource.com/java/RSPEC-108)
* SpotBugs ID: N/A

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
* SpotBugs ID: N/A

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
* SpotBugs ID: N/A

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
* SpotBugs ID: N/A

#### Examples
```diff
                     return;
             }

-            dataLdr.close();
         }
     }
```

<ul> </ul>


### Remove redundant calls to Collection::addAll

* PMD ID: N/A
* SonarSource ID: N/A
* SpotBugs ID: N/A


<ul> </ul>


### Remove redundant casts

* PMD ID: N/A
* SonarSource ID: [S1905](null)
* SpotBugs ID: N/A


<ul> </ul>


### Remove redundant collection copies

* PMD ID: N/A
* SonarSource ID: N/A
* SpotBugs ID: N/A


<ul> </ul>


### Remove redundant try blocks

* PMD ID: N/A
* SonarSource ID: N/A
* SpotBugs ID: N/A


<ul> </ul>


### Remove repeated unary operators

* PMD ID: [AvoidMultipleUnaryOperators](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#avoidmultipleunaryoperators)
* SonarSource ID: [S2761](https://rules.sonarsource.com/java/RSPEC-2761)
* SpotBugs ID: N/A


<ul> </ul>


### Remove unnecessary calls to String.valueOf

* PMD ID: [UselessStringValueOf](https://pmd.github.io/latest/pmd_rules_java_performance.html#uselessstringvalueof)
* SonarSource ID: [S1153](https://rules.sonarsource.com/java/RSPEC-1153)
* SpotBugs ID: N/A

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


### Remove unnecessary declarations above return statements

* PMD ID: [UnnecessaryLocalBeforeReturn](https://pmd.github.io/latest/pmd_rules_java_codestyle.html#unnecessarylocalbeforereturn)
* SonarSource ID: [S1488](https://rules.sonarsource.com/java/RSPEC-1488)
* SpotBugs ID: N/A

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


### Remove unnecessary null check before string equals comparison

* PMD ID: N/A
* SonarSource ID: N/A
* SpotBugs ID: N/A


<ul> </ul>


### Remove unnecessary return statements

* PMD ID: [UnnecessaryReturn](https://pmd.github.io/latest/pmd_rules_java_codestyle.html#unnecessaryreturn)
* SonarSource ID: [S3626](https://rules.sonarsource.com/java/RSPEC-3626)
* SpotBugs ID: N/A

#### Examples
```diff
             into.addError(error);
         }
-        return;
     }

     public static void main(String[] argv) throws IOException {
```

<ul> </ul>


### Remove unused assignments

* PMD ID: [UnusedAssignment](https://pmd.github.io/pmd-6.36.0/pmd_rules_java_bestpractices.html#unusedassignment)
* SonarSource ID: [S1854](https://rules.sonarsource.com/java/RSPEC-1854)
* SpotBugs ID: N/A


<ul> </ul>


### Remove unused imports

* PMD ID: [UnnecessaryImport](https://pmd.github.io/latest/pmd_rules_java_codestyle.html#unnecessaryimport)
* SonarSource ID: [S1128](https://rules.sonarsource.com/java/tag/unused/RSPEC-1128)
* SpotBugs ID: N/A


<ul> </ul>


### Remove unused local variables

* PMD ID: [UnusedLocalVariable](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#unusedlocalvariable)
* SonarSource ID: [S1481](https://rules.sonarsource.com/java/RSPEC-1481)
* SpotBugs ID: N/A

#### Examples
```diff
 import spoon.reflect.declaration.CtCompilationUnit;
 import spoon.reflect.declaration.CtElement;
 import spoon.reflect.declaration.CtNamedElement;
-import spoon.reflect.declaration.CtShadowable;
 import spoon.reflect.declaration.CtType;
 import spoon.reflect.declaration.ParentNotInitializedException;
 import spoon.reflect.factory.Factory;
@@ -170,9 +169,6 @@ public boolean equals(Object o) {
 
 	@Override
 	public List<CtAnnotation<? extends Annotation>> getAnnotations() {
-		if (this instanceof CtShadowable) {
-			CtShadowable shadowable = (CtShadowable) this;
-		}
 		return unmodifiableList(annotations);
 	}
 
```

<ul> </ul>


### Simplify boolean expressions

* PMD ID: [SimplifyBooleanExpressions](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifybooleanexpressions)
* SonarSource ID: [S1125](https://rules.sonarsource.com/java/RSPEC-1125)
* SpotBugs ID: N/A

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
* SpotBugs ID: [DMI_USING_REMOVEALL_TO_CLEAR_COLLECTION](https://spotbugs.readthedocs.io/en/latest/bugDescriptions.html#dmi-don-t-use-removeall-to-clear-a-collection-dmi-using-removeall-to-clear-collection)


<ul> </ul>


### Simplify calls to constructor for string conversion

* PMD ID: N/A
* SonarSource ID: [S2131](https://rules.sonarsource.com/java/RSPEC-2131)
* SpotBugs ID: N/A


<ul> </ul>


### Simplify calls to String.substring

* PMD ID: N/A
* SonarSource ID: [S2121](https://rules.sonarsource.com/java/RSPEC-2121)
* SpotBugs ID: [DMI_USELESS_SUBSTRING](https://spotbugs.readthedocs.io/en/latest/bugDescriptions.html#dmi-invocation-of-substring-0-which-returns-the-original-value-dmi-useless-substring)

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
* SpotBugs ID: N/A

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


### Simplify code using Collection::isEmpty

* PMD ID: [UseCollectionIsEmpty](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#usecollectionisempty)
* SonarSource ID: [S1155](https://rules.sonarsource.com/java/RSPEC-1155)
* SpotBugs ID: N/A

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


### Simplify code using lambda expressions

* PMD ID: N/A
* SonarSource ID: [S1604](https://rules.sonarsource.com/java/RSPEC-1604)
* SpotBugs ID: N/A


<ul> </ul>


### Simplify code using Map::computeIfAbsent

* PMD ID: N/A
* SonarSource ID: [S3824](https://rules.sonarsource.com/java/RSPEC-3824)
* SpotBugs ID: N/A

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
+        return requestFileDeleteMap.computeIfAbsent(request, ArrayList::new);
     }

     @Override
```

<ul> </ul>


### Simplify code using method references

* PMD ID: N/A
* SonarSource ID: [S1612](https://rules.sonarsource.com/java/RSPEC-1612)
* SpotBugs ID: N/A

#### Examples
```diff
                int off = getStart();
                ElementSourceFragment child = getFirstChild();
                while (child != null) {
-                       forEachConstantFragment(off, child.getStart(), cf -> children.add(cf));
+                       forEachConstantFragment(off, child.getStart(), children::add);
                        children.add(child);
                        off = child.getEnd();
                        child = child.getNextSibling();
```

<ul> </ul>


### Simplify code using streams

* PMD ID: N/A
* SonarSource ID: N/A
* SpotBugs ID: N/A

#### Examples
```diff
        private List<FlashMap> getExpiredFlashMaps(List<FlashMap> allMaps) {
-               List<FlashMap> result = new ArrayList<>();
-               for (FlashMap map : allMaps) {
-                       if (map.isExpired()) {
-                               result.add(map);
-                       }
-               }
-               return result;
+               return allMaps.stream().filter(FlashMap::isExpired).collect(Collectors.toList());
        }
```

<ul> </ul>


### Simplify inverted boolean expressions

* PMD ID: [LogicInversion](https://pmd.github.io/latest/pmd_rules_java_design.html#logicinversion)
* SonarSource ID: [S1940](https://rules.sonarsource.com/java/RSPEC-1940)
* SpotBugs ID: N/A

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
* SpotBugs ID: N/A

#### Examples
```diff
                        break;
                default:
                        // per line suffix
-                       printCommentContent(printer, comment, s -> { return (" * " + s).replaceAll(" *$", ""); });
+                       printCommentContent(printer, comment, s -> (" * " + s).replaceAll(" *$", ""));
        }
        // suffix
        switch (commentType) {
```

<ul> </ul>


### Simplify return of boolean expressions

* PMD ID: [SimplifyBooleanReturns](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifybooleanreturns)
* SonarSource ID: [S1126](https://rules.sonarsource.com/java/RSPEC-1126)
* SpotBugs ID: N/A

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
* SpotBugs ID: N/A

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


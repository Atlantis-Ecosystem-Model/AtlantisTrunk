# atlantisUtil/atUtilXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-01](#util-01) | 755 | H/M | wrong operator / always-false | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_che… |  | [ ] |
| [UTIL-02](#util-02) | 1270-1275 (with 1205-1216) | H/M | wrong return-value test | `Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller t… |  | [ ] |
| [UTIL-03](#util-03) | 1358-1363 | H/M | NULL dereference | When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to … |  | [ ] |
| [UTIL-04](#util-04) | 1413-1415 (with 2237-2278) | H/M | wrong return-value test (dead check) | `skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never ret… |  | [ ] |
| [UTIL-05](#util-05) | 1412, 1432-1436, 1444 | H/M | error handler silenced / NULL deref | `set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value ch… |  | [ ] |
| [UTIL-06](#util-06) ⭐ | 1654-1656 | H/H | buffer overrun (heap) | `valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies … |  | [ ] |
| [UTIL-07](#util-07) | 1523, 1542-1549 | H/M | buffer truncation | Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%… |  | [ ] |
| [UTIL-08](#util-08) | 1112-1115 | H/L | NULL dereference | `if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode… |  | [ ] |
| [UTIL-09](#util-09) | 868-871 | M/L | wrong free | On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. … |  | [ ] |
| [UTIL-10](#util-10) | 2222 | H/L | inverted condition | `if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function … |  | [ ] |
| [UTIL-11](#util-11) | 1590 (with 1510) | H/L | swapped arguments | `Parse_Into_Int_Array(key, fileName, values, ...)`, but the signature is `(char *fileName, char *paramName, ..… |  | [ ] |
| [UTIL-12](#util-12) | 1600-1601, 1609 | H/L | format specifier mismatch | `%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error mess… |  | [ ] |
| [UTIL-13](#util-13) | 1766-1774 | M/M | NULL dereference | If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Token… |  | [ ] |
| [UTIL-14](#util-14) | 1433-1435 | H/L | use before NULL check | `strcpy(lengthStr, valuestr)` runs before `if (valuestr == NULL)`, so a missing length crashes instead of givi… |  | [ ] |
| [UTIL-15](#util-15) | 1805-1811, 1816 | H/L | unreachable code / duplicated condition | The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_f… |  | [ ] |
| [UTIL-16](#util-16) | 925, 931 | M/L | format mismatch / NULL deref | `quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while … |  | [ ] |

---

## UTIL-01

**wrong operator / always-false** · Conf H / Sev M  
**Lines (current code):** 755  
**Lines at review (Oct 2026):** 702

**Function:** `Check_Input_Array()`

**Why it's thought to be a bug**

`if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_check` never fires. Only the sum test runs, so negative entries that are offset by entries >1 still pass. The `proportion_check` case just above (line 745) correctly uses `||`. Function: `Check_Input_Array()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 753–757) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  753: 				if (verbose > 2)
  754: 					printf("%e (sum %e)\n", (*p)[i], sum);
  755: 			}
  756: 		}
  757: 		diff = fabs(sum - 1.0);
```

**Suggested fix (from review):** Change `&&` to `||`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-02

**wrong return-value test** · Conf H / Sev M  
**Lines (current code):** 1270-1275 (with 1205-1216)  
**Lines at review (Oct 2026):** 1217-1222 (with 1152-1163)

**Function:** `Util_XML_Get_Value_Double()`

**Why it's thought to be a bug**

`Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller tests `if(!Util_XML_Get_Node_Value_Double(...))`, but `!(-1)` is 0, so a missing value is never caught. `value` stays 0, `Check_Value` runs on 0, and the function returns TRUE even when `isRequired` is TRUE. Function: `Util_XML_Get_Value_Double()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1268–1277)

```c
 1268: 		return FALSE;
 1269: 	}
 1270: 	if (verbose > 2)
 1271: 		printf("value = %e\n", value);
 1272: 	Check_Value(valueName, value, entry_type, valueName);
 1273: 	
 1274: 	if (localecotest > 2)
 1275: 		printf("Ecosystem parameter: %s = %f\n", valueName, value);
 1276: 	
 1277: 	*returnValue = value;
```

**Suggested fix (from review):** Use `if (Util_XML_Get_Node_Value_Double(...) < 0)`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-03

**NULL dereference** · Conf H / Sev M  
**Lines (current code):** 1358-1363  
**Lines at review (Oct 2026):** 1305-1310

**Function:** `Util_XML_Get_Value_String()`

**Why it's thought to be a bug**

When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to `strcpy(returnValue, value)`, which crashes. Function: `Util_XML_Get_Value_String()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1356–1365)

```c
 1356: 	/* Copy the found string into the destination */
 1357: 	strcpy(returnValue, value);
 1358: 
 1359: 	free(value);
 1360: 	return TRUE;
 1361: }
 1362: 
 1363: /**
 1364:  * \brief Get the value string of the given node.
 1365:  * NOTE: the returned string must be freed by the caller.
```

**Suggested fix (from review):** If `value == NULL` (not required), return FALSE before the strcpy.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-04

**wrong return-value test (dead check)** · Conf H / Sev M  
**Lines (current code):** 1413-1415 (with 2237-2278)  
**Lines at review (Oct 2026):** 1360-1362 (with 2184-2225)

**Function:** `Util_XML_Parse_Create_Node()`

**Why it's thought to be a bug**

`skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never returns a negative. So `if(skipToKeyEndNoRewind(infile,varstr) < 0)` is never true and duplicate parameters in the .prm file are never detected during conversion. Function: `Util_XML_Parse_Create_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1411–1417) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1411: 				valuestr = strtok(NULL, seps);
 1412: 
 1413: 				/* grab the next token in case its a unit */
 1414: 				unitstr = strtok(NULL, seps);
 1415: 
 1416: 				if(unitstr != NULL){
 1417: 					for(i = 0; i < numUnits; i++){
```

**Suggested fix (from review):** Change `< 0` to `> 0`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-05

**error handler silenced / NULL deref** · Conf H / Sev M  
**Lines (current code):** 1412, 1432-1436, 1444  
**Lines at review (Oct 2026):** 1359, 1379-1383, 1391

**Function:** `Util_XML_Parse_Create_Node()`

**Why it's thought to be a bug**

`set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value checks. `keyprm_errfn(...)` at 1380/1382 is therefore `quiet` and doesn't stop execution, so a missing value (`valuestr == NULL`) reaches `strlen(valuestr)` and then `Util_XML_Create_Node`, which crashes. Function: `Util_XML_Parse_Create_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1410–1446)

```c
 1410: 
 1411: 				valuestr = strtok(NULL, seps);
 1412: 
 1413: 				/* grab the next token in case its a unit */
 1414: 				unitstr = strtok(NULL, seps);
 1415: 
 1416: 				if(unitstr != NULL){
 1417: 					for(i = 0; i < numUnits; i++){
 1418: 						if(strcmp(unitstr, unitTypes[i]) == 0){
 1419: 							snprintf(tempBuf, sizeof(tempBuf), "%s %s", valuestr, unitstr);
 1420: 							strcpy(valuestr, tempBuf);
 1421: 							break;
 1422: 						}
 1423: 					}
 1424: 				}
 1425: 
 1426: 				if (valuestr == NULL)
 1427: 					keyprm_errfn("\nUtil_XML_Create_Node: %s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
 1428: 				if (strlen(valuestr) == 0) {
 1429: 					keyprm_errfn("\nUtil_XML_Create_Node: %s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
 1430: 				}
 1431: 
 1432: 				/* Create the new node */
 1433: 				returnNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, cur, valueName, comment, units, valuestr);
 1434: 
 1435: 				Util_XML_Add_Node_Property(returnNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
 1436: 				Util_XML_Add_Node_Property(returnNode, "AttributeRecommendedValue", recommendedValue);
 1437: 
 1438: 				set_keyprm_errfn(fn);
 1439: 
 1440: 				return returnNode;
 1441: 			}
 1442: 		}
 1443: 	}
 1444: 	keyprm_errfn("Could not find parameter '%s' in input file %s\n\n", valueName, fileName);
 1445: 	return NULL;
 1446: }
```

**Suggested fix (from review):** Call `set_keyprm_errfn(fn)` right after the duplicate check, or use `quit` for the missing-value errors.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-06

**buffer overrun (heap)** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1654-1656  
**Lines at review (Oct 2026):** 1601-1603

**Function:** `Parse_Into_Array()`

**Why it's thought to be a bug**

`valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies the full XML array string with no length check. Long arrays (per-box arrays in large models, long prey/diet vectors) easily go past 2000 chars and overflow the heap. The copy is only used in an error message. Function: `Parse_Into_Array()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1652–1658) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1652: 
 1653: 	varStr = NULL;
 1654: 	// Read in value until size is reached.
 1655: 	for (i = 0; i < size; ++i) {
 1656: 
 1657: 		if (i == 0)
 1658: 			// Get the first token in the line.
```

**Suggested fix (from review):** Use `valueCopy = strdup(valueStr)` (or malloc `strlen(valueStr)+1`).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-07

**buffer truncation** · Conf H / Sev M  
**Lines (current code):** 1523, 1542-1549  
**Lines at review (Oct 2026):** 1470, 1489-1496

**Function:** `Replace_String_Entry()`

**Why it's thought to be a bug**

Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, ...)` and copies it back. Once the string passes 255 chars it is silently truncated, so the rewritten array loses its later entries and has the wrong length. `returnString` itself is `size*100` bytes. Function: `Replace_String_Entry()` (used by `Util_XML_Replace_Node_Value`).

**Current code** (`atlantisUtil/atUtilXML.c` lines 1521–1551)

```c
 1521: 	/* Read in value until size is reached. */
 1522: 	for (i = 0; i < size; ++i) {
 1523: 
 1524: 		if (i == 0)
 1525: 			varStr = strtok(valueStr, seps);
 1526: 		else
 1527: 			varStr = strtok(NULL, seps);
 1528: 
 1529: 		if (varStr == NULL)
 1530: 			varStr = dummyString;
 1531: 
 1532: 		if (i == replaceIndex) {
 1533: 			if (i == 0)
 1534: 				strcpy(tempStr, replaceStr);
 1535: 			else
 1536: 				snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, replaceStr);
 1537: 			strcpy(returnString, tempStr);
 1538: 		} else {
 1539: 			if (i == 0)
 1540: 				strcpy(tempStr, varStr);
 1541: 			else
 1542: 				snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, varStr);
 1543: 			strcpy(returnString, tempStr);
 1544: 		}
 1545: 	}
 1546: 	return returnString;
 1547: }
 1548: 
 1549: /**
 1550:  * 	\brief Parse the values in the valueStr and return a newly allocated integer array.
 1551:  *
```

**Suggested fix (from review):** Append directly into `returnString` (strcat, or track an offset) and drop `tempStr`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-08

**NULL dereference** · Conf H / Sev L  
**Lines (current code):** 1112-1115  
**Lines at review (Oct 2026):** 1059-1062

**Function:** `Util_XML_Get_Node()`

**Why it's thought to be a bug**

`if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode->nodesetval`. `Execute_Xpath_Expression` returns NULL on XPath failure. Callers at 1105 and 1127 (`Util_XML_Set_Node_Value`, `Util_XML_Replace_Node_Value`) also skip the NULL check. Function: `Util_XML_Get_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1110–1117)

```c
 1110: 
 1111: 	xmlXPathFreeObject(attributeNode);
 1112: 
 1113: 	return returnValue;
 1114: }
 1115: 
 1116: xmlNodePtr Util_XML_Get_Or_Create_Node(int type, xmlNodePtr parent, char *attributeName) {
 1117: 
```

**Suggested fix (from review):** `if (attributeNode == NULL) return NULL;`, and check `nodesetval != NULL`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-09

**wrong free** · Conf M / Sev L  
**Lines (current code):** 868-871  
**Lines at review (Oct 2026):** 815-818

**Function:** `Execute_Xpath_Expression()`

**Why it's thought to be a bug**

On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. The caller keeps using and later frees the doc, so this is a use-after-free or double free. Function: `Execute_Xpath_Expression()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 866–873) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  866: 	}
  867: 
  868: 	/* Register namespaces from list (if any) */
  869: 	if ((nsList != NULL) && (Register_Namespaces(xpathCtx, nsList) < 0)) {
  870: 		fprintf(stderr, "Error: failed to register namespaces list \"%s\"\n", nsList);
  871: 		xmlXPathFreeContext(xpathCtx);
  872: 		return (NULL);
  873: 	}
```

**Suggested fix (from review):** Remove `xmlFreeDoc(doc)`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-10

**inverted condition** · Conf H / Sev L  
**Lines (current code):** 2222  
**Lines at review (Oct 2026):** 2169

**Function:** `Util_Get_Node_Name()`

**Why it's thought to be a bug**

`if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function returns the name property of the first node type that doesn't match (usually NULL), not the matching one. It's used for error-message node names in atManageParamIO.c/atassessParamIO.c. Function: `Util_Get_Node_Name()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 2220–2224) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2220: }
 2221: 
 2222: 
 2223: /** Skip forward from the current file position to
 2224:   * the next line beginning with key, positioned at the character
```

**Suggested fix (from review):** Use `strcmp(...) == 0`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-11

**swapped arguments** · Conf H / Sev L  
**Lines (current code):** 1590 (with 1510)  
**Lines at review (Oct 2026):** 1590 (with 1510)

**Function:** `Util_XML_Read_Array_Integer()`

**Why it's thought to be a bug**

`Parse_Into_Int_Array(key, fileName, values, ...)`, but the signature is `(char *fileName, char *paramName, ...)`. Every error message therefore swaps the file name and the parameter name. Function: `Util_XML_Read_Array_Integer()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1588–1592)

```c
 1588: 			lastval = val;
 1589: 		}
 1590: 		switch (entry_type) {
 1591: 		case binary_check:
 1592: 			/* Flag values only (i.e. either 1 or 0 only) */
```

**Suggested fix (from review):** Call `Parse_Into_Int_Array(fileName, key, values, size, entry_type)`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-12

**format specifier mismatch** · Conf H / Sev L  
**Lines (current code):** 1600-1601, 1609  
**Lines at review (Oct 2026):** 1547-1548, 1556

**Function:** `Parse_Into_Int_Array()`

**Why it's thought to be a bug**

`%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error message. Function: `Parse_Into_Int_Array()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1598–1611)

```c
 1598: 		case integer_check:
 1599: 			/* Check that integer values are integer */
 1600: 			base = floor(values[i]);
 1601: 			diff = fabs(values[i] - base);
 1602: 			if (diff > 0)
 1603: 				quit("Error: In input file %s, entries in %s are supposed to be integer values, but %d entry is %e\n", fileName, paramName, i, values[i]);
 1604: 			break;
 1605: 		default:
 1606: 			quit("Parse_Into_Int_Array - Check id for array check must be integer or binary check when reading in integer arrays\n");
 1607: 			break;
 1608: 		}
 1609: 	}
 1610: 
 1611: 	free(valueStr);
```

**Suggested fix (from review):** Use `%d`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-13

**NULL dereference** · Conf M / Sev M  
**Lines (current code):** 1766-1774  
**Lines at review (Oct 2026):** 1713-1721

**Function:** `Util_XML_Read_Array_String()`

**Why it's thought to be a bug**

If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Tokens longer than STRLEN would also overflow. Function: `Util_XML_Read_Array_String()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1764–1776)

```c
 1764: 		else
 1765: 			// Get the first token in the line.
 1766: 			varStr = strtok(NULL, seps);
 1767: 
 1768: 		strcpy((*p)[i], varStr);
 1769: 	}
 1770: 	free(values);
 1771: 	return TRUE;
 1772: }
 1773: 
 1774: 
 1775: int Util_XML_Get_Param_Index(SpeciesParamStruct array[], int size, int paramID) {
 1776: 	int index;
```

**Suggested fix (from review):** Check for NULL and quit with a clear message; use a bounded copy.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-14

**use before NULL check** · Conf H / Sev L  
**Lines (current code):** 1433-1435  
**Lines at review (Oct 2026):** 1433-1435

**Function:** `Util_XML_Create_Node_Next_Line()`

**Why it's thought to be a bug**

`strcpy(lengthStr, valuestr)` runs before `if (valuestr == NULL)`, so a missing length crashes instead of giving the error message. Also, the `fgets` return at 1442 isn't checked. Function: `Util_XML_Create_Node_Next_Line()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1431–1437)

```c
 1431: 
 1432: 				/* Create the new node */
 1433: 				returnNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, cur, valueName, comment, units, valuestr);
 1434: 
 1435: 				Util_XML_Add_Node_Property(returnNode, "AttributeType", AtlantisXMLAttributeTypeStrings[type]);
 1436: 				Util_XML_Add_Node_Property(returnNode, "AttributeRecommendedValue", recommendedValue);
 1437: 
```

**Suggested fix (from review):** Move the strcpy after the NULL check, and check the fgets result.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-15

**unreachable code / duplicated condition** · Conf H / Sev L  
**Lines (current code):** 1805-1811, 1816  
**Lines at review (Oct 2026):** 1752-1758, 1763

**Function:** `Util_XML_Get_Species_Node_Value()`

**Why it's thought to be a bug**

The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_finite(value)) || !(_finite(value))` tests the same thing twice (probably copied from an `isnan`/`_finite` pair). Neither changes behaviour. Function: `Util_XML_Get_Species_Node_Value()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1803–1818)

```c
 1803: 	else{
 1804: 		if(result == -1)
 1805: 			quit("Cannot find value %s for group %s\n", paramStruct[paramID].tag, FunctGroupArray[guild].groupCode);
 1806: 
 1807: 		value = value / paramStruct[paramID].divider;
 1808: 	}
 1809: 
 1810: 	if (!(_finite(value)) || !(_finite(value))) {
 1811: 		quit("Util_XML_Get_Species_Node_Value value is undefined for group %s value = %e", FunctGroupArray[guild].groupCode, value);
 1812: 	}
 1813: 	if (verbose > 2)
 1814: 		printf("Ecosystem parameter: %s = %e\n", FunctGroupArray[guild].groupCode, value);
 1815: 	Check_Value(FunctGroupArray[guild].groupCode, value, paramStruct[paramID].checkType, paramStruct[paramID].tag);
 1816: 
 1817: 	return value;
 1818: }
```

**Suggested fix (from review):** Remove the dead test and the duplicate condition.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## UTIL-16

**format mismatch / NULL deref** · Conf M / Sev L  
**Lines (current code):** 925, 931  
**Lines at review (Oct 2026):** 872, 878

**Function:** `Add_Comment_Node()`

**Why it's thought to be a bug**

`quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while `comment` is NULL-checked. Function: `Add_Comment_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 923–933)

```c
  923: 		}
  924: 	}
  925: 	if (strlen(units) > 0) {
  926: 		Util_XML_Add_Node_Property(rootNode, "AttributeUnits", units);
  927: 	}
  928: }
  929: 
  930: /**
  931:  * \Add a property to the given node.
  932:  *
  933:  * This will either be done using an attribute or creating a new text child node.
```

**Suggested fix (from review):** Use `%zu`; add `units != NULL &&`.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

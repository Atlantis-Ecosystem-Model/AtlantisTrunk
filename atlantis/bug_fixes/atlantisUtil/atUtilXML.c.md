# atlantisUtil/atUtilXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-01](#util-01) | 743 | H/M | wrong operator / always-false | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_che… |  | [ ] |
| [UTIL-02](#util-02) | 1264 | H/M | wrong return-value test | `Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller t… |  | [ ] |
| [UTIL-03](#util-03) | 1357 | H/M | NULL dereference | When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to … |  | [ ] |
| [UTIL-04](#util-04) | 1407 | H/M | wrong return-value test (dead check) | `skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never ret… |  | [ ] |
| [UTIL-05](#util-05) | 1406, 1426 | H/M | error handler silenced / NULL deref | `set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value ch… |  | [ ] |
| [UTIL-06](#util-06) ⭐ | 1648 | H/H | buffer overrun (heap) | `valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies … |  | [ ] |
| [UTIL-07](#util-07) | 1517, 1536 | H/M | buffer truncation | Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%… |  | [ ] |
| [UTIL-08](#util-08) | 1106, 1109 | H/L | NULL dereference | `if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode… |  | [ ] |
| [UTIL-09](#util-09) | 864 | M/L | wrong free | On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. … |  | [ ] |
| [UTIL-10](#util-10) | 2216 | H/L | inverted condition | `if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function … |  | [ ] |
| [UTIL-11](#util-11) | 1637, 1557 | H/L | swapped arguments | `Parse_Into_Int_Array(key, fileName, values, ...)`, but the signature is `(char *fileName, char *paramName, ..… |  | [ ] |
| [UTIL-12](#util-12) | 1600 | H/L | format specifier mismatch | `%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error mess… |  | [ ] |
| [UTIL-13](#util-13) | 1768 | M/M | NULL dereference | If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Token… |  | [ ] |
| [UTIL-14](#util-14) | 1480 | H/L | use before NULL check | `strcpy(lengthStr, valuestr)` runs before `if (valuestr == NULL)`, so a missing length crashes instead of givi… |  | [ ] |
| [UTIL-15](#util-15) | 1804, 1810, 47 | H/L | unreachable code / duplicated condition | The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_f… |  | [ ] |
| [UTIL-16](#util-16) | 919, 925 | M/L | format mismatch / NULL deref | `quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while … |  | [ ] |

---

## UTIL-01

**wrong operator / always-false** · Conf H / Sev M  
**Found at (current code):** `atUtilXML.c` 743
**Lines at review (Oct 2026):** 702

**Function:** `Check_Input_Array()`

**Why it's thought to be a bug**

`if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_check` never fires. Only the sum test runs, so negative entries that are offset by entries >1 still pass. The `proportion_check` case just above (line 745) correctly uses `||`. Function: `Check_Input_Array()`.

**Code at issue** — `atUtilXML.c` line 743 (located by name only; check this is the right place)  
Search for: `case probability_check:`

```c
  741: 		}
  742: 		break;
  743→ 	case probability_check:
  744: 		/* Values are between 0 and 1, and sum is 1.0 */
  745: 		if (verbose > 2)
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
**Found at (current code):** `atUtilXML.c` 1264
**Lines at review (Oct 2026):** 1217-1222 (with 1152-1163)

**Function:** `Util_XML_Get_Value_Double()`

**Why it's thought to be a bug**

`Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller tests `if(!Util_XML_Get_Node_Value_Double(...))`, but `!(-1)` is 0, so a missing value is never caught. `value` stays 0, `Check_Value` runs on 0, and the function returns TRUE even when `isRequired` is TRUE. Function: `Util_XML_Get_Value_Double()`.

**Code at issue** — `atUtilXML.c` line 1264  
Search for: `if(!Util_XML_Get_Node_Value_Double(`

```c
 1262: 
 1263: 	/* Successfully found parameter - convert to double, check and return.*/
 1264→ 	if(!Util_XML_Get_Node_Value_Double(ATLANTIS_ATTRIBUTE, attributeNode, &value)){
 1265: 		if (isRequired == TRUE)
 1266: 			quit("Error: Cannot find atlantis parameter '%s' in file %s.\n", valueName, fileName);
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
**Found at (current code):** `atUtilXML.c` 1357
**Lines at review (Oct 2026):** 1305-1310

**Function:** `Util_XML_Get_Value_String()`

**Why it's thought to be a bug**

When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to `strcpy(returnValue, value)`, which crashes. Function: `Util_XML_Get_Value_String()`.

**Code at issue** — `atUtilXML.c` line 1357  
Search for: `strcpy(returnValue, value)`

```c
 1355: 
 1356: 	/* Copy the found string into the destination */
 1357→ 	strcpy(returnValue, value);
 1358: 
 1359: 	free(value);
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
**Found at (current code):** `atUtilXML.c` 1407
**Lines at review (Oct 2026):** 1360-1362 (with 2184-2225)

**Function:** `Util_XML_Parse_Create_Node()`

**Why it's thought to be a bug**

`skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never returns a negative. So `if(skipToKeyEndNoRewind(infile,varstr) < 0)` is never true and duplicate parameters in the .prm file are never detected during conversion. Function: `Util_XML_Parse_Create_Node()`.

**Code at issue** — `atUtilXML.c` line 1407  
Search for: `if(skipToKeyEndNoRewind(infile,varstr) < 0)`

```c
 1405: 				/* Check there isn't a duplicate value */
 1406: 				set_keyprm_errfn(quiet);
 1407→ 				if(skipToKeyEndNoRewind(infile,varstr) < 0){
 1408: 					quit("Util_XML_Parse_Create_Node: Duplicate key found %s\n", varstr);
 1409: 				}
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
**Found at (current code):** `atUtilXML.c` 1406, `atUtilXML.c` 1426
**Lines at review (Oct 2026):** 1359, 1379-1383, 1391

**Function:** `Util_XML_Parse_Create_Node()`

**Why it's thought to be a bug**

`set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value checks. `keyprm_errfn(...)` at 1380/1382 is therefore `quiet` and doesn't stop execution, so a missing value (`valuestr == NULL`) reaches `strlen(valuestr)` and then `Util_XML_Create_Node`, which crashes. Function: `Util_XML_Parse_Create_Node()`.

**Code at issue** — `atUtilXML.c` line 1406  
Search for: `set_keyprm_errfn(quiet)`

```c
 1404: 
 1405: 				/* Check there isn't a duplicate value */
 1406→ 				set_keyprm_errfn(quiet);
 1407: 				if(skipToKeyEndNoRewind(infile,varstr) < 0){
 1408: 					quit("Util_XML_Parse_Create_Node: Duplicate key found %s\n", varstr);
```

**Code at issue** — `atUtilXML.c` line 1426  
Search for: `valuestr == NULL`

```c
 1424: 				}
 1425: 
 1426→ 				if (valuestr == NULL)
 1427: 					keyprm_errfn("\nUtil_XML_Create_Node: %s parameter not found. Please ensure the format for this parameter is: \n%s value \n", valueName, valueName);
 1428: 				if (strlen(valuestr) == 0) {
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
**Found at (current code):** `atUtilXML.c` 1648
**Lines at review (Oct 2026):** 1601-1603

**Function:** `Parse_Into_Array()`

**Why it's thought to be a bug**

`valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies the full XML array string with no length check. Long arrays (per-box arrays in large models, long prey/diet vectors) easily go past 2000 chars and overflow the heap. The copy is only used in an error message. Function: `Parse_Into_Array()`.

**Code at issue** — `atUtilXML.c` line 1648 (closest match to the quoted code; wording differs slightly)  
Search for: `char *valueCopy = (char *)malloc(sizeof(char) *STRLEN);`

```c
 1646: 	char seps[] = " ,\t";
 1647: 	double val, lastval = 0.0;
 1648→ 	char *valueCopy = (char *)malloc(sizeof(char) *STRLEN);
 1649: 
 1650: 	valueCopy  = strcpy(valueCopy, valueStr);
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
**Found at (current code):** `atUtilXML.c` 1517, `atUtilXML.c` 1536
**Lines at review (Oct 2026):** 1470, 1489-1496

**Function:** `Replace_String_Entry()`

**Why it's thought to be a bug**

Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, ...)` and copies it back. Once the string passes 255 chars it is silently truncated, so the rewritten array loses its later entries and has the wrong length. `returnString` itself is `size*100` bytes. Function: `Replace_String_Entry()` (used by `Util_XML_Replace_Node_Value`).

**Code at issue** — `atUtilXML.c` line 1517  
Search for: `char tempStr[256]`

```c
 1515: 	char seps[] = " ";
 1516: 	int i;
 1517→ 	char tempStr[256];
 1518: 	char dummyString[100] = "0.0";
 1519: 	char *returnString = (char *) malloc(sizeof(char) * (size_t)(size * 100));
```

**Code at issue** — `atUtilXML.c` line 1536  
Search for: `snprintf(tempStr, sizeof(tempStr), "%s %s", returnString,`

```c
 1534: 				strcpy(tempStr, replaceStr);
 1535: 			else
 1536→ 				snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, replaceStr);
 1537: 			strcpy(returnString, tempStr);
 1538: 		} else {
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
**Found at (current code):** `atUtilXML.c` 1106, `atUtilXML.c` 1109
**Lines at review (Oct 2026):** 1059-1062

**Function:** `Util_XML_Get_Node()`

**Why it's thought to be a bug**

`if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode->nodesetval`. `Execute_Xpath_Expression` returns NULL on XPath failure. Callers at 1105 and 1127 (`Util_XML_Set_Node_Value`, `Util_XML_Replace_Node_Value`) also skip the NULL check. Function: `Util_XML_Get_Node()`.

**Code at issue** — `atUtilXML.c` line 1106  
Search for: `if (attributeNode == NULL)`

```c
 1104: 	xmlXPathObjectPtr attributeNode = Util_XML_Get_Node_List(type, parent, attributeName);
 1105: 
 1106→ 	if (attributeNode == NULL)
 1107: 		returnValue = NULL;
 1108: 	if (attributeNode->nodesetval->nodeNr > 0)
```

**Code at issue** — `atUtilXML.c` line 1109  
Search for: `attributeNode->nodesetval`

```c
 1107: 		returnValue = NULL;
 1108: 	if (attributeNode->nodesetval->nodeNr > 0)
 1109→ 		returnValue = attributeNode->nodesetval->nodeTab[0];
 1110: 
 1111: 	xmlXPathFreeObject(attributeNode);
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
**Found at (current code):** `atUtilXML.c` 864
**Lines at review (Oct 2026):** 815-818

**Function:** `Execute_Xpath_Expression()`

**Why it's thought to be a bug**

On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. The caller keeps using and later frees the doc, so this is a use-after-free or double free. Function: `Execute_Xpath_Expression()`.

**Code at issue** — `atUtilXML.c` line 864  
Search for: `xmlFreeDoc(doc)`

```c
  862: 	if (xpathCtx == NULL) {
  863: 		fprintf(stderr, "Error: unable to create new XPath context\n");
  864→ 		xmlFreeDoc(doc);
  865: 		return (NULL);
  866: 	}
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
**Found at (current code):** `atUtilXML.c` 2216
**Lines at review (Oct 2026):** 2169

**Function:** `Util_Get_Node_Name()`

**Why it's thought to be a bug**

`if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function returns the name property of the first node type that doesn't match (usually NULL), not the matching one. It's used for error-message node names in atManageParamIO.c/atassessParamIO.c. Function: `Util_Get_Node_Name()`.

**Code at issue** — `atUtilXML.c` line 2216  
Search for: `if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))`

```c
 2214: 
 2215: 	for(i = 0; i < NUM_ATLANTIS_NODE_TYPES; i++){
 2216→ 		if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))
 2217: 			return Get_Node_Property(node, AtlantisXMLObjectNAMES[i][ATTRIBUTE_NAME]);
 2218: 	}
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
**Found at (current code):** `atUtilXML.c` 1637, `atUtilXML.c` 1557
**Lines at review (Oct 2026):** 1590 (with 1510)

**Function:** `Util_XML_Read_Array_Integer()`

**Why it's thought to be a bug**

`Parse_Into_Int_Array(key, fileName, values, ...)`, but the signature is `(char *fileName, char *paramName, ...)`. Every error message therefore swaps the file name and the parameter name. Function: `Util_XML_Read_Array_Integer()`.

**Code at issue** — `atUtilXML.c` line 1637  
Search for: `Parse_Into_Int_Array(key, fileName, values,`

```c
 1635: 
 1636: 	// Read the data into a temp array.
 1637→ 	*p = Parse_Into_Int_Array(key, fileName, values, size, entry_type);
 1638: 	return TRUE;
 1639: }
```

**Code at issue** — `atUtilXML.c` line 1557  
Search for: `(char *fileName, char *paramName,`

```c
 1555:  *
 1556:  */
 1557→ static int *Parse_Into_Int_Array(char *fileName, char *paramName, char *valueStr, int size, int entry_type) {
 1558: 	int i;
 1559: 	int *values = (int *) malloc((size_t)size * sizeof(int));
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
**Found at (current code):** `atUtilXML.c` 1600
**Lines at review (Oct 2026):** 1547-1548, 1556

**Function:** `Parse_Into_Int_Array()`

**Why it's thought to be a bug**

`%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error message. Function: `Parse_Into_Int_Array()`.

**Code at issue** — `atUtilXML.c` line 1600  
Search for: `values[i]`

```c
 1598: 		case integer_check:
 1599: 			/* Check that integer values are integer */
 1600→ 			base = floor(values[i]);
 1601: 			diff = fabs(values[i] - base);
 1602: 			if (diff > 0)
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
**Found at (current code):** `atUtilXML.c` 1768
**Lines at review (Oct 2026):** 1713-1721

**Function:** `Util_XML_Read_Array_String()`

**Why it's thought to be a bug**

If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Tokens longer than STRLEN would also overflow. Function: `Util_XML_Read_Array_String()`.

**Code at issue** — `atUtilXML.c` line 1768  
Search for: `strcpy((*p)[i], varStr)`

```c
 1766: 			varStr = strtok(NULL, seps);
 1767: 
 1768→ 		strcpy((*p)[i], varStr);
 1769: 	}
 1770: 	free(values);
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
**Found at (current code):** `atUtilXML.c` 1480
**Lines at review (Oct 2026):** 1433-1435

**Function:** `Util_XML_Create_Node_Next_Line()`

**Why it's thought to be a bug**

`strcpy(lengthStr, valuestr)` runs before `if (valuestr == NULL)`, so a missing length crashes instead of giving the error message. Also, the `fgets` return at 1442 isn't checked. Function: `Util_XML_Create_Node_Next_Line()`.

**Code at issue** — `atUtilXML.c` line 1480  
Search for: `strcpy(lengthStr, valuestr)`

```c
 1478: 				valuestr = strtok(NULL, seps);
 1479: 				/* Copy the length string so we can use it later */
 1480→ 				strcpy(lengthStr, valuestr);
 1481: 
 1482: 				if (valuestr == NULL)
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
**Found at (current code):** `atUtilXML.c` 1804, `atUtilXML.c` 1810, `atUtilXML.c` 47
**Lines at review (Oct 2026):** 1752-1758, 1763

**Function:** `Util_XML_Get_Species_Node_Value()`

**Why it's thought to be a bug**

The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_finite(value)) || !(_finite(value))` tests the same thing twice (probably copied from an `isnan`/`_finite` pair). Neither changes behaviour. Function: `Util_XML_Get_Species_Node_Value()`.

**Code at issue** — `atUtilXML.c` line 1804  
Search for: `if(result == -1)`

```c
 1802: 	}
 1803: 	else{
 1804→ 		if(result == -1)
 1805: 			quit("Cannot find value %s for group %s\n", paramStruct[paramID].tag, FunctGroupArray[guild].groupCode);
 1806: 
```

**Code at issue** — `atUtilXML.c` line 1810  
Search for: `!(_finite(value)) || !(_finite(value))`

```c
 1808: 	}
 1809: 
 1810→ 	if (!(_finite(value)) || !(_finite(value))) {
 1811: 		quit("Util_XML_Get_Species_Node_Value value is undefined for group %s value = %e", FunctGroupArray[guild].groupCode, value);
 1812: 	}
```

**Code at issue** — `atUtilXML.c` line 47  
Search for: `Util_XML_Get_Species_Node_Value()`

```c
   45:  *	29-01-2010 Bec Gorton
   46:  *	Moved the definition of the speciesParamStructArray array from the biology library into this file.
   47→  *	Also moved the functions Util_XML_Get_Param_Index() and Util_XML_Get_Species_Node_Value() into here.
   48:  *
   49:  *	11-01-2011 Bec Gorton
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
**Found at (current code):** `atUtilXML.c` 919, `atUtilXML.c` 925
**Lines at review (Oct 2026):** 872, 878

**Function:** `Add_Comment_Node()`

**Why it's thought to be a bug**

`quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while `comment` is NULL-checked. Function: `Add_Comment_Node()`.

**Code at issue** — `atUtilXML.c` line 919  
Search for: `quit("`

```c
  917: 	if (comment != NULL) {
  918: 		if (strlen(comment) > 2000) {
  919→ 			quit("Comment is too long length = %d\n", strlen(comment));
  920: 		}
  921: 		if (strlen(comment) > 0) {
```

**Code at issue** — `atUtilXML.c` line 925  
Search for: `strlen(units)`

```c
  923: 		}
  924: 	}
  925→ 	if (strlen(units) > 0) {
  926: 		Util_XML_Add_Node_Property(rootNode, "AttributeUnits", units);
  927: 	}
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

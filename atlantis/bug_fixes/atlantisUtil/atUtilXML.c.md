# atlantisUtil/atUtilXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-01](#util-01) | 702 | H/M | wrong operator / always-false | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_che… |  | [ ] |
| [UTIL-02](#util-02) | 1217-1222 (with 1152-1163) | H/M | wrong return-value test | `Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller t… |  | [ ] |
| [UTIL-03](#util-03) | 1305-1310 | H/M | NULL dereference | When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to … |  | [ ] |
| [UTIL-04](#util-04) | 1360-1362 (with 2184-2225) | H/M | wrong return-value test (dead check) | `skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never ret… |  | [ ] |
| [UTIL-05](#util-05) | 1359, 1379-1383, 1391 | H/M | error handler silenced / NULL deref | `set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value ch… |  | [ ] |
| [UTIL-06](#util-06) ⭐ | 1601-1603 | H/H | buffer overrun (heap) | `valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies … |  | [ ] |
| [UTIL-07](#util-07) | 1470, 1489-1496 | H/M | buffer truncation | Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%… |  | [ ] |
| [UTIL-08](#util-08) | 1059-1062 | H/L | NULL dereference | `if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode… |  | [ ] |
| [UTIL-09](#util-09) | 815-818 | M/L | wrong free | On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. … |  | [ ] |
| [UTIL-10](#util-10) | 2169 | H/L | inverted condition | `if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function … |  | [ ] |
| [UTIL-11](#util-11) | 1590 (with 1510) | H/L | swapped arguments | `Parse_Into_Int_Array(key, fileName, values, ...)`, but the signature is `(char *fileName, char *paramName, ..… |  | [ ] |
| [UTIL-12](#util-12) | 1547-1548, 1556 | H/L | format specifier mismatch | `%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error mess… |  | [ ] |
| [UTIL-13](#util-13) | 1713-1721 | M/M | NULL dereference | If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Token… |  | [ ] |
| [UTIL-14](#util-14) | 1433-1435 | H/L | use before NULL check | `strcpy(lengthStr, valuestr)` runs before `if (valuestr == NULL)`, so a missing length crashes instead of givi… |  | [ ] |
| [UTIL-15](#util-15) | 1752-1758, 1763 | H/L | unreachable code / duplicated condition | The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_f… |  | [ ] |
| [UTIL-16](#util-16) | 872, 878 | M/L | format mismatch / NULL deref | `quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while … |  | [ ] |

---

## UTIL-01

**wrong operator / always-false** · Conf H / Sev M  
**Listed lines:** 702

**Function:** `Check_Input_Array()`

**Why it's thought to be a bug**

`if ((*p)[i] < 0.0 && (*p)[i] > 1.0)`: a value can't be both <0 and >1, so the range check in `probability_check` never fires. Only the sum test runs, so negative entries that are offset by entries >1 still pass. The `proportion_check` case just above (line 692) correctly uses `||`. Function: `Check_Input_Array()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 718–722) — code appears to have moved +18 lines since the review

```c
  718:         {"StringArray" }};
  719: 
  720: static int Check_Input_Array(double **p, int sized, int entry_type, char *key) {
  721: 	int returnValue = TRUE;
  722: 	int i;
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
**Listed lines:** 1217-1222 (with 1152-1163)

**Function:** `Util_XML_Get_Value_Double()`

**Why it's thought to be a bug**

`Util_XML_Get_Node_Value_Double` returns -1 when the value attribute is missing and 1 on success. The caller tests `if(!Util_XML_Get_Node_Value_Double(...))`, but `!(-1)` is 0, so a missing value is never caught. `value` stays 0, `Check_Value` runs on 0, and the function returns TRUE even when `isRequired` is TRUE. Function: `Util_XML_Get_Value_Double()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1197–1206) — code appears to have moved -18 lines since the review

```c
 1197: }
 1198: 
 1199: int Util_XML_Get_Node_Value_Double(int type, xmlNodePtr node, double *returnValue) {
 1200: 	char *value = Get_Node_Property(node, AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE]);
 1201: 
 1202: 	if(value == NULL){
 1203: 		return -1;
 1204: 	}
 1205: 	*returnValue = atof(value);
 1206: 
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
**Listed lines:** 1305-1310

**Function:** `Util_XML_Get_Value_String()`

**Why it's thought to be a bug**

When the value attribute is missing and `isRequired == FALSE`, `value` is NULL and execution falls through to `strcpy(returnValue, value)`, which crashes. Function: `Util_XML_Get_Value_String()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1303–1312)

```c
 1303: 			quit("Error: Cannot find atlantis parameter '%s' in input file %s.\n", valueName, fileName);
 1304: 		*returnValue = 0;
 1305: 		return FALSE;
 1306: 	}
 1307: 
 1308: 	/* Sucessfully found parameter - convert to double, check and return.*/
 1309: 	value = Util_XML_Get_Node_Value_Int(fileName, ATLANTIS_ATTRIBUTE, attributeNode);
 1310: 
 1311: 	if (verbose > 2)
 1312: 		printf("Util_XML_Get_Value_Integer %s value = %d\n", valueName, value);
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
**Listed lines:** 1360-1362 (with 2184-2225)

**Function:** `Util_XML_Parse_Create_Node()`

**Why it's thought to be a bug**

`skipToKeyEndNoRewind` returns 1 when it finds the key again (a duplicate) and 0 when it doesn't; it never returns a negative. So `if(skipToKeyEndNoRewind(infile,varstr) < 0)` is never true and duplicate parameters in the .prm file are never detected during conversion. Function: `Util_XML_Parse_Create_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1380–1386) — code appears to have moved +22 lines since the review

```c
 1380:  *
 1381:  */
 1382: xmlNodePtr Util_XML_Parse_Create_Node(FILE *infile, char *fileName, xmlNodePtr cur, char *valueName, char *comment, char *units, int type,
 1383: 		char *recommendedValue) {
 1384: 	char ch, buf[STRLEN], seps[] = " ,\t\n";
 1385: 	char *varstr, *valuestr, *unitstr;
 1386: 	int buflen = STRLEN;
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
**Listed lines:** 1359, 1379-1383, 1391

**Function:** `Util_XML_Parse_Create_Node()`

**Why it's thought to be a bug**

`set_keyprm_errfn(quiet)` is set for the duplicate check but only restored at 1391, after the missing-value checks. `keyprm_errfn(...)` at 1380/1382 is therefore `quiet` and doesn't stop execution, so a missing value (`valuestr == NULL`) reaches `strlen(valuestr)` and then `Util_XML_Create_Node`, which crashes. Function: `Util_XML_Parse_Create_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1380–1416) — code appears to have moved +23 lines since the review

```c
 1380:  *
 1381:  */
 1382: xmlNodePtr Util_XML_Parse_Create_Node(FILE *infile, char *fileName, xmlNodePtr cur, char *valueName, char *comment, char *units, int type,
 1383: 		char *recommendedValue) {
 1384: 	char ch, buf[STRLEN], seps[] = " ,\t\n";
 1385: 	char *varstr, *valuestr, *unitstr;
 1386: 	int buflen = STRLEN;
 1387: 	xmlNodePtr returnNode;
 1388: 	char tempBuf[STRLEN];
 1389: 	char *unitTypes[] =  { "us", "usec",  "ms",  "msec",  "s",  "sec",  "second",  "min", "minute",  "h",  "hr", "hour",  "d",  "day",  "week"};
 1390: 	int numUnits = 15;
 1391: 	int i;
 1392: 	void (*fn)(char *format, ...) = keyprm_errfn;
 1393: 
 1394: 	/* Loop for all lines */
 1395: 	fseek(infile, 0L, 0);
 1396: 	while (fgets(buf, buflen, infile) != NULL) {
 1397: 		ch = buf[0];
 1398: 		if ((ch != '#') && (ch != ' ') && (ch != '\n') && (ch != '\t')) {
 1399: 			/* it's a parameter line */
 1400: 			varstr = strtok(buf, seps);
 1401: 
 1402: 			if (strcmp(varstr, valueName) == 0) {
 1403: 
 1404: 
 1405: 				/* Check there isn't a duplicate value */
 1406: 				set_keyprm_errfn(quiet);
 1407: 				if(skipToKeyEndNoRewind(infile,varstr) < 0){
 1408: 					quit("Util_XML_Parse_Create_Node: Duplicate key found %s\n", varstr);
 1409: 				}
 1410: 
 1411: 				valuestr = strtok(NULL, seps);
 1412: 
 1413: 				/* grab the next token in case its a unit */
 1414: 				unitstr = strtok(NULL, seps);
 1415: 
 1416: 				if(unitstr != NULL){
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
**Listed lines:** 1601-1603

**Function:** `Parse_Into_Array()`

**Why it's thought to be a bug**

`valueCopy = malloc(STRLEN)` (STRLEN=2000, atlantisboxmodel.h:331), then `strcpy(valueCopy, valueStr)` copies the full XML array string with no length check. Long arrays (per-box arrays in large models, long prey/diet vectors) easily go past 2000 chars and overflow the heap. The copy is only used in an error message. Function: `Parse_Into_Array()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1557–1563) — code appears to have moved -42 lines since the review

```c
 1557: static int *Parse_Into_Int_Array(char *fileName, char *paramName, char *valueStr, int size, int entry_type) {
 1558: 	int i;
 1559: 	int *values = (int *) malloc((size_t)size * sizeof(int));
 1560: 	char *varStr;
 1561: 	char seps[] = " ,\t";
 1562: 	int val;
 1563: 	int lastval = 0;
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
**Listed lines:** 1470, 1489-1496

**Function:** `Replace_String_Entry()`

**Why it's thought to be a bug**

Each step rebuilds the whole growing output in `char tempStr[256]` with `snprintf(tempStr, sizeof(tempStr), "%s %s", returnString, ...)` and copies it back. Once the string passes 255 chars it is silently truncated, so the rewritten array loses its later entries and has the wrong length. `returnString` itself is `size*100` bytes. Function: `Replace_String_Entry()` (used by `Util_XML_Replace_Node_Value`).

**Current code** (`atlantisUtil/atUtilXML.c` lines 1511–1541) — code appears to have moved +43 lines since the review

```c
 1511:  *
 1512:  */
 1513: static char *Replace_String_Entry(char *valueStr, char *replaceStr, int size, int replaceIndex) {
 1514: 	char *varStr;
 1515: 	char seps[] = " ";
 1516: 	int i;
 1517: 	char tempStr[256];
 1518: 	char dummyString[100] = "0.0";
 1519: 	char *returnString = (char *) malloc(sizeof(char) * (size_t)(size * 100));
 1520: 
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
**Listed lines:** 1059-1062

**Function:** `Util_XML_Get_Node()`

**Why it's thought to be a bug**

`if (attributeNode == NULL) returnValue = NULL;` doesn't return, and the next line dereferences `attributeNode->nodesetval`. `Execute_Xpath_Expression` returns NULL on XPath failure. Callers at 1105 and 1127 (`Util_XML_Set_Node_Value`, `Util_XML_Replace_Node_Value`) also skip the NULL check. Function: `Util_XML_Get_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1057–1064)

```c
 1057: 		if (strchr(value, '\n') != NULL)
 1058: 			*strchr(value, '\n') = '\0';
 1059: 		Util_XML_Add_Node_Property(attributeNode, AtlantisXMLObjectNAMES[type][ATTRIBUTE_VALUE], value);
 1060: 	}
 1061: 
 1062: 	return attributeNode;
 1063: }
 1064: 
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
**Listed lines:** 815-818

**Function:** `Execute_Xpath_Expression()`

**Why it's thought to be a bug**

On context-creation failure, `Execute_Xpath_Expression` calls `xmlFreeDoc(doc)` on a document it doesn't own. The caller keeps using and later frees the doc, so this is a use-after-free or double free. Function: `Execute_Xpath_Expression()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 842–849) — code appears to have moved +29 lines since the review

```c
  842: 
  843: /**
  844:  * Execute_Xpath_Expression:
  845:  * @filename:       the input XML filename.
  846:  * @xpathExpr:      the xpath expression for evaluation.
  847:  * @nsList:     the optional list of known namespaces in
  848:  *          "<prefix1>=<href1> <prefix2>=href2> ..." format.
  849:  *
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
**Listed lines:** 2169

**Function:** `Util_Get_Node_Name()`

**Why it's thought to be a bug**

`if(strcmp((char *)node->name, AtlantisXMLObjectNAMES[i][0]))` is true when the names differ, so the function returns the name property of the first node type that doesn't match (usually NULL), not the matching one. It's used for error-message node names in atManageParamIO.c/atassessParamIO.c. Function: `Util_Get_Node_Name()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 2210–2214) — code appears to have moved +43 lines since the review

```c
 2210:  *
 2211:  */
 2212: char *Util_Get_Node_Name(xmlNodePtr node){
 2213: 	int i;
 2214: 
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
**Listed lines:** 1590 (with 1510)

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
**Listed lines:** 1547-1548, 1556

**Function:** `Parse_Into_Int_Array()`

**Why it's thought to be a bug**

`%e` is given `values[i]`, which is an `int`. This is undefined behaviour and prints garbage in the error message. Function: `Parse_Into_Int_Array()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1548–1561) — code appears to have moved +3 lines since the review

```c
 1548: 
 1549: /**
 1550:  * 	\brief Parse the values in the valueStr and return a newly allocated integer array.
 1551:  *
 1552:  * Note this function allocates memory and the caller is responsible for freeing this memory.
 1553:  *
 1554:  *
 1555:  *
 1556:  */
 1557: static int *Parse_Into_Int_Array(char *fileName, char *paramName, char *valueStr, int size, int entry_type) {
 1558: 	int i;
 1559: 	int *values = (int *) malloc((size_t)size * sizeof(int));
 1560: 	char *varStr;
 1561: 	char seps[] = " ,\t";
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
**Listed lines:** 1713-1721

**Function:** `Util_XML_Read_Array_String()`

**Why it's thought to be a bug**

If the string has fewer than `size` tokens, `strtok` returns NULL and `strcpy((*p)[i], varStr)` crashes. Tokens longer than STRLEN would also overflow. Function: `Util_XML_Read_Array_String()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1711–1723)

```c
 1711: int Util_XML_Read_Array_Double(int type, char *fileName, char *errorPath, xmlNodePtr parent, int entry_type, char *key, double **p, int size) {
 1712: 	xmlNodePtr attributeNode;
 1713: 	char *values;
 1714: 	char xpath[1000];
 1715: 	snprintf(xpath, sizeof(xpath), "%s/%s", errorPath, key);
 1716: 
 1717: 	attributeNode = Util_XML_Get_Node(type, parent, key);
 1718: 	if (attributeNode == NULL) {
 1719: 		return FALSE;
 1720: 	}
 1721: 
 1722: 	values = Util_XML_Get_Node_Value_String(type, attributeNode);
 1723: 	if (values == NULL) {
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
**Listed lines:** 1433-1435

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
**Listed lines:** 1752-1758, 1763

**Function:** `Util_XML_Get_Species_Node_Value()`

**Why it's thought to be a bug**

The `else` branch is only reached when `result >= 0`, so `if(result == -1)` inside it can never be true. `!(_finite(value)) || !(_finite(value))` tests the same thing twice (probably copied from an `isnan`/`_finite` pair). Neither changes behaviour. Function: `Util_XML_Get_Species_Node_Value()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 1750–1765)

```c
 1750: 	if (attributeNode == NULL) {
 1751: 		return FALSE;
 1752: 	}
 1753: 
 1754: 	values = Util_XML_Get_Node_Value_String(type, attributeNode);
 1755: 	if (values == NULL) {
 1756: 		return FALSE;
 1757: 	}
 1758: 
 1759: 	// Read the data into a temp array.
 1760: 	for (i = 0; i < size; ++i) {
 1761: 		if (i == 0)
 1762: 			// Get the first token in the line.
 1763: 			varStr = strtok(values, seps);
 1764: 		else
 1765: 			// Get the first token in the line.
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
**Listed lines:** 872, 878

**Function:** `Add_Comment_Node()`

**Why it's thought to be a bug**

`quit("... %d", strlen(comment))` passes a `size_t` to `%d`. `strlen(units)` runs without a NULL check, while `comment` is NULL-checked. Function: `Add_Comment_Node()`.

**Current code** (`atlantisUtil/atUtilXML.c` lines 913–923) — code appears to have moved +43 lines since the review

```c
  913: }
  914: 
  915: static void Add_Comment_Node(xmlNodePtr rootNode, char *comment, char *units) {
  916: 
  917: 	if (comment != NULL) {
  918: 		if (strlen(comment) > 2000) {
  919: 			quit("Comment is too long length = %d\n", strlen(comment));
  920: 		}
  921: 		if (strlen(comment) > 0) {
  922: 			Util_XML_Add_Node_Property(rootNode, "AttributeDescription", comment);
  923: 		}
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

# ConvertAtlantis/atCreateXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV1-18](#cnv1-18) | 33-45 | H/M | infinite loop | `while(1){ if(fgets(...) != NULL){...} }` has no else branch. At EOF fgets keeps returning NULL and the loop n… |  | [ ] |
| [CNV1-19](#cnv1-19) | 1836-1839 | M/M | inconsistent parsing / unchecked read | The value line is read with a raw `fgets(buf, buflen, inputFP)` (return value unchecked). Comment and blank li… |  | [ ] |
| [CNV1-20](#cnv1-20) | 1477-1478, 1629-1630 | H/L | NULL deref | `lengthString = strtok(NULL, seps); lengthValue = atoi(lengthString);` has no NULL check. A multi-stage key li… |  | [ ] |
| [CNV1-21](#cnv1-21) | 297-303 | M/L | NULL deref | `valueStr = strtok(NULL, seps)` can be NULL (key with no value on the line) and is passed unchecked to `fp(...… |  | [ ] |
| [CNV1-22](#cnv1-22) | 903/1165-1283, 1729/1809-1823, 1899/1950-2034 | M/M | buffer overrun (latent) | `char strPtr[50]; strcpy(strPtr, varStr);` where varStr is an unbounded token from a STRLEN/5000-byte line. Lo… |  | [ ] |
| [CNV1-23](#cnv1-23) | 731-732 | L/L | OOB read (error path) | `cohortStrings[cohort]` has 2 entries (atUtil.c:175), but `Check_Species_Cohort_Param` loops `cohort < numStag… |  | [ ] |
| [CNV1-24](#cnv1-24) | 1320, 2376 | L/L | format/arg mismatch | `quit("...paramID not found = %d (%s) in switch statement. \n", paramID, tag, fileName)`: `fileName` is passed… |  | [ ] |

---

## CNV1-18

**infinite loop** · Conf H / Sev M  
**Lines (current code):** 33-45  
**Lines at review (Oct 2026):** 33-45

**Function:** `Get_Next_Line()`

**Why it's thought to be a bug**

`while(1){ if(fgets(...) != NULL){...} }` has no else branch. At EOF fgets keeps returning NULL and the loop never ends. A parameter key on the last line with no value line, or followed only by comments/blank lines, makes the converter hang instead of printing the "No value found" error at 193-195 / 1490-1492. Function: `Get_Next_Line()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 31–47)

```c
   31: static void Print_Fishery_Species_Param_Error(MSEBoxModel *bm, char *fileName, int paramID, int guild);
   32: 
   33: static void Get_Next_Line(MSEBoxModel *bm, FILE *inputFP, char *buf){
   34: 	while(1){
   35: 		if(fgets(buf, STRLEN, inputFP) != NULL){
   36: 			trim(buf);
   37: 			if(buf[0] == '\n')
   38: 				continue;
   39: 			if(buf[0] == '#')
   40: 				continue;
   41: 			if(strlen(buf) > 0)
   42: 				return;
   43: 		}
   44: 	}
   45: }
   46: 
   47: 
```

**Suggested fix (from review):** On a NULL fgets, set `buf[0]='\0'` and return. The callers already quit on `strlen(buf)==0`.

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

## CNV1-19

**inconsistent parsing / unchecked read** · Conf M / Sev M  
**Lines (current code):** 1836-1839  
**Lines at review (Oct 2026):** 1836-1839

**Function:** `Create_Harvest_Fishery_Group_ParamXML()`

**Why it's thought to be a bug**

The value line is read with a raw `fgets(buf, buflen, inputFP)` (return value unchecked). Comment and blank lines between key and values are not skipped, the trailing `\n` is not trimmed, there is no numeric check, and the declared length is not checked. Every other array reader in this file uses `Get_Next_Line` + `trim` + `CheckIsDigits`. A comment line under a `sel_XXX`/`q_XXX` key is silently stored as the parameter value. Function: `Create_Harvest_Fishery_Group_ParamXML()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 1834–1841)

```c
 1834: 				guild = Util_Get_FG_Index(bm, speciesStr);
 1835: 				if (guild != -1) {
 1836: 
 1837: 					guildCheck[guild] = TRUE;
 1838: 					// Get the next line.
 1839: 					fgets(buf, buflen, inputFP);
 1840: 					/* Set the group attribute - if the node does not already exist it will be created */
 1841: 					Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, newNode, FunctGroupArray[guild].groupCode, buf);
```

**Suggested fix (from review):** Use `Get_Next_Line(bm,inputFP,buf); trim(buf);` + `CheckIsDigits`, and validate the token count against K_num_fisheries.

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

## CNV1-20

**NULL deref** · Conf H / Sev L  
**Lines (current code):** 1477-1478, 1629-1630  
**Lines at review (Oct 2026):** 1477-1478, 1629-1630

**Why it's thought to be a bug**

`lengthString = strtok(NULL, seps); lengthValue = atoi(lengthString);` has no NULL check. A multi-stage key line with no length token crashes. `Parse_File` (179-181) checks the same case. Also, a length mismatch only produces a warning, and all stages are then marked present anyway (1483-1485 / 1635-1637). Functions: `Create_Species_Cohort_ParamXML()`, `Create_Species_Spawn_ParamXML()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 1475–1480)

```c
 1475: 
 1476: 				/* Put the values in */
 1477: 				if(size > 1){
 1478: 					/* Check the length */
 1479: 					lengthString = strtok(NULL, seps);
 1480: 					lengthValue = atoi(lengthString);
```

**Suggested fix (from review):** Add `if(lengthString==NULL) quit(...)`.

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

## CNV1-21

**NULL deref** · Conf M / Sev L  
**Lines (current code):** 297-303  
**Lines at review (Oct 2026):** 297-303

**Function:** `Parse_File_Single_Line()`

**Why it's thought to be a bug**

`valueStr = strtok(NULL, seps)` can be NULL (key with no value on the line) and is passed unchecked to `fp(...)` and from there to `Util_XML_Set_Node_Value`. Function: `Parse_File_Single_Line()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 295–305)

```c
  295: 			varStr = strtok(tempBuf, seps);
  296: 			/* Get the value token - must get this before we pass to the regular expression as it stuffs up the bufder */
  297: 			valueStr = strtok(NULL, seps);
  298: 
  299: 			/* Check to see if our varStr matches our expression */
  300: 			if (regexec(&(regBuffer), varStr, elementsof ( pmatch ), pmatch, 0) == 0) {
  301: 
  302: 				/* Call the given function pointer to parse the data and add it to the XML DOM */
  303: 				fp(bm, fileName, newnode, varStr, valueStr);
  304: 				found = TRUE;
  305: 			}
```

**Suggested fix (from review):** Quit with a message if `valueStr == NULL`.

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

## CNV1-22

**buffer overrun (latent)** · Conf M / Sev M  
**Lines (current code):** 903/1165-1283, 1729/1809-1823, 1899/1950-2034  
**Lines at review (Oct 2026):** 903/1165-1283, 1729/1809-1823, 1899/1950-2034

**Why it's thought to be a bug**

`char strPtr[50]; strcpy(strPtr, varStr);` where varStr is an unbounded token from a STRLEN/5000-byte line. Long keys (e.g. `XXXX_coral_max_accel_trans_...`, long fishery codes + param name) overflow the stack buffer. `strstr`/`strchr` results are also dereferenced without NULL checks (e.g. 1166, 1173, 1199, 1206). Functions: `Create_Species_ParamXML()`, `Create_Harvest_Fishery_Group_ParamXML()`, `Create_Fishery_ParamXML()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 901–905)

```c
  901: 	char ch, buf[5000], seps[] = " ,\t\n";
  902: 	char *varStr, *valuestr;
  903: 	char strPtr[50];
  904: 	char tempBuf[5000];
  905: 	xmlNodePtr newNode;
```

**Suggested fix (from review):** Use `snprintf(strPtr, sizeof(strPtr), "%s", varStr)` (or a STRLEN buffer) and check the strstr/strchr results.

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

## CNV1-23

**OOB read (error path)** · Conf L / Sev L  
**Lines (current code):** 731-732  
**Lines at review (Oct 2026):** 731-732

**Function:** `Print_Species_Cohort_Param_Error()`

**Why it's thought to be a bug**

`cohortStrings[cohort]` has 2 entries (atUtil.c:175), but `Check_Species_Cohort_Param` loops `cohort < numStages`, which can be greater than 2 (seagrass/coral stages). Printing the error then reads out of bounds. Function: `Print_Species_Cohort_Param_Error()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 729–734)

```c
  729: 
  730: void Print_Species_Cohort_Param_Error(MSEBoxModel *bm, char *fileName, int paramIndex, int guild, int cohort) {
  731: 	fprintf(stderr, "\n\nERROR: Failed to find species parameter %s in file %s for group %s, cohort %s", cohortParamStrings[paramIndex], fileName,
  732: 			FunctGroupArray[guild].groupCode, cohortStrings[cohort]);
  733: 	fflush(stderr);
  734: 	fflush(stdout);
```

**Suggested fix (from review):** Print `stage %d` when cohort > 1.

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

## CNV1-24

**format/arg mismatch** · Conf L / Sev L  
**Lines (current code):** 1320, 2376  
**Lines at review (Oct 2026):** 1320, 2376

**Why it's thought to be a bug**

`quit("...paramID not found = %d (%s) in switch statement. \n", paramID, tag, fileName)`: `fileName` is passed but has no `%s`, so the file is never reported (extra arg, harmless). Functions: `Create_Species_ParamXML()`, `Create_RBC_Species_ParamXML()`.

**Current code** (`ConvertAtlantis/atCreateXML.c` lines 1318–1322)

```c
 1318:                     (*strstr(speciesStr, "MultiyrMigrate")) = '\0';
 1319:                     break;
 1320:                  */
 1321: 				default:
 1322: 					quit("Create_Species_ParamXML: paramID not found = %d (%s) in switch statement. \n", paramID, speciesParamStructArray[index].tag, fileName);
```

**Suggested fix (from review):** Add `in file %s` to the format.

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

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atCreateXML.c `Check_Species_Param()` case `SP_MICROALGAE` (516-526) is identical to `SP_PP` (493-503). This may be intended to exclude macrophytes.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

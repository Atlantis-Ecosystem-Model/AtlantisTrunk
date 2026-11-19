# ConvertAtlantis/atCreateXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV1-18](#cnv1-18) | 34 | H/M | infinite loop | `while(1){ if(fgets(...) != NULL){...} }` has no else branch. At EOF fgets keeps returning NULL and the loop n… |  | [ ] |
| [CNV1-19](#cnv1-19) | 1839 | M/M | inconsistent parsing / unchecked read | The value line is read with a raw `fgets(buf, buflen, inputFP)` (return value unchecked). Comment and blank li… |  | [ ] |
| [CNV1-20](#cnv1-20) | 1479 | H/L | NULL deref | `lengthString = strtok(NULL, seps); lengthValue = atoi(lengthString);` has no NULL check. A multi-stage key li… |  | [ ] |
| [CNV1-21](#cnv1-21) | 297 | M/L | NULL deref | `valueStr = strtok(NULL, seps)` can be NULL (key with no value on the line) and is passed unchecked to `fp(...… |  | [ ] |
| [CNV1-22](#cnv1-22) | 1200 | M/M | buffer overrun (latent) | `char strPtr[50]; strcpy(strPtr, varStr);` where varStr is an unbounded token from a STRLEN/5000-byte line. Lo… |  | [ ] |
| [CNV1-23](#cnv1-23) | 732, 748 | L/L | OOB read (error path) | `cohortStrings[cohort]` has 2 entries (atUtil.c:175), but `Check_Species_Cohort_Param` loops `cohort < numStag… |  | [ ] |
| [CNV1-24](#cnv1-24) | 1322 | L/L | format/arg mismatch | `quit("...paramID not found = %d (%s) in switch statement. \n", paramID, tag, fileName)`: `fileName` is passed… |  | [ ] |

---

## CNV1-18

**infinite loop** · Conf H / Sev M  
**Found at (current code):** `atCreateXML.c` 34
**Lines at review (Oct 2026):** 33-45

**Function:** `Get_Next_Line()`

**Why it's thought to be a bug**

`while(1){ if(fgets(...) != NULL){...} }` has no else branch. At EOF fgets keeps returning NULL and the loop never ends. A parameter key on the last line with no value line, or followed only by comments/blank lines, makes the converter hang instead of printing the "No value found" error at 193-195 / 1490-1492. Function: `Get_Next_Line()`.

**Code at issue** — `atCreateXML.c` line 34  
Search for: `while(1){`

```c
   32: 
   33: static void Get_Next_Line(MSEBoxModel *bm, FILE *inputFP, char *buf){
   34→ 	while(1){
   35: 		if(fgets(buf, STRLEN, inputFP) != NULL){
   36: 			trim(buf);
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
**Found at (current code):** `atCreateXML.c` 1839
**Lines at review (Oct 2026):** 1836-1839

**Function:** `Create_Harvest_Fishery_Group_ParamXML()`

**Why it's thought to be a bug**

The value line is read with a raw `fgets(buf, buflen, inputFP)` (return value unchecked). Comment and blank lines between key and values are not skipped, the trailing `\n` is not trimmed, there is no numeric check, and the declared length is not checked. Every other array reader in this file uses `Get_Next_Line` + `trim` + `CheckIsDigits`. A comment line under a `sel_XXX`/`q_XXX` key is silently stored as the parameter value. Function: `Create_Harvest_Fishery_Group_ParamXML()`.

**Code at issue** — `atCreateXML.c` line 1839  
Search for: `fgets(buf, buflen, inputFP)`

```c
 1837: 					guildCheck[guild] = TRUE;
 1838: 					// Get the next line.
 1839→ 					fgets(buf, buflen, inputFP);
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
**Found at (current code):** `atCreateXML.c` 1479
**Lines at review (Oct 2026):** 1477-1478, 1629-1630

**Why it's thought to be a bug**

`lengthString = strtok(NULL, seps); lengthValue = atoi(lengthString);` has no NULL check. A multi-stage key line with no length token crashes. `Parse_File` (179-181) checks the same case. Also, a length mismatch only produces a warning, and all stages are then marked present anyway (1483-1485 / 1635-1637). Functions: `Create_Species_Cohort_ParamXML()`, `Create_Species_Spawn_ParamXML()`.

**Code at issue** — `atCreateXML.c` line 1479  
Search for: `lengthString = strtok(NULL, seps);`

```c
 1477: 				if(size > 1){
 1478: 					/* Check the length */
 1479→ 					lengthString = strtok(NULL, seps);
 1480: 					lengthValue = atoi(lengthString);
 1481: 					if(lengthValue != size){
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
**Found at (current code):** `atCreateXML.c` 297
**Lines at review (Oct 2026):** 297-303

**Function:** `Parse_File_Single_Line()`

**Why it's thought to be a bug**

`valueStr = strtok(NULL, seps)` can be NULL (key with no value on the line) and is passed unchecked to `fp(...)` and from there to `Util_XML_Set_Node_Value`. Function: `Parse_File_Single_Line()`.

**Code at issue** — `atCreateXML.c` line 297  
Search for: `valueStr = strtok(NULL, seps)`

```c
  295: 			varStr = strtok(tempBuf, seps);
  296: 			/* Get the value token - must get this before we pass to the regular expression as it stuffs up the bufder */
  297→ 			valueStr = strtok(NULL, seps);
  298: 
  299: 			/* Check to see if our varStr matches our expression */
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
**Found at (current code):** `atCreateXML.c` 1200
**Lines at review (Oct 2026):** 903/1165-1283, 1729/1809-1823, 1899/1950-2034

**Why it's thought to be a bug**

`char strPtr[50]; strcpy(strPtr, varStr);` where varStr is an unbounded token from a STRLEN/5000-byte line. Long keys (e.g. `XXXX_coral_max_accel_trans_...`, long fishery codes + param name) overflow the stack buffer. `strstr`/`strchr` results are also dereferenced without NULL checks (e.g. 1166, 1173, 1199, 1206). Functions: `Create_Species_ParamXML()`, `Create_Harvest_Fishery_Group_ParamXML()`, `Create_Fishery_ParamXML()`.

**Code at issue** — `atCreateXML.c` line 1200 (located by name only; check this is the right place)  
Search for: `(*strchr(strPtr, '_')) = '\0';`

```c
 1198: 				case min_O2_id:
 1199: 					strcpy(strPtr, varStr);
 1200→ 					(*strchr(strPtr, '_')) = '\0';
 1201: 					speciesStr = strPtr;
 1202: 					break;
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
**Found at (current code):** `atCreateXML.c` 732, `atCreateXML.c` 748
**Lines at review (Oct 2026):** 731-732

**Function:** `Print_Species_Cohort_Param_Error()`

**Why it's thought to be a bug**

`cohortStrings[cohort]` has 2 entries (atUtil.c:175), but `Check_Species_Cohort_Param` loops `cohort < numStages`, which can be greater than 2 (seagrass/coral stages). Printing the error then reads out of bounds. Function: `Print_Species_Cohort_Param_Error()`.

**Code at issue** — `atCreateXML.c` line 732  
Search for: `cohortStrings[cohort]`

```c
  730: void Print_Species_Cohort_Param_Error(MSEBoxModel *bm, char *fileName, int paramIndex, int guild, int cohort) {
  731: 	fprintf(stderr, "\n\nERROR: Failed to find species parameter %s in file %s for group %s, cohort %s", cohortParamStrings[paramIndex], fileName,
  732→ 			FunctGroupArray[guild].groupCode, cohortStrings[cohort]);
  733: 	fflush(stderr);
  734: 	fflush(stdout);
```

**Code at issue** — `atCreateXML.c` line 748 (closest match to the quoted code; wording differs slightly)  
Search for: `for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {`

```c
  746: 			if (FunctGroupArray[guild].speciesParams[flag_id] == TRUE && FunctGroupArray[guild].isDetritus == FALSE) {                
  747:                 if (FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED || FunctGroupArray[guild].groupAgeType == AGE_STRUCTURED_BIOMASS) {
  748→ 					for (cohort = 0; cohort < FunctGroupArray[guild].numStages; cohort++) {
  749: 						if (guildCheck[guild][cohort] == FALSE)
  750: 							Print_Species_Cohort_Param_Error(bm, fileName, paramID, guild, cohort);
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
**Found at (current code):** `atCreateXML.c` 1322
**Lines at review (Oct 2026):** 1320, 2376

**Why it's thought to be a bug**

`quit("...paramID not found = %d (%s) in switch statement. \n", paramID, tag, fileName)`: `fileName` is passed but has no `%s`, so the file is never reported (extra arg, harmless). Functions: `Create_Species_ParamXML()`, `Create_RBC_Species_ParamXML()`.

**Code at issue** — `atCreateXML.c` line 1322  
Search for: `quit("`

```c
 1320:                  */
 1321: 				default:
 1322→ 					quit("Create_Species_ParamXML: paramID not found = %d (%s) in switch statement. \n", paramID, speciesParamStructArray[index].tag, fileName);
 1323: 					break;
 1324: 				}
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

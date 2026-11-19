# atCLAMLink/atCLAMComm.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-01](#link-01) | 228 | H/M | NULL dereference | `*(strstr(valueStr, ".")) = '\0';` — crashes if the second token has no '.', and `valueStr` itself is NULL whe… |  | [ ] |
| [LINK-02](#link-02) ⭐ | 246 | H/H | buffer overrun | `while ((valueStr = strtok(NULL, seps)) != NULL) { outcomeValueArray[...][i] = atof(valueStr); i++; }` — `i` h… |  | [ ] |
| [LINK-03](#link-03) | 232 | M/M | wrong index / overrun | `indicatorIndex = Get_CLAM_IndicatorID(...)` returns an index into `indicators[]` (0..numIndicators-1, read fr… |  | [ ] |
| [LINK-04](#link-04) | 146 | M/L | off-by-one | `if (optionSelected > maxOptionValue) quit(...)` — `optionSelected` is a 0-based strategy ID (it is written as… |  | [ ] |
| [LINK-05](#link-05) | 216 | L/L | buffer / truncation | `fgets(buf, 200, ...)`: lines longer than 199 chars are split, and the rest is parsed as a new line (its first… |  | [ ] |

---

## LINK-01

**NULL dereference** · Conf H / Sev M  
**Found at (current code):** `atCLAMComm.c` 228
**Lines at review (Oct 2026):** 228

**Function:** `Parse_CLAM_OutputFile()`

**Why it's thought to be a bug**

`*(strstr(valueStr, ".")) = '\0';` — crashes if the second token has no '.', and `valueStr` itself is NULL when a line has only one token (strtok returns NULL). Data comes from an external CLAM output file. Function: `Parse_CLAM_OutputFile()`.

**Code at issue** — `atCLAMComm.c` line 228  
Search for: `*(strstr(valueStr, ".")) = '\0'`

```c
  226: 
  227: 		/* Strip off the text after the '.' */
  228→ 		*(strstr(valueStr, ".")) = '\0';
  229: 		printf("Output Parameter = %s\n", valueStr);
  230: 
```

**Suggested fix (from review):** Check `valueStr != NULL` and the strstr result before writing.

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

## LINK-02

**buffer overrun** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atCLAMComm.c` 246
**Lines at review (Oct 2026):** 245-254

**Function:** `Parse_CLAM_OutputFile()`

**Why it's thought to be a bug**

`while ((valueStr = strtok(NULL, seps)) != NULL) { outcomeValueArray[...][i] = atof(valueStr); i++; }` — `i` has no upper bound, but the last dimension is MAXNUMOUTPUTSTATES (10) (atCLAMLinkSetup.c:68). A CLAM line with more than 10 values after the name writes past the end of the row (heap corruption). Function: `Parse_CLAM_OutputFile()`.

**Code at issue** — `atCLAMComm.c` line 246  
Search for: `while ((valueStr = strtok(NULL, seps)) != NULL) {`

```c
  244: 
  245: 			i = 0;
  246→ 			while ((valueStr = strtok(NULL, seps)) != NULL) {
  247: 				outcomeValueArray[indicatorIndex][scenarioIndex][strategyIndex][i] = atof(valueStr);
  248: 				printf("outcomeValueArray[%d][%d][%d][%d] = %e, \n", indicatorIndex, scenarioIndex, strategyIndex, i,
```

**Suggested fix (from review):** Stop the loop at `i < MAXNUMOUTPUTSTATES` (or at `numStates`) and warn.

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

## LINK-03

**wrong index / overrun** · Conf M / Sev M  
**Found at (current code):** `atCLAMComm.c` 232
**Lines at review (Oct 2026):** 232, 247

**Function:** `Parse_CLAM_OutputFile()`

**Why it's thought to be a bug**

`indicatorIndex = Get_CLAM_IndicatorID(...)` returns an index into `indicators[]` (0..numIndicators-1, read from XML), but it is used as the first index of `outcomeValueArray`, which has size `numClamIndicators` (7) (atCLAMLinkSetup.c:68). The same mix-up affects `expectedValue/variance/distance[..][..][indicatorIndex]` and `indicatorWeightings[indicatorIndex]` in atCLAMManage.c (all sized numClamIndicators). Overruns if the XML lists more than 7 indicators (for example two indicators mapped to the same CLAM output). Function: `Parse_CLAM_OutputFile()`.

**Code at issue** — `atCLAMComm.c` line 232  
Search for: `indicatorIndex = Get_CLAM_IndicatorID(`

```c
  230: 
  231: 		/* Find the index of this output parameter  */
  232→ 		indicatorIndex = Get_CLAM_IndicatorID(bm, valueStr);
  233: 
  234: 		// indicatorIndex = 1;
```

**Suggested fix (from review):** Size these arrays by `numIndicators`, or index them by `indicators[i].CLAMOutputIndex`.

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

## LINK-04

**off-by-one** · Conf M / Sev L  
**Found at (current code):** `atCLAMComm.c` 146
**Lines at review (Oct 2026):** 146

**Function:** `Create_CLAM_Input_File()`

**Why it's thought to be a bug**

`if (optionSelected > maxOptionValue) quit(...)` — `optionSelected` is a 0-based strategy ID (it is written as `+1`, line 149) and `maxOptionValue` is the number of options (e.g. 4 for "Sea Level Rise", which has IDs 0..3), so the check lets ID == maxOptionValue through. Function: `Create_CLAM_Input_File()`.

**Code at issue** — `atCLAMComm.c` line 146 (the line the review pointed to, found by its text)  
Search for: `if (CLAMScenarioOptions[i].optionSelected > CLAMScenarioOptions[i].maxOptionValue)`

```c
  144: 	fprintf(fp, "INPUT RULES\n");
  145: 	for (i = 0; i < numScenarioOptions; i++) {
  146→ 		if (CLAMScenarioOptions[i].optionSelected > CLAMScenarioOptions[i].maxOptionValue)
  147: 			quit("Create_CLAM_Input_File The CLAM Scenario option for %s is greater than the allowed value. You selected %d but the max index is %d\n",
  148: 					CLAMScenarioOptions[i].scenarioOptionName, CLAMScenarioOptions[i].optionSelected, CLAMScenarioOptions[i].maxOptionValue);
```

**Suggested fix (from review):** Use `>=`.

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

## LINK-05

**buffer / truncation** · Conf L / Sev L  
**Found at (current code):** `atCLAMComm.c` 216
**Lines at review (Oct 2026):** 216

**Function:** `Parse_CLAM_OutputFile()`

**Why it's thought to be a bug**

`fgets(buf, 200, ...)`: lines longer than 199 chars are split, and the rest is parsed as a new line (its first token is skipped and its second is treated as a name). Function: `Parse_CLAM_OutputFile()`.

**Code at issue** — `atCLAMComm.c` line 216 (closest match to the quoted code; wording differs slightly)  
Search for: `while (fgets(buf, buflen, infile) != NULL) {`

```c
  214: 	/* Loop for all lines */
  215: 	fseek(infile, 0L, 0);
  216→ 	while (fgets(buf, buflen, infile) != NULL) {
  217: 		ch = buf[0];
  218: 
```

**Suggested fix (from review):** Use a larger buffer, or detect a missing '\n' and handle it.

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

atCLAMComm.c Start_CLAM (WIN32 only): `WaitForSingleObject(..., processWaitTime)` takes a DWORD but is passed a double, and the return is not checked. If the process times out, the code still looks for the finished file and does not terminate the process on the success path.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

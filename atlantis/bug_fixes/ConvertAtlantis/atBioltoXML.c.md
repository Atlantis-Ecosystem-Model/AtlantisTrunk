# ConvertAtlantis/atBioltoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **17 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 4 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV1-01](#cnv1-01) ⭐ | 1782, 1785 | H/H | buffer overrun / wrong variable | `tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';`: `secondlastLetter` points at the la… |  | [ ] |
| [CNV1-02](#cnv1-02) | 1164, 1175, 1161 | H/M | wrong variable / logic | When `bm->flag_macro_model` is on, the `else { cohort = main_biomass_id; }` branch overwrites the `cohort = ju… |  | [ ] |
| [CNV1-03](#cnv1-03) | 976 | H/L | stale variable / not reset in loop | `groupNode` is set to NULL once, before the guild loop. For a guild that is `isOverWinter` but not mobile (or … |  | [ ] |
| [CNV1-04](#cnv1-04) | 4083 | M/M | format mismatch / wrong check | `quit("...(%d)... (value of %d)...", numGroups, convertedXMLFileName, bm->K_num_tot_sp)`: the 2nd `%d` gets a … |  | [ ] |
| [CNV1-05](#cnv1-05) | 4130 | M/M | missing bounds check | `start_stage` and `migrationID` come straight from the CSV and are used unchecked as `MIGRATION[sp].X_Prm[star… |  | [ ] |
| [CNV1-06](#cnv1-06) | 636 | M/H | buffer overrun | `char sedimentValues[1000]; char finalValues[1000];` collect K_num_tot_sp tokens with `strcat`, and the input … |  | [ ] |
| [CNV1-07](#cnv1-07) | 753, 726 | L/M | OOB index / wrong error text | `cohortNode = ...cohortStrings[i]` (753) runs *before* the `i == -1` check at 755. `cohortStrings` has only 2 … |  | [ ] |
| [CNV1-08](#cnv1-08) | 96, 99 | M/M | buffer overrun (edge) | `malloc(3*size)` followed by `strcpy(zeroString,"")`: with size==0 this is malloc(0) followed by a 1-byte writ… |  | [ ] |
| [CNV1-09](#cnv1-09) | 1124 | M/M | hard-coded code length | `speciesStr = str + strlen(str) - 3;` assumes the predator code is exactly 3 characters (`p_BCFDB` gives prey … |  | [ ] |
| [CNV1-10](#cnv1-10) | 1124 | M/M | hard-coded code length | `if(strlen(str) == 9) speciesStr = str + 6; else speciesStr = str + 5;` for `mS_FDxxx`/`mS_SBxxx`. A 4-charact… |  | [ ] |
| [CNV1-11](#cnv1-11) | 2441 | H/L | copy-paste (metadata) | `max_move_temp_id` is described as "Min tolerated temperatures" and `min_move_temp_id` as "Maximum tolerated t… |  | [ ] |
| [CNV1-12](#cnv1-12) | 3460 | H/L | copy-paste (metadata) | Copy-pasted descriptions or units: FLAG_LYSIS_RATE units "mg O2 m-3" (rate is d-1); `coral_compete_id` reuses … |  | [ ] |
| [CNV1-13](#cnv1-13) | 2112 | H/L | wrong XML type | `flag_refuge_model` is declared XML_TYPE_BOOLEAN but has a 3-value lookup (0/1/2, and 2 = rugosity_refuge_mode… |  | [ ] |
| [CNV1-14](#cnv1-14) | 3762 | H/L | duplicate block | `Util_XML_Parse_Create_Node(..., "InvadersEntering", ...)` is called twice, so two identical attribute nodes a… |  | [ ] |
| [CNV1-15](#cnv1-15) | 3961 | L/M | NULL deref on malformed input | Each of the 17 `strtok(NULL, seps)` results is passed straight to `Util_XML_Create_Node` as a value. A data ro… |  | [ ] |
| [CNV1-16](#cnv1-16) | 1345 | L/L | uninitialised variable | `char param[30]` is left uninitialised if none of the `strstr` branches match, then used as a node name. The f… |  | [ ] |
| [CNV1-17](#cnv1-17) | 3799, 3880, 4029 | L/L | resource leak | `if (doc == NULL) { ... return; }` returns without `fclose(fp)` / `fclose(inputFP)`. Functions: `Convert_Biol_… |  | [ ] |

---

## CNV1-01

**buffer overrun / wrong variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atBioltoXML.c` 1782, `atBioltoXML.c` 1785
**Lines at review (Oct 2026):** 1780-1786

**Function:** `PopRatioStockXMLFunction()`

**Why it's thought to be a bug**

`tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';`: `secondlastLetter` points at the last 2 chars, so `strlen(secondlastLetter)` is 2. `2 - 13 - 2` is computed as size_t and wraps, so the write lands about 13 bytes *before* `tempStr` on the stack, and `tempStr` itself is never truncated. The single-digit branch at 1785 uses `strlen(tempStr)` correctly. This path runs whenever a stock index has 2 digits, i.e. `FBP1popratioStock10` (regex at 3278 allows `[1-9][0-9]{0,2}`) with K_num_stocks_per_sp >= 10. A 3-digit stock index is not handled either. Function: `PopRatioStockXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 1782  
Search for: `tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0'`

```c
 1780: 	if(isdigit(secondlastLetter[0])) {  // Is the first digit a number if yes then a two digit case
 1781: 		stockIndex = atoi(secondlastLetter);
 1782→ 		tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';
 1783: 	} else {  // No make it a single string case
 1784: 		stockIndex = atoi(lastLetter);
```

**Code at issue** — `atBioltoXML.c` line 1785  
Search for: `strlen(tempStr)`

```c
 1783: 	} else {  // No make it a single string case
 1784: 		stockIndex = atoi(lastLetter);
 1785→ 		tempStr[strlen(tempStr) - strlen("popratioStock") - 1] = '\0';
 1786: 	}
 1787: 
```

**Suggested fix (from review):** Use `tempStr[strlen(tempStr) - strlen("popratioStock") - 2] = '\0';`, or better, find `strstr(tempStr,"popratioStock")` and put the terminator there.

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

## CNV1-02

**wrong variable / logic** · Conf H / Sev M  
**Found at (current code):** `atBioltoXML.c` 1164, `atBioltoXML.c` 1175, `atBioltoXML.c` 1161
**Lines at review (Oct 2026):** 1159-1177

**Function:** `InvertebrateClearanceRateXMLFunction()`

**Why it's thought to be a bug**

When `bm->flag_macro_model` is on, the `else { cohort = main_biomass_id; }` branch overwrites the `cohort = juv_id` / `adult_id` value set at 1149/1161 for every non-Epi/Root parameter, including non-seagrass groups. For AGE_STRUCTURED_BIOMASS inverts (e.g. CEP/PWN), both `C_CEP_T15` and `C_jCEP_T15` (and the `mum_` versions, via the call at 2977) then go to index 0 (main_biomass_id = 0) in `Util_XML_Replace_Node_Value(...,2,cohort)`. Whichever comes last wins, and the adult slot (index 1) stays at the "0.0" default (atUtilXML.c:1189-1195). Result: adult clearance/growth = 0 for those groups in macro-model runs. Function: `InvertebrateClearanceRateXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 1164  
Search for: `flag_macro_model`

```c
 1162: 	}
 1163: 
 1164→ 	if(bm->flag_macro_model){
 1165: 		/* Allow for seagrass cohorts as well */
 1166: 		if (strstr(speciesStr, "Epi_") != NULL) {
```

**Code at issue** — `atBioltoXML.c` line 1175 (closest match to the quoted code; wording differs slightly)  
Search for: `cohort = main_biomass_id;`

```c
 1173: 		}
 1174: 		else{
 1175→ 			cohort = main_biomass_id;
 1176: 		}
 1177: 	}
```

**Code at issue** — `atBioltoXML.c` line 1161  
Search for: `cohort = juv_id`

```c
 1159: 	if (strstr(speciesStr, "j") != NULL) {
 1160: 		strPtr++;
 1161→ 		cohort = juv_id;
 1162: 	}
 1163: 
```

**Suggested fix (from review):** Do the macro-model Epi/Root/main override only when the group is SEAGRASS. Otherwise keep the juv/adult cohort.

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

## CNV1-03

**stale variable / not reset in loop** · Conf H / Sev L  
**Found at (current code):** `atBioltoXML.c` 976
**Lines at review (Oct 2026):** 948-985

**Function:** `InitVertDistributionXML()`

**Why it's thought to be a bug**

`groupNode` is set to NULL once, before the guild loop. For a guild that is `isOverWinter` but not mobile (or is EPIFAUNA), `if(groupNode == NULL)` is false because groupNode still holds the *previous* guild's node. The "overwinter" temporal node (plus juv/adult children) is therefore attached to the wrong group, and that group can end up with duplicate "overwinter" nodes. Function: `InitVertDistributionXML()`.

**Code at issue** — `atBioltoXML.c` line 976  
Search for: `if(groupNode == NULL)`

```c
  974: 		if (FunctGroupArray[guild].isOverWinter == TRUE ){
  975: 
  976→ 			if(groupNode == NULL){
  977: 				groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
  978: 			}
```

**Suggested fix (from review):** Reset `groupNode = NULL;` at the top of each guild iteration.

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

## CNV1-04

**format mismatch / wrong check** · Conf M / Sev M  
**Found at (current code):** `atBioltoXML.c` 4083
**Lines at review (Oct 2026):** 4078-4079

**Function:** `Util_Read_Migration_XML()`

**Why it's thought to be a bug**

`quit("...(%d)... (value of %d)...", numGroups, convertedXMLFileName, bm->K_num_tot_sp)`: the 2nd `%d` gets a `char*` (UB on the error path). Separately, the check itself (`numGroups > K_num_tot_sp`) is wrong: each CSV row is one (group, stage, MigID) migration, so a valid file can have more rows than species (e.g. juv + adult rows for many groups), and this would quit spuriously. Function: `Util_Read_Migration_XML()`.

**Code at issue** — `atBioltoXML.c` line 4083  
Search for: `quit("`

```c
 4081:         numGroups = groupList->nodesetval->nodeNr;
 4082:         if (numGroups > bm->K_num_tot_sp) {
 4083→             quit("Util_Read_Migration_XML: Number of groups specified (%d) in your migration definition file is greater than the number of species - K_num_tot_sp (value of %d) in your run file\n", numGroups, convertedXMLFileName, bm->K_num_tot_sp);
 4084:         }
 4085:         
```

**Suggested fix (from review):** Remove or replace the check (e.g. validate per group against numStages*num_migs). Fix the format string to match its arguments.

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

## CNV1-05

**missing bounds check** · Conf M / Sev M  
**Found at (current code):** `atBioltoXML.c` 4130
**Lines at review (Oct 2026):** 4092-4128

**Function:** `Util_Read_Migration_XML()`

**Why it's thought to be a bug**

`start_stage` and `migrationID` come straight from the CSV and are used unchecked as `MIGRATION[sp].X_Prm[start_stage][migrationID]`. Those arrays are `[numStages][num_migrate+1]` (atBiologyXMLParamIO.c:4376, 4402-4421?) and Stagger_Prm is `[numStages][num_migs][2]`. A stage >= numStages, a negative value, or a MigID > num_migrate gives a silent heap write out of bounds. Function: `Util_Read_Migration_XML()`.

**Code at issue** — `atBioltoXML.c` line 4130 (the line the review pointed to, found by its text)  
Search for: `MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] = MIGRATIO`

```c
 4128:             MIGRATION[speciesIndex].ReprodAllowedPrm[start_stage][migrationID] = ReprodAllowedPartial;
 4129:             
 4130→             MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] = MIGRATION[speciesIndex].MaxYearsAway_Prm[start_stage][migrationID] - MIGRATION[speciesIndex].MinYearsAway_Prm[start_stage][migrationID];
 4131:             if( MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] > 0) {
 4132:                 MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][return_stagger_id] = 1;
```

**Suggested fix (from review):** Add `if(start_stage<0 || start_stage>=numStages || migrationID<0 || migrationID>num_migrate) quit(...)`.

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

## CNV1-06

**buffer overrun** · Conf M / Sev H  
**Found at (current code):** `atBioltoXML.c` 636
**Lines at review (Oct 2026):** 636-680

**Function:** `spInvertEatXMLFunction()`

**Why it's thought to be a bug**

`char sedimentValues[1000]; char finalValues[1000];` collect K_num_tot_sp tokens with `strcat`, and the input line can be up to STRLEN (2000) chars (Parse_File buf). Example: 90+ groups with values like `1.234e-05` (about 10 chars each) overflows the 1000-byte stack buffer. `spVERTEatXMLFunction` uses STRLEN buffers for the same data. `speciesStr[20]` filled by unbounded `sscanf("%s")` (643) is also latent. Function: `spInvertEatXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 636  
Search for: `char sedimentValues[1000];`

```c
  634: 	char speciesStr[20];
  635: 	int cohort = -1;
  636→ 	char sedimentValues[1000];
  637: 	char finalValues[1000];
  638: 	int sp;
```

**Suggested fix (from review):** Size the buffers to STRLEN (or 2*STRLEN) and use bounded concatenation (`strncat`/`snprintf` with remaining length).

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

## CNV1-07

**OOB index / wrong error text** · Conf L / Sev M  
**Found at (current code):** `atBioltoXML.c` 753, `atBioltoXML.c` 726
**Lines at review (Oct 2026):** 744-757, 762-767

**Function:** `spVERTEatXMLFunction()`

**Why it's thought to be a bug**

`cohortNode = ...cohortStrings[i]` (753) runs *before* the `i == -1` check at 755. `cohortStrings` has only 2 entries (atUtil.c:175), and neither `i` (last char) nor `j` (leading digit) is range-checked. The error message at 756 names `spInvertEatXMLFunction` (copy-paste). `finalValues[STRLEN]` with `%.10e` (17 chars/value) also overflows for more than about 117 groups. Function: `spVERTEatXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 753  
Search for: `cohortNode =`

```c
  751: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
  752: 	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
  753→ 	cohortNode = Util_XML_Get_Or_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[i]);
  754: 
  755: 	if(i ==-1 && FunctGroupArray[speciesIndex].numCohorts > 1){
```

**Code at issue** — `atBioltoXML.c` line 726  
Search for: `finalValues[STRLEN]`

```c
  724: 	int i, j, sp;
  725: 	char sedimentValues[STRLEN];
  726→ 	char finalValues[STRLEN];
  727: 	char *tempValues = (char *) malloc(sizeof(char) * STRLEN);
  728: 	int speciesIndex;
```

**Suggested fix (from review):** Validate `0<=i<=1 && 0<=j<=1` before using them. Fix the message text. Bound the strcat.

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

## CNV1-08

**buffer overrun (edge)** · Conf M / Sev M  
**Found at (current code):** `atBioltoXML.c` 96, `atBioltoXML.c` 99
**Lines at review (Oct 2026):** 94-107

**Function:** `CreateZeroString()`

**Why it's thought to be a bug**

`malloc(3*size)` followed by `strcpy(zeroString,"")`: with size==0 this is malloc(0) followed by a 1-byte write. Possible callers with size 0: `InitSuppDietXML`/`InitHabitatXML` (K_num_cover_types), `InitCatchPropGrazersXML` (K_num_fisheries). Also no NULL check on malloc. Function: `CreateZeroString()`.

**Code at issue** — `atBioltoXML.c` line 96 (closest match to the quoted code; wording differs slightly)  
Search for: `char *zeroString = (char *) malloc(sizeof(char) * (unsigned int long)3 * (unsigned int lon`

```c
   94: static char *CreateZeroString(int size){
   95: 	int i;
   96→ 	char *zeroString = (char *) malloc(sizeof(char) * (unsigned int long)3 * (unsigned int long)size);
   97: 
   98: 
```

**Code at issue** — `atBioltoXML.c` line 99  
Search for: `strcpy(zeroString, "")`

```c
   97: 
   98: 
   99→ 	strcpy(zeroString, "");
  100: 
  101: 	for(i = 0; i < size; i++){
```

**Suggested fix (from review):** Use `malloc(3*size + 1)` and check for NULL.

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

## CNV1-09

**hard-coded code length** · Conf M / Sev M  
**Found at (current code):** `atBioltoXML.c` 1124
**Lines at review (Oct 2026):** 1115-1135

**Function:** `AgeDietXMLFunction()`

**Why it's thought to be a bug**

`speciesStr = str + strlen(str) - 3;` assumes the predator code is exactly 3 characters (`p_BCFDB` gives prey BC, pred FDB). Atlantis codes of 2 or 4 characters give the wrong prey/pred split, and the node is created under a bogus group name with no validation (no FG lookup). Function: `AgeDietXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 1124  
Search for: `speciesStr = str + strlen(str) - 3`

```c
 1122: 	// copy the key to the key string.
 1123: 	strcpy(tempStr, str);
 1124→ 	speciesStr = str + strlen(str) - 3;
 1125: 	strcpy(tempStr, speciesStr);
 1126: 
```

**Suggested fix (from review):** Look up the predator by trying FG codes that match the string suffix (or split using the known prey code), and validate both with `Util_Get_FG_Index_From_Token`.

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

## CNV1-10

**hard-coded code length** · Conf M / Sev M  
**Found at (current code):** `atBioltoXML.c` 1124
**Lines at review (Oct 2026):** 1267-1271

**Function:** `ImplicitMortalityRateXMLFunction()`

**Why it's thought to be a bug**

`if(strlen(str) == 9) speciesStr = str + 6; else speciesStr = str + 5;` for `mS_FDxxx`/`mS_SBxxx`. A 4-character group code (`mS_FDABCD`, length 9) is cut to `BCD`: wrong group or quit. The prm files seen use `mS_FDFBP` (length 8), so the +6 branch looks like it is only for an optional separator. Function: `ImplicitMortalityRateXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 1124 (located by name only; check this is the right place)  
Search for: `speciesStr = str + strlen(str) - 3;`

```c
 1122: 	// copy the key to the key string.
 1123: 	strcpy(tempStr, str);
 1124→ 	speciesStr = str + strlen(str) - 3;
 1125: 	strcpy(tempStr, speciesStr);
 1126: 
```

**Suggested fix (from review):** Use `str + 5` and skip one optional `_`.

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

## CNV1-11

**copy-paste (metadata)** · Conf H / Sev L  
**Found at (current code):** `atBioltoXML.c` 2441
**Lines at review (Oct 2026):** 2426-2427, 2433-2435, 2439-2440

**Function:** `createMovementXML()`

**Why it's thought to be a bug**

`max_move_temp_id` is described as "Min tolerated temperatures" and `min_move_temp_id` as "Maximum tolerated temperatures". The salinity pair is swapped the same way. `min_O2_id` is described as "Maximum tolerated O2". `K_salt_const_id` and `K_o2_const_id` have units "deg C". Function: `createMovementXML()`.

**Code at issue** — `atBioltoXML.c` line 2441 (the line the review pointed to, found by its text)  
Search for: `Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_O2_id, "Maximum tolerated O2",`

```c
 2439: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagO2depend", &flagdepend);
 2440: 	if (flagdepend) {
 2441→ 		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_O2_id, "Maximum tolerated O2", "", XML_TYPE_FLOAT, "");
 2442:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_o2_const_id, "Constant in bilogistic tolerated O2 relationship", "deg C", XML_TYPE_FLOAT, "");
 2443: 	}
```

**Suggested fix (from review):** Swap the descriptions back and fix the units.

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

## CNV1-12

**copy-paste (metadata)** · Conf H / Sev L  
**Found at (current code):** `atBioltoXML.c` 3460
**Lines at review (Oct 2026):** 2465, 2636, 2644, 3104, 3129, 3458, 3460

**Why it's thought to be a bug**

Copy-pasted descriptions or units: FLAG_LYSIS_RATE units "mg O2 m-3" (rate is d-1); `coral_compete_id` reuses "Rate macroalgae overgrow coral"; `HAB_mum_scalar_id` has units "W m-2" and range "3 - 65" (copied from KI); FLAG_SALT_SENSITIVE is described as "Temperature sensitivty"; FLAG_PH_CONSTC as "Coefficient B"; `mBleach_id` as "Slow recovery period", "days", "10 - 200"; `bleach_recovery_rate_id` (d-1) has range "10 - 200". Functions: `createMortalityXML()`, `createPrimaryProduceXML()`, `createQ10XML()`, `createCoralXML()`.

**Code at issue** — `atBioltoXML.c` line 3460 (the line the review pointed to, found by its text)  
Search for: `Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, mBleach_id, "Slow recovery pe`

```c
 3458: 	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_periodB_id, "Slow recovery period (regain new zooxanthellae)", "days", XML_TYPE_FLOAT, "10 - 200");
 3459: 
 3460→ 	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, mBleach_id, "Slow recovery period (regain new zooxanthellae)", "days", XML_TYPE_FLOAT, "10 - 200");
 3461: 	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleaching_rate_id, "Bleaching rate", "d-1", XML_TYPE_FLOAT, "");
 3462: 	Create_Species_ParamXML(bm, fileName, fp, childGroupingNode, bleach_recovery_rate_id, "Bleaching recovery rate", "d-1", XML_TYPE_FLOAT, "10 - 200");
```

**Suggested fix (from review):** Correct each description/units/range.

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

## CNV1-13

**wrong XML type** · Conf H / Sev L  
**Found at (current code):** `atBioltoXML.c` 2112
**Lines at review (Oct 2026):** 2106-2110, 2172

**Function:** `createPhysicalLim_n_FLagsXML()`

**Why it's thought to be a bug**

`flag_refuge_model` is declared XML_TYPE_BOOLEAN but has a 3-value lookup (0/1/2, and 2 = rugosity_refuge_model is used at 2112). `K_max_num_DHW` is declared XML_TYPE_BOOLEAN with recommended value "14". The readers use no_checking/integer_check, so only metadata is wrong today, but any type-based validation/editor would reject valid values. Function: `createPhysicalLim_n_FLagsXML()`.

**Code at issue** — `atBioltoXML.c` line 2112 (the line the review pointed to, found by its text)  
Search for: `if(bm->flag_refuge_model == rugosity_refuge_model)`

```c
 2110: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Rugosity related");
 2111: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flag_refuge_model", &bm->flag_refuge_model);
 2112→ 	if(bm->flag_refuge_model == rugosity_refuge_model)
 2113: 		bm->flag_adv_habitat = 1;
 2114: 	else
```

**Suggested fix (from review):** Use XML_TYPE_LOOKUP / XML_TYPE_INTEGER.

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

## CNV1-14

**duplicate block** · Conf H / Sev L  
**Found at (current code):** `atBioltoXML.c` 3762
**Lines at review (Oct 2026):** 3760, 3764

**Function:** `createInvaderXML()`

**Why it's thought to be a bug**

`Util_XML_Parse_Create_Node(..., "InvadersEntering", ...)` is called twice, so two identical attribute nodes are emitted in the XML. Function: `createInvaderXML()`.

**Code at issue** — `atBioltoXML.c` line 3762  
Search for: `Util_XML_Parse_Create_Node(`

```c
 3760:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderMinDepth", "", "", XML_TYPE_FLOAT, "");
 3761:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderMaxDepth", "", "", XML_TYPE_FLOAT, "");
 3762→     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderStartDay", "", "", XML_TYPE_FLOAT, "");
 3763:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderEndDay", "", "", XML_TYPE_FLOAT, "");
 3764:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvadersEntering", "", "", XML_TYPE_INTEGER, "");
```

**Suggested fix (from review):** Delete the first call (3762).

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

## CNV1-15

**NULL deref on malformed input** · Conf L / Sev M  
**Found at (current code):** `atBioltoXML.c` 3961
**Lines at review (Oct 2026):** 3953-4018

**Function:** `Convert_Migration_To_XML()`

**Why it's thought to be a bug**

Each of the 17 `strtok(NULL, seps)` results is passed straight to `Util_XML_Create_Node` as a value. A data row with fewer columns than the header (only the header row is counted, 3928-3937) passes NULL. Also, `seps=","` keeps the trailing `\r\n` on the last field. Function: `Convert_Migration_To_XML()`.

**Code at issue** — `atBioltoXML.c` line 3961  
Search for: `strtok(NULL, seps)`

```c
 3959:             
 3960:             /* Starting stage */
 3961→             varStr = strtok(NULL, seps);
 3962:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "StartStage", "", "", varStr);
 3963:             
```

**Suggested fix (from review):** Check each token for NULL and quit with row info. Add `\r\n` to seps.

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

## CNV1-16

**uninitialised variable** · Conf L / Sev L  
**Found at (current code):** `atBioltoXML.c` 1345
**Lines at review (Oct 2026):** 1345-1379

**Function:** `MigrateReturnXMLFunction()`

**Why it's thought to be a bug**

`char param[30]` is left uninitialised if none of the `strstr` branches match, then used as a node name. The function currently has no callers (dead code), so it is latent. Function: `MigrateReturnXMLFunction()`.

**Code at issue** — `atBioltoXML.c` line 1345  
Search for: `char param[30]`

```c
 1343: 	int adultIndex = -1; // default value.
 1344: 	int speciesIndex;
 1345→ 	char param[30];
 1346: 	xmlNodePtr paramNode, speciesNode;
 1347: 
```

**Suggested fix (from review):** Add an `else quit(...)` branch, or remove the dead function.

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

## CNV1-17

**resource leak** · Conf L / Sev L  
**Found at (current code):** `atBioltoXML.c` 3799, `atBioltoXML.c` 3880, `atBioltoXML.c` 4029
**Lines at review (Oct 2026):** 3795-3798, 3910-3913

**Why it's thought to be a bug**

`if (doc == NULL) { ... return; }` returns without `fclose(fp)` / `fclose(inputFP)`. Functions: `Convert_Biol_To_XML()`, `Convert_Migration_To_XML()`.

**Code at issue** — `atBioltoXML.c` line 3799  
Search for: `if (doc == NULL) {`

```c
 3797: 	doc = xmlNewDoc((unsigned char *) "1.0");
 3798: 
 3799→ 	if (doc == NULL) {
 3800: 		fprintf(stderr, "Document not created successfully. \n");
 3801: 		return;
```

**Code at issue** — `atBioltoXML.c` line 3880  
Search for: `fclose(fp)`

```c
 3878: 	xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
 3879: 	xmlFreeDoc(doc);
 3880→ 	fclose(fp);
 3881: 
 3882: 	/* Shutdown libxml */
```

**Code at issue** — `atBioltoXML.c` line 4029  
Search for: `fclose(inputFP)`

```c
 4027:     xmlSaveFormatFileDestFolder(bm->destFolder, outputFileName, doc, 1);
 4028:     xmlFreeDoc(doc);
 4029→     fclose(inputFP);
 4030:     
 4031:     /* Shutdown libxml */
```

**Suggested fix (from review):** fclose before returning (or quit).

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

atBioltoXML.c `MigrateIOBoxXMLFunction()` (1499-1519): after truncating at "ad"/"juv", the migration index digit is only looked for at the end of the remaining species string. A name like `MigIOBox_CEPad2` (index after the age tag) would silently map to Migrate1. The prm files seen only use `MigIOBox_CEPad` / `MigIOBox_BML`, so I could not confirm the intended naming.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atBioltoXML.c `InitSuppDietXML()` builds `zeroString` but never uses it (nodes get ""). Probably meant to be the default value.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atBioltoXML.c `createPrimaryProduceXML()` (2646-2649) creates an empty `HAB_Location` subgroup but puts `HAB_box` under `groupingNode`. The reader (atBiologyXMLParamIO.c:1250-1251) also looks under the parent, so the two are consistent; only the empty subgroup is odd.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

atCreateXML.c `Create_Species_Spawn_ParamXML()` is called with `XML_TYPE_INTEGER` (atBioltoXML.c:3340) but writes array values and sets ArrayLength. The type metadata should probably be INTEGERARRAY.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

# ConvertAtlantis/atBioltoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **17 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 4 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV1-01](#cnv1-01) ⭐ | 1780-1786 | H/H | buffer overrun / wrong variable | `tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';`: `secondlastLetter` points at the la… |  | [ ] |
| [CNV1-02](#cnv1-02) | 1159-1177 | H/M | wrong variable / logic | When `bm->flag_macro_model` is on, the `else { cohort = main_biomass_id; }` branch overwrites the `cohort = ju… |  | [ ] |
| [CNV1-03](#cnv1-03) | 948-985 | H/L | stale variable / not reset in loop | `groupNode` is set to NULL once, before the guild loop. For a guild that is `isOverWinter` but not mobile (or … |  | [ ] |
| [CNV1-04](#cnv1-04) | 4080-4081 | M/M | format mismatch / wrong check | `quit("...(%d)... (value of %d)...", numGroups, convertedXMLFileName, bm->K_num_tot_sp)`: the 2nd `%d` gets a … |  | [ ] |
| [CNV1-05](#cnv1-05) | 4094-4130 | M/M | missing bounds check | `start_stage` and `migrationID` come straight from the CSV and are used unchecked as `MIGRATION[sp].X_Prm[star… |  | [ ] |
| [CNV1-06](#cnv1-06) | 636-680 | M/H | buffer overrun | `char sedimentValues[1000]; char finalValues[1000];` collect K_num_tot_sp tokens with `strcat`, and the input … |  | [ ] |
| [CNV1-07](#cnv1-07) | 744-757, 762-767 | L/M | OOB index / wrong error text | `cohortNode = ...cohortStrings[i]` (753) runs *before* the `i == -1` check at 755. `cohortStrings` has only 2 … |  | [ ] |
| [CNV1-08](#cnv1-08) | 94-107 | M/M | buffer overrun (edge) | `malloc(3*size)` followed by `strcpy(zeroString,"")`: with size==0 this is malloc(0) followed by a 1-byte writ… |  | [ ] |
| [CNV1-09](#cnv1-09) | 1115-1135 | M/M | hard-coded code length | `speciesStr = str + strlen(str) - 3;` assumes the predator code is exactly 3 characters (`p_BCFDB` gives prey … |  | [ ] |
| [CNV1-10](#cnv1-10) | 1267-1271 | M/M | hard-coded code length | `if(strlen(str) == 9) speciesStr = str + 6; else speciesStr = str + 5;` for `mS_FDxxx`/`mS_SBxxx`. A 4-charact… |  | [ ] |
| [CNV1-11](#cnv1-11) | 2426-2427, 2433-2435, 2439-2440 | H/L | copy-paste (metadata) | `max_move_temp_id` is described as "Min tolerated temperatures" and `min_move_temp_id` as "Maximum tolerated t… |  | [ ] |
| [CNV1-12](#cnv1-12) | 2465, 2636, 2644, 3104, 3129, 3458, 3460 | H/L | copy-paste (metadata) | Copy-pasted descriptions or units: FLAG_LYSIS_RATE units "mg O2 m-3" (rate is d-1); `coral_compete_id` reuses … |  | [ ] |
| [CNV1-13](#cnv1-13) | 2106-2110, 2172 | H/L | wrong XML type | `flag_refuge_model` is declared XML_TYPE_BOOLEAN but has a 3-value lookup (0/1/2, and 2 = rugosity_refuge_mode… |  | [ ] |
| [CNV1-14](#cnv1-14) | 3762, 3766 | H/L | duplicate block | `Util_XML_Parse_Create_Node(..., "InvadersEntering", ...)` is called twice, so two identical attribute nodes a… |  | [ ] |
| [CNV1-15](#cnv1-15) | 3955-4020 | L/M | NULL deref on malformed input | Each of the 17 `strtok(NULL, seps)` results is passed straight to `Util_XML_Create_Node` as a value. A data ro… |  | [ ] |
| [CNV1-16](#cnv1-16) | 1345-1379 | L/L | uninitialised variable | `char param[30]` is left uninitialised if none of the `strstr` branches match, then used as a node name. The f… |  | [ ] |
| [CNV1-17](#cnv1-17) | 3797-3800, 3912-3915 | L/L | resource leak | `if (doc == NULL) { ... return; }` returns without `fclose(fp)` / `fclose(inputFP)`. Functions: `Convert_Biol_… |  | [ ] |

---

## CNV1-01

**buffer overrun / wrong variable** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 1780-1786  
**Lines at review (Oct 2026):** 1780-1786

**Function:** `PopRatioStockXMLFunction()`

**Why it's thought to be a bug**

`tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';`: `secondlastLetter` points at the last 2 chars, so `strlen(secondlastLetter)` is 2. `2 - 13 - 2` is computed as size_t and wraps, so the write lands about 13 bytes *before* `tempStr` on the stack, and `tempStr` itself is never truncated. The single-digit branch at 1785 uses `strlen(tempStr)` correctly. This path runs whenever a stock index has 2 digits, i.e. `FBP1popratioStock10` (regex at 3278 allows `[1-9][0-9]{0,2}`) with K_num_stocks_per_sp >= 10. A 3-digit stock index is not handled either. Function: `PopRatioStockXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 1778–1788)

```c
 1778: 	secondlastLetter = step1Str + strlen(step1Str) - 2;
 1779: 
 1780: 	if(isdigit(secondlastLetter[0])) {  // Is the first digit a number if yes then a two digit case
 1781: 		stockIndex = atoi(secondlastLetter);
 1782: 		tempStr[strlen(secondlastLetter) - strlen("popratioStock") - 2] = '\0';
 1783: 	} else {  // No make it a single string case
 1784: 		stockIndex = atoi(lastLetter);
 1785: 		tempStr[strlen(tempStr) - strlen("popratioStock") - 1] = '\0';
 1786: 	}
 1787: 
 1788: 	// Get the species string section of the string.
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
**Lines (current code):** 1159-1177  
**Lines at review (Oct 2026):** 1159-1177

**Function:** `InvertebrateClearanceRateXMLFunction()`

**Why it's thought to be a bug**

When `bm->flag_macro_model` is on, the `else { cohort = main_biomass_id; }` branch overwrites the `cohort = juv_id` / `adult_id` value set at 1149/1161 for every non-Epi/Root parameter, including non-seagrass groups. For AGE_STRUCTURED_BIOMASS inverts (e.g. CEP/PWN), both `C_CEP_T15` and `C_jCEP_T15` (and the `mum_` versions, via the call at 2977) then go to index 0 (main_biomass_id = 0) in `Util_XML_Replace_Node_Value(...,2,cohort)`. Whichever comes last wins, and the adult slot (index 1) stays at the "0.0" default (atUtilXML.c:1189-1195). Result: adult clearance/growth = 0 for those groups in macro-model runs. Function: `InvertebrateClearanceRateXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 1157–1179)

```c
 1157: 		strPtr[strlen(strPtr) - 4] = '\0';
 1158: 
 1159: 	if (strstr(speciesStr, "j") != NULL) {
 1160: 		strPtr++;
 1161: 		cohort = juv_id;
 1162: 	}
 1163: 
 1164: 	if(bm->flag_macro_model){
 1165: 		/* Allow for seagrass cohorts as well */
 1166: 		if (strstr(speciesStr, "Epi_") != NULL) {
 1167: 			strPtr+= 4;
 1168: 			cohort = epiphyte_biomass_id;
 1169: 		}
 1170: 		else if (strstr(speciesStr, "Root_") != NULL) {
 1171: 			strPtr+= 5;
 1172: 			cohort = below_ground_biomass_id;
 1173: 		}
 1174: 		else{
 1175: 			cohort = main_biomass_id;
 1176: 		}
 1177: 	}
 1178: 
 1179: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);
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
**Lines (current code):** 948-985  
**Lines at review (Oct 2026):** 948-985

**Function:** `InitVertDistributionXML()`

**Why it's thought to be a bug**

`groupNode` is set to NULL once, before the guild loop. For a guild that is `isOverWinter` but not mobile (or is EPIFAUNA), `if(groupNode == NULL)` is false because groupNode still holds the *previous* guild's node. The "overwinter" temporal node (plus juv/adult children) is therefore attached to the wrong group, and that group can end up with duplicate "overwinter" nodes. Function: `InitVertDistributionXML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 946–985 (truncated))

```c
  946: void InitVertDistributionXML(MSEBoxModel *bm, xmlNodePtr parent) {
  947: 
  948: 	xmlNodePtr groupNode = NULL;
  949: 	xmlNodePtr tempNode;
  950: 	int guild;
  951: 
  952: 	/* Create a node for each functional group*/
  953: 	for (guild = 0; guild < bm->K_num_tot_sp; guild++) {
  954: 
  955: 		if (FunctGroupArray[guild].isMobile == TRUE && FunctGroupArray[guild].habitatType != EPIFAUNA){
  956: 
  957: 			groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
  958: 
  959: 			tempNode = Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, groupNode, "day", "", "", "");
  960: 			/* If this is a vertebrate group add the adult and juv nodes*/
  961: 			if (FunctGroupArray[guild].isVertebrate) {
  962: 				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", "", "", "");
  963: 				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", "", "", "");
  964: 			}
  965: 
  966: 			tempNode = Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, groupNode, "night", "", "", "");
  967: 			/* If this is a vertebrate group add the adult and juv nodes*/
  968: 			if (FunctGroupArray[guild].isVertebrate) {
  969: 				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", "", "", "");
  970: 				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", "", "", "");
  971: 			}
  972: 		}
  973: 		/* If overwinter add that in too */
  974: 		if (FunctGroupArray[guild].isOverWinter == TRUE ){
  975: 
  976: 			if(groupNode == NULL){
  977: 				groupNode = Util_XML_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[guild].groupCode, "", "", "");
  978: 			}
  979: 			tempNode = Util_XML_Create_Node(ATLANTIS_TEMPORAL_ATTRIBUTE, groupNode, "overwinter", "", "", "");
  980: 			/* If this is a vertebrate group add the adult and juv nodes*/
  981: 			if (FunctGroupArray[guild].isVertebrate) {
  982: 				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "juv", "", "", "");
  983: 				Util_XML_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, tempNode, "adult", "", "", "");
  984: 			}
  985: 		}
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
**Lines (current code):** 4080-4081  
**Lines at review (Oct 2026):** 4078-4079

**Function:** `Util_Read_Migration_XML()`

**Why it's thought to be a bug**

`quit("...(%d)... (value of %d)...", numGroups, convertedXMLFileName, bm->K_num_tot_sp)`: the 2nd `%d` gets a `char*` (UB on the error path). Separately, the check itself (`numGroups > K_num_tot_sp`) is wrong: each CSV row is one (group, stage, MigID) migration, so a valid file can have more rows than species (e.g. juv + adult rows for many groups), and this would quit spuriously. Function: `Util_Read_Migration_XML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 4078–4083)

```c
 4078:     if (groupList && groupList->nodesetval->nodeNr > 0) {
 4079:         
 4080:         /* Check the number of groups match the values in the run input file*/
 4081:         numGroups = groupList->nodesetval->nodeNr;
 4082:         if (numGroups > bm->K_num_tot_sp) {
 4083:             quit("Util_Read_Migration_XML: Number of groups specified (%d) in your migration definition file is greater than the number of species - K_num_tot_sp (value of %d) in your run file\n", numGroups, convertedXMLFileName, bm->K_num_tot_sp);
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
**Lines (current code):** 4094-4130  
**Lines at review (Oct 2026):** 4092-4128

**Function:** `Util_Read_Migration_XML()`

**Why it's thought to be a bug**

`start_stage` and `migrationID` come straight from the CSV and are used unchecked as `MIGRATION[sp].X_Prm[start_stage][migrationID]`. Those arrays are `[numStages][num_migrate+1]` (atBiologyXMLParamIO.c:4376, 4402-4421?) and Stagger_Prm is `[numStages][num_migs][2]`. A stage >= numStages, a negative value, or a MigID > num_migrate gives a silent heap write out of bounds. Function: `Util_Read_Migration_XML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 4092–4131 (truncated))

```c
 4092: 
 4093:             if (verbose > 0)
 4094:                 printf("Reading in migration definition of %s\n", FunctGroupArray[speciesIndex].groupCode);
 4095: 
 4096:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "StartStage", &start_stage);
 4097:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MigID", &migrationID);
 4098:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "StartTofY", &start_tofy);
 4099:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "EndTofY", &end_tofy);
 4100:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "Leave_Period", &mig_period);
 4101:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "Return_Period", &mig_period2);
 4102:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "IsAnnualMigration", &isAnnual);
 4103:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MinTimeAway", &minaway);
 4104:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MaxTimeAway", &maxaway);
 4105:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, integer_check, "MigrateStockReturn", &ret_stock);
 4106:             Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MigPropSizeInc", &grow_rate);
 4107:             Util_XML_Get_Value_Double(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MigPropSurvive", &survive_rate);
 4108: 
 4109:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "PartialExit", &flagPartialExit);
 4110:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MinAgeLeave", &MinAgePartial);
 4111:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "MaxAgeLeave", &MaxAgePartial);
 4112:             Util_XML_Get_Value_Integer(convertedXMLFileName, ATLANTIS_ATTRIBUTE, 0, TRUE, groupNode, no_checking, "ReprodAllowed", &ReprodAllowedPartial);
 4113:             
 4114:             // Set parameters
 4115:             MIGRATION[speciesIndex].StartDay_Prm[start_stage][migrationID] = start_tofy;
 4116:             MIGRATION[speciesIndex].EndDay_Prm[start_stage][migrationID] = end_tofy;
 4117:             MIGRATION[speciesIndex].Leave_Period_Prm[start_stage][migrationID] = mig_period;
 4118:             MIGRATION[speciesIndex].Return_Period_Prm[start_stage][migrationID] = mig_period2;
 4119:             MIGRATION[speciesIndex].MinYearsAway_Prm[start_stage][migrationID] = minaway;
 4120:             MIGRATION[speciesIndex].MaxYearsAway_Prm[start_stage][migrationID] = maxaway;
 4121:             MIGRATION[speciesIndex].IsAnnualMigration_Prm[start_stage][migrationID] = isAnnual;
 4122:             MIGRATION[speciesIndex].survival_Prm[start_stage][migrationID] = survive_rate;
 4123:             MIGRATION[speciesIndex].growth_Prm[start_stage][migrationID] = grow_rate;
 4124:             MIGRATION[speciesIndex].returnstock_Prm[start_stage][migrationID] = ret_stock;
 4125:             MIGRATION[speciesIndex].IsPartialMigration_Prm[start_stage][migrationID] = flagPartialExit;
 4126:             MIGRATION[speciesIndex].PartialMigration_MinPrm[start_stage][migrationID] = MinAgePartial;
 4127:             MIGRATION[speciesIndex].PartialMigration_MaxPrm[start_stage][migrationID] = MaxAgePartial;
 4128:             MIGRATION[speciesIndex].ReprodAllowedPrm[start_stage][migrationID] = ReprodAllowedPartial;
 4129:             
 4130:             MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] = MIGRATION[speciesIndex].MaxYearsAway_Prm[start_stage][migrationID] - MIGRATION[speciesIndex].MinYearsAway_Prm[start_stage][migrationID];
 4131:             if( MIGRATION[speciesIndex].Stagger_Prm[start_stage][migrationID][stagger_years_id] > 0) {
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
**Lines (current code):** 636-680  
**Lines at review (Oct 2026):** 636-680

**Function:** `spInvertEatXMLFunction()`

**Why it's thought to be a bug**

`char sedimentValues[1000]; char finalValues[1000];` collect K_num_tot_sp tokens with `strcat`, and the input line can be up to STRLEN (2000) chars (Parse_File buf). Example: 90+ groups with values like `1.234e-05` (about 10 chars each) overflows the 1000-byte stack buffer. `spVERTEatXMLFunction` uses STRLEN buffers for the same data. `speciesStr[20]` filled by unbounded `sscanf("%s")` (643) is also latent. Function: `spInvertEatXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 634–673 (truncated))

```c
  634: 	char speciesStr[20];
  635: 	int cohort = -1;
  636: 	char sedimentValues[1000];
  637: 	char finalValues[1000];
  638: 	int sp;
  639: 	char *varStr;
  640: 	int speciesIndex;
  641: 	xmlNodePtr speciesNode, attributeGroupNode, attributeNode;
  642: 
  643:     sscanf(str, "pPREY%s", speciesStr);
  644: 	strcpy(sedimentValues, "");
  645: 	strcpy(finalValues, "");
  646: 
  647: 	/* See if there is a cohort value on the end */
  648: 	if (isdigit(speciesStr[strlen(speciesStr) - 1])) {
  649: 		cohort = atoi(speciesStr + strlen(speciesStr) - 1) - 1;
  650: 		speciesStr[strlen(speciesStr) - 1] = '\0';
  651: 	}
  652: 
  653: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
  654: 
  655: 	if(cohort == juv_id && FunctGroupArray[speciesIndex].numCohorts < FunctGroupArray[speciesIndex].numStages){
  656: 		quit("spInvertEatXMLFunction - %s - Found a value for a juv cohort but %s has less cohorts than stages\n", str, FunctGroupArray[speciesIndex].groupCode);
  657: 	}
  658: 
  659: 	if(cohort ==-1 && FunctGroupArray[speciesIndex].numCohorts > 1){
  660: 		quit("spInvertEatXMLFunction - %s - Age structured invertebrate groups must have cohort structured diet matrix values. You need a value per cohort.\n", FunctGroupArray[speciesIndex].groupCode);
  661: 	}
  662: 
  663: 	/* Split the last 5 values off the end - ignore the last two values.*/
  664: 	for (sp = 0; sp < (bm->K_num_tot_sp + bm->K_num_detritus); sp++) {
  665: 
  666: 		if (sp == 0)
  667: 			varStr = strtok(valueStr, seps);
  668: 		else
  669: 			varStr = strtok(NULL, seps);
  670: 
  671: 		if(varStr == NULL){
  672: 			quit("spInvertEatXMLFunction: Found %d values instead of the required %d for parameter %s\n", sp, (bm->K_num_tot_sp + bm->K_num_detritus), str);
  673: 		}
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
**Lines (current code):** 744-757, 762-767  
**Lines at review (Oct 2026):** 744-757, 762-767

**Function:** `spVERTEatXMLFunction()`

**Why it's thought to be a bug**

`cohortNode = ...cohortStrings[i]` (753) runs *before* the `i == -1` check at 755. `cohortStrings` has only 2 entries (atUtil.c:175), and neither `i` (last char) nor `j` (leading digit) is range-checked. The error message at 756 names `spInvertEatXMLFunction` (copy-paste). `finalValues[STRLEN]` with `%.10e` (17 chars/value) also overflows for more than about 117 groups. Function: `spVERTEatXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 742–769)

```c
  742: 	strcpy(finalValues, "");
  743: 
  744:     sscanf(str, "pPREY%d%s", &j, speciesStr);
  745: 
  746: 	i = atoi(speciesStr + strlen(speciesStr) - 1);
  747: 	*(speciesStr + strlen(speciesStr) - 1) = '\0';
  748: 	j--;
  749: 	i--;
  750: 
  751: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
  752: 	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, FunctGroupArray[speciesIndex].groupCode);
  753: 	cohortNode = Util_XML_Get_Or_Create_Node(ATLANTIS_AGE_CLASS_ATTRIBUTE, speciesNode, cohortStrings[i]);
  754: 
  755: 	if(i ==-1 && FunctGroupArray[speciesIndex].numCohorts > 1){
  756: 		quit("spInvertEatXMLFunction - %s - Age structured invertebrate groups must have cohort structured diet matrix values\n", FunctGroupArray[speciesIndex].groupCode);
  757: 	}
  758: 	/* parse the data into an array */
  759: 	p = Parse_Into_Array(fileName, str, valueStr, size, proportion_check);
  760: 
  761: 	/* Now grab the sediment values */
  762: 	for (sp = 0; sp < (bm->K_num_tot_sp + bm->K_num_detritus); sp++) {
  763: 
  764: 		if (sp < bm->K_num_tot_sp) {
  765: 			snprintf(tempStr, sizeof(tempStr), "%s%.10e", sp>0?" ":"", p[sp]);
  766: 			strcat(finalValues, tempStr);
  767: 
  768: 			//snprintf(finalValues, sizeof(finalValue), "%s%s%.10e", finalValues, sp>0?" ":"", p[sp]);
  769: 		} else {
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
**Lines (current code):** 94-107  
**Lines at review (Oct 2026):** 94-107

**Function:** `CreateZeroString()`

**Why it's thought to be a bug**

`malloc(3*size)` followed by `strcpy(zeroString,"")`: with size==0 this is malloc(0) followed by a 1-byte write. Possible callers with size 0: `InitSuppDietXML`/`InitHabitatXML` (K_num_cover_types), `InitCatchPropGrazersXML` (K_num_fisheries). Also no NULL check on malloc. Function: `CreateZeroString()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 92–109)

```c
   92: 		char *units, int type, char *recommendedValue, void(*fp)(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr));
   93: 
   94: static char *CreateZeroString(int size){
   95: 	int i;
   96: 	char *zeroString = (char *) malloc(sizeof(char) * (unsigned int long)3 * (unsigned int long)size);
   97: 
   98: 
   99: 	strcpy(zeroString, "");
  100: 
  101: 	for(i = 0; i < size; i++){
  102: 		strcat(zeroString, " 0");
  103: 	}
  104: 
  105: 	trim(zeroString);
  106: 	return zeroString;
  107: 
  108: }
  109: /**
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
**Lines (current code):** 1115-1135  
**Lines at review (Oct 2026):** 1115-1135

**Function:** `AgeDietXMLFunction()`

**Why it's thought to be a bug**

`speciesStr = str + strlen(str) - 3;` assumes the predator code is exactly 3 characters (`p_BCFDB` gives prey BC, pred FDB). Atlantis codes of 2 or 4 characters give the wrong prey/pred split, and the node is created under a bogus group name with no validation (no FG lookup). Function: `AgeDietXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 1113–1137)

```c
 1113:  </AgeDietAvail>
 1114:  */
 1115: void AgeDietXMLFunction(MSEBoxModel *bm, char *fileName, xmlNodePtr parent, char *str, char *valueStr) {
 1116: 
 1117: 	char tempStr[50];
 1118: 	char *speciesStr;
 1119: 	char *strPtr;
 1120: 	xmlNodePtr speciesNode;
 1121: 
 1122: 	// copy the key to the key string.
 1123: 	strcpy(tempStr, str);
 1124: 	speciesStr = str + strlen(str) - 3;
 1125: 	strcpy(tempStr, speciesStr);
 1126: 
 1127: 	*speciesStr = '\0';
 1128: 	strPtr = strchr(str, '_');
 1129: 	strPtr++;
 1130: 
 1131: 	/* Find the parent node in the tree*/
 1132: 	speciesNode = Util_XML_Get_Or_Create_Node(ATLANTIS_GROUP_ATTRIBUTE, parent, tempStr);
 1133: 
 1134: 	Util_XML_Set_Node_Value(ATLANTIS_GROUP_ATTRIBUTE, speciesNode, strPtr, valueStr);
 1135: }
 1136: /**
 1137:  Want the XML for these parameter to be:
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
**Lines (current code):** 1267-1271  
**Lines at review (Oct 2026):** 1267-1271

**Function:** `ImplicitMortalityRateXMLFunction()`

**Why it's thought to be a bug**

`if(strlen(str) == 9) speciesStr = str + 6; else speciesStr = str + 5;` for `mS_FDxxx`/`mS_SBxxx`. A 4-character group code (`mS_FDABCD`, length 9) is cut to `BCD`: wrong group or quit. The prm files seen use `mS_FDFBP` (length 8), so the +6 branch looks like it is only for an optional separator. Function: `ImplicitMortalityRateXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 1265–1273)

```c
 1265: 	int speciesIndex;
 1266: 
 1267: 	if(strlen(str) == 9){
 1268: 		speciesStr = str + 6;
 1269: 	}else{
 1270: 		speciesStr = str + 5;
 1271: 	}
 1272: 
 1273: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, speciesStr, fileName, str, TRUE);
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
**Lines (current code):** 2426-2427, 2433-2435, 2439-2440  
**Lines at review (Oct 2026):** 2426-2427, 2433-2435, 2439-2440

**Function:** `createMovementXML()`

**Why it's thought to be a bug**

`max_move_temp_id` is described as "Min tolerated temperatures" and `min_move_temp_id` as "Maximum tolerated temperatures". The salinity pair is swapped the same way. `min_O2_id` is described as "Maximum tolerated O2". `K_salt_const_id` and `K_o2_const_id` have units "deg C". Function: `createMovementXML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 2424–2442)

```c
 2424:     Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagtempdepend_move", &bm->flagtempdepend_move);
 2425:     Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagtempdepend_reprod", &bm->flagtempdepend_reprod);
 2426: 
 2427:     if (bm->flagtempdepend_move) {
 2428:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_move_temp_id, "Min tolerated temperatures", "deg C", XML_TYPE_FLOAT, "");
 2429:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_move_temp_id, "Maximum tolerated temperatures", "deg C", XML_TYPE_FLOAT, "");
 2430:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_temp_const_id, "Constant in bilogistic tolerated temperature relationship", "deg C", XML_TYPE_FLOAT, "");
 2431:     }
 2432: 
 2433: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagsaltdepend", &bm->flagsaltdepend);
 2434: 	if (bm->flagsaltdepend) {
 2435: 		Create_Species_ParamXML(bm, fileName, fp, groupingNode, max_move_salt_id, "Min tolerated salinity", "", XML_TYPE_FLOAT, "");
 2436: 		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_move_salt_id, "Maximum tolerated salinity", "", XML_TYPE_FLOAT, "");
 2437:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_salt_const_id, "Constant in bilogistic tolerated salinity relationship", "deg C", XML_TYPE_FLOAT, "");
 2438: 	}
 2439: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flagO2depend", &flagdepend);
 2440: 	if (flagdepend) {
 2441: 		Create_Species_ParamXML(bm, fileName, fp, groupingNode, min_O2_id, "Maximum tolerated O2", "", XML_TYPE_FLOAT, "");
 2442:         Create_Species_ParamXML(bm, fileName, fp, groupingNode, K_o2_const_id, "Constant in bilogistic tolerated O2 relationship", "deg C", XML_TYPE_FLOAT, "");
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
**Lines (current code):** 2465, 2636, 2644, 3104, 3129, 3458, 3460  
**Lines at review (Oct 2026):** 2465, 2636, 2644, 3104, 3129, 3458, 3460

**Why it's thought to be a bug**

Copy-pasted descriptions or units: FLAG_LYSIS_RATE units "mg O2 m-3" (rate is d-1); `coral_compete_id` reuses "Rate macroalgae overgrow coral"; `HAB_mum_scalar_id` has units "W m-2" and range "3 - 65" (copied from KI); FLAG_SALT_SENSITIVE is described as "Temperature sensitivty"; FLAG_PH_CONSTC as "Coefficient B"; `mBleach_id` as "Slow recovery period", "days", "10 - 200"; `bleach_recovery_rate_id` (d-1) has range "10 - 200". Functions: `createMortalityXML()`, `createPrimaryProduceXML()`, `createQ10XML()`, `createCoralXML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 2463–2467) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2463: 	xmlNodePtr newnode;
 2464: 
 2465: 	childGroupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Mortality", "Mortality Parameters", "", "");
 2466: 
 2467: 	InitPrimaryProducerSpeciesZeroValuesXML(bm, childGroupingNode, "FLAG_LYSIS_RATE", "Mircoalgae groups lysis rate", "mg O2 m-3");
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
**Lines (current code):** 2106-2110, 2172  
**Lines at review (Oct 2026):** 2106-2110, 2172

**Function:** `createPhysicalLim_n_FLagsXML()`

**Why it's thought to be a bug**

`flag_refuge_model` is declared XML_TYPE_BOOLEAN but has a 3-value lookup (0/1/2, and 2 = rugosity_refuge_model is used at 2112). `K_max_num_DHW` is declared XML_TYPE_BOOLEAN with recommended value "14". The readers use no_checking/integer_check, so only metadata is wrong today, but any type-based validation/editor would reject valid values. Function: `createPhysicalLim_n_FLagsXML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 2104–2112)

```c
 2104: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_rel_cover", "Flag indicates whether using relative habitats (sum to 1) or cumulative (sum > 1, not as sensitive if lots of habitats).", "", XML_TYPE_BOOLEAN,"0");
 2105: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_report_water_detritus", "Flag indicates whether to report on detritus in water column as well as sediment.", "", XML_TYPE_BOOLEAN,"0");
 2106: 	node = Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_refuge_model", "Flag indicating which refugia model to use", "", XML_TYPE_BOOLEAN,"0");
 2107: 	lookupNode = Util_XML_Create_Lookup_Node(node);
 2108: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "0", "None");
 2109: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "1", "Original");
 2110: 	Util_XML_Create_Lookup_Value_Node(lookupNode, "2", "Rugosity related");
 2111: 	Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, no_checking, "flag_refuge_model", &bm->flag_refuge_model);
 2112: 	if(bm->flag_refuge_model == rugosity_refuge_model)
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
**Lines (current code):** 3762, 3766  
**Lines at review (Oct 2026):** 3760, 3764

**Function:** `createInvaderXML()`

**Why it's thought to be a bug**

`Util_XML_Parse_Create_Node(..., "InvadersEntering", ...)` is called twice, so two identical attribute nodes are emitted in the XML. Function: `createInvaderXML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 3760–3768)

```c
 3760:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderMinDepth", "", "", XML_TYPE_FLOAT, "");
 3761:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderMaxDepth", "", "", XML_TYPE_FLOAT, "");
 3762:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderStartDay", "", "", XML_TYPE_FLOAT, "");
 3763:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderEndDay", "", "", XML_TYPE_FLOAT, "");
 3764:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvadersEntering", "", "", XML_TYPE_INTEGER, "");
 3765:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderScalar", "", "", XML_TYPE_FLOAT, "");
 3766:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvaderSpeed", "", "", XML_TYPE_FLOAT, "");
 3767: 
 3768:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "InvadersEntering", "", "", XML_TYPE_INTEGER, "");
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
**Lines (current code):** 3955-4020  
**Lines at review (Oct 2026):** 3953-4018

**Function:** `Convert_Migration_To_XML()`

**Why it's thought to be a bug**

Each of the 17 `strtok(NULL, seps)` results is passed straight to `Util_XML_Create_Node` as a value. A data row with fewer columns than the header (only the header row is counted, 3928-3937) passes NULL. Also, `seps=","` keeps the trailing `\r\n` on the last field. Function: `Convert_Migration_To_XML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 3953–3992 (truncated))

```c
 3953:             /* Create the node for the group */
 3954:             groupNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "MigrationGroup", "The defintion of a migration", "", "");
 3955:             
 3956:             /* Group Code value. */
 3957:             varStr = strtok(buf, seps);
 3958:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "GroupCode", "", "", varStr);
 3959:             
 3960:             /* Starting stage */
 3961:             varStr = strtok(NULL, seps);
 3962:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "StartStage", "", "", varStr);
 3963:             
 3964:             /* MigID - in case multiple migrations per calendar year */
 3965:             varStr = strtok(NULL, seps);
 3966:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MigID", "", "", varStr);
 3967:             
 3968:             /* Start TofY */
 3969:             varStr = strtok(NULL, seps);
 3970:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "StartTofY", "", "", varStr);
 3971:             
 3972:             /* End TofY */
 3973:             varStr = strtok(NULL, seps);
 3974:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "EndTofY", "", "", varStr);
 3975:             
 3976:             /* Leave_Period */
 3977:             varStr = strtok(NULL, seps);
 3978:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "Leave_Period", "", "", varStr);
 3979:             
 3980:             /* Return_Period */
 3981:             varStr = strtok(NULL, seps);
 3982:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "Return_Period", "", "", varStr);
 3983:             
 3984:             /* IsAnnualMigration */
 3985:             varStr = strtok(NULL, seps);
 3986:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "IsAnnualMigration", "", "", varStr);
 3987:             
 3988:             /* Min time away */
 3989:             varStr = strtok(NULL, seps);
 3990:             Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "MinTimeAway", "", "", varStr);
 3991:             
 3992:             /* Max time away */
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
**Lines (current code):** 1345-1379  
**Lines at review (Oct 2026):** 1345-1379

**Function:** `MigrateReturnXMLFunction()`

**Why it's thought to be a bug**

`char param[30]` is left uninitialised if none of the `strstr` branches match, then used as a node name. The function currently has no callers (dead code), so it is latent. Function: `MigrateReturnXMLFunction()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 1343–1381)

```c
 1343: 	int adultIndex = -1; // default value.
 1344: 	int speciesIndex;
 1345: 	char param[30];
 1346: 	xmlNodePtr paramNode, speciesNode;
 1347: 
 1348: 	strcpy(speciesStr, str);
 1349: 	strPtr = strchr(speciesStr, '_');
 1350: 	*strPtr = '\0';
 1351: 	// Point it back to the front.
 1352: 	strPtr = speciesStr;
 1353: 
 1354: 	// Check to see if there is a j present.
 1355: 	if (*strPtr == 'j') {
 1356: 		adultIndex = juv_id;
 1357: 		strPtr++;
 1358: 	} else {
 1359: 		adultIndex = adult_id;
 1360: 	}
 1361: 	speciesIndex = Util_Get_FG_Index_From_Token(bm, strPtr, fileName, str, TRUE);
 1362: 
 1363: 	if (strstr(str, "Migrate_Time") != NULL) {
 1364: 		snprintf(param, sizeof(param), "MigrateTime");
 1365: 	} else if (strstr(str, "Migrate_Return") != NULL) {
 1366: 		snprintf(param, sizeof(param), "MigrateReturn");
 1367: 	} else if (strstr(str, "Period") != NULL) {
 1368: 		snprintf(param, sizeof(param), "MigratePeriod");
 1369:     } else if (strstr(str, "YearsAway") != NULL) {
 1370:         snprintf(param, sizeof(param), "MigrateYearsAway");
 1371:     } else if (strstr(str, "ReturnStock") != NULL) {
 1372: 		snprintf(param, sizeof(param), "MigrateStockReturn");
 1373: 	} else if (strstr(str, "FSMG") != NULL) {
 1374: 		snprintf(param, sizeof(param), "MigrateProportionIncrease");
 1375: 	} else if (strstr(str, "FSM") != NULL) {
 1376: 		snprintf(param, sizeof(param), "MigrateSurvivorshipProportion");
 1377: 	}
 1378: 
 1379: 	paramNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent->parent, param);
 1380: 
 1381: 	/* If this is a biomass group just create a species attribute node - else create a species node and then add a child age class
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
**Lines (current code):** 3797-3800, 3912-3915  
**Lines at review (Oct 2026):** 3795-3798, 3910-3913

**Why it's thought to be a bug**

`if (doc == NULL) { ... return; }` returns without `fclose(fp)` / `fclose(inputFP)`. Functions: `Convert_Biol_To_XML()`, `Convert_Migration_To_XML()`.

**Current code** (`ConvertAtlantis/atBioltoXML.c` lines 3795–3802) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 3795: 	}
 3796: 
 3797: 	doc = xmlNewDoc((unsigned char *) "1.0");
 3798: 
 3799: 	if (doc == NULL) {
 3800: 		fprintf(stderr, "Document not created successfully. \n");
 3801: 		return;
 3802: 	}
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

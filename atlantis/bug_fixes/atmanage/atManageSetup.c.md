# atmanage/atManageSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 4 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGA-24](#mnga-24) ⭐ | 1083 | H/H | wrong variable / OOB | Inside `for (i = 0; i < K_num_fisheries; i++)` the test uses `bm->TACamt[sp][nf][now_id]`. `nf` is stale from … |  | [ ] |
| [MNGA-25](#mnga-25) 🔀 | 986 | M/M | OOB | `for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) BiTAC_sp[bim][nreg]...`. BiTAC_sp has only `totalreg_id+1… |  | [ ] |
| [MNGA-26](#mnga-26) | 1007, 471 | H/L | format mismatch | `warn("... id number %d\n", ..., FunctGroupArray[sp].groupCode)` passes a `char*` to `%d`. Function: `Calculat… |  | [ ] |
| [MNGA-27](#mnga-27) | 1477 | H/M | logic (every class incremented) | `for (nimp...) if (flagimposecatch > 0) impose_classes[nimp] += 1;` increments every class equally, so the max… |  | [ ] |
| [MNGA-28](#mnga-28) | 1552 | H/M | copy-paste | The "top" skipper CDF uses `cpue_cdf_top_r_id` with `cpue_cdf_poor_p_id`. `cpue_cdf_top_p_id` exists (atlantis… |  | [ ] |
| [MNGA-29](#mnga-29) | 483, 465, 287 | M/M | init only in debug | `speciesParams[TACorig_id] = totTAC` is set only inside `if (bm->debug == debug_assess)`. Otherwise it is set … |  | [ ] |
| [MNGA-30](#mnga-30) | 1068 | M/L | wrong condition | `if (!i) FISHERYprms[i][flagconflict_id] = 0;` resets the flag for fleet 0 only, so the other fleets keep what… |  | [ ] |
| [MNGA-31](#mnga-31) | 1260, 329 | H/L | memory leak | `Manage_Free()` never frees `bm->MPA`, `bm->TAC_over` or `bm->sp_basket`. In the fisheries-off path (Manage_In… |  | [ ] |
| [MNGA-32](#mnga-32) | 352, 348, 287 | M/L | NULL deref | `*(strstr(convertedXMLFileName, ".nc")) = '\0';` crashes if the output name has no ".nc". `strcpy(fisheriesfil… |  | [ ] |

---

## MNGA-24

**wrong variable / OOB** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atManageSetup.c` 1083
**Lines at review (Oct 2026):** 1083-1085

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

Inside `for (i = 0; i < K_num_fisheries; i++)` the test uses `bm->TACamt[sp][nf][now_id]`. `nf` is stale from the earlier loop (== K_num_fisheries, or uninitialised if no impacted species), so it reads `TACamt[sp][K_num_fisheries]`, past the fleet dimension (TACamt `[sp][nfleets][entries]`, line 1196). That is a garbage pointer dereference and a wrong filter. Function: `Calculate_BiTAC()`.

**Code at issue** — `atManageSetup.c` line 1083  
Search for: `for (i = 0; i < bm->K_num_fisheries; i++)`

```c
 1081: 				for (bim = 0; bim < 6; bim++) {
 1082: 					sumBiTAC = 0;
 1083→ 					for (i = 0; i < bm->K_num_fisheries; i++) {
 1084:                         if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
 1085:                             sumBiTAC += bm->BiTAC_sp[bim][nreg][sp][now_id] * bm->TACamt[sp][i][now_id];
```

**Suggested fix (from review):** Use `bm->TACamt[sp][i][now_id]`.

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

## MNGA-25

**OOB** · Conf M / Sev M  
**Found at (current code):** `atManageSetup.c` 986
**Lines at review (Oct 2026):** 1080 (alloc 1195)

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

`for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) BiTAC_sp[bim][nreg]...`. BiTAC_sp has only `totalreg_id+1 = 3` region rows, but K_num_active_reg can be any number of regions (atbiolsetup.c:583). With more than 3 regions this writes out of bounds, and with exactly 3 it overwrites the total row. The earlier loop clamps to `Kreg` (max 2). Function: `Calculate_BiTAC()`.

> 🔀 **Changed by the merge of main (Nov 2026):** BiTAC_sp now zero-initialised. Check whether this finding is now fixed before acting.

**Code at issue** — `atManageSetup.c` line 986 (located by name only; check this is the right place)  
Search for: `bm->RegionalData[sp][nreg][reg_tac_id] = bm->BiTAC_sp[bim][nreg][sp][now_id] / (bm->BiTAC_`

```c
  984: 					if (bm->FISHERYprms[nf][flagmanage_id] == cumTACclosure_mgmt) {
  985: 						for (nreg = 0; nreg < Kreg; nreg++) {
  986→ 							bm->RegionalData[sp][nreg][reg_tac_id] = bm->BiTAC_sp[bim][nreg][sp][now_id] / (bm->BiTAC_sp[bim][totalreg_id][sp][now_id]
  987: 									+ small_num);
  988: 						}
```

**Suggested fix (from review):** Loop `nreg < min(K_num_active_reg, 2)` (the same Kreg clamp).

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

## MNGA-26

**format mismatch** · Conf H / Sev L  
**Found at (current code):** `atManageSetup.c` 1007, `atManageSetup.c` 471
**Lines at review (Oct 2026):** 1007-1008

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

`warn("... id number %d\n", ..., FunctGroupArray[sp].groupCode)` passes a `char*` to `%d`. Function: `Calculate_BiTAC()`.

**Code at issue** — `atManageSetup.c` line 1007  
Search for: `warn("`

```c
 1005: 			if (FunctGroupArray[sp].isImpacted == TRUE) {
 1006: 				if (bm->SP_FISHERYprms[sp][i][p_escape_id] > 1)
 1007→ 					warn("Manage_Init: p_escape is > 1 (it's %e) for species or group with id number %d\n", bm->SP_FISHERYprms[sp][i][p_escape_id],
 1008: 							FunctGroupArray[sp].groupCode);
 1009: 
```

**Code at issue** — `atManageSetup.c` line 471  
Search for: `, FunctGroupArray[sp].groupCode)`

```c
  469: 				if (flag_sp) {
  470: 					totTAC = 0;
  471→ 					fprintf(llogfp, "Time: %e %s", bm->dayt, FunctGroupArray[sp].groupCode);
  472: 					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  473: 						in_quota = bm->inQuota[nf][sp];
```

**Suggested fix (from review):** Use `%s`.

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

## MNGA-27

**logic (every class incremented)** · Conf H / Sev M  
**Found at (current code):** `atManageSetup.c` 1477
**Lines at review (Oct 2026):** 1477-1480

**Function:** `Calculate_Flag_Impose()`

**Why it's thought to be a bug**

`for (nimp...) if (flagimposecatch > 0) impose_classes[nimp] += 1;` increments every class equally, so the max search always picks `nimp = 0` and `flagimposeglobal` is always 0. The intent is to count the class equal to the flag value. Function: `Calculate_Flag_Impose()`.

**Code at issue** — `atManageSetup.c` line 1477  
Search for: `for (nimp`

```c
 1475: 			if (FunctGroupArray[sp].isImpacted == TRUE) {
 1476: 				/* Find out what setting of flagimpose global should be */
 1477→ 				for (nimp = 0; nimp < K_num_impose_classes; nimp++) {
 1478: 					if (bm->SP_FISHERYprms[sp][fleet][flagimposecatch_id] > 0)
 1479: 						impose_classes[nimp] += 1;
```

**Suggested fix (from review):** `nimp = (int)SP_FISHERYprms[sp][fleet][flagimposecatch_id]; if (nimp >= 0 && nimp < K_num_impose_classes) impose_classes[nimp]++;`.

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

## MNGA-28

**copy-paste** · Conf H / Sev M  
**Found at (current code):** `atManageSetup.c` 1552
**Lines at review (Oct 2026):** 1552

**Function:** `GenerateCPUEDistribution()`

**Why it's thought to be a bug**

The "top" skipper CDF uses `cpue_cdf_top_r_id` with `cpue_cdf_poor_p_id`. `cpue_cdf_top_p_id` exists (atlantisboxmodel.h:1780). Function: `GenerateCPUEDistribution()` (called from ateconomicsetup.c:640).

**Code at issue** — `atManageSetup.c` line 1552 (located by name only; check this is the right place)  
Search for: `step2 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_top_r_id`

```c
 1550:                     case negbiom_type:
 1551:                         step1 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_poor_r_id], FunctGroupArray[sp].speciesParams[cpue_cdf_poor_p_id]);
 1552→                         step2 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_top_r_id], FunctGroupArray[sp].speciesParams[cpue_cdf_poor_p_id]);
 1553:                         break;
 1554:                     default: quit("Other distributions are possible - refer to cdflib and instantiate\n");
```

**Suggested fix (from review):** Use `cpue_cdf_top_p_id`.

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

## MNGA-29

**init only in debug** · Conf M / Sev M  
**Found at (current code):** `atManageSetup.c` 483, `atManageSetup.c` 465, `atManageSetup.c` 287
**Lines at review (Oct 2026):** 465-484

**Function:** `Manage_Init()`

**Why it's thought to be a bug**

`speciesParams[TACorig_id] = totTAC` is set only inside `if (bm->debug == debug_assess)`. Otherwise it is set only by the economics setup (ateconomicsetup.c:583). Without econ or debug, `TACorig = 0` in `AMS_Tiered_Assessment()` (618), so with `flagQuotaBound` the check `RBC > TACbound*TACorig` forces RBC to 0, and line 874 divides `0/TACbound`. Function: `Manage_Init()`.

**Code at issue** — `atManageSetup.c` line 483  
Search for: `speciesParams[TACorig_id] = totTAC`

```c
  481: 						totTAC = 0;
  482: 					fprintf(llogfp, " TAC = %f (%e)", totTAC, totTAC); /* No conversion necessary as already in tonnes */
  483→ 					FunctGroupArray[sp].speciesParams[TACorig_id] = totTAC;
  484: 				}
  485: 				fprintf(llogfp, "\n");
```

**Code at issue** — `atManageSetup.c` line 465  
Search for: `if (bm->debug == debug_assess)`

```c
  463: 
  464: 	/* TAC and Fisheries_Indices Check */
  465→ 	if (bm->debug == debug_assess) {
  466: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  467: 			if (FunctGroupArray[sp].isFished == TRUE) {
```

**Code at issue** — `atManageSetup.c` line 287  
Search for: `Manage_Init()`

```c
  285:  * and is called by main() in transport module.
  286:  *
  287→  * The routine Manage_Init() is called once at the beginning of
  288:  * the run. It carries out the following steps.
  289:  *
```

**Suggested fix (from review):** Compute TACorig outside the debug block (keep only the fprintfs debug-gated).

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

## MNGA-30

**wrong condition** · Conf M / Sev L  
**Found at (current code):** `atManageSetup.c` 1068
**Lines at review (Oct 2026):** 1068-1069

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

`if (!i) FISHERYprms[i][flagconflict_id] = 0;` resets the flag for fleet 0 only, so the other fleets keep whatever value they had before the `gear_conflict` scan sets it to 1. The intent is to reset for every fleet. Function: `Calculate_BiTAC()`.

**Code at issue** — `atManageSetup.c` line 1068  
Search for: `if (!i)`

```c
 1066: 			}
 1067: 		}
 1068→ 		if (!i)
 1069: 			bm->FISHERYprms[i][flagconflict_id] = 0;
 1070: 
```

**Suggested fix (from review):** Drop the `if (!i)`.

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

## MNGA-31

**memory leak** · Conf H / Sev L  
**Found at (current code):** `atManageSetup.c` 1260, `atManageSetup.c` 329
**Lines at review (Oct 2026):** 1362-1463

**Function:** `Manage_Free()`

**Why it's thought to be a bug**

`Manage_Free()` never frees `bm->MPA`, `bm->TAC_over` or `bm->sp_basket`. In the fisheries-off path (Manage_Init 329-331) `sp_basket`, `TAC_over` and `rolling_cap_initialised` are allocated, but Manage_Free returns immediately. Function: `Manage_Free()`.

**Code at issue** — `atManageSetup.c` line 1260  
Search for: `TAC_over`

```c
 1258: 	//printf("Creating management PostLoad arrays\n");
 1259: 
 1260→ 	bm->TAC_over = Util_Alloc_Init_3D_Int(bm->K_num_basket, bm->K_num_fisheries, bm->K_num_tot_sp, 0);
 1261:     
 1262: 	if(bm->useRBCTiers)
```

**Code at issue** — `atManageSetup.c` line 329  
Search for: `sp_basket`

```c
  327: 		printf("Skipping fisheries as not active\n");
  328: 		/* Create basket quota management arrays */
  329→ 		bm->sp_basket = Util_Alloc_Init_2D_Int(1, bm->K_num_basket, bm->K_num_tot_sp);
  330: 		bm->TAC_over = Util_Alloc_Init_3D_Int(1, bm->K_num_fisheries, bm->K_num_tot_sp, 0);
  331:         bm->rolling_cap_initialised = Util_Alloc_Init_2D_Int((bm->K_num_max_cohort * bm->K_num_max_genetypes), bm->K_num_tot_sp, 0);
```

**Suggested fix (from review):** Add the matching free calls in both paths.

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

## MNGA-32

**NULL deref** · Conf M / Sev L  
**Found at (current code):** `atManageSetup.c` 352, `atManageSetup.c` 348, `atManageSetup.c` 287
**Lines at review (Oct 2026):** 352

**Function:** `Manage_Init()`

**Why it's thought to be a bug**

`*(strstr(convertedXMLFileName, ".nc")) = '\0';` crashes if the output name has no ".nc". `strcpy(fisheriesfile[120], bm->fishprmIfname)` (348) can overflow and the variable is unused. Function: `Manage_Init()`.

**Code at issue** — `atManageSetup.c` line 352  
Search for: `*(strstr(convertedXMLFileName, ".nc")) = '\0'`

```c
  350: 	/* Build the converted filename */
  351: 	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->ncOfname);
  352→ 	*(strstr(convertedXMLFileName, ".nc")) = '\0';
  353: 	strcat(convertedXMLFileName, "_management.xml");
  354: 
```

**Code at issue** — `atManageSetup.c` line 348 (closest match to the quoted code; wording differs slightly)  
Search for: `strcpy(fisheriesfile, bm->fishprmIfname);`

```c
  346: 	/* Build global vectors ******************************************************************
  347: 	 Using sjwlib routines for doubles and ints but spell it out in standard c for Name case */
  348→ 	strcpy(fisheriesfile, bm->fishprmIfname);
  349: 
  350: 	/* Build the converted filename */
```

**Code at issue** — `atManageSetup.c` line 287  
Search for: `Manage_Init()`

```c
  285:  * and is called by main() in transport module.
  286:  *
  287→  * The routine Manage_Init() is called once at the beginning of
  288:  * the run. It carries out the following steps.
  289:  *
```

**Suggested fix (from review):** Check for NULL before writing; remove `fisheriesfile` or use snprintf.

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

atManageSetup.c:329: in the fisheries-off branch, `Util_Alloc_Init_2D_Int(1, K_num_basket, K_num_tot_sp)` gives `[K_num_basket][1]`, unlike the normal `[K_num_tot_sp][K_num_basket]` (atManageParamIO.c:896). K_num_basket is not set yet on this path (Init_Manage_Flags is not called). Harmless only if sp_basket is never touched when fisheries are off.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atManageSetup.c:1061-1064: for every fleet with `inQuota` set, BiTACamt is overwritten with `no_quota` right after being computed. This looks inverted (`!inQuota`?), but `Make_Mgmt_Decisions()` recomputes BiTACamt annually, so the impact is limited to year 0.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atManageSetup.c:978: the loop variable `nreg` actually indexes the now/old slot (last dim, size 2). It works only because `totalreg_id == 2`. The region-1 term is used even when Kreg==1 (zero, so harmless).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

atManageSetup.c:519-549: byproduct/bycatch tiers do not set `BrefE_id`, so tier14 has BrefE=0 for those species.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

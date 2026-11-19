# atmanage/atManageSetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **9 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 4 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGA-24](#mnga-24) ⭐ | 1083-1085 | H/H | wrong variable / OOB | Inside `for (i = 0; i < K_num_fisheries; i++)` the test uses `bm->TACamt[sp][nf][now_id]`. `nf` is stale from … |  | [ ] |
| [MNGA-25](#mnga-25) | 1080 (alloc 1195) | M/M | OOB | `for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) BiTAC_sp[bim][nreg]...`. BiTAC_sp has only `totalreg_id+1… |  | [ ] |
| [MNGA-26](#mnga-26) | 1007-1008 | H/L | format mismatch | `warn("... id number %d\n", ..., FunctGroupArray[sp].groupCode)` passes a `char*` to `%d`. Function: `Calculat… |  | [ ] |
| [MNGA-27](#mnga-27) | 1477-1480 | H/M | logic (every class incremented) | `for (nimp...) if (flagimposecatch > 0) impose_classes[nimp] += 1;` increments every class equally, so the max… |  | [ ] |
| [MNGA-28](#mnga-28) | 1552 | H/M | copy-paste | The "top" skipper CDF uses `cpue_cdf_top_r_id` with `cpue_cdf_poor_p_id`. `cpue_cdf_top_p_id` exists (atlantis… |  | [ ] |
| [MNGA-29](#mnga-29) | 465-484 | M/M | init only in debug | `speciesParams[TACorig_id] = totTAC` is set only inside `if (bm->debug == debug_assess)`. Otherwise it is set … |  | [ ] |
| [MNGA-30](#mnga-30) | 1068-1069 | M/L | wrong condition | `if (!i) FISHERYprms[i][flagconflict_id] = 0;` resets the flag for fleet 0 only, so the other fleets keep what… |  | [ ] |
| [MNGA-31](#mnga-31) | 1362-1463 | H/L | memory leak | `Manage_Free()` never frees `bm->MPA`, `bm->TAC_over` or `bm->sp_basket`. In the fisheries-off path (Manage_In… |  | [ ] |
| [MNGA-32](#mnga-32) | 352 | M/L | NULL deref | `*(strstr(convertedXMLFileName, ".nc")) = '\0';` crashes if the output name has no ".nc". `strcpy(fisheriesfil… |  | [ ] |

---

## MNGA-24

**wrong variable / OOB** · Conf H / Sev H ⭐ priority  
**Listed lines:** 1083-1085

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

Inside `for (i = 0; i < K_num_fisheries; i++)` the test uses `bm->TACamt[sp][nf][now_id]`. `nf` is stale from the earlier loop (== K_num_fisheries, or uninitialised if no impacted species), so it reads `TACamt[sp][K_num_fisheries]`, past the fleet dimension (TACamt `[sp][nfleets][entries]`, line 1197). That is a garbage pointer dereference and a wrong filter. Function: `Calculate_BiTAC()`.

**Current code** (`atmanage/atManageSetup.c` lines 1081–1087)

```c
 1081: 				for (bim = 0; bim < 6; bim++) {
 1082: 					sumBiTAC = 0;
 1083: 					for (i = 0; i < bm->K_num_fisheries; i++) {
 1084:                         if(!FunctGroupArray[sp].isTAC || (bm->TACamt[sp][nf][now_id] < no_quota)) {
 1085:                             sumBiTAC += bm->BiTAC_sp[bim][nreg][sp][now_id] * bm->TACamt[sp][i][now_id];
 1086:                         }
 1087: 					}
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
**Listed lines:** 1080 (alloc 1195)

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

`for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) BiTAC_sp[bim][nreg]...`. BiTAC_sp has only `totalreg_id+1 = 3` region rows, but K_num_active_reg can be any number of regions (atbiolsetup.c:583). With more than 3 regions this writes out of bounds, and with exactly 3 it overwrites the total row. The earlier loop clamps to `Kreg` (max 2). Function: `Calculate_BiTAC()`.

**Current code** (`atmanage/atManageSetup.c` lines 1078–1082)

```c
 1078: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 1079: 		if (FunctGroupArray[sp].isImpacted == TRUE) {
 1080: 			for (nreg = 0; nreg < bm->K_num_active_reg; nreg++) {
 1081: 				for (bim = 0; bim < 6; bim++) {
 1082: 					sumBiTAC = 0;
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
**Listed lines:** 1007-1008

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

`warn("... id number %d\n", ..., FunctGroupArray[sp].groupCode)` passes a `char*` to `%d`. Function: `Calculate_BiTAC()`.

**Current code** (`atmanage/atManageSetup.c` lines 1005–1010)

```c
 1005: 			if (FunctGroupArray[sp].isImpacted == TRUE) {
 1006: 				if (bm->SP_FISHERYprms[sp][i][p_escape_id] > 1)
 1007: 					warn("Manage_Init: p_escape is > 1 (it's %e) for species or group with id number %d\n", bm->SP_FISHERYprms[sp][i][p_escape_id],
 1008: 							FunctGroupArray[sp].groupCode);
 1009: 
 1010: 				if (FunctGroupArray[sp].isFished == TRUE) {
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
**Listed lines:** 1477-1480

**Function:** `Calculate_Flag_Impose()`

**Why it's thought to be a bug**

`for (nimp...) if (flagimposecatch > 0) impose_classes[nimp] += 1;` increments every class equally, so the max search always picks `nimp = 0` and `flagimposeglobal` is always 0. The intent is to count the class equal to the flag value. Function: `Calculate_Flag_Impose()`.

**Current code** (`atmanage/atManageSetup.c` lines 1475–1482)

```c
 1475: 			if (FunctGroupArray[sp].isImpacted == TRUE) {
 1476: 				/* Find out what setting of flagimpose global should be */
 1477: 				for (nimp = 0; nimp < K_num_impose_classes; nimp++) {
 1478: 					if (bm->SP_FISHERYprms[sp][fleet][flagimposecatch_id] > 0)
 1479: 						impose_classes[nimp] += 1;
 1480: 				}
 1481: 
 1482: 	            /* If using Bulk TAC then scale up TACamt accordingly */
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
**Listed lines:** 1552

**Function:** `GenerateCPUEDistribution()`

**Why it's thought to be a bug**

The "top" skipper CDF uses `cpue_cdf_top_r_id` with `cpue_cdf_poor_p_id`. `cpue_cdf_top_p_id` exists (atlantisboxmodel.h:1756). Function: `GenerateCPUEDistribution()` (called from ateconomicsetup.c:640).

**Current code** (`atmanage/atManageSetup.c` lines 1550–1554)

```c
 1550:                     case negbiom_type:
 1551:                         step1 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_poor_r_id], FunctGroupArray[sp].speciesParams[cpue_cdf_poor_p_id]);
 1552:                         step2 = negbinomial_cdf_values(i, (int)FunctGroupArray[sp].speciesParams[cpue_cdf_top_r_id], FunctGroupArray[sp].speciesParams[cpue_cdf_poor_p_id]);
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
**Listed lines:** 465-484

**Function:** `Manage_Init()`

**Why it's thought to be a bug**

`speciesParams[TACorig_id] = totTAC` is set only inside `if (bm->debug == debug_assess)`. Otherwise it is set only by the economics setup (ateconomicsetup.c:583). Without econ or debug, `TACorig = 0` in `AMS_Tiered_Assessment()` (618), so with `flagQuotaBound` the check `RBC > TACbound*TACorig` forces RBC to 0, and line 875 divides `0/TACbound`. Function: `Manage_Init()`.

**Current code** (`atmanage/atManageSetup.c` lines 463–486)

```c
  463: 
  464: 	/* TAC and Fisheries_Indices Check */
  465: 	if (bm->debug == debug_assess) {
  466: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  467: 			if (FunctGroupArray[sp].isFished == TRUE) {
  468: 				flag_sp = (int) (FunctGroupArray[sp].speciesParams[flag_id]);
  469: 				if (flag_sp) {
  470: 					totTAC = 0;
  471: 					fprintf(llogfp, "Time: %e %s", bm->dayt, FunctGroupArray[sp].groupCode);
  472: 					for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  473: 						in_quota = bm->inQuota[nf][sp];
  474: 						fprintf(llogfp, "nf = %d, in_quota = %d\n", nf, in_quota);
  475: 						if (in_quota) {
  476: 							totTAC += bm->TACamt[sp][nf][now_id];
  477: 							fprintf(llogfp, " %s=%e", FisheryArray[nf].fisheryCode, bm->TACamt[sp][nf][now_id]);
  478: 						}
  479: 					}
  480: 					if (totTAC < 0)
  481: 						totTAC = 0;
  482: 					fprintf(llogfp, " TAC = %f (%e)", totTAC, totTAC); /* No conversion necessary as already in tonnes */
  483: 					FunctGroupArray[sp].speciesParams[TACorig_id] = totTAC;
  484: 				}
  485: 				fprintf(llogfp, "\n");
  486: 			}
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
**Listed lines:** 1068-1069

**Function:** `Calculate_BiTAC()`

**Why it's thought to be a bug**

`if (!i) FISHERYprms[i][flagconflict_id] = 0;` resets the flag for fleet 0 only, so the other fleets keep whatever value they had before the `gear_conflict` scan sets it to 1. The intent is to reset for every fleet. Function: `Calculate_BiTAC()`.

**Current code** (`atmanage/atManageSetup.c` lines 1066–1071)

```c
 1066: 			}
 1067: 		}
 1068: 		if (!i)
 1069: 			bm->FISHERYprms[i][flagconflict_id] = 0;
 1070: 
 1071: 		for (fishery_id = 0; fishery_id < bm->K_num_fisheries; fishery_id++) {
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
**Listed lines:** 1362-1463

**Function:** `Manage_Free()`

**Why it's thought to be a bug**

`Manage_Free()` never frees `bm->MPA`, `bm->TAC_over` or `bm->sp_basket`. In the fisheries-off path (Manage_Init 329-331) `sp_basket`, `TAC_over` and `rolling_cap_initialised` are allocated, but Manage_Free returns immediately. Function: `Manage_Free()`.

**Current code** (`atmanage/atManageSetup.c` lines 1360–1394 (range truncated))

```c
 1360:  * \brief Free fishery arrays
 1361:  */
 1362: void Manage_Free(MSEBoxModel *bm) {
 1363: 
 1364: //	/* Close the management performance output file */
 1365: //	Close_Management_Index_File(bm);
 1366: //	Close_Manage_Output_Files(bm);
 1367: //
 1368: //	free2d(bm->TripCatch);
 1369: //	/** The management performance indicators */
 1370: //	c_free2d(manageindxNAME);
 1371: //	free2d(manageindx);
 1372: 
 1373: 	if (!bm->flag_fisheries_on) {
 1374: 		/* If not using fishing don't bother with any of these arrays */
 1375: 
 1376: 		return;
 1377: 	}
 1378:     
 1379:     if (bm->useRBCTiers) {
 1380:         Close_Tiered_Output_Files(bm);
 1381:     }
 1382:     
 1383: 
 1384: 	i_free1d(bm->reg_season);
 1385: 	free2d(bm->TotCumRecCatch);
 1386: 
 1387: 	free2d(bm->phased_out);
 1388: 	free3d(bm->TACamt);
 1389: 	free4d(bm->BiTACamt);
 1390: 	free4d(bm->BiTAC_sp);
 1391: 	free2d(bm->TotOldCumCatch);
 1392: 	free2d(bm->TotCumBiCatch);
 1393: 	free2d(bm->TotCumDiscards);
 1394: 	free2d(bm->TotOldCumDiscards);
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
**Listed lines:** 352

**Function:** `Manage_Init()`

**Why it's thought to be a bug**

`*(strstr(convertedXMLFileName, ".nc")) = '\0';` crashes if the output name has no ".nc". `strcpy(fisheriesfile[120], bm->fishprmIfname)` (348) can overflow and the variable is unused. Function: `Manage_Init()`.

**Current code** (`atmanage/atManageSetup.c` lines 350–354)

```c
  350: 	/* Build the converted filename */
  351: 	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->ncOfname);
  352: 	*(strstr(convertedXMLFileName, ".nc")) = '\0';
  353: 	strcat(convertedXMLFileName, "_management.xml");
  354: 
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

atManageSetup.c:1062-1065: for every fleet with `inQuota` set, BiTACamt is overwritten with `no_quota` right after being computed. This looks inverted (`!inQuota`?), but `Make_Mgmt_Decisions()` recomputes BiTACamt annually, so the impact is limited to year 0.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atManageSetup.c:979: the loop variable `nreg` actually indexes the now/old slot (last dim, size 2). It works only because `totalreg_id == 2`. The region-1 term is used even when Kreg==1 (zero, so harmless).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

atManageSetup.c:519-549: byproduct/bycatch tiers do not set `BrefE_id`, so tier14 has BrefE=0 for those species.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

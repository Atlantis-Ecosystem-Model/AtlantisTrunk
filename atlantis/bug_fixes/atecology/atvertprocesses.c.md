# atecology/atvertprocesses.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CONT-20](#cont-20) | 726, 734 | H/M | cohort vs cohort×genotype | `preychrt = bpreychrt / numGeneTypes; ... bm->FCcaughttemp[prey][fleet][preychrt]` — FCcaughttemp is indexed b… |  | [ ] |
| [CONT-21](#cont-21) | 1617 | H/L | printf format mismatch | Format string contains `"... or %d (incfemental fixed % drops)"`; `% d` is parsed as a conversion (space flag … |  | [ ] |
| [CONT-22](#cont-22) | 1594-1597, 1610-1613 | M/M | wrong formula | Non-dynamic branch uses `opt_SN = RN / (1.0 + X_RS)` and `opt_RN = (1.0 + X_RS) * newSN`, but optimal reserve … |  | [ ] |
| [CONT-23](#cont-23) | 437-439, 383 | L/M | accumulator not reset | `like_coral` is set inside the `stage` loop but never reset per stage, so a coral-liking juvenile stage forces… |  | [ ] |
| [CONT-24](#cont-24) | 830 | M/M | division by zero | `*FRC_sp = toS / avail_intake;` with no guard; `Vertebrate_Activities()` guards `avail_intake > 0` (1393) but … |  | [ ] |

---

## CONT-20

**cohort vs cohort×genotype** · Conf H / Sev M  
**Listed lines:** 726, 734

**Function:** `Avail_Catch()`

**Why it's thought to be a bug**

`preychrt = bpreychrt / numGeneTypes; ... bm->FCcaughttemp[prey][fleet][preychrt]` — FCcaughttemp is indexed by cohort×genotype everywhere else (atprocess.c:797, 1094, 2005; atecology.c:1122; reset loop atecology.c:3990). With numGeneTypes>1 the wrong cohort's catch is used. Function: `Avail_Catch()`.

**Current code** (`atecology/atvertprocesses.c` lines 724–736)

```c
  724: 	}
  725: 
  726: 	preychrt = bpreychrt / FunctGroupArray[prey].numGeneTypes;
  727: 	if (FunctGroupArray[prey].speciesParams[flag_id] == TRUE && FunctGroupArray[prey].isVertebrate == TRUE) {
  728: 		preyage = FunctGroupArray[prey].cohort_stage[bpreychrt];
  729: 		if (bm->pSPFCeat[guildcase][prey][chrtstage][preyage] > 0.0) {
  730: 			if (SP[prey][bpreychrt][SN_id] >= FunctGroupArray[guildcase].speciesParams[KLP_id] * SN) {
  731: 				if (SP[prey][bpreychrt][SN_id] <= FunctGroupArray[guildcase].speciesParams[KUP_id] * SN) {
  732: 					step1 = 0;
  733: 					for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
  734: 						step1 += bm->FCcaughttemp[prey][fleet][preychrt] * bm->pFLEET[guildcase][fleet];
  735: 					}
  736: 
```

**Suggested fix (from review):** Index with `bpreychrt`

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

## CONT-21

**printf format mismatch** · Conf H / Sev L  
**Listed lines:** 1617

**Function:** `Check_for_Starvation()`

**Why it's thought to be a bug**

Format string contains `"... or %d (incfemental fixed % drops)"`; `% d` is parsed as a conversion (space flag + `d`) with no matching argument — UB. Function: `Check_for_Starvation()`.

**Current code** (`atecology/atvertprocesses.c` lines 1603–1607) — code appears to have moved -12 lines since the review

```c
 1603:                 break;
 1604:             default:
 1605:                 quit("No such shrinkage case - you set size_shrinking_type to %d but it needs to be %d (no shrinking) or %d (reset based on RN) or %d (incfemental fixed % drops)\n", bm->size_shrinking_type, no_shrink, XRS_shrink, incremental_shrink);
 1606:                 break;
 1607:         }
```

**Suggested fix (from review):** Escape as `%%`

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

## CONT-22

**wrong formula** · Conf M / Sev M  
**Listed lines:** 1594-1597, 1610-1613

**Function:** `Check_for_Starvation()`

**Why it's thought to be a bug**

Non-dynamic branch uses `opt_SN = RN / (1.0 + X_RS)` and `opt_RN = (1.0 + X_RS) * newSN`, but optimal reserve is `RN = X_RS*SN` elsewhere in non-dynamic mode (`Partition_Weight()` 766/770 `RN/(X_RS*SN)`; total weight `(1+X_RS)*SN` in `Vert_Mortality()` 896). XRS_shrink shrinks SN too far; incremental_shrink yields negative `diff_RN` so RN *grows* on starvation. Function: `Check_for_Starvation()`.

**Current code** (`atecology/atvertprocesses.c` lines 1592–1615)

```c
 1592:             case incremental_shrink: // Shrink x% per month
 1593:                 diff_SN = SN * propDrop;
 1594:                 newSN = SN - diff_SN;
 1595:                 if (bm->flag_dynamicXRS) {
 1596:                     // This assumes opt_RN = X_RS * SN
 1597:                     opt_RN = X_RS * newSN;
 1598:                 } else {
 1599:                     // This is the original version which assumes opt_RN = (1.0 + X_RS) * SN
 1600:                     opt_RN = (1.0 + X_RS) * newSN;
 1601:                 }
 1602:                 diff_RN = RN - opt_RN;
 1603:                 break;
 1604:             default:
 1605:                 quit("No such shrinkage case - you set size_shrinking_type to %d but it needs to be %d (no shrinking) or %d (reset based on RN) or %d (incfemental fixed % drops)\n", bm->size_shrinking_type, no_shrink, XRS_shrink, incremental_shrink);
 1606:                 break;
 1607:         }
 1608:         
 1609:         /* Update the growth vectors */
 1610:         FunctGroupArray[guildcase].grow[cohort][SN_id] -= diff_SN;
 1611:         FunctGroupArray[guildcase].grow[cohort][RN_id] -= diff_RN;
 1612:     }
 1613:     return ;
 1614: }
 1615: 
```

**Suggested fix (from review):** Use `RN / X_RS` and `X_RS * newSN`

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

## CONT-23

**accumulator not reset** · Conf L / Sev M  
**Listed lines:** 437-439, 383

**Function:** `Vertebrate_Assess_Enviro()`

**Why it's thought to be a bug**

`like_coral` is set inside the `stage` loop but never reset per stage, so a coral-liking juvenile stage forces the rugosity model for the adult stage too. Function: `Vertebrate_Assess_Enviro()`.

**Current code** (`atecology/atvertprocesses.c` lines 381–420 (truncated))

```c
  381: 	int habitat_dependent, stage, kij;
  382: 	int sp;
  383: 	int like_coral = 0;
  384: 	double sp_like, step1;
  385:     
  386:     /*
  387:     if(verbose > 1){
  388:     	printf("Vertebrate_Assess_Enviro for %s\n", FunctGroupArray[guildcase].groupCode);
  389:     }
  390:     */
  391: 
  392: 	Box_Rugosity(bm, &bm->boxes[boxin], rugosity_check_done, llogfp);
  393: 
  394: 	habitat_dependent = FunctGroupArray[guildcase].isHabDepend;
  395: 	Kcov_juv = FunctGroupArray[guildcase].speciesParams[Kcov_juv_id];
  396: 	Bcov_juv = FunctGroupArray[guildcase].speciesParams[Bcov_juv_id];
  397: 	Acov_juv = FunctGroupArray[guildcase].speciesParams[Acov_juv_id];
  398: 	Kcov_ad = FunctGroupArray[guildcase].speciesParams[Kcov_ad_id];
  399: 	Bcov_ad = FunctGroupArray[guildcase].speciesParams[Bcov_ad_id];
  400: 	Acov_ad = FunctGroupArray[guildcase].speciesParams[Acov_ad_id];
  401: 	Rscalar = FunctGroupArray[guildcase].speciesParams[RugCover_scalar_id];
  402: 	sp_num_biogen_hab_ad = FunctGroupArray[guildcase].speciesParams[K_num_biogen_habad_id];
  403: 	sp_num_biogen_hab_juv = FunctGroupArray[guildcase].speciesParams[K_num_biogen_habjuv_id];
  404: 
  405: 	if (bm->flaghabdepend && habitat_dependent) {
  406: 
  407: 		/* Determine coastal habitat degradation effects */
  408: 		for (kij = 0; kij < bm->K_num_bed_types; kij++) {
  409: 			if (Box_degradedi[boxin] && (cdepth < -250) && bm->flagdegrade) {
  410: 				BED_scale[kij] = Util_Get_Accumulative_Change_Scale(bm, BEDchange_max_num, BEDchange[kij]);
  411: 			} else {
  412: 				BED_scale[kij] = 1.0;
  413: 			}
  414: 		}
  415: 
  416: 		for (stage = 0; stage < FunctGroupArray[guildcase].numStages; stage++) {
  417: 			if(stage)
  418: 				sp_num_biogen_hab = sp_num_biogen_hab_ad;
  419: 			else
  420: 				sp_num_biogen_hab = sp_num_biogen_hab_juv;
```

**Suggested fix (from review):** Reset `like_coral = 0` at top of the stage loop

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

## CONT-24

**division by zero** · Conf M / Sev M  
**Listed lines:** 830

**Function:** `Partition_Weight_Dynamic()`

**Why it's thought to be a bug**

`*FRC_sp = toS / avail_intake;` with no guard; `Vertebrate_Activities()` guards `avail_intake > 0` (1393) but `External_Box_Ecology()` (atexternalpop.c:164) calls it unconditionally, so avail_intake ≤ 0 gives inf/NaN or negative FRC (no lower clamp). Function: `Partition_Weight_Dynamic()`.

**Current code** (`atecology/atvertprocesses.c` lines 828–832)

```c
  828:     toS += (RNcost_SP * SNcost_SP * intake / (RNcost_SP + X_RS * SNcost_SP));
  829:     
  830:     *FRC_sp = toS / avail_intake; // this is the proportion of total original intake (avail_intake) that goes to SN
  831:     
  832:     if (*FRC_sp > 1.0)
```

**Suggested fix (from review):** Guard `avail_intake <= 0` (fallback to Partition_Weight) and clamp at 0

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

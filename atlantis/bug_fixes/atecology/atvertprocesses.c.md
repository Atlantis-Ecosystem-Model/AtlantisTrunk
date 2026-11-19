# atecology/atvertprocesses.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CONT-20](#cont-20) | 726, 734 | H/M | cohort vs cohort×genotype | `preychrt = bpreychrt / numGeneTypes; ... bm->FCcaughttemp[prey][fleet][preychrt]` — FCcaughttemp is indexed b… |  | [ ] |
| [CONT-21](#cont-21) | 1605 | H/L | printf format mismatch | Format string contains `"... or %d (incfemental fixed % drops)"`; `% d` is parsed as a conversion (space flag … |  | [ ] |
| [CONT-22](#cont-22) | 1584, 1600, 1596 | M/M | wrong formula | Non-dynamic branch uses `opt_SN = RN / (1.0 + X_RS)` and `opt_RN = (1.0 + X_RS) * newSN`, but optimal reserve … |  | [ ] |
| [CONT-23](#cont-23) | 438 | L/M | accumulator not reset | `like_coral` is set inside the `stage` loop but never reset per stage, so a coral-liking juvenile stage forces… |  | [ ] |
| [CONT-24](#cont-24) | 830, 1025, 1381 | M/M | division by zero | `*FRC_sp = toS / avail_intake;` with no guard; `Vertebrate_Activities()` guards `avail_intake > 0` (1393) but … |  | [ ] |

---

## CONT-20

**cohort vs cohort×genotype** · Conf H / Sev M  
**Found at (current code):** `atvertprocesses.c` 726, `atvertprocesses.c` 734
**Lines at review (Oct 2026):** 726, 734

**Function:** `Avail_Catch()`

**Why it's thought to be a bug**

`preychrt = bpreychrt / numGeneTypes; ... bm->FCcaughttemp[prey][fleet][preychrt]` — FCcaughttemp is indexed by cohort×genotype everywhere else (atprocess.c:910, 1217, 2168; atecology.c:1122; reset loop atecology.c:3990). With numGeneTypes>1 the wrong cohort's catch is used. Function: `Avail_Catch()`.

**Code at issue** — `atvertprocesses.c` line 726 (closest match to the quoted code; wording differs slightly)  
Search for: `preychrt = bpreychrt / FunctGroupArray[prey].numGeneTypes;`

```c
  724: 	}
  725: 
  726→ 	preychrt = bpreychrt / FunctGroupArray[prey].numGeneTypes;
  727: 	if (FunctGroupArray[prey].speciesParams[flag_id] == TRUE && FunctGroupArray[prey].isVertebrate == TRUE) {
  728: 		preyage = FunctGroupArray[prey].cohort_stage[bpreychrt];
```

**Code at issue** — `atvertprocesses.c` line 734  
Search for: `FCcaughttemp[prey][fleet][preychrt]`

```c
  732: 					step1 = 0;
  733: 					for (fleet = 0; fleet < bm->K_num_fisheries; fleet++) {
  734→ 						step1 += bm->FCcaughttemp[prey][fleet][preychrt] * bm->pFLEET[guildcase][fleet];
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
**Found at (current code):** `atvertprocesses.c` 1605
**Lines at review (Oct 2026):** 1617

**Function:** `Check_for_Starvation()`

**Why it's thought to be a bug**

Format string contains `"... or %d (incfemental fixed % drops)"`; `% d` is parsed as a conversion (space flag + `d`) with no matching argument — UB. Function: `Check_for_Starvation()`.

**Code at issue** — `atvertprocesses.c` line 1605 (closest match to the quoted code; wording differs slightly)  
Search for: `quit("No such shrinkage case - you set size_shrinking_type to %d but it needs to be %d (no`

```c
 1603:                 break;
 1604:             default:
 1605→                 quit("No such shrinkage case - you set size_shrinking_type to %d but it needs to be %d (no shrinking) or %d (reset based on RN) or %d (incfemental fixed % drops)\n", bm->size_shrinking_type, no_shrink, XRS_shrink, incremental_shrink);
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
**Found at (current code):** `atvertprocesses.c` 1584, `atvertprocesses.c` 1600, `atvertprocesses.c` 1596
**Lines at review (Oct 2026):** 1594-1597, 1610-1613

**Function:** `Check_for_Starvation()`

**Why it's thought to be a bug**

Non-dynamic branch uses `opt_SN = RN / (1.0 + X_RS)` and `opt_RN = (1.0 + X_RS) * newSN`, but optimal reserve is `RN = X_RS*SN` elsewhere in non-dynamic mode (`Partition_Weight()` 766/770 `RN/(X_RS*SN)`; total weight `(1+X_RS)*SN` in `Vert_Mortality()` 896). XRS_shrink shrinks SN too far; incremental_shrink yields negative `diff_RN` so RN *grows* on starvation. Function: `Check_for_Starvation()`.

**Code at issue** — `atvertprocesses.c` line 1584  
Search for: `opt_SN = RN / (1.0 + X_RS)`

```c
 1582:                 } else {
 1583:                     // This is the original version which assumes opt_RN = (1.0 + X_RS) * SN
 1584→                     opt_SN = RN / (1.0 + X_RS);
 1585:                 }
 1586:                 diff_SN = SN - opt_SN;
```

**Code at issue** — `atvertprocesses.c` line 1600  
Search for: `opt_RN = (1.0 + X_RS) * newSN`

```c
 1598:                 } else {
 1599:                     // This is the original version which assumes opt_RN = (1.0 + X_RS) * SN
 1600→                     opt_RN = (1.0 + X_RS) * newSN;
 1601:                 }
 1602:                 diff_RN = RN - opt_RN;
```

**Code at issue** — `atvertprocesses.c` line 1596  
Search for: `RN = X_RS * SN`

```c
 1594:                 newSN = SN - diff_SN;
 1595:                 if (bm->flag_dynamicXRS) {
 1596→                     // This assumes opt_RN = X_RS * SN
 1597:                     opt_RN = X_RS * newSN;
 1598:                 } else {
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
**Found at (current code):** `atvertprocesses.c` 438
**Lines at review (Oct 2026):** 437-439, 383

**Function:** `Vertebrate_Assess_Enviro()`

**Why it's thought to be a bug**

`like_coral` is set inside the `stage` loop but never reset per stage, so a coral-liking juvenile stage forces the rugosity model for the adult stage too. Function: `Vertebrate_Assess_Enviro()`.

**Code at issue** — `atvertprocesses.c` line 438 (the line the review pointed to, found by its text)  
Search for: `like_coral = 1;`

```c
  436: 
  437: 						if(((FunctGroupArray[sp].groupType == CORAL) || (FunctGroupArray[sp].groupType == SPONGE)) && (sp_like > 0)) {
  438→ 							like_coral = 1;
  439: 						}
  440: 					//}
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
**Found at (current code):** `atvertprocesses.c` 830, `atvertprocesses.c` 1025, `atvertprocesses.c` 1381
**Lines at review (Oct 2026):** 830

**Function:** `Partition_Weight_Dynamic()`

**Why it's thought to be a bug**

`*FRC_sp = toS / avail_intake;` with no guard; `Vertebrate_Activities()` guards `avail_intake > 0` (1393) but `External_Box_Ecology()` (atexternalpop.c:164) calls it unconditionally, so avail_intake ≤ 0 gives inf/NaN or negative FRC (no lower clamp). Function: `Partition_Weight_Dynamic()`.

**Code at issue** — `atvertprocesses.c` line 830  
Search for: `*FRC_sp = toS / avail_intake`

```c
  828:     toS += (RNcost_SP * SNcost_SP * intake / (RNcost_SP + X_RS * SNcost_SP));
  829:     
  830→     *FRC_sp = toS / avail_intake; // this is the proportion of total original intake (avail_intake) that goes to SN
  831:     
  832:     if (*FRC_sp > 1.0)
```

**Code at issue** — `atvertprocesses.c` line 1025  
Search for: `Vertebrate_Activities()`

```c
 1023: 
 1024: 				/* Note that only non-vertebrate prey read in here, vertebrate prey
 1025→ 				 dealt with directly in the Vertebrate_Activities() subroutine */
 1026: 
 1027: 				Vertebrate_Activities(bm, boxLayerInfo, level_id, llogfp, guildcase, spSPinfo[guildcase][cohort][SN_id], spSPinfo[guildcase][cohort][RN_id], spSPinfo[guildcase][cohort][DEN_id], predcase_sp, cohort, E1_sp, E2_sp, E3_sp, E4_sp, PB_scale, avail_BB * BB_scale, spPREYinfo, spGRAZEinfo, spCATCHGRAZEinfo, spSPinfo, &VGrowth, &VGrazelive, &FRC_V, &Vtotchrt, &Vmortality, &VreleaseNHchrt, &VprodDLchrt, &VprodDRchrt);
```

**Code at issue** — `atvertprocesses.c` line 1381  
Search for: `avail_intake > 0`

```c
 1379:     avail_intake = *Growth;
 1380:     
 1381→     if (bm->flag_dynamicXRS && (avail_intake > 0)) {  // Test needs to be here for consistency with old code
 1382:         Partition_Weight_Dynamic(bm, guildcase, SN, RN, X_RS, &FRC_sp, avail_intake, llogfp);
 1383:     }
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

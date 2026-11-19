# atassess/atdiet.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [DIE-1](#die-1) ⭐ | 115-138 | H/H | accumulator in wrong loop | `Calculate_Overall_Diet_Composition` runs inside the box loop on running totals, so early boxes are counted ma… |  | [ ] |
| [DIE-2](#die-2) | 162-164 | M/M | wrong slot | Prey availability uses `biolbiom[prey][b][WC]` only, so infauna and sediment bacteria are never eaten (see DAT… |  | [ ] |
| [DIE-3](#die-3) ✔ ⭐ | 243 (alloc atsamplesetup.c:277) | H/H | swapped dims in Init | Allocated `Util_Alloc_Init_2D_Double(2, K)` = `[K][2]`, but reset with `Util_Init_2D_Double(contribvert, 2, K)… |  | [ ] |
| [DIE-4](#die-4) | 547-555 | H/L | misplaced break | `if (checkdone) break;` sits inside the same inner loop, so it never fires (performance only). |  | [ ] |
| [DIE-5](#die-5) ⭐ | 667-671, 690-692 | H/H | wrong variable | Diagonal uses `pred*numStages+stage`; the rest of the matrix uses `pred*2+stage`. Detritus rows stay zero, so … |  | [ ] |
| [DIE-6](#die-6) | 767-784 | M/M | copy-paste | Detritus preference is always written to `allStage=0`, so adults never eat detritus. |  | [ ] |
| [DIE-7](#die-7) | 804-807 | M/L | wrong variable | `pSPEat[predStage][prey][preyStage]`: the 3rd index is habitat, not stage; `/2` is hard-coded. |  | [ ] |

---

## DIE-1

**accumulator in wrong loop** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 115-138  
**Lines at review (Oct 2026):** 115-138

**Why it's thought to be a bug**

`Calculate_Overall_Diet_Composition` runs inside the box loop on running totals, so early boxes are counted many times (atattribute.c:1115 does it after the loop).

**Current code** (`atassess/atdiet.c` lines 113–140)

```c
  113: 	Util_Init_4D_Double(totareadiet, bm->K_num_tot_sp, 2, bm->K_num_tot_sp, 2, 0.0);
  114: 
  115: 	for (bb = 0; bb < bm->nsbox; bb++) {
  116: 		b = bm->nsboxes[bb];
  117: 		d = phys[wc_depth_id][b];
  118: 
  119: 		Util_Init_2D_Int(stomachs, bm->K_num_tot_sp, 2, 0);
  120: 
  121: 		/* This will set up the food available to each predator.
  122: 		 * The values per predator are then normalised to sum to 1.0
  123: 		 * So its basically the proportion that each prey group makes up the
  124: 		 * diet of each predator.
  125: 		 */
  126: 		Setup_Avail_Food(bm, b, ofp);
  127: 
  128: 		if (d < bm->cdz) {
  129: 			/* Inshore diets */
  130: 			Sample_Diet(bm, inshorediet);
  131: 		} else {
  132: 			/* Offshore diets */
  133: 			Sample_Diet(bm, offshorediet);
  134: 		}
  135: 
  136: 		/* Overall diet composition */
  137: 		Calculate_Overall_Diet_Composition(bm, totareadiet, inshorediet, offshorediet);
  138: 	}
  139: 
  140: 	/* Normalise diet composition */
```

**Suggested fix (from review):** move after the loop

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

## DIE-2

**wrong slot** · Conf M / Sev M  
**Lines (current code):** 162-164  
**Lines at review (Oct 2026):** 162-164

**Why it's thought to be a bug**

Prey availability uses `biolbiom[prey][b][WC]` only, so infauna and sediment bacteria are never eaten (see DAT-1).

**Current code** (`atassess/atdiet.c` lines 160–166)

```c
  160: 				availfood[pred][predStage][prey][SED] = biolbiom[prey][b][SED] * dietpref[pred][predStage][prey][SED];
  161: 
  162: 			} else {
  163: 				availfood[pred][predStage][prey][WC] = biolbiom[prey][b][WC] * dietpref[pred][predStage][prey][WC];
  164: 			}
  165: 		}
  166: 	}
```

**Suggested fix (from review):** WC+SED

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

## DIE-3

**swapped dims in Init** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 243 (alloc atsamplesetup.c:277)  
**Lines at review (Oct 2026):** 243 (alloc atsamplesetup.c:277)

**Why it's thought to be a bug**

Allocated `Util_Alloc_Init_2D_Double(2, K)` = `[K][2]`, but reset with `Util_Init_2D_Double(contribvert, 2, K)`, which loops `[0..1][0..K-1]` and covers only flat elements 0..K+1. Entries for pred ≥ (K+2)/2 are never reset and accumulate.

**Current code** (`atassess/atdiet.c` lines 241–245)

```c
  241: 
  242: 	/* Could possibly just allocate this here instead of it being a global */
  243: 	Util_Init_2D_Double(contribvert, 2, bm->K_num_tot_sp, 0.0);
  244: 
  245: 	/* Determine vertebrate biomass */
```

**Suggested fix (from review):** `Util_Init_2D_Double(contribvert, K_num_tot_sp, 2, 0.0)`

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

## DIE-4

**misplaced break** · Conf H / Sev L  
**Lines (current code):** 547-555  
**Lines at review (Oct 2026):** 547-555

**Why it's thought to be a bug**

`if (checkdone) break;` sits inside the same inner loop, so it never fires (performance only).

**Current code** (`atassess/atdiet.c` lines 545–557)

```c
  545: 				upper += p[bin][j];
  546: 
  547: 				if ((lower <= u) && (u < upper)){
  548: 					count[bin][j]++;
  549:                     checkdone = 1;
  550: 					break;
  551: 				}
  552:                 if (checkdone) {
  553:                     break;
  554:                 }
  555: 			}
  556: 		}
  557: 	}
```

**Suggested fix (from review):** move out one level

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

## DIE-5

**wrong variable** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 667-671, 690-692  
**Lines at review (Oct 2026):** 667-671, 690-692

**Why it's thought to be a bug**

Diagonal uses `pred*numStages+stage`; the rest of the matrix uses `pred*2+stage`. Detritus rows stay zero, so the matrix is singular, and Doolittle's -1 return is ignored.

**Current code** (`atassess/atdiet.c` lines 665–694)

```c
  665: 	}
  666: 
  667: 	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
  668: 		for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
  669: 			lhsmat[(pred * FunctGroupArray[pred].numStages) + predStage][(pred * FunctGroupArray[pred].numStages) + predStage] = 1.0;
  670: 		}
  671: 	}
  672: 
  673: 	/* Set up the matrix */
  674: 	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
  675: 		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isDetritus == FALSE) {
  676: 			for (predStage = 0; predStage < K_num_invageclass; predStage++) {
  677: 				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
  678: 					if (FunctGroupArray[prey].isAssessed == TRUE) {
  679: 						for (preyStage = 0; preyStage < K_num_invageclass; preyStage++) {
  680: 							lhsmat[(pred * 2) + predStage][(prey * 2) + preyStage] = -diet[pred][predStage][prey][preyStage];
  681: 						}
  682: 					}
  683: 				}
  684: 				lhsmat[(pred * 2) + predStage][(pred * 2) + predStage] = 1.0 - diet[pred][predStage][pred][predStage];
  685: 			}
  686: 		}
  687: 	}
  688: 
  689: 	/* Find the solution */
  690: 	num_to_check = bm->K_num_tot_sp * 2;
  691: 	Doolittle_LU_Decomposition(*lhsmat, num_to_check);
  692: 	Doolittle_LU_Solve(*lhsmat, rhsmat, t, num_to_check);
  693: 
  694: 	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
```

**Suggested fix (from review):** `pred*2+stage`, loop to 2, check return

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

## DIE-6

**copy-paste** · Conf M / Sev M  
**Lines (current code):** 767-784  
**Lines at review (Oct 2026):** 767-784

**Why it's thought to be a bug**

Detritus preference is always written to `allStage=0`, so adults never eat detritus.

**Current code** (`atassess/atdiet.c` lines 765–786)

```c
  765: 
  766: 		case AGE_STRUCTURED:
  767: 			for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
  768: 				for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
  769: 					switch (FunctGroupArray[prey].groupAgeType) {
  770: 					case AGE_STRUCTURED: /* intentional follow through */
  771: 						//case AGE_STRUCTURED_BIOMASS:
  772: 						for (preyStage = 0; preyStage < FunctGroupArray[prey].numStages; preyStage++) {
  773: 							dietpref[pred][predStage][prey][preyStage] = bm->pSPVERTeat[pred][prey][predStage][preyStage];
  774: 
  775: 						}
  776: 						break;
  777: 					case BIOMASS:
  778: 					case AGE_STRUCTURED_BIOMASS:
  779: 						allStage = 0;
  780: 						if (FunctGroupArray[prey].isDetritus == TRUE) {
  781: 							/* allow for the sediment and wc values */
  782: 							for (habitat = WC; habitat <= SED; habitat++) {
  783: 								dietpref[pred][allStage][prey][habitat] = FunctGroupArray[pred].pSPEat[allStage][prey][habitat];
  784: 							}
  785: 						} else {
  786: 							dietpref[pred][predStage][prey][WC] = FunctGroupArray[pred].pSPEat[predStage][prey][WC];
```

**Suggested fix (from review):** use `predStage`

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

## DIE-7

**wrong variable** · Conf M / Sev L  
**Lines (current code):** 804-807  
**Lines at review (Oct 2026):** 804-807

**Why it's thought to be a bug**

`pSPEat[predStage][prey][preyStage]`: the 3rd index is habitat, not stage; `/2` is hard-coded.

**Current code** (`atassess/atdiet.c` lines 802–809)

```c
  802: 						sum = 0.0;
  803: 						allStage = 0;
  804: 						for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
  805: 							sum += FunctGroupArray[pred].pSPEat[predStage][prey][preyStage];
  806: 						}
  807: 						dietpref[pred][allStage][prey][preyStage] = sum / 2;
  808: 					}
  809: 					break;
```

**Suggested fix (from review):** use `pSPVERTeat`

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

# atassess/atdiet.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 3; confidence H and severity H: 3; on the priority list: 3).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [DIE-1](#die-1) ⭐ | 137 | H/H | accumulator in wrong loop | `Calculate_Overall_Diet_Composition` runs inside the box loop on running totals, so early boxes are counted ma… |  | [ ] |
| [DIE-2](#die-2) | 163 | M/M | wrong slot | Prey availability uses `biolbiom[prey][b][WC]` only, so infauna and sediment bacteria are never eaten (see DAT… |  | [ ] |
| [DIE-3](#die-3) ✔ ⭐ | 243 | H/H | swapped dims in Init | Allocated `Util_Alloc_Init_2D_Double(2, K)` = `[K][2]`, but reset with `Util_Init_2D_Double(contribvert, 2, K)… |  | [ ] |
| [DIE-4](#die-4) | 549, 552 | H/L | misplaced break | `if (checkdone) break;` sits inside the same inner loop, so it never fires (performance only). |  | [ ] |
| [DIE-5](#die-5) ⭐ | 668 | H/H | wrong variable | Diagonal uses `pred*numStages+stage`; the rest of the matrix uses `pred*2+stage`. Detritus rows stay zero, so … |  | [ ] |
| [DIE-6](#die-6) | 779 | M/M | copy-paste | Detritus preference is always written to `allStage=0`, so adults never eat detritus. |  | [ ] |
| [DIE-7](#die-7) | 805 | M/L | wrong variable | `pSPEat[predStage][prey][preyStage]`: the 3rd index is habitat, not stage; `/2` is hard-coded. |  | [ ] |

---

## DIE-1

**accumulator in wrong loop** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atdiet.c` 137
**Lines at review (Oct 2026):** 115-138

**Why it's thought to be a bug**

`Calculate_Overall_Diet_Composition` runs inside the box loop on running totals, so early boxes are counted many times (atattribute.c:1115 does it after the loop).

**Code at issue** — `atdiet.c` line 137 (located by name only; check this is the right place)  
Search for: `Calculate_Overall_Diet_Composition(bm, totareadiet, inshorediet, offshorediet);`

```c
  135: 
  136: 		/* Overall diet composition */
  137→ 		Calculate_Overall_Diet_Composition(bm, totareadiet, inshorediet, offshorediet);
  138: 	}
  139: 
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
**Found at (current code):** `atdiet.c` 163
**Lines at review (Oct 2026):** 162-164

**Why it's thought to be a bug**

Prey availability uses `biolbiom[prey][b][WC]` only, so infauna and sediment bacteria are never eaten (see DAT-1).

**Code at issue** — `atdiet.c` line 163  
Search for: `biolbiom[prey][b][WC]`

```c
  161: 
  162: 			} else {
  163→ 				availfood[pred][predStage][prey][WC] = biolbiom[prey][b][WC] * dietpref[pred][predStage][prey][WC];
  164: 			}
  165: 		}
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
**Found at (current code):** `atdiet.c` 243
**Lines at review (Oct 2026):** 243 (alloc atsamplesetup.c:277)

**Why it's thought to be a bug**

Allocated `Util_Alloc_Init_2D_Double(2, K)` = `[K][2]`, but reset with `Util_Init_2D_Double(contribvert, 2, K)`, which loops `[0..1][0..K-1]` and covers only flat elements 0..K+1. Entries for pred ≥ (K+2)/2 are never reset and accumulate.

**Code at issue** — `atdiet.c` line 243 (the line the review pointed to, found by its text)  
Search for: `Util_Init_2D_Double(contribvert, 2, bm->K_num_tot_sp, 0.0);`

```c
  241: 
  242: 	/* Could possibly just allocate this here instead of it being a global */
  243→ 	Util_Init_2D_Double(contribvert, 2, bm->K_num_tot_sp, 0.0);
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
**Found at (current code):** `atdiet.c` 549, `atdiet.c` 552
**Lines at review (Oct 2026):** 547-555

**Why it's thought to be a bug**

`if (checkdone) break;` sits inside the same inner loop, so it never fires (performance only).

**Code at issue** — `atdiet.c` line 549 (the line the review pointed to, found by its text)  
Search for: `checkdone = 1;`

```c
  547: 				if ((lower <= u) && (u < upper)){
  548: 					count[bin][j]++;
  549→                     checkdone = 1;
  550: 					break;
  551: 				}
```

**Code at issue** — `atdiet.c` line 552 (the line the review pointed to, found by its text)  
Search for: `if (checkdone) {`

```c
  550: 					break;
  551: 				}
  552→                 if (checkdone) {
  553:                     break;
  554:                 }
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
**Found at (current code):** `atdiet.c` 668
**Lines at review (Oct 2026):** 667-671, 690-692

**Why it's thought to be a bug**

Diagonal uses `pred*numStages+stage`; the rest of the matrix uses `pred*2+stage`. Detritus rows stay zero, so the matrix is singular, and Doolittle's -1 return is ignored.

**Code at issue** — `atdiet.c` line 668 (the line the review pointed to, found by its text)  
Search for: `for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {`

```c
  666: 
  667: 	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
  668→ 		for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
  669: 			lhsmat[(pred * FunctGroupArray[pred].numStages) + predStage][(pred * FunctGroupArray[pred].numStages) + predStage] = 1.0;
  670: 		}
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
**Found at (current code):** `atdiet.c` 779
**Lines at review (Oct 2026):** 767-784

**Why it's thought to be a bug**

Detritus preference is always written to `allStage=0`, so adults never eat detritus.

**Code at issue** — `atdiet.c` line 779  
Search for: `allStage = 0`

```c
  777: 					case BIOMASS:
  778: 					case AGE_STRUCTURED_BIOMASS:
  779→ 						allStage = 0;
  780: 						if (FunctGroupArray[prey].isDetritus == TRUE) {
  781: 							/* allow for the sediment and wc values */
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
**Found at (current code):** `atdiet.c` 805
**Lines at review (Oct 2026):** 804-807

**Why it's thought to be a bug**

`pSPEat[predStage][prey][preyStage]`: the 3rd index is habitat, not stage; `/2` is hard-coded.

**Code at issue** — `atdiet.c` line 805  
Search for: `pSPEat[predStage][prey][preyStage]`

```c
  803: 						allStage = 0;
  804: 						for (predStage = 0; predStage < FunctGroupArray[pred].numStages; predStage++) {
  805→ 							sum += FunctGroupArray[pred].pSPEat[predStage][prey][preyStage];
  806: 						}
  807: 						dietpref[pred][allStage][prey][preyStage] = sum / 2;
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

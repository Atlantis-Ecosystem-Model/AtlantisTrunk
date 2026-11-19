# atassess/atdatareading.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [DAT-1](#dat-1) ⭐ | 332 | H/H | wrong slot | SED_BACT, SM_INF, LG_INF, MPB and sediment PHYTOBEN/TURF are stored in `biolbiom[..][SED]`; all non-detritus r… |  | [ ] |
| [DAT-2](#dat-2) ✔ ⭐ | 227 | H/H | never reset | `zonearea[z] +=` runs every sampling step; it is zeroed only once (atsamplesetup.c:412). |  | [ ] |
| [DAT-3](#dat-3) | 261 | M/M | units | Salinity, light, temperature and O2 are summed over layers, not depth-averaged (Chl a on line 299 is weighted)… |  | [ ] |
| [DAT-4](#dat-4) | 536 | L/M | units | Production is multiplied by numbers but consumption is not. |  | [ ] |
| [DAT-5](#dat-5) | 455 | H/L | reused variable | Zone variable `z` is reused as a cohort loop index. |  | [ ] |
| [DAT-6](#dat-6) | 550 | M/L | wrong condition | Verbose print uses `isGrazer == FALSE` (should be TRUE). |  | [ ] |

---

## DAT-1

**wrong slot** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atdatareading.c` 332
**Lines at review (Oct 2026):** 325-333, 348-355

**Why it's thought to be a bug**

SED_BACT, SM_INF, LG_INF, MPB and sediment PHYTOBEN/TURF are stored in `biolbiom[..][SED]`; all non-detritus readers use `[WC]`, so they read zero.

**Code at issue** — `atdatareading.c` line 332 (closest match to the quoted code; wording differs slightly)  
Search for: `biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b]`

```c
  330: 							/* Sediment biomass data */
  331: 							for (k = 0; k < bm->sednz; k++)
  332→ 								biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].sm.dz[k];
  333: 
  334: 						break;
```

**Suggested fix (from review):** store in WC, or read both

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

## DAT-2

**never reset** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atdatareading.c` 227
**Lines at review (Oct 2026):** 226-227

**Why it's thought to be a bug**

`zonearea[z] +=` runs every sampling step; it is zeroed only once (atsamplesetup.c:413).

**Code at issue** — `atdatareading.c` line 227  
Search for: `zonearea[z] +=`

```c
  225: 			}
  226: 			z = bm->boxes[b].zone - 1;
  227→ 			zonearea[z] += bm->boxes[b].area;
  228: 
  229: 			/* Move physical data from storage to data arrays
```

**Suggested fix (from review):** reset in Init_Data_Array

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

## DAT-3

**units** · Conf M / Sev M  
**Found at (current code):** `atdatareading.c` 261
**Lines at review (Oct 2026):** 261-273

**Why it's thought to be a bug**

Salinity, light, temperature and O2 are summed over layers, not depth-averaged (Chl a on line 299 is weighted).

**Code at issue** — `atdatareading.c` line 261 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for (k = 0; k < bm->wcnz; k++) {`

```c
  259: 				}
  260: 
  261→ 				for (k = 0; k < bm->wcnz; k++) {
  262: 					phys[salinity_id][b] += bm->wctr[b][k][Salinity_i];
  263: 					phys[light_wc_id][b] += bm->wctr[b][k][Light_i];
```

**Suggested fix (from review):** weight by dz/depth

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

## DAT-4

**units** · Conf L / Sev M  
**Found at (current code):** `atdatareading.c` 536
**Lines at review (Oct 2026):** 536-538

**Why it's thought to be a bug**

Production is multiplied by numbers but consumption is not.

**Code at issue** — `atdatareading.c` line 536 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `biolVERTinfo[bprod_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].GrowthTracers`

```c
  534: 					if (FunctGroupArray[sp].isVertebrate == TRUE) {
  535: 						for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
  536→ 							biolVERTinfo[bprod_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].GrowthTracers[cohort]]
  537: 									* biolVERTinfo[bstocknums_id][sp][cohort][b] * (-1.0 * bm->boxes[b].botz);
  538: 							biolVERTinfo[beat_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].EatTracers[cohort]] * (-1.0 * bm->boxes[b].botz);
```

**Suggested fix (from review):** confirm units

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

## DAT-5

**reused variable** · Conf H / Sev L  
**Found at (current code):** `atdatareading.c` 455
**Lines at review (Oct 2026):** 474

**Why it's thought to be a bug**

Zone variable `z` is reused as a cohort loop index.

**Code at issue** — `atdatareading.c` line 455 (located by name only; check this is the right place)  
Search for: `stage = FunctGroupArray[sp].cohort_stage[cohort];`

```c
  453: 							if ((biolVERTinfo[bstruct_id][sp][cohort][b]) && (biolVERTinfo[bres_id][sp][cohort][b])
  454: 									&& (biolVERTinfo[bstocknums_id][sp][cohort][b])) {
  455→ 								stage = FunctGroupArray[sp].cohort_stage[cohort];
  456: 								bm->rep_box_of_zone[sp][z][stage] = b;
  457: 							}
```

**Suggested fix (from review):** use `cohort`

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

## DAT-6

**wrong condition** · Conf M / Sev L  
**Found at (current code):** `atdatareading.c` 550
**Lines at review (Oct 2026):** 550

**Why it's thought to be a bug**

Verbose print uses `isGrazer == FALSE` (should be TRUE).

**Code at issue** — `atdatareading.c` line 550  
Search for: `isGrazer == FALSE`

```c
  548: 					}
  549: 					for (k = 0; k < bm->K_num_tot_sp; k++) {
  550→ 						if (FunctGroupArray[k].isVertebrate == FALSE && FunctGroupArray[k].isGrazer == FALSE) {
  551: 							fprintf(ofp, "bioleat[%d][%d]: %e\n", k, b, bioleat[k][b]);
  552: 						}
```

**Suggested fix (from review):** flip

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

# atassess/atdatareading.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [DAT-1](#dat-1) ⭐ | 325-333, 348-355 | H/H | wrong slot | SED_BACT, SM_INF, LG_INF, MPB and sediment PHYTOBEN/TURF are stored in `biolbiom[..][SED]`; all non-detritus r… |  | [ ] |
| [DAT-2](#dat-2) ✔ ⭐ | 226-227 | H/H | never reset | `zonearea[z] +=` runs every sampling step; it is zeroed only once (atsamplesetup.c:412). |  | [ ] |
| [DAT-3](#dat-3) | 261-273 | M/M | units | Salinity, light, temperature and O2 are summed over layers, not depth-averaged (Chl a on line 299 is weighted)… |  | [ ] |
| [DAT-4](#dat-4) | 536-538 | L/M | units | Production is multiplied by numbers but consumption is not. |  | [ ] |
| [DAT-5](#dat-5) | 474 | H/L | reused variable | Zone variable `z` is reused as a cohort loop index. |  | [ ] |
| [DAT-6](#dat-6) | 550 | M/L | wrong condition | Verbose print uses `isGrazer == FALSE` (should be TRUE). |  | [ ] |

---

## DAT-1

**wrong slot** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 325-333, 348-355  
**Lines at review (Oct 2026):** 325-333, 348-355

**Why it's thought to be a bug**

SED_BACT, SM_INF, LG_INF, MPB and sediment PHYTOBEN/TURF are stored in `biolbiom[..][SED]`; all non-detritus readers use `[WC]`, so they read zero.

**Current code** (`atassess/atdatareading.c` lines 323–357)

```c
  323: 						/* ignore the vertebrates and age structured fished invertebrates (for now) */
  324: 						break;
  325: 					case SED_BACT:
  326: 					case SM_INF:
  327: 					case LG_INF:
  328: 					case MICROPHTYBENTHOS:
  329: 						if (bm->flagecosysassess)
  330: 							/* Sediment biomass data */
  331: 							for (k = 0; k < bm->sednz; k++)
  332: 								biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].sm.dz[k];
  333: 
  334: 						break;
  335: 
  336: 					case SEAGRASS:
  337: 
  338: 						if (bm->flag_macro_model) {
  339: 							/* Epibenthic biomass data */
  340: 							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[main_biomass_id]];
  341: 							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[below_ground_biomass_id]];
  342: 							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[epiphyte_biomass_id]];
  343: 						}else{
  344: 							biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[0]];
  345: 						}
  346: 						break;
  347: 
  348: 					case PHYTOBEN:
  349: 					case TURF:
  350: 						if (bm->flagecosysassess) {
  351: 							if (FunctGroupArray[sp].habitatType == SED)
  352: 								/* Sediment biomass data */
  353: 								for (k = 0; k < bm->sednz; k++){
  354: 									biolbiom[sp][b][SED] += bm->sedtr[b][k][FunctGroupArray[sp].totNTracers[0]] * bm->boxes[b].sm.dz[k];
  355: 								}
  356: 							else {
  357: 								biolbiom[sp][b][WC] += bm->epi[b][FunctGroupArray[sp].totNTracers[0]];
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
**Lines (current code):** 226-227  
**Lines at review (Oct 2026):** 226-227

**Why it's thought to be a bug**

`zonearea[z] +=` runs every sampling step; it is zeroed only once (atsamplesetup.c:413).

**Current code** (`atassess/atdatareading.c` lines 224–229)

```c
  224: 				sed_dz += bm->boxes[b].sm.dz[k];
  225: 			}
  226: 			z = bm->boxes[b].zone - 1;
  227: 			zonearea[z] += bm->boxes[b].area;
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
**Lines (current code):** 261-273  
**Lines at review (Oct 2026):** 261-273

**Why it's thought to be a bug**

Salinity, light, temperature and O2 are summed over layers, not depth-averaged (Chl a on line 299 is weighted).

**Current code** (`atassess/atdatareading.c` lines 259–275)

```c
  259: 				}
  260: 
  261: 				for (k = 0; k < bm->wcnz; k++) {
  262: 					phys[salinity_id][b] += bm->wctr[b][k][Salinity_i];
  263: 					phys[light_wc_id][b] += bm->wctr[b][k][Light_i];
  264: 					phys[don_wc_id][b] += bm->wctr[b][k][DON_i];
  265: 					phys[din_wc_id][b] += (bm->wctr[b][k][NH3_i] + bm->wctr[b][k][NO3_i]);
  266: 					phys[vol_id][b] += bm->boxes[b].volume[k];
  267: 					phys[temperature_id][b] += bm->wctr[b][k][Temp_i];
  268: 					phys[o2_wc_id][b] += bm->wctr[b][k][Oxygen_i];
  269:                     
  270:                     if(bm->flagIsEstuary)
  271:                         phys[TSS_id][b] += bm->wctr[b][k][SED_i];
  272:                     
  273: 				}
  274: 
  275: 				/* If verbose enough read back the data values */
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
**Lines (current code):** 536-538  
**Lines at review (Oct 2026):** 536-538

**Why it's thought to be a bug**

Production is multiplied by numbers but consumption is not.

**Current code** (`atassess/atdatareading.c` lines 534–540)

```c
  534: 					if (FunctGroupArray[sp].isVertebrate == TRUE) {
  535: 						for (cohort = 0; cohort < FunctGroupArray[sp].numCohortsXnumGenes; cohort++) {
  536: 							biolVERTinfo[bprod_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].GrowthTracers[cohort]]
  537: 									* biolVERTinfo[bstocknums_id][sp][cohort][b] * (-1.0 * bm->boxes[b].botz);
  538: 							biolVERTinfo[beat_id][sp][cohort][b] += bm->diagnost[b][FunctGroupArray[sp].EatTracers[cohort]] * (-1.0 * bm->boxes[b].botz);
  539: 						}
  540: 					}
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
**Lines (current code):** 474  
**Lines at review (Oct 2026):** 474

**Why it's thought to be a bug**

Zone variable `z` is reused as a cohort loop index.

**Current code** (`atassess/atdatareading.c` lines 472–476) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  472: 				for (k = 0; k < bm->K_num_tot_sp; k++) {
  473: 					if (FunctGroupArray[k].isVertebrate == TRUE) {
  474: 						for (z = 0; z < FunctGroupArray[k].numCohortsXnumGenes; z++)
  475: 							for (ij = 0; ij < 7; ij++)
  476: 								fprintf(ofp, "biolVERT[%d][%d][%d][%d]: %e\n", ij, k, z, b, biolVERTinfo[ij][k][z][b]);
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
**Lines (current code):** 550  
**Lines at review (Oct 2026):** 550

**Why it's thought to be a bug**

Verbose print uses `isGrazer == FALSE` (should be TRUE).

**Current code** (`atassess/atdatareading.c` lines 548–552)

```c
  548: 					}
  549: 					for (k = 0; k < bm->K_num_tot_sp; k++) {
  550: 						if (FunctGroupArray[k].isVertebrate == FALSE && FunctGroupArray[k].isGrazer == FALSE) {
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

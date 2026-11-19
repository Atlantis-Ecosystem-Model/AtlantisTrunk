# atecology/atbiolsetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BSET-01](#bset-01) ⭐ | 3161-3170 | H/H | wrong accumulator | `dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is … |  | [ ] |
| [BSET-02](#bset-02) ⭐ | 3187-3197 | H/H | wrong index / accumulator | In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test… |  | [ ] |
| [BSET-03](#bset-03) | 1902-1907 | H/M | double normalisation | `if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg… |  | [ ] |
| [BSET-04](#bset-04) | 892-895 | H/M | copy-paste / ordering | `speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never re… |  | [ ] |
| [BSET-05](#bset-05) | 675-676 | H/L | format mismatch | `quit("... group %s must use prescribed movement only. %s\n", FunctGroupArray[b].groupCode);` has two `%s` but… |  | [ ] |
| [BSET-06](#bset-06) | 3929, 3941-3951, 3996-3998 | M/M | uninitialised index | `speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `matur… |  | [ ] |
| [BSET-07](#bset-07) | 3838-3842 vs 2626-2652, 3996 | M/M | alloc/use mismatch | `Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution… |  | [ ] |
| [BSET-08](#bset-08) | 1876-1884, 1911 | M/L | out-of-range struct use | The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` … |  | [ ] |
| [BSET-09](#bset-09) | 1452-1453 | M/L | NULL/negative index | When verbose > 1, the code prints `FunctGroupArray[guild].groupCode` even if the tracer wasn't found (`guild =… |  | [ ] |
| [BSET-10](#bset-10) | 1743-1748 | M/L | double counting | In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1627… |  | [ ] |
| [BSET-11](#bset-11) | 3593-3602 | M/L | cohort vs cohort×genotype / negative index | `nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGene… |  | [ ] |
| [BSET-12](#bset-12) | 529-535 vs 2899, 3011-3023, 2827, 951 | M/M | alloc size | `bm->maxspage` is the largest `ageClassSize` over vertebrates only. `bm->tempPopRatio` (and `readinpopratio`, … |  | [ ] |
| [BSET-13](#bset-13) | 564 | M/L | off-by-one | `if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` pass… |  | [ ] |
| [BSET-14](#bset-14) | 3363-3365, 3407-3411 | L/L | bounds check after write | `ids[idnum]` is written before checking `n > nbox`. In the first branch (3363) there is no check at all, and i… |  | [ ] |
| [BSET-15](#bset-15) | 2924 vs 2353 | H/L | memory leak | `totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` … |  | [ ] |
| [BSET-16](#bset-16) | 3821 vs 2582-2617 | H/L | memory leak | `EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it. |  | [ ] |

---

## BSET-01

**wrong accumulator** · Conf H / Sev H ⭐ priority  
**Listed lines:** 3161-3170

**Function:** `Setup_Invade()`

**Why it's thought to be a bug**

`dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is meant to find the largest distance from the entry box to any box. Because it uses `+=`, it adds up the distances to all boxes instead (and `dist_step1` is never reset). That makes `maxdist`, and with it `InvaderSatDay` and `Invadendt` (the size of `InvadeArray`), much too large. Function: `Setup_Invade()`.

**Current code** (`atecology/atbiolsetup.c` lines 3325–3338) — code appears to have moved +166 lines since the review

```c
 3325: static void Setup_Invade(MSEBoxModel *bm) {
 3326: 	int b, chkbox, nb = -1, ndist, k, sp_move;
 3327: 	double xdiff = 0.0, ydiff = 0.0, avgdist = 0.0, basedt = 0.0, dt = 0.0, dist_step1 = 0.0, maxdist = 0.0, nsteps = 0.0;
 3328:     
 3329:     if (verbose)
 3330:         printf("Setup_Invade\n");
 3331: 
 3332: 	/* Don't bother setting things up if never invading or can't spread as have zero speed */
 3333: 	if ((bm->invading_sp_model == no_invader) || (!bm->InvaderSpeed))
 3334: 		return;
 3335:     
 3336:     bm->invader_spread_scalars = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);  // Just make in nbox so don't have to try to map chbox to box_id etc (just easier even if slightly higher memory footprint)
 3337:     
 3338: 	/* Rescale InvaderScalar to a per bm->dt rate */
```

**Suggested fix (from review):** `dist_step1 = sqrt(...)`

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

## BSET-02

**wrong index / accumulator** · Conf H / Sev H ⭐ priority  
**Listed lines:** 3187-3197

**Function:** `Setup_Invade()`

**Why it's thought to be a bug**

In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test `dist_step1 < InvaderSpeed*Invadedt*86400` is effectively always false. Even when it is true, it writes `InvadeArray[b][k]` (the source box, which is already > 0) rather than the neighbour `nb`, and it writes time step `k` rather than `k+1`. The invasion therefore never spreads past the entry box until the forced saturation at the last step. Function: `Setup_Invade()`.

**Current code** (`atecology/atbiolsetup.c` lines 3331–3345) — code appears to have moved +146 lines since the review

```c
 3331: 
 3332: 	/* Don't bother setting things up if never invading or can't spread as have zero speed */
 3333: 	if ((bm->invading_sp_model == no_invader) || (!bm->InvaderSpeed))
 3334: 		return;
 3335:     
 3336:     bm->invader_spread_scalars = Util_Alloc_Init_2D_Double(bm->wcnz, bm->nbox, 0.0);  // Just make in nbox so don't have to try to map chbox to box_id etc (just easier even if slightly higher memory footprint)
 3337:     
 3338: 	/* Rescale InvaderScalar to a per bm->dt rate */
 3339: 	nsteps = 365.0 * 86400.0 / bm->dt;
 3340: 	bm->InvaderScalar = (bm->InvaderScalar - 1.0) / nsteps;
 3341: 
 3342: 	/* Start with functional group tracer ID - get youngest cohort for all
 3343: 	 as actually only used for invertebrates, any vertebrates have a
 3344: 	 cohort specific call to FunctGroupArray[].NumsTracers[] in
 3345: 	 the Invading_Species()
```

**Suggested fix (from review):** Compute distance per neighbour (`dist = sqrt(...)`) and set `InvadeArray[nb][k+1]` (when `k+1 < Invadendt`); also carry earlier values forward

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

## BSET-03

**double normalisation** · Conf H / Sev M  
**Listed lines:** 1902-1907

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

`if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg] / (regtot + small_num);`: `initreg_prop` was already turned into a proportion at line 1900. `lastreg_prop` was set to 0 at line 1541, so the condition is always true at init. With manage_reg on, the proportions are divided by the total biomass a second time and become close to 0. Function: `Ecology_Init_Populations()`.

**Current code** (`atecology/atbiolsetup.c` lines 1900–1909)

```c
 1900: 			bm->initreg_prop[sp][nreg] = bm->initreg_prop[sp][nreg] / (regtot + small_num);
 1901: 		}
 1902: 		if (bm->manage_reg) {
 1903: 			for (nreg = 0; nreg < bm->K_num_reg; nreg++) {
 1904: 				if (bm->lastreg_prop[sp][nreg] < no_quota) { // So can have regions where no TAC used
 1905: 					bm->initreg_prop[sp][nreg] = bm->initreg_prop[sp][nreg] / (regtot + small_num);
 1906: 				}
 1907: 			}
 1908: 		}
 1909:         
```

**Suggested fix (from review):** Remove the second division (or set no-TAC regions to 0 and renormalise across the rest)

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

## BSET-04

**copy-paste / ordering** · Conf H / Sev M  
**Listed lines:** 892-895

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never reaches the stored value. The horizontal block above (881-883) clamps first and then stores. If `wcnz == 0` the value stays `-MAXDOUBLE`, and if all `v_stock_struct` entries are 0 it is 0, which feeds into the stock-structure logic. Function: `Ecology_Init()`.

**Current code** (`atecology/atbiolsetup.c` lines 890–897)

```c
  890: 				}
  891: 			}
  892: 			FunctGroupArray[sp].speciesParams[vstockstruct_id] = sum;
  893: 			/* Set number of stocks to a minimum of 1 */
  894: 			if (sum < 1)
  895: 				sum = 1;
  896:             
  897:             /* Check total number of stocks */
```

**Suggested fix (from review):** Move the clamp above the assignment

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

## BSET-05

**format mismatch** · Conf H / Sev L  
**Listed lines:** 675-676

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`quit("... group %s must use prescribed movement only.  %s\n", FunctGroupArray[b].groupCode);` has two `%s` but only one argument. This is undefined behaviour and can crash while reporting the error. Function: `Ecology_Init()`.

**Current code** (`atecology/atbiolsetup.c` lines 673–678)

```c
  673: 				fprintf(bm->logFile, "Ecology_Init: Land based group %s must use prescribed movement only.\n",
  674: 											FunctGroupArray[b].groupCode);
  675: 				quit("Ecology_Init: Land based group %s must use prescribed movement only.  %s\n",
  676: 						FunctGroupArray[b].groupCode);
  677: 			}
  678: 		}
```

**Suggested fix (from review):** Remove the second `%s`

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

## BSET-06

**uninitialised index** · Conf M / Sev M  
**Listed lines:** 3929, 3941-3951, 3996-3998

**Function:** `Create_Evolution_Parameters()`

**Why it's thought to be a bug**

`speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `maturation_trait_id` ("intentionally not handled"). If `DNA[sp].trait_active[]` is set for one of these traits, the loop still reads `speciesParams[-1]` (out of bounds) or reuses the previous trait's index. Function: `Create_Evolution_Parameters()`.

**Current code** (`atecology/atbiolsetup.c` lines 4165–4169) — code appears to have moved +238 lines since the review

```c
 4165:         
 4166:         switch (traitIndex) {
 4167:             case growth_trait_id:  		/* Growth rate */
 4168:                 /* Intentionally not handled - this functionality is covered by other code */
 4169:                 break;
```

**Suggested fix (from review):** `continue` the trait loop for unhandled traits (or skip when `speciesParamIndex < 0`)

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

## BSET-07

**alloc/use mismatch** · Conf M / Sev M  
**Listed lines:** 3838-3842 vs 2626-2652, 3996

**Why it's thought to be a bug**

`Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution_Parameters()` (on `flag_do_evolution`) and `Free_Evolution()` (on `flag_do_evolution || flag_mult_grow_curves`) still dereference `DNA[sp]`, so the run crashes if evolution or multiple growth curves are on with one genotype. The reverse case also leaks: when genotypes ≥ 2 but neither flag is set, all `DNA` arrays are allocated and never freed. Functions: `Ecology_Setup_Evolution()`, `Free_Evolution()`.

**Current code** (`atecology/atbiolsetup.c` lines 4061–4069) — code appears to have moved +225 lines since the review

```c
 4061:  *
 4062:  */
 4063: void Ecology_Setup_Evolution(MSEBoxModel *bm, FILE *llogfp) {
 4064: 	int sp = 0, gene_cohort = 0, num_ages = 0, ntrait = 0, cohort  = 0;
 4065: 
 4066: 	if(bm->K_num_max_genetypes < 2)  // Nothing to do
 4067: 		return;
 4068: 
 4069: 	// Create DNA data structure
```

**Suggested fix (from review):** Use the same condition for allocation and free, or quit on an inconsistent configuration

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

## BSET-08

**out-of-range struct use** · Conf M / Sev L  
**Listed lines:** 1876-1884, 1911

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` and reads `FunctGroupArray[sp].habitatType`. `FunctGroupArray` is malloc'd with K_num_tot_sp+1 entries (atlantisUtil/atUtilGroupIO.c:252), so this entry exists but was never initialised. The log gets a garbage name, and the read is undefined behaviour. Function: `Ecology_Init_Populations()`.

**Current code** (`atecology/atbiolsetup.c` lines 1874–1913)

```c
 1874:     }
 1875: 
 1876:     for (sp = 0; sp < bm->K_num_tot_sp + 1; sp++) {
 1877: 		bm->totinitpop[sp] = bm->totbiom[sp];
 1878: 		if (sp < bm->K_num_tot_sp)
 1879: 			bm->estinitpop[sp] = bm->totbiom[sp] * bm->X_CN * mg_2_tonne;
 1880: 		if (sp == bm->K_num_tot_sp || ((FunctGroupArray[sp].isDetritus == TRUE || (FunctGroupArray[sp].isDetritus == FALSE
 1881: 				&& (int) (FunctGroupArray[sp].speciesParams[flag_id]))))) {
 1882: 			fprintf(llogfp, "Time: %e, species %s virgin biomass is %f t\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->totinitpop[sp] * bm->X_CN
 1883: 					* mg_2_tonne);
 1884: 		}
 1885: 
 1886: 		/** Regional contributions **/
 1887: 		/* As no migrators considered for the regional proportions - as yet - then
 1888: 		 sum here as regtot may not equal totbiom (which includes migrators)
 1889: 		 */
 1890: 		regtot = 0;
 1891: 		for (nreg = 0; nreg < bm->K_num_reg; nreg++) {
 1892:             /* CHANGE In OUTPUT - this if statement changes biomreg.txt output */
 1893:             // if (bm->initreg_prop[sp][nreg] < no_quota) // Filter out no participating regions
 1894: 			regtot += bm->initreg_prop[sp][nreg];
 1895: 		}
 1896: 		/* Make reg_prop trully proportions */
 1897: 		for (nreg = 0; nreg < bm->K_num_reg; nreg++) {
 1898:             /* CHANGE In OUTPUT - this if statement changes biomreg.txt output */
 1899:             // if (bm->initreg_prop[sp][nreg] < no_quota) // Filter out no participating regions
 1900: 			bm->initreg_prop[sp][nreg] = bm->initreg_prop[sp][nreg] / (regtot + small_num);
 1901: 		}
 1902: 		if (bm->manage_reg) {
 1903: 			for (nreg = 0; nreg < bm->K_num_reg; nreg++) {
 1904: 				if (bm->lastreg_prop[sp][nreg] < no_quota) { // So can have regions where no TAC used
 1905: 					bm->initreg_prop[sp][nreg] = bm->initreg_prop[sp][nreg] / (regtot + small_num);
 1906: 				}
 1907: 			}
 1908: 		}
 1909:         
 1910:         // For non-mobile invertebrates constrain them to initial distributions - so store the original distribution
 1911:         if(bm->flagconstrain_epiwander && (FunctGroupArray[sp].habitatType == EPIFAUNA) && !FunctGroupArray[sp].sp_geo_move) {
 1912:             pid = FunctGroupArray[sp].totNTracers[0];
 1913:             for (ij = 0; ij < bm->nbox; ij++) {
```

**Suggested fix (from review):** Use the literal "DIN" for the extra slot and guard line 1911 with `sp < K_num_tot_sp`

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

## BSET-09

**NULL/negative index** · Conf M / Sev L  
**Listed lines:** 1452-1453

**Function:** `Set_Active_Flag()`

**Why it's thought to be a bug**

When verbose > 1, the code prints `FunctGroupArray[guild].groupCode` even if the tracer wasn't found (`guild == -1`, the `checkdone < 0` branch). That is an out-of-bounds read at index -1. Function: `Set_Active_Flag()`.

**Current code** (`atecology/atbiolsetup.c` lines 1450–1455)

```c
 1450: 			}
 1451: 
 1452: 			if(verbose > 1)
 1453: 				fprintf(llogfp, "Doing %s -> %s(%d) has Activeflag %d (id %d)\n", trnamelist[ij].name, FunctGroupArray[guild].groupCode, guild, Activeflag[id], id);
 1454: 		}
 1455: 	}
```

**Suggested fix (from review):** Guard with `guild >= 0`

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

## BSET-10

**double counting** · Conf M / Sev L  
**Listed lines:** 1743-1748

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1627 already counted them all, unconditionally. When the retry triggers, `initVERTinfo[sp][n][DEN_id]` (and so `totinitpop`) is too high by the in-domain numbers. Function: `Ecology_Init_Populations()`.

**Current code** (`atecology/atbiolsetup.c` lines 1741–1750)

```c
 1741: 
 1742: 					/* If no individuals present size again with relaxed density constraints */
 1743: 					if (((!anycounted) || (sizeMinMax[min_SN_id] > (MAXDOUBLE / 3.0)))) {
 1744: 						for (ij = 0; ij < bm->nbox; ij++) {
 1745: 							if (bm->boxes[ij].type != BOUNDARY) {
 1746: 								for (k = 0; k < bm->boxes[ij].nz; k++) {
 1747: 									sp_abund += bm->boxes[ij].tr[k][den];
 1748: 
 1749: 									/* Check minimum SN */
 1750: 									if (bm->boxes[ij].tr[k][sn] < sizeMinMax[min_SN_id])
```

**Suggested fix (from review):** Remove the `sp_abund +=` line in the retry loop

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

## BSET-11

**cohort vs cohort×genotype / negative index** · Conf M / Sev L  
**Listed lines:** 3593-3602

**Function:** `Set_spMinMax_Values()`

**Why it's thought to be a bug**

`nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGenes (cohort = age*numGeneTypes+gene; see line 550). With several genotypes, `[nage]` and `[nmat]` pick the wrong age class. If `age_mat_id == 0`, `nage = -1` reads out of bounds. Line 3603 does use numCohortsXnumGenes-1, which is inconsistent. Function: `Set_spMinMax_Values()`.

**Current code** (`atecology/atbiolsetup.c` lines 3805–3818) — code appears to have moved +214 lines since the review

```c
 3805:  */
 3806: static void Set_spMinMax_Values(MSEBoxModel *bm) {
 3807: 	int sp, nage, nmat;
 3808: 
 3809: 	if (verbose > 0)
 3810: 		printf("Get vert ids\n");
 3811: 
 3812: 	/* Get indices to use in movement allocation routines */
 3813: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 3814: 		if ((int) FunctGroupArray[sp].speciesParams[flag_id] == TRUE) {
 3815: 			switch (FunctGroupArray[sp].groupAgeType){
 3816: 			case AGE_STRUCTURED:
 3817: 				/* Remember the indices for smallest and largest sizes of the current fish eaters */
 3818: 				nage = (int) (FunctGroupArray[sp].speciesParams[age_mat_id] - 1);
```

**Suggested fix (from review):** Use `max(0,nage)*numGeneTypes` and `nmat*numGeneTypes`, clamped against numCohorts-1

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

## BSET-12

**alloc size** · Conf M / Sev M  
**Listed lines:** 529-535 vs 2899, 3011-3023, 2827, 951

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`bm->maxspage` is the largest `ageClassSize` over vertebrates only. `bm->tempPopRatio` (and `readinpopratio`, `nSTOCK`, `sumSTOCK`) is allocated with `maxspage` in its age dimension, but `Init_Population_Ratios()` fills `tempPopRatio[..][i]` for `i < ageClassSize` for AGE_STRUCTURED_BIOMASS invertebrates too. If an age-structured invertebrate has a larger `ageClassSize` than every vertebrate, or the model has no vertebrates (maxspage = 0), this writes out of bounds. Function: `Ecology_Init()` / `Init_Population_Ratios()`.

**Current code** (`atecology/atbiolsetup.c` lines 527–537)

```c
  527:     
  528:     /* Determine maximum vertebrate ageclass size */
  529: 	bm->maxspage = 0;
  530: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  531: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
  532: 			spage = (int) (FunctGroupArray[sp].ageClassSize);
  533: 			if (spage > bm->maxspage)
  534: 				bm->maxspage = spage;
  535: 
  536: 			/* Check recruitment matches assumptions behind reproduction model */
  537: 			if (FunctGroupArray[sp].speciesParams[Recruit_Time_id] < 2)
```

**Suggested fix (from review):** Compute maxspage over all AGE_STRUCTURED/AGE_STRUCTURED_BIOMASS groups

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

## BSET-13

**off-by-one** · Conf M / Sev L  
**Listed lines:** 564

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` passes the check and then indexes past stage-sized arrays such as `pSPEat`. Function: `Ecology_Init()`.

**Current code** (`atecology/atbiolsetup.c` lines 562–566)

```c
  562: 			FunctGroupArray[sp].cohort_stage[cohort] = chrtstage;
  563:             
  564:             if (FunctGroupArray[sp].cohort_stage[cohort] > FunctGroupArray[sp].numStages){
  565:                 printf("%s has cohort_stage %d which is > numStages: %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].cohort_stage[cohort], FunctGroupArray[sp].numStages);
  566:                 quit("%s has cohort_stage %d which is > numStages: %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].cohort_stage[cohort], FunctGroupArray[sp].numStages);
```

**Suggested fix (from review):** Use `>=`

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

## BSET-14

**bounds check after write** · Conf L / Sev L  
**Listed lines:** 3363-3365, 3407-3411

**Function:** `Setup_HomeRanges()`

**Why it's thought to be a bug**

`ids[idnum]` is written before checking `n > nbox`. In the first branch (3363) there is no check at all, and in the second the check comes after the write, so `ids[nbox]` can be written before the quit. Function: `Setup_HomeRanges()`.

**Current code** (`atecology/atbiolsetup.c` lines 3474–3513 (truncated)) — code appears to have moved +113 lines since the review

```c
 3474:  */
 3475: static void Setup_HomeRanges(MSEBoxModel *bm, FILE *llogfp) {
 3476: 	int b, sp, sp_ddepend_move, n, den, k, bb, chkbox, idnum, ij, nij, rij, last_checked, new_checked, more_to_do, sides_covered, start_box, surrounded;
 3477: 	double totarea = 0.0, dist = 0.0;
 3478: 	int *connected;
 3479: 
 3480:     if(verbose)
 3481:         printf("Creating Homerange arrays\n");
 3482: 
 3483: 	connected = Util_Alloc_Init_1D_Int(bm->nbox, 0);
 3484: 
 3485: 	/* Initialise home range related counters */
 3486: 	bm->K_num_homerange = 0;
 3487: 	for (b = 0; b < bm->nbox; b++) {
 3488: 		if ((bm->boxes[b].HomeRangeInfo = (HomeRange *) malloc((size_t)bm->K_num_tot_sp * sizeof(HomeRange))) == NULL)
 3489: 			quit("Setup_HomeRanges: Can't allocate memory for HomeRangeInfo in box %d\n", b);
 3490: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 3491: 			bm->boxes[b].HomeRangeInfo[sp].n = 0;
 3492: 			bm->boxes[b].HomeRangeInfo[sp].ids = (int *) i_alloc1d(bm->nbox); // As should never be more home ranges than there are boxes
 3493: 			for (rij = 0; rij < bm->nbox; rij++) {
 3494: 				bm->boxes[b].HomeRangeInfo[sp].ids[rij] = -1;
 3495: 			}
 3496: 		}
 3497: 	}
 3498: 
 3499: 	printf("Constructing home ranges\n");
 3500: 
 3501: 	/* Find max number of home ranges */
 3502: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 3503: 		sp_ddepend_move = (int) (FunctGroupArray[sp].speciesParams[ddepend_move_id]);
 3504: 
 3505: 		//fprintf(llogfp,"%s has move %d vs %d\n", FunctGroupArray[sp].groupCode, sp_ddepend_move, homerange_move);
 3506: 
 3507: 		if (sp_ddepend_move == homerange_move) {
 3508: 
 3509: 			for (ij = 0; ij < bm->nbox; ij++) {
 3510: 				if (bm->boxes[ij].type != BOUNDARY) {
 3511: 
 3512: 					//fprintf(llogfp, "%s depth: %e vs mindepth: %e, maxdepth: %e\n", FunctGroupArray[sp].groupCode, -1.0 * bm->boxes[ij].botz,
 3513: 					//		FunctGroupArray[sp].speciesParams[mindepth_id], FunctGroupArray[sp].speciesParams[maxdepth_id]);
```

**Suggested fix (from review):** Check `idnum >= nbox` before writing

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

## BSET-15

**memory leak** · Conf H / Sev L  
**Listed lines:** 2924 vs 2353

**Why it's thought to be a bug**

`totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` but not `totden_check`.

**Current code** (`atecology/atbiolsetup.c` lines 2914–2918) — code appears to have moved -8 lines since the review

```c
 2914:     totboxden = Util_Alloc_Init_1D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
 2915:     totden = Util_Alloc_Init_2D_Double(totdensize, bm->K_num_tot_sp, 0.0);
 2916:     totden_check = Util_Alloc_Init_2D_Double(totdensize, bm->K_num_tot_sp, 0.0);
 2917:     totroc = Util_Alloc_Init_1D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
 2918:     
```

**Suggested fix (from review):** Add `free2d(totden_check);`

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

## BSET-16

**memory leak** · Conf H / Sev L  
**Listed lines:** 3821 vs 2582-2617

**Why it's thought to be a bug**

`EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it.

**Current code** (`atecology/atbiolsetup.c` lines 3997–4001) — code appears to have moved +178 lines since the review

```c
 3997:  *
 3998:  */
 3999: void Ecology_Setup_Reproduction(MSEBoxModel *bm) {
 4000: 	int sp = 0, cohort = 0;
 4001: 	int max_num = 1;
```

**Suggested fix (from review):** Add `i_free1d(EMBRYO[sp].migIDmatch);`

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

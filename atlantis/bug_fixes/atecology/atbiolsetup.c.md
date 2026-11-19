# atecology/atbiolsetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BSET-01](#bset-01) ⭐ | 3389-3398 | H/H | wrong accumulator | `dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is … |  | [ ] |
| [BSET-02](#bset-02) ⭐ | 3415-3425 | H/H | wrong index / accumulator | In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test… |  | [ ] |
| [BSET-03](#bset-03) | 1900-1905 | H/M | double normalisation | `if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg… |  | [ ] |
| [BSET-04](#bset-04) | 890-893 | H/M | copy-paste / ordering | `speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never re… |  | [ ] |
| [BSET-05](#bset-05) | 673-674 | H/L | format mismatch | `quit("... group %s must use prescribed movement only. %s\n", FunctGroupArray[b].groupCode);` has two `%s` but… |  | [ ] |
| [BSET-06](#bset-06) | 4157, 4169-4179, 4224-4226 | M/M | uninitialised index | `speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `matur… |  | [ ] |
| [BSET-07](#bset-07) | 4066-4070 vs 2629-2655, 4224 | M/M | alloc/use mismatch | `Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution… |  | [ ] |
| [BSET-08](#bset-08) | 1874-1882, 1909 | M/L | out-of-range struct use | The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` … |  | [ ] |
| [BSET-09](#bset-09) | 1452-1453 | M/L | NULL/negative index | When verbose > 1, the code prints `FunctGroupArray[guild].groupCode` even if the tracer wasn't found (`guild =… |  | [ ] |
| [BSET-10](#bset-10) | 1741-1746 | M/L | double counting | In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1627… |  | [ ] |
| [BSET-11](#bset-11) | 3821-3830 | M/L | cohort vs cohort×genotype / negative index | `nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGene… |  | [ ] |
| [BSET-12](#bset-12) | 529-535 vs 2895, 3239-3251, 2824, 949 | M/M | alloc size | `bm->maxspage` is the largest `ageClassSize` over vertebrates only. `bm->tempPopRatio` (and `readinpopratio`, … |  | [ ] |
| [BSET-13](#bset-13) | 562 | M/L | off-by-one | `if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` pass… |  | [ ] |
| [BSET-14](#bset-14) | 3591-3593, 3635-3639 | L/L | bounds check after write | `ids[idnum]` is written before checking `n > nbox`. In the first branch (3363) there is no check at all, and i… |  | [ ] |
| [BSET-15](#bset-15) | 2920 vs 2352 | H/L | memory leak | `totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` … |  | [ ] |
| [BSET-16](#bset-16) | 4049 vs 2585-2620 | H/L | memory leak | `EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it. |  | [ ] |

---

## BSET-01

**wrong accumulator** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 3389-3398  
**Lines at review (Oct 2026):** 3161-3170

**Function:** `Setup_Invade()`

**Why it's thought to be a bug**

`dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is meant to find the largest distance from the entry box to any box. Because it uses `+=`, it adds up the distances to all boxes instead (and `dist_step1` is never reset). That makes `maxdist`, and with it `InvaderSatDay` and `Invadendt` (the size of `InvadeArray`), much too large. Function: `Setup_Invade()`.

**Current code** (`atecology/atbiolsetup.c` lines 3387–3400)

```c
 3387:     nb = bm->InvaderEntryBox;
 3388: 	for (b = 0; b < bm->nbox; b++) {
 3389: 		xdiff = bm->boxes[b].inside.x - bm->boxes[nb].inside.x;
 3390: 		ydiff = bm->boxes[b].inside.y - bm->boxes[nb].inside.y;
 3391: 		dist_step1 += sqrt(xdiff * xdiff + ydiff * ydiff);
 3392:         if (dist_step1 > maxdist) {
 3393:             maxdist = dist_step1;
 3394:         }
 3395: 	}
 3396: 
 3397: 	/* Find time to saturation - maximum distance from Entry box that is in the model domain */
 3398: 	basedt = (maxdist / bm->InvaderSpeed) / (86400.0);
 3399: 	bm->InvaderSatDay = basedt + bm->InvaderStartDay;
 3400: 	bm->Invadendt = (int) ceil(basedt / bm->Invadedt);
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
**Lines (current code):** 3415-3425  
**Lines at review (Oct 2026):** 3187-3197

**Function:** `Setup_Invade()`

**Why it's thought to be a bug**

In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test `dist_step1 < InvaderSpeed*Invadedt*86400` is effectively always false. Even when it is true, it writes `InvadeArray[b][k]` (the source box, which is already > 0) rather than the neighbour `nb`, and it writes time step `k` rather than `k+1`. The invasion therefore never spreads past the entry box until the forced saturation at the last step. Function: `Setup_Invade()`.

**Current code** (`atecology/atbiolsetup.c` lines 3413–3427)

```c
 3413: 			for (b = 0; b < bm->nbox; b++) {
 3414: 				if (bm->InvadeArray[b][k] > 0) {
 3415: 					for (chkbox = 0; chkbox < bm->boxes[b].nconn; chkbox++) {
 3416: 						nb = bm->boxes[b].ibox[chkbox];
 3417: 						xdiff = bm->boxes[b].inside.x - bm->boxes[nb].inside.x;
 3418: 						ydiff = bm->boxes[b].inside.y - bm->boxes[nb].inside.y;
 3419: 						dist_step1 += sqrt(xdiff * xdiff + ydiff * ydiff);
 3420: 						if (dist_step1 < (bm->InvaderSpeed * bm->Invadedt * 86400.0))
 3421: 							bm->InvadeArray[b][k] = bm->InvaderScalar;
 3422: 					}
 3423: 				}
 3424: 				if ((k > (bm->Invadendt - 2)) && (bm->InvadeArray[b][k] < bm->InvaderScalar)) {
 3425: 					/* For last array entry all array should be saturated */
 3426: 					bm->InvadeArray[b][k] = bm->InvaderScalar;
 3427: 				}
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
**Lines (current code):** 1900-1905  
**Lines at review (Oct 2026):** 1902-1907

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

`if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg] / (regtot + small_num);`: `initreg_prop` was already turned into a proportion at line 1898. `lastreg_prop` was set to 0 at line 1539, so the condition is always true at init. With manage_reg on, the proportions are divided by the total biomass a second time and become close to 0. Function: `Ecology_Init_Populations()`.

**Current code** (`atecology/atbiolsetup.c` lines 1898–1907)

```c
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
**Lines (current code):** 890-893  
**Lines at review (Oct 2026):** 892-895

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never reaches the stored value. The horizontal block above (879-881) clamps first and then stores. If `wcnz == 0` the value stays `-MAXDOUBLE`, and if all `v_stock_struct` entries are 0 it is 0, which feeds into the stock-structure logic. Function: `Ecology_Init()`.

**Current code** (`atecology/atbiolsetup.c` lines 888–895)

```c
  888: 				if (bm->v_stock_struct[b][sp] > sum) {
  889: 					sum = bm->v_stock_struct[b][sp];
  890: 				}
  891: 			}
  892: 			FunctGroupArray[sp].speciesParams[vstockstruct_id] = sum;
  893: 			/* Set number of stocks to a minimum of 1 */
  894: 			if (sum < 1)
  895: 				sum = 1;
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
**Lines (current code):** 673-674  
**Lines at review (Oct 2026):** 675-676

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`quit("... group %s must use prescribed movement only.  %s\n", FunctGroupArray[b].groupCode);` has two `%s` but only one argument. This is undefined behaviour and can crash while reporting the error. Function: `Ecology_Init()`.

**Current code** (`atecology/atbiolsetup.c` lines 671–676)

```c
  671: 		if (FunctGroupArray[b].isVertebrate == TRUE) {
  672: 			if ((FunctGroupArray[b].speciesParams[ddepend_move_id] > 1) && (FunctGroupArray[b].habitatCoeffs[LAND_BASED] > 0)){
  673: 				fprintf(bm->logFile, "Ecology_Init: Land based group %s must use prescribed movement only.\n",
  674: 											FunctGroupArray[b].groupCode);
  675: 				quit("Ecology_Init: Land based group %s must use prescribed movement only.  %s\n",
  676: 						FunctGroupArray[b].groupCode);
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
**Lines (current code):** 4157, 4169-4179, 4224-4226  
**Lines at review (Oct 2026):** 3929, 3941-3951, 3996-3998

**Function:** `Create_Evolution_Parameters()`

**Why it's thought to be a bug**

`speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `maturation_trait_id` ("intentionally not handled"). If `DNA[sp].trait_active[]` is set for one of these traits, the loop still reads `speciesParams[-1]` (out of bounds) or reuses the previous trait's index. Function: `Create_Evolution_Parameters()`.

**Current code** (`atecology/atbiolsetup.c` lines 4155–4181)

```c
 4155:     
 4156:     /* Check that ecolution is turned on - else return */
 4157:     if (!bm->flag_do_evolution)
 4158:         return;
 4159:     
 4160:     /* Allocate the arrays */
 4161:     bm->evolTraitValues = Util_Alloc_Init_3D_Double(K_num_traits, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
 4162:     bm->evolTraitOriginalValues = Util_Alloc_Init_3D_Double(K_num_traits, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
 4163:     
 4164:     for (traitIndex = 0; traitIndex < K_num_traits; traitIndex++){
 4165:         
 4166:         switch (traitIndex) {
 4167:             case growth_trait_id:  		/* Growth rate */
 4168:                 /* Intentionally not handled - this functionality is covered by other code */
 4169:                 break;
 4170:             case consumption_trait_id:
 4171:                 /* Intentionally not handled - this functionality is covered by other code */
 4172:                 break;
 4173:             case maturation_trait_id: 	/* Size of maturity */
 4174:                 /* Intentionally not handled - this functionality is covered by other code */
 4175:                 fprintf(stderr , "WARNING - NOT TESTED AT ALL!!!!!!");
 4176:                 break;
 4177:             case reproduce_trait_id:  	/* Reproductive investment - TODO: Will need to make this have a genetic component */
 4178:                 speciesParamIndex = FSP_id;
 4179:                 fprintf(stderr , "WARNING - NOT TESTED AT ALL!!!!!!");
 4180:                 
 4181:                 break;
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
**Lines (current code):** 4066-4070 vs 2629-2655, 4224  
**Lines at review (Oct 2026):** 3838-3842 vs 2626-2652, 3996

**Why it's thought to be a bug**

`Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution_Parameters()` (on `flag_do_evolution`) and `Free_Evolution()` (on `flag_do_evolution || flag_mult_grow_curves`) still dereference `DNA[sp]`, so the run crashes if evolution or multiple growth curves are on with one genotype. The reverse case also leaks: when genotypes ≥ 2 but neither flag is set, all `DNA` arrays are allocated and never freed. Functions: `Ecology_Setup_Evolution()`, `Free_Evolution()`.

**Current code** (`atecology/atbiolsetup.c` lines 4064–4072)

```c
 4064: 	int sp = 0, gene_cohort = 0, num_ages = 0, ntrait = 0, cohort  = 0;
 4065: 
 4066: 	if(bm->K_num_max_genetypes < 2)  // Nothing to do
 4067: 		return;
 4068: 
 4069: 	// Create DNA data structure
 4070: 	//printf("Creating DNA arrays\n");
 4071: 
 4072: 	DNA = (EvolutionStruct *) malloc(sizeof(EvolutionStruct) * (size_t)(bm->K_num_tot_sp + 1));
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
**Lines (current code):** 1874-1882, 1909  
**Lines at review (Oct 2026):** 1876-1884, 1911

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` and reads `FunctGroupArray[sp].habitatType`. `FunctGroupArray` is malloc'd with K_num_tot_sp+1 entries (atlantisUtil/atUtilGroupIO.c:252), so this entry exists but was never initialised. The log gets a garbage name, and the read is undefined behaviour. Function: `Ecology_Init_Populations()`.

**Current code** (`atecology/atbiolsetup.c` lines 1872–1911)

```c
 1872:             tot_dyn_sea_area += bm->boxes[ij].area;
 1873:         }
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
```

**Suggested fix (from review):** Use the literal "DIN" for the extra slot and guard line 1909 with `sp < K_num_tot_sp`

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
**Lines (current code):** 1452-1453  
**Lines at review (Oct 2026):** 1452-1453

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
**Lines (current code):** 1741-1746  
**Lines at review (Oct 2026):** 1743-1748

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1625 already counted them all, unconditionally. When the retry triggers, `initVERTinfo[sp][n][DEN_id]` (and so `totinitpop`) is too high by the in-domain numbers. Function: `Ecology_Init_Populations()`.

**Current code** (`atecology/atbiolsetup.c` lines 1739–1748)

```c
 1739: 						fprintf(llogfp, "inital size of %s:%d-RN is VERY large: %e (cohort (%d) - will retry without looking at numbers present\n", FunctGroupArray[sp].groupCode,  n, sizeMinMax[min_RN_id], n);
 1740: 					}
 1741: 
 1742: 					/* If no individuals present size again with relaxed density constraints */
 1743: 					if (((!anycounted) || (sizeMinMax[min_SN_id] > (MAXDOUBLE / 3.0)))) {
 1744: 						for (ij = 0; ij < bm->nbox; ij++) {
 1745: 							if (bm->boxes[ij].type != BOUNDARY) {
 1746: 								for (k = 0; k < bm->boxes[ij].nz; k++) {
 1747: 									sp_abund += bm->boxes[ij].tr[k][den];
 1748: 
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
**Lines (current code):** 3821-3830  
**Lines at review (Oct 2026):** 3593-3602

**Function:** `Set_spMinMax_Values()`

**Why it's thought to be a bug**

`nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGenes (cohort = age*numGeneTypes+gene; see line 550). With several genotypes, `[nage]` and `[nmat]` pick the wrong age class. If `age_mat_id == 0`, `nage = -1` reads out of bounds. Line 3831 does use numCohortsXnumGenes-1, which is inconsistent. Function: `Set_spMinMax_Values()`.

**Current code** (`atecology/atbiolsetup.c` lines 3819–3832)

```c
 3819: 				nmat = (int) (FunctGroupArray[sp].speciesParams[age_mat_id]);
 3820: 				if(nmat >= FunctGroupArray[sp].numCohorts)  // Need this check in case of unusual parameterisations (otherwise it stomps memory)
 3821: 					nmat = FunctGroupArray[sp].numCohorts - 1;
 3822: 				/* For the vertebrate eaters, set bounds on juvenile and adult size (upper and lower
 3823: 				 bounds on structural weight) so can determine which grey groups can fit through
 3824: 				 their gapes. */
 3825: 				FunctGroupArray[sp].spMinMax[0] = FunctGroupArray[sp].structNTracers[0];
 3826: 				FunctGroupArray[sp].spMinMax[1] = FunctGroupArray[sp].structNTracers[nage];
 3827: 				FunctGroupArray[sp].spMinMax[2] = FunctGroupArray[sp].structNTracers[nmat];
 3828: 				FunctGroupArray[sp].spMinMax[3] = FunctGroupArray[sp].structNTracers[(FunctGroupArray[sp].numCohortsXnumGenes) - 1];
 3829: 				break;
 3830: 			case AGE_STRUCTURED_BIOMASS: /* Done a different way, but may refer to totNTracers */
 3831: 			case BIOMASS:
 3832: 				break;
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
**Lines (current code):** 529-535 vs 2895, 3239-3251, 2824, 949  
**Lines at review (Oct 2026):** 529-535 vs 2899, 3011-3023, 2827, 951

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
**Lines (current code):** 562  
**Lines at review (Oct 2026):** 564

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` passes the check and then indexes past stage-sized arrays such as `pSPEat`. Function: `Ecology_Init()`.

**Current code** (`atecology/atbiolsetup.c` lines 560–564)

```c
  560: 				chrtstage = chrt;
  561: 			}
  562: 			FunctGroupArray[sp].cohort_stage[cohort] = chrtstage;
  563:             
  564:             if (FunctGroupArray[sp].cohort_stage[cohort] > FunctGroupArray[sp].numStages){
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
**Lines (current code):** 3591-3593, 3635-3639  
**Lines at review (Oct 2026):** 3363-3365, 3407-3411

**Function:** `Setup_HomeRanges()`

**Why it's thought to be a bug**

`ids[idnum]` is written before checking `n > nbox`. In the first branch (3591) there is no check at all, and in the second the check comes after the write, so `ids[nbox]` can be written before the quit. Function: `Setup_HomeRanges()`.

**Current code** (`atecology/atbiolsetup.c` lines 3589–3595)

```c
 3589: 									bm->boxes[nij].HomeRangeInfo[sp].ids[idnum] = bm->K_num_homerange;
 3590: 									bm->boxes[nij].HomeRangeInfo[sp].n++;
 3591: 
 3592: 									fprintf(llogfp, "Created home range for %s in box %d-to-box%d\n", FunctGroupArray[sp].groupCode, ij, nij);
 3593: 								}
 3594: 							}
 3595: 							bm->K_num_homerange++;
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
**Lines (current code):** 2920 vs 2352  
**Lines at review (Oct 2026):** 2924 vs 2353

**Why it's thought to be a bug**

`totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` but not `totden_check`.

**Current code** (`atecology/atbiolsetup.c` lines 2918–2922) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 2918:     
 2919:     fprintf(llogfp, "totboxden created with dimension bm->K_num_max_cohort: %d bm->K_num_max_genetypes: %d (final dim %d)\n", bm->K_num_max_cohort, bm->K_num_max_genetypes, bm->K_num_max_cohort * bm->K_num_max_genetypes);
 2920:     
 2921: 	bm->diagnosticBiom = Util_Alloc_Init_1D_Double(ntotsp + 1, 0.0);
 2922: 
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
**Lines (current code):** 4049 vs 2585-2620  
**Lines at review (Oct 2026):** 3821 vs 2582-2617

**Why it's thought to be a bug**

`EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it.

**Current code** (`atecology/atbiolsetup.c` lines 4047–4051)

```c
 4047:         
 4048:         if(bm->track_contaminants) {
 4049:             EMBRYO[sp].Contam = Util_Alloc_Init_4D_Double(bm->num_contaminants, max_num, FunctGroupArray[sp].numGeneTypes, num_stocks, 0);
 4050:             EMBRYO[sp].AverageContam = Util_Alloc_Init_2D_Double(bm->num_contaminants, FunctGroupArray[sp].numGeneTypes, 0);
 4051:             EMBRYO[sp].RecruitContam = Util_Alloc_Init_5D_Double(bm->num_contaminants, max_num, FunctGroupArray[sp].numGeneTypes, bm->wcnz, bm->nbox, 0);
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

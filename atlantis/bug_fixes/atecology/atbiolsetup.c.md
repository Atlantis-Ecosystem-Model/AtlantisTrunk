# atecology/atbiolsetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BSET-01](#bset-01) ⭐ | 3391 | H/H | wrong accumulator | `dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is … |  | [ ] |
| [BSET-02](#bset-02) ⭐ | 3420, 3414 | H/H | wrong index / accumulator | In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test… |  | [ ] |
| [BSET-03](#bset-03) | 1902 | H/M | double normalisation | `if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg… |  | [ ] |
| [BSET-04](#bset-04) | 892, 886, 2046 | H/M | copy-paste / ordering | `speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never re… |  | [ ] |
| [BSET-05](#bset-05) | 675, 2046 | H/L | format mismatch | `quit("... group %s must use prescribed movement only. %s\n", FunctGroupArray[b].groupCode);` has two `%s` but… |  | [ ] |
| [BSET-06](#bset-06) | 4090 | M/M | uninitialised index | `speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `matur… |  | [ ] |
| [BSET-07](#bset-07) | 4066, 2629, 4078 | M/M | alloc/use mismatch | `Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution… |  | [ ] |
| [BSET-08](#bset-08) | 1880, 1911 | M/L | out-of-range struct use | The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` … |  | [ ] |
| [BSET-09](#bset-09) | 1453, 1491, 1421 | M/L | NULL/negative index | When verbose > 1, the code prints `FunctGroupArray[guild].groupCode` even if the tracer wasn't found (`guild =… |  | [ ] |
| [BSET-10](#bset-10) | 1747, 1777 | M/L | double counting | In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1627… |  | [ ] |
| [BSET-11](#bset-11) | 3826 | M/L | cohort vs cohort×genotype / negative index | `nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGene… |  | [ ] |
| [BSET-12](#bset-12) | 529, 2891, 2046 | M/M | alloc size | `bm->maxspage` is the largest `ageClassSize` over vertebrates only. `bm->tempPopRatio` (and `readinpopratio`, … |  | [ ] |
| [BSET-13](#bset-13) | 564, 2046 | M/L | off-by-one | `if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` pass… |  | [ ] |
| [BSET-14](#bset-14) | 3589, 3635, 3492 | L/L | bounds check after write | `ids[idnum]` is written before checking `n > nbox`. In the first branch (3363) there is no check at all, and i… |  | [ ] |
| [BSET-15](#bset-15) | 2916 | H/L | memory leak | `totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` … |  | [ ] |
| [BSET-16](#bset-16) | 4046 | H/L | memory leak | `EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it. |  | [ ] |

---

## BSET-01

**wrong accumulator** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atbiolsetup.c` 3391
**Lines at review (Oct 2026):** 3161-3170

**Function:** `Setup_Invade()`

**Why it's thought to be a bug**

`dist_step1 += sqrt(xdiff*xdiff + ydiff*ydiff); if (dist_step1 > maxdist) maxdist = dist_step1;`: the code is meant to find the largest distance from the entry box to any box. Because it uses `+=`, it adds up the distances to all boxes instead (and `dist_step1` is never reset). That makes `maxdist`, and with it `InvaderSatDay` and `Invadendt` (the size of `InvadeArray`), much too large. Function: `Setup_Invade()`.

**Code at issue** — `atbiolsetup.c` line 3391 (the line the review pointed to, found by its text)  
Search for: `dist_step1 += sqrt(xdiff * xdiff + ydiff * ydiff);`

```c
 3389: 		xdiff = bm->boxes[b].inside.x - bm->boxes[nb].inside.x;
 3390: 		ydiff = bm->boxes[b].inside.y - bm->boxes[nb].inside.y;
 3391→ 		dist_step1 += sqrt(xdiff * xdiff + ydiff * ydiff);
 3392:         if (dist_step1 > maxdist) {
 3393:             maxdist = dist_step1;
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
**Found at (current code):** `atbiolsetup.c` 3420, `atbiolsetup.c` 3414
**Lines at review (Oct 2026):** 3187-3197

**Function:** `Setup_Invade()`

**Why it's thought to be a bug**

In the no_move invasion spread, `dist_step1` keeps growing from the BSET-01 loop and never resets, so the test `dist_step1 < InvaderSpeed*Invadedt*86400` is effectively always false. Even when it is true, it writes `InvadeArray[b][k]` (the source box, which is already > 0) rather than the neighbour `nb`, and it writes time step `k` rather than `k+1`. The invasion therefore never spreads past the entry box until the forced saturation at the last step. Function: `Setup_Invade()`.

**Code at issue** — `atbiolsetup.c` line 3420 (closest match to the quoted code; wording differs slightly)  
Search for: `if (dist_step1 < (bm->InvaderSpeed * bm->Invadedt * 86400.0))`

```c
 3418: 						ydiff = bm->boxes[b].inside.y - bm->boxes[nb].inside.y;
 3419: 						dist_step1 += sqrt(xdiff * xdiff + ydiff * ydiff);
 3420→ 						if (dist_step1 < (bm->InvaderSpeed * bm->Invadedt * 86400.0))
 3421: 							bm->InvadeArray[b][k] = bm->InvaderScalar;
 3422: 					}
```

**Code at issue** — `atbiolsetup.c` line 3414  
Search for: `InvadeArray[b][k]`

```c
 3412: 		for (k = 0; k < bm->Invadendt; k++) {
 3413: 			for (b = 0; b < bm->nbox; b++) {
 3414→ 				if (bm->InvadeArray[b][k] > 0) {
 3415: 					for (chkbox = 0; chkbox < bm->boxes[b].nconn; chkbox++) {
 3416: 						nb = bm->boxes[b].ibox[chkbox];
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
**Found at (current code):** `atbiolsetup.c` 1902
**Lines at review (Oct 2026):** 1902-1907

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

`if (bm->manage_reg) ... if (lastreg_prop[sp][nreg] < no_quota) initreg_prop[sp][nreg] = initreg_prop[sp][nreg] / (regtot + small_num);`: `initreg_prop` was already turned into a proportion at line 1898. `lastreg_prop` was set to 0 at line 1539, so the condition is always true at init. With manage_reg on, the proportions are divided by the total biomass a second time and become close to 0. Function: `Ecology_Init_Populations()`.

**Code at issue** — `atbiolsetup.c` line 1902  
Search for: `if (bm->manage_reg)`

```c
 1900: 			bm->initreg_prop[sp][nreg] = bm->initreg_prop[sp][nreg] / (regtot + small_num);
 1901: 		}
 1902→ 		if (bm->manage_reg) {
 1903: 			for (nreg = 0; nreg < bm->K_num_reg; nreg++) {
 1904: 				if (bm->lastreg_prop[sp][nreg] < no_quota) { // So can have regions where no TAC used
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
**Found at (current code):** `atbiolsetup.c` 892, `atbiolsetup.c` 886, `atbiolsetup.c` 2046
**Lines at review (Oct 2026):** 892-895

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`speciesParams[vstockstruct_id] = sum;` is stored before `if (sum < 1) sum = 1;`, so the minimum of 1 never reaches the stored value. The horizontal block above (879-881) clamps first and then stores. If `wcnz == 0` the value stays `-MAXDOUBLE`, and if all `v_stock_struct` entries are 0 it is 0, which feeds into the stock-structure logic. Function: `Ecology_Init()`.

**Code at issue** — `atbiolsetup.c` line 892  
Search for: `speciesParams[vstockstruct_id] = sum`

```c
  890: 				}
  891: 			}
  892→ 			FunctGroupArray[sp].speciesParams[vstockstruct_id] = sum;
  893: 			/* Set number of stocks to a minimum of 1 */
  894: 			if (sum < 1)
```

**Code at issue** — `atbiolsetup.c` line 886  
Search for: `-MAXDOUBLE`

```c
  884: 
  885:             /** Vertical stocks **/
  886→ 			sum = -MAXDOUBLE;
  887: 			for (b = 0; b < bm->wcnz; b++) {
  888: 				if (bm->v_stock_struct[b][sp] > sum) {
```

**Code at issue** — `atbiolsetup.c` line 2046  
Search for: `Ecology_Init()`

```c
 2044:     
 2045:     /* If using bi-logistic feeding window - set up max_scalar
 2046→      Ideally this would go in Ecology_Init() but has to be here
 2047:      as need initVERTinfo to have been populated first */
 2048:     
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
**Found at (current code):** `atbiolsetup.c` 675, `atbiolsetup.c` 2046
**Lines at review (Oct 2026):** 675-676

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`quit("... group %s must use prescribed movement only.  %s\n", FunctGroupArray[b].groupCode);` has two `%s` but only one argument. This is undefined behaviour and can crash while reporting the error. Function: `Ecology_Init()`.

**Code at issue** — `atbiolsetup.c` line 675  
Search for: `quit("`

```c
  673: 				fprintf(bm->logFile, "Ecology_Init: Land based group %s must use prescribed movement only.\n",
  674: 											FunctGroupArray[b].groupCode);
  675→ 				quit("Ecology_Init: Land based group %s must use prescribed movement only.  %s\n",
  676: 						FunctGroupArray[b].groupCode);
  677: 			}
```

**Code at issue** — `atbiolsetup.c` line 2046  
Search for: `Ecology_Init()`

```c
 2044:     
 2045:     /* If using bi-logistic feeding window - set up max_scalar
 2046→      Ideally this would go in Ecology_Init() but has to be here
 2047:      as need initVERTinfo to have been populated first */
 2048:     
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
**Found at (current code):** `atbiolsetup.c` 4090
**Lines at review (Oct 2026):** 3929, 3941-3951, 3996-3998

**Function:** `Create_Evolution_Parameters()`

**Why it's thought to be a bug**

`speciesParamIndex` starts at -1 and is never set for `growth_trait_id` (=0), `consumption_trait_id` or `maturation_trait_id` ("intentionally not handled"). If `DNA[sp].trait_active[]` is set for one of these traits, the loop still reads `speciesParams[-1]` (out of bounds) or reuses the previous trait's index. Function: `Create_Evolution_Parameters()`.

**Code at issue** — `atbiolsetup.c` line 4090 (closest match to the quoted code; wording differs slightly)  
Search for: `DNA[sp].trait_active = Util_Alloc_Init_1D_Int(K_num_traits, 0);`

```c
 4088: 		DNA[sp].trait_shift = Util_Alloc_Init_4D_Long_Double(K_num_evol_shift, num_ages, FunctGroupArray[sp].numCohorts, K_num_traits, 0);
 4089: 
 4090→ 		DNA[sp].trait_active = Util_Alloc_Init_1D_Int(K_num_traits, 0);
 4091: 
 4092: 		for (ntrait = 0; ntrait < K_num_traits; ntrait++) {
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
**Found at (current code):** `atbiolsetup.c` 4066, `atbiolsetup.c` 2629, `atbiolsetup.c` 4078
**Lines at review (Oct 2026):** 3838-3842 vs 2626-2652, 3996

**Why it's thought to be a bug**

`Ecology_Setup_Evolution()` returns without allocating `DNA` when `K_num_max_genetypes < 2`. `Create_Evolution_Parameters()` (on `flag_do_evolution`) and `Free_Evolution()` (on `flag_do_evolution || flag_mult_grow_curves`) still dereference `DNA[sp]`, so the run crashes if evolution or multiple growth curves are on with one genotype. The reverse case also leaks: when genotypes ≥ 2 but neither flag is set, all `DNA` arrays are allocated and never freed. Functions: `Ecology_Setup_Evolution()`, `Free_Evolution()`.

**Code at issue** — `atbiolsetup.c` line 4066  
Search for: `K_num_max_genetypes < 2`

```c
 4064: 	int sp = 0, gene_cohort = 0, num_ages = 0, ntrait = 0, cohort  = 0;
 4065: 
 4066→ 	if(bm->K_num_max_genetypes < 2)  // Nothing to do
 4067: 		return;
 4068: 
```

**Code at issue** — `atbiolsetup.c` line 2629 (closest match to the quoted code; wording differs slightly)  
Search for: `if(!bm->flag_do_evolution && !bm->flag_mult_grow_curves)  // Nothing to do`

```c
 2627: 	int sp;
 2628: 
 2629→ 	if(!bm->flag_do_evolution && !bm->flag_mult_grow_curves)  // Nothing to do
 2630: 		return;
 2631: 
```

**Code at issue** — `atbiolsetup.c` line 4078  
Search for: `DNA[sp]`

```c
 4076: 		num_ages = FunctGroupArray[sp].ageClassSize;
 4077: 
 4078→ 		DNA[sp].stdev_gene_map = Util_Alloc_Init_1D_Double(FunctGroupArray[sp].numGeneTypes, 0);  // This is used if there are multiple growth curves
 4079: 
 4080: 		DNA[sp].tot_num = Util_Alloc_Init_1D_Double(FunctGroupArray[sp].numCohorts, 0);
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
**Found at (current code):** `atbiolsetup.c` 1880, `atbiolsetup.c` 1911
**Lines at review (Oct 2026):** 1876-1884, 1911

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

The loop goes to `sp == K_num_tot_sp` (the DIN slot). For that slot it prints `FunctGroupArray[sp].groupCode` and reads `FunctGroupArray[sp].habitatType`. `FunctGroupArray` is malloc'd with K_num_tot_sp+1 entries (atlantisUtil/atUtilGroupIO.c:252), so this entry exists but was never initialised. The log gets a garbage name, and the read is undefined behaviour. Function: `Ecology_Init_Populations()`.

**Code at issue** — `atbiolsetup.c` line 1880  
Search for: `sp == bm->K_num_tot_sp`

```c
 1878: 		if (sp < bm->K_num_tot_sp)
 1879: 			bm->estinitpop[sp] = bm->totbiom[sp] * bm->X_CN * mg_2_tonne;
 1880→ 		if (sp == bm->K_num_tot_sp || ((FunctGroupArray[sp].isDetritus == TRUE || (FunctGroupArray[sp].isDetritus == FALSE
 1881: 				&& (int) (FunctGroupArray[sp].speciesParams[flag_id]))))) {
 1882: 			fprintf(llogfp, "Time: %e, species %s virgin biomass is %f t\n", bm->dayt, FunctGroupArray[sp].groupCode, bm->totinitpop[sp] * bm->X_CN
```

**Code at issue** — `atbiolsetup.c` line 1911  
Search for: `FunctGroupArray[sp].habitatType`

```c
 1909:         
 1910:         // For non-mobile invertebrates constrain them to initial distributions - so store the original distribution
 1911→         if(bm->flagconstrain_epiwander && (FunctGroupArray[sp].habitatType == EPIFAUNA) && !FunctGroupArray[sp].sp_geo_move) {
 1912:             pid = FunctGroupArray[sp].totNTracers[0];
 1913:             for (ij = 0; ij < bm->nbox; ij++) {
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
**Found at (current code):** `atbiolsetup.c` 1453, `atbiolsetup.c` 1491, `atbiolsetup.c` 1421
**Lines at review (Oct 2026):** 1452-1453

**Function:** `Set_Active_Flag()`

**Why it's thought to be a bug**

When verbose > 1, the code prints `FunctGroupArray[guild].groupCode` even if the tracer wasn't found (`guild == -1`, the `checkdone < 0` branch). That is an out-of-bounds read at index -1. Function: `Set_Active_Flag()`.

**Code at issue** — `atbiolsetup.c` line 1453  
Search for: `FunctGroupArray[guild].groupCode`

```c
 1451: 
 1452: 			if(verbose > 1)
 1453→ 				fprintf(llogfp, "Doing %s -> %s(%d) has Activeflag %d (id %d)\n", trnamelist[ij].name, FunctGroupArray[guild].groupCode, guild, Activeflag[id], id);
 1454: 		}
 1455: 	}
```

**Code at issue** — `atbiolsetup.c` line 1491  
Search for: `guild == -1`

```c
 1489: 
 1490: 
 1491→ 			if (guild == -1) {
 1492: 				quit("Set_Active_Flag: %s - guild not found \n", epinamelist[ij].name);
 1493: 			}
```

**Code at issue** — `atbiolsetup.c` line 1421  
Search for: `checkdone < 0`

```c
 1419: 
 1420: 			/* If the tracer is not found make it is inactive */
 1421→ 			if(checkdone < 0){
 1422: 				Activeflag[id] = 0;
 1423: 				Activeflag[id + numwcvar] = 0;
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
**Found at (current code):** `atbiolsetup.c` 1747, `atbiolsetup.c` 1777
**Lines at review (Oct 2026):** 1743-1748

**Function:** `Ecology_Init_Populations()`

**Why it's thought to be a bug**

In the relaxed-density retry, `sp_abund += bm->boxes[ij].tr[k][den]` adds every box's numbers again. Line 1625 already counted them all, unconditionally. When the retry triggers, `initVERTinfo[sp][n][DEN_id]` (and so `totinitpop`) is too high by the in-domain numbers. Function: `Ecology_Init_Populations()`.

**Code at issue** — `atbiolsetup.c` line 1747  
Search for: `sp_abund += bm->boxes[ij].tr[k][den]`

```c
 1745: 							if (bm->boxes[ij].type != BOUNDARY) {
 1746: 								for (k = 0; k < bm->boxes[ij].nz; k++) {
 1747→ 									sp_abund += bm->boxes[ij].tr[k][den];
 1748: 
 1749: 									/* Check minimum SN */
```

**Code at issue** — `atbiolsetup.c` line 1777  
Search for: `initVERTinfo[sp][n][DEN_id]`

```c
 1775: 
 1776: 					/* Store final results - including average size for each cohort of species */
 1777→ 					initVERTinfo[sp][n][DEN_id] = sp_abund;
 1778: 					initVERTinfo[sp][n][SN_id] = (sizeMinMax[min_SN_id] + sizeMinMax[max_SN_id]) / 2.0;
 1779: 					initVERTinfo[sp][n][RN_id] = (sizeMinMax[min_RN_id] + sizeMinMax[max_RN_id]) / 2.0;
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
**Found at (current code):** `atbiolsetup.c` 3826
**Lines at review (Oct 2026):** 3593-3602

**Function:** `Set_spMinMax_Values()`

**Why it's thought to be a bug**

`nage = age_mat - 1; nmat = age_mat;` index `structNTracers`, which is sized and ordered by numCohortsXnumGenes (cohort = age*numGeneTypes+gene; see line 550). With several genotypes, `[nage]` and `[nmat]` pick the wrong age class. If `age_mat_id == 0`, `nage = -1` reads out of bounds. Line 3831 does use numCohortsXnumGenes-1, which is inconsistent. Function: `Set_spMinMax_Values()`.

**Code at issue** — `atbiolsetup.c` line 3826  
Search for: `[nage]`

```c
 3824: 				 their gapes. */
 3825: 				FunctGroupArray[sp].spMinMax[0] = FunctGroupArray[sp].structNTracers[0];
 3826→ 				FunctGroupArray[sp].spMinMax[1] = FunctGroupArray[sp].structNTracers[nage];
 3827: 				FunctGroupArray[sp].spMinMax[2] = FunctGroupArray[sp].structNTracers[nmat];
 3828: 				FunctGroupArray[sp].spMinMax[3] = FunctGroupArray[sp].structNTracers[(FunctGroupArray[sp].numCohortsXnumGenes) - 1];
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
**Found at (current code):** `atbiolsetup.c` 529, `atbiolsetup.c` 2891, `atbiolsetup.c` 2046
**Lines at review (Oct 2026):** 529-535 vs 2899, 3011-3023, 2827, 951

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`bm->maxspage` is the largest `ageClassSize` over vertebrates only. `bm->tempPopRatio` (and `readinpopratio`, `nSTOCK`, `sumSTOCK`) is allocated with `maxspage` in its age dimension, but `Init_Population_Ratios()` fills `tempPopRatio[..][i]` for `i < ageClassSize` for AGE_STRUCTURED_BIOMASS invertebrates too. If an age-structured invertebrate has a larger `ageClassSize` than every vertebrate, or the model has no vertebrates (maxspage = 0), this writes out of bounds. Function: `Ecology_Init()` / `Init_Population_Ratios()`.

**Code at issue** — `atbiolsetup.c` line 529  
Search for: `maxspage`

```c
  527:     
  528:     /* Determine maximum vertebrate ageclass size */
  529→ 	bm->maxspage = 0;
  530: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  531: 		if (FunctGroupArray[sp].isVertebrate == TRUE) {
```

**Code at issue** — `atbiolsetup.c` line 2891  
Search for: `tempPopRatio`

```c
 2889: 
 2890: 	tempdistrib = (double **) alloc2d(bm->K_num_max_stages, bm->wcnz);
 2891→ 	bm->tempPopRatio = (double ****) alloc4d(bm->maxspage, ncohorts * ngenetypes, bm->K_num_tot_sp, nstock);
 2892: 	bm->totbiom = Util_Alloc_Init_1D_Double(ntotsp + 1, 0.0);
 2893: 	bm->groupTotCatch = Util_Alloc_Init_2D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 0.0);
```

**Code at issue** — `atbiolsetup.c` line 2046  
Search for: `Ecology_Init()`

```c
 2044:     
 2045:     /* If using bi-logistic feeding window - set up max_scalar
 2046→      Ideally this would go in Ecology_Init() but has to be here
 2047:      as need initVERTinfo to have been populated first */
 2048:     
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
**Found at (current code):** `atbiolsetup.c` 564, `atbiolsetup.c` 2046
**Lines at review (Oct 2026):** 564

**Function:** `Ecology_Init()`

**Why it's thought to be a bug**

`if (cohort_stage[cohort] > numStages)`: valid stages run 0..numStages-1, so a stage equal to `numStages` passes the check and then indexes past stage-sized arrays such as `pSPEat`. Function: `Ecology_Init()`.

**Code at issue** — `atbiolsetup.c` line 564 (closest match to the quoted code; wording differs slightly)  
Search for: `if (FunctGroupArray[sp].cohort_stage[cohort] > FunctGroupArray[sp].numStages){`

```c
  562: 			FunctGroupArray[sp].cohort_stage[cohort] = chrtstage;
  563:             
  564→             if (FunctGroupArray[sp].cohort_stage[cohort] > FunctGroupArray[sp].numStages){
  565:                 printf("%s has cohort_stage %d which is > numStages: %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].cohort_stage[cohort], FunctGroupArray[sp].numStages);
  566:                 quit("%s has cohort_stage %d which is > numStages: %d\n", FunctGroupArray[sp].groupCode, FunctGroupArray[sp].cohort_stage[cohort], FunctGroupArray[sp].numStages);
```

**Code at issue** — `atbiolsetup.c` line 2046  
Search for: `Ecology_Init()`

```c
 2044:     
 2045:     /* If using bi-logistic feeding window - set up max_scalar
 2046→      Ideally this would go in Ecology_Init() but has to be here
 2047:      as need initVERTinfo to have been populated first */
 2048:     
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
**Found at (current code):** `atbiolsetup.c` 3589, `atbiolsetup.c` 3635, `atbiolsetup.c` 3492
**Lines at review (Oct 2026):** 3363-3365, 3407-3411

**Function:** `Setup_HomeRanges()`

**Why it's thought to be a bug**

`ids[idnum]` is written before checking `n > nbox`. In the first branch (3591) there is no check at all, and in the second the check comes after the write, so `ids[nbox]` can be written before the quit. Function: `Setup_HomeRanges()`.

**Code at issue** — `atbiolsetup.c` line 3589  
Search for: `ids[idnum]`

```c
 3587: 								if (connected[nij] > 0) {
 3588: 									idnum = bm->boxes[nij].HomeRangeInfo[sp].n;
 3589→ 									bm->boxes[nij].HomeRangeInfo[sp].ids[idnum] = bm->K_num_homerange;
 3590: 									bm->boxes[nij].HomeRangeInfo[sp].n++;
 3591: 
```

**Code at issue** — `atbiolsetup.c` line 3635  
Search for: `n > bm->nbox`

```c
 3633: 										bm->boxes[nij].HomeRangeInfo[sp].ids[idnum] = bm->K_num_homerange;
 3634: 										bm->boxes[nij].HomeRangeInfo[sp].n++;
 3635→ 										if (bm->boxes[nij].HomeRangeInfo[sp].n > bm->nbox)
 3636: 											quit("Trying to create more home ranges than there are boxes - this will cause a memory stomp\n");
 3637: 									}
```

**Code at issue** — `atbiolsetup.c` line 3492 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->boxes[b].HomeRangeInfo[sp].ids = (int *) i_alloc1d(bm->nbox); // As should never be mo`

```c
 3490: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
 3491: 			bm->boxes[b].HomeRangeInfo[sp].n = 0;
 3492→ 			bm->boxes[b].HomeRangeInfo[sp].ids = (int *) i_alloc1d(bm->nbox); // As should never be more home ranges than there are boxes
 3493: 			for (rij = 0; rij < bm->nbox; rij++) {
 3494: 				bm->boxes[b].HomeRangeInfo[sp].ids[rij] = -1;
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
**Found at (current code):** `atbiolsetup.c` 2916
**Lines at review (Oct 2026):** 2924 vs 2353

**Why it's thought to be a bug**

`totden_check` is allocated in `Allocate_Arrays_Post_Load()` but never freed. `Ecology_Free()` frees `totden` but not `totden_check`.

**Code at issue** — `atbiolsetup.c` line 2916 (located by name only; check this is the right place)  
Search for: `totden_check = Util_Alloc_Init_2D_Double(totdensize, bm->K_num_tot_sp, 0.0);`

```c
 2914:     totboxden = Util_Alloc_Init_1D_Double(bm->K_num_max_cohort * bm->K_num_max_genetypes, 0.0);
 2915:     totden = Util_Alloc_Init_2D_Double(totdensize, bm->K_num_tot_sp, 0.0);
 2916→     totden_check = Util_Alloc_Init_2D_Double(totdensize, bm->K_num_tot_sp, 0.0);
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
**Found at (current code):** `atbiolsetup.c` 4046
**Lines at review (Oct 2026):** 3821 vs 2582-2617

**Why it's thought to be a bug**

`EMBRYO[sp].migIDmatch` is allocated in `Ecology_Setup_Reproduction()` but `Free_Embryo()` never frees it.

**Code at issue** — `atbiolsetup.c` line 4046  
Search for: `EMBRYO[sp].migIDmatch`

```c
 4044:         
 4045:         // Relevant EMBRYO alignment with MIGRATION array counter
 4046→         EMBRYO[sp].migIDmatch = Util_Alloc_Init_1D_Int(max_num, 0);
 4047:         
 4048:         if(bm->track_contaminants) {
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

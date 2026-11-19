# atecology/atbiolUtil.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-22](#emisc-22) ⭐ | 143-157 | H/H | data loss | A new zeroed 5D array is allocated and only `[speciesIndex][cohort][box]` is copied (the memcpy is commented o… |  | [ ] |
| [EMISC-23](#emisc-23) | 143 | H/M | wrong dimension | `Util_Alloc_Init_5D_Double(3, maxMortChange, bm->nbox, 2, bm->K_num_tot_sp, 0)` uses stage dimension 2, but th… |  | [ ] |
| [EMISC-24](#emisc-24) ⭐ | 210-219 | H/H | data loss | Same problem for habitats: only `BEDchange[habitatIndex]` is copied into the new `[K_num_bed_types][max][3]` a… |  | [ ] |
| [EMISC-25](#emisc-25) | 138 | H/L | debug output | An unconditional `printf("maxMortChange = %d\n", ...)` runs on every call. Function: `Scale_Group_Linear_Morta… |  | [ ] |

---

## EMISC-22

**data loss** · Conf H / Sev H ⭐ priority  
**Listed lines:** 143-157

**Function:** `Scale_Group_Linear_Mortality()`

**Why it's thought to be a bug**

A new zeroed 5D array is allocated and only `[speciesIndex][cohort][box]` is copied (the memcpy is commented out); then the old array is freed. Every other group/stage/box's scheduled linear-mortality changes are wiped, while `numMortChanges` still counts them. Function: `Scale_Group_Linear_Mortality()`.

**Current code** (`atecology/atbiolUtil.c` lines 138–156) — code appears to have moved -3 lines since the review

```c
  138: 	printf("maxMortChange = %d\n", maxMortChange);
  139: 	if (((int) numMortChanges[speciesIndex][cohort][box]) > maxMortChange) {
  140: 		maxMortChange = numMortChanges[speciesIndex][cohort][box];
  141: 	}
  142: 	/* Allocate a new array */
  143: 	tempArray = Util_Alloc_Init_5D_Double(3, maxMortChange, bm->nbox, 2, bm->K_num_tot_sp, 0);
  144: 
  145: 	/* Copy all existing values across */
  146: //	memcpy(tempArray, LinearMortChange, sizeof(LinearMortChange));
  147: 
  148: 	/* Now insert the new value in the correct place */
  149: 	Util_Copy_Change_Values(bm, LinearMortChange[speciesIndex][cohort][box],
  150: 			tempArray[speciesIndex][cohort][box],
  151: 			numMortChanges[speciesIndex][cohort][box],
  152: 			start, period, mult);
  153: 
  154: 	/* Now do some pointer magic and free up the 'old' array*/
  155: 	tempArrayPointer = LinearMortChange;
  156: 	LinearMortChange = tempArray;
```

**Suggested fix (from review):** Copy all [sp][stage][box][0..oldmax) entries into tempArray before inserting

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

## EMISC-23

**wrong dimension** · Conf H / Sev M  
**Listed lines:** 143

**Function:** `Scale_Group_Linear_Mortality()`

**Why it's thought to be a bug**

`Util_Alloc_Init_5D_Double(3, maxMortChange, bm->nbox, 2, bm->K_num_tot_sp, 0)` uses stage dimension 2, but the original allocation uses `K_num_max_stages` (atbiolsetup.c:2777). Groups with more than 2 stages index out of bounds after the first realloc. Function: `Scale_Group_Linear_Mortality()`.

**Current code** (`atecology/atbiolUtil.c` lines 141–145)

```c
  141: 	}
  142: 	/* Allocate a new array */
  143: 	tempArray = Util_Alloc_Init_5D_Double(3, maxMortChange, bm->nbox, 2, bm->K_num_tot_sp, 0);
  144: 
  145: 	/* Copy all existing values across */
```

**Suggested fix (from review):** Use `bm->K_num_max_stages`

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

## EMISC-24

**data loss** · Conf H / Sev H ⭐ priority  
**Listed lines:** 210-219

**Function:** `Scale_Habitat()`

**Why it's thought to be a bug**

Same problem for habitats: only `BEDchange[habitatIndex]` is copied into the new `[K_num_bed_types][max][3]` array, so the scheduled changes of the other habitat types are zeroed. Function: `Scale_Habitat()`.

**Current code** (`atecology/atbiolUtil.c` lines 208–221)

```c
  208: 
  209: 	/* Now allocate a temp array to store the updated values */
  210: 	tempArray = Util_Alloc_Init_3D_Double(3, BEDchange_max_num, bm->K_num_bed_types, 0);
  211: 
  212: 	/* Now copy the values across inserting the new value in the correct place.*/
  213: 	Util_Copy_Change_Values(bm, BEDchange[habitatIndex], tempArray[habitatIndex],
  214: 			BEDchange_max_num, start, period, mult);
  215: 
  216: 	/* Now do some pointer magic and free up the 'old' array*/
  217: 	tempArrayPointer = BEDchange;
  218: 	BEDchange = tempArray;
  219: 	free3d(tempArrayPointer);
  220: 
  221: 	for(i = 0; i < BEDchange_max_num; i++){
```

**Suggested fix (from review):** Copy all bed types' rows before inserting

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

## EMISC-25

**debug output** · Conf H / Sev L  
**Listed lines:** 138

**Function:** `Scale_Group_Linear_Mortality()`

**Why it's thought to be a bug**

An unconditional `printf("maxMortChange = %d\n", ...)` runs on every call. Function: `Scale_Group_Linear_Mortality()`.

**Current code** (`atecology/atbiolUtil.c` lines 136–140)

```c
  136: 	numMortChanges[speciesIndex][cohort][box]++;
  137: 
  138: 	printf("maxMortChange = %d\n", maxMortChange);
  139: 	if (((int) numMortChanges[speciesIndex][cohort][box]) > maxMortChange) {
  140: 		maxMortChange = numMortChanges[speciesIndex][cohort][box];
```

**Suggested fix (from review):** Guard with verbose

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

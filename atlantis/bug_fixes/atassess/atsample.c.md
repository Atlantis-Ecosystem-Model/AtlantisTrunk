# atassess/atsample.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SMP-01](#smp-01) | 233 | H/M | off-by-one | `if (sample > num_sampled_phy_id)` should be `>=`; no lower bound (sample<2 gives a negative index). |  | [ ] |
| [SMP-02](#smp-02) | 258 | H/M | off-by-one | `if (sample > 3)` should be `>= 3` (`sampleprocess` has 3 rows). |  | [ ] |
| [SMP-03](#smp-03) | 117, 134 | M/M | schedule | `teatassess` is not ceil'd like `tassess`, so it drifts and stops firing; next indices are not bounds-checked. |  | [ ] |
| [SMP-04](#smp-04) | 186 | M/M | always true | `flagpatchy < 2` but flagpatchy is binary. |  | [ ] |
| [SMP-05](#smp-05) | 205 | M/H | buffer overflow | `nsbox = nsbox_o` can exceed the `samplearea`/`totalarea` allocation (nsbox_i). |  | [ ] |
| [SMP-06](#smp-06) | 238, 265 | M/M | normalisation | Getters divide inconsistently by `annual_sample` (can be 0, giving inf). |  | [ ] |
| [SMP-07](#smp-07) | 273 | H/L | off-by-one / copy-paste | `>` should be `>=`; error message names the wrong function. |  | [ ] |

---

## SMP-01

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atsample.c` 233
**Lines at review (Oct 2026):** 233, 238

**Why it's thought to be a bug**

`if (sample > num_sampled_phy_id)` should be `>=`; no lower bound (sample<2 gives a negative index).

**Code at issue** — `atsample.c` line 233  
Search for: `if(sample > num_sampled_phy_id)`

```c
  231: 	zone = bm->boxes[b].zone - 1;
  232: 
  233→ 	if(sample > num_sampled_phy_id){
  234: 		quit("Get_Physical_Sampled_Value attempting to sample physical property %d that isn't sampled\n", sample);
  235: 	}
```

**Suggested fix (from review):** `>=` and `< salinity_id`

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

## SMP-02

**off-by-one** · Conf H / Sev M  
**Found at (current code):** `atsample.c` 258
**Lines at review (Oct 2026):** 258

**Why it's thought to be a bug**

`if (sample > 3)` should be `>= 3` (`sampleprocess` has 3 rows).

**Code at issue** — `atsample.c` line 258  
Search for: `if(sample > 3)`

```c
  256: 	int zone;
  257: 
  258→ 	if(sample > 3){
  259: 		quit("Assess_Get_Process_Sampled_Value attempting to sample property %d that isn't sampled\n", sample);
  260: 	}
```

**Suggested fix (from review):** `>= 3 || < 0`

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

## SMP-03

**schedule** · Conf M / Sev M  
**Found at (current code):** `atsample.c` 117, `atsample.c` 134
**Lines at review (Oct 2026):** 117, 134, 143, 173

**Why it's thought to be a bug**

`teatassess` is not ceil'd like `tassess`, so it drifts and stops firing; next indices are not bounds-checked.

**Code at issue** — `atsample.c` line 117 (the line the review pointed to, found by its text)  
Search for: `if ((bm->dayt == bm->teatassess) && datain) {`

```c
  115: 
  116: 		/* Do diet sampling if appropriate */
  117→ 		if ((bm->dayt == bm->teatassess) && datain) {
  118: 			Year_Of_Stomach(bm, llogfp);
  119: 
```

**Code at issue** — `atsample.c` line 134 (the line the review pointed to, found by its text)  
Search for: `if ((bm->dayt == bm->teatassess) && datain) {`

```c
  132: 
  133: 		/* Do true diet sampling if appropriate */
  134→ 		if ((bm->dayt == bm->teatassess) && datain) {
  135: 			Calculate_Real_Stomachs(bm, llogfp);
  136: 
```

**Suggested fix (from review):** match tassess

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

## SMP-04

**always true** · Conf M / Sev M  
**Found at (current code):** `atsample.c` 186
**Lines at review (Oct 2026):** 186

**Why it's thought to be a bug**

`flagpatchy < 2` but flagpatchy is binary.

**Code at issue** — `atsample.c` line 186  
Search for: `flagpatchy < 2`

```c
  184: 
  185: 	/* Is this a case of punctuated snapshots with low level in between (if not skip ahead) */
  186→ 	if (bm->flagpatchy < 2) {
  187: 
  188: 		/* Don't reload sample boxes until year 2 and beyond (just use intensive in year 1 */
```

**Suggested fix (from review):** `== 1`

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

## SMP-05

**buffer overflow** · Conf M / Sev H  
**Found at (current code):** `atsample.c` 205
**Lines at review (Oct 2026):** 202-213

**Why it's thought to be a bug**

`nsbox = nsbox_o` can exceed the `samplearea`/`totalarea` allocation (nsbox_i).

**Code at issue** — `atsample.c` line 205  
Search for: `nsbox = bm->nsbox_o`

```c
  203: 			bm->nsbox = bm->nsbox_i;
  204: 		else
  205→ 			bm->nsbox = bm->nsbox_o;
  206: 
  207: 		/* Load list of sample boxes */
```

**Suggested fix (from review):** allocate with the max

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

## SMP-06

**normalisation** · Conf M / Sev M  
**Found at (current code):** `atsample.c` 238, `atsample.c` 265
**Lines at review (Oct 2026):** 238, 265, 279

**Why it's thought to be a bug**

Getters divide inconsistently by `annual_sample` (can be 0, giving inf).

**Code at issue** — `atsample.c` line 238 (the line the review pointed to, found by its text)  
Search for: `return samplephys[sample - 2][zone][sample_id] / (phys_samplingsize * bm->annual_sample);`

```c
  236: 
  237: 	/* TODO: Not 100% sure this is correct - dividing by the sampleingsize */
  238→ 	return samplephys[sample - 2][zone][sample_id] / (phys_samplingsize * bm->annual_sample);
  239: }
  240: 
```

**Code at issue** — `atsample.c` line 265 (the line the review pointed to, found by its text)  
Search for: `return sampleprocess[sample][zone][sample_id]/(bm->annual_sample);`

```c
  263: 
  264: 	/* Not 100% sure this is correct - dividing by the sampleingsize */
  265→ 	return sampleprocess[sample][zone][sample_id]/(bm->annual_sample);
  266: }
  267: 
```

**Suggested fix (from review):** unify and guard

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

## SMP-07

**off-by-one / copy-paste** · Conf H / Sev L  
**Found at (current code):** `atsample.c` 273
**Lines at review (Oct 2026):** 273-274

**Why it's thought to be a bug**

`>` should be `>=`; error message names the wrong function.

**Code at issue** — `atsample.c` line 273 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `if(sample > (bm->K_num_tot_sp)){`

```c
  271: 	int zone;
  272: 
  273→ 	if(sample > (bm->K_num_tot_sp)){
  274: 		quit("Assess_Get_Process_Sampled_Value attempting to sample species property %d that isn't sampled\n", sample);
  275: 	}
```

**Suggested fix (from review):** fix both

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

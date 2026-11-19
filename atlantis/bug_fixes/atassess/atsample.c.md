# atassess/atsample.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **7 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SMP-01](#smp-01) | 233, 238 | H/M | off-by-one | `if (sample > num_sampled_phy_id)` should be `>=`; no lower bound (sample<2 gives a negative index). |  | [ ] |
| [SMP-02](#smp-02) | 258 | H/M | off-by-one | `if (sample > 3)` should be `>= 3` (`sampleprocess` has 3 rows). |  | [ ] |
| [SMP-03](#smp-03) | 117, 134, 143, 173 | M/M | schedule | `teatassess` is not ceil'd like `tassess`, so it drifts and stops firing; next indices are not bounds-checked. |  | [ ] |
| [SMP-04](#smp-04) | 186 | M/M | always true | `flagpatchy < 2` but flagpatchy is binary. |  | [ ] |
| [SMP-05](#smp-05) | 202-213 | M/H | buffer overflow | `nsbox = nsbox_o` can exceed the `samplearea`/`totalarea` allocation (nsbox_i). |  | [ ] |
| [SMP-06](#smp-06) | 238, 265, 279 | M/M | normalisation | Getters divide inconsistently by `annual_sample` (can be 0, giving inf). |  | [ ] |
| [SMP-07](#smp-07) | 273-274 | H/L | off-by-one / copy-paste | `>` should be `>=`; error message names the wrong function. |  | [ ] |

---

## SMP-01

**off-by-one** · Conf H / Sev M  
**Listed lines:** 233, 238

**Why it's thought to be a bug**

`if (sample > num_sampled_phy_id)` should be `>=`; no lower bound (sample<2 gives a negative index).

**Current code** (`atassess/atsample.c` lines 231–240)

```c
  231: 	zone = bm->boxes[b].zone - 1;
  232: 
  233: 	if(sample > num_sampled_phy_id){
  234: 		quit("Get_Physical_Sampled_Value attempting to sample physical property %d that isn't sampled\n", sample);
  235: 	}
  236: 
  237: 	/* TODO: Not 100% sure this is correct - dividing by the sampleingsize */
  238: 	return samplephys[sample - 2][zone][sample_id] / (phys_samplingsize * bm->annual_sample);
  239: }
  240: 
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
**Listed lines:** 258

**Why it's thought to be a bug**

`if (sample > 3)` should be `>= 3` (`sampleprocess` has 3 rows).

**Current code** (`atassess/atsample.c` lines 256–260)

```c
  256: 	int zone;
  257: 
  258: 	if(sample > 3){
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
**Listed lines:** 117, 134, 143, 173

**Why it's thought to be a bug**

`teatassess` is not ceil'd like `tassess`, so it drifts and stops firing; next indices are not bounds-checked.

**Current code** (`atassess/atsample.c` lines 115–145)

```c
  115: 
  116: 		/* Do diet sampling if appropriate */
  117: 		if ((bm->dayt == bm->teatassess) && datain) {
  118: 			Year_Of_Stomach(bm, llogfp);
  119: 
  120: 			/* Calculate diet based indices */
  121: 			Diet_Indices(bm);
  122: 		}
  123: 
  124: 		/* Calculate Indices and write output */
  125: 		if ((bm->dayt == bm->tassess) && bm->annual_assess) {
  126: 			/* Calculate Other Indices */
  127: 			Indices(bm, llogfp);
  128: 
  129: 			/* Write output files */
  130: 			Write_Index_Out(bm, nt);
  131: 		}
  132: 
  133: 		/* Do true diet sampling if appropriate */
  134: 		if ((bm->dayt == bm->teatassess) && datain) {
  135: 			Calculate_Real_Stomachs(bm, llogfp);
  136: 
  137: 			/* Calculate real OI */
  138: 			Calculate_True_Diet_Indices(bm);
  139: 
  140: 			/* Write diet output files */
  141: 			Write_True_Diet_Out(bm, nt);
  142: 
  143: 			bm->teatassess += (bm->teatassessinc * bm->tassPatchy[nexteatij][tasseat_id]);
  144: 			nexteatij++;
  145: 
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
**Listed lines:** 186

**Why it's thought to be a bug**

`flagpatchy < 2` but flagpatchy is binary.

**Current code** (`atassess/atsample.c` lines 184–188)

```c
  184: 
  185: 	/* Is this a case of punctuated snapshots with low level in between (if not skip ahead) */
  186: 	if (bm->flagpatchy < 2) {
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
**Listed lines:** 202-213

**Why it's thought to be a bug**

`nsbox = nsbox_o` can exceed the `samplearea`/`totalarea` allocation (nsbox_i).

**Current code** (`atassess/atsample.c` lines 200–215)

```c
  200: 
  201: 		/* Set nsbox for this year */
  202: 		if (intense_sampling)
  203: 			bm->nsbox = bm->nsbox_i;
  204: 		else
  205: 			bm->nsbox = bm->nsbox_o;
  206: 
  207: 		/* Load list of sample boxes */
  208: 		for (bb = 0; bb < bm->nsbox; bb++) {
  209: 			if (intense_sampling) {
  210: 				bm->nsboxes[bb] = bm->nsboxes_intense[bb];
  211: 			} else {
  212: 				bm->nsboxes[bb] = bm->nsboxes_other[bb];
  213: 			}
  214: 		}
  215: 	}
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
**Listed lines:** 238, 265, 279

**Why it's thought to be a bug**

Getters divide inconsistently by `annual_sample` (can be 0, giving inf).

**Current code** (`atassess/atsample.c` lines 236–275 (truncated))

```c
  236: 
  237: 	/* TODO: Not 100% sure this is correct - dividing by the sampleingsize */
  238: 	return samplephys[sample - 2][zone][sample_id] / (phys_samplingsize * bm->annual_sample);
  239: }
  240: 
  241: /**
  242:  * \brief Return the Standard Deviation of the sample in the given zone.
  243:  *
  244:  */
  245: double Assess_Get_Physical_Sampled_SD(MSEBoxModel *bm, int sample, int b) {
  246: 	int zone;
  247: 
  248: 	zone = bm->boxes[b].zone - 1;
  249: 	return physicalSigma[sample][zone];
  250: 
  251: }
  252: 
  253: 
  254: double Assess_Get_Process_Sampled_Value(MSEBoxModel *bm, int sample, int b){
  255: 
  256: 	int zone;
  257: 
  258: 	if(sample > 3){
  259: 		quit("Assess_Get_Process_Sampled_Value attempting to sample property %d that isn't sampled\n", sample);
  260: 	}
  261: 
  262: 	zone = bm->boxes[b].zone - 1;
  263: 
  264: 	/* Not 100% sure this is correct - dividing by the sampleingsize */
  265: 	return sampleprocess[sample][zone][sample_id]/(bm->annual_sample);
  266: }
  267: 
  268: 
  269: double Assess_Get_Biomass_Sampled_Value(MSEBoxModel *bm, int sample, int b){
  270: 
  271: 	int zone;
  272: 
  273: 	if(sample > (bm->K_num_tot_sp)){
  274: 		quit("Assess_Get_Process_Sampled_Value attempting to sample species property %d that isn't sampled\n", sample);
  275: 	}
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
**Listed lines:** 273-274

**Why it's thought to be a bug**

`>` should be `>=`; error message names the wrong function.

**Current code** (`atassess/atsample.c` lines 271–276)

```c
  271: 	int zone;
  272: 
  273: 	if(sample > (bm->K_num_tot_sp)){
  274: 		quit("Assess_Get_Process_Sampled_Value attempting to sample species property %d that isn't sampled\n", sample);
  275: 	}
  276: 
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

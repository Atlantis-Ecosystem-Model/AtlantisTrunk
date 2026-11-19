# atecology/atecologyts.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-19](#emisc-19) | 351-352 | H/M | state not reset | `if (fabs(scalar - 1.0) > 1e-100) scaled_mum = mum*scalar;` When the time series returns to 1.0, `scaled_mum` … |  | [ ] |
| [EMISC-20](#emisc-20) | 397-401 | H/M | state not reset / missing clamp | Same pattern for FSPB: when the addend returns to 0, `scaled_FSPB` stays at its old value. There is also no lo… |  | [ ] |
| [EMISC-21](#emisc-21) | 45, 91, 131, 460 | M/L | global state | `set_keyprm_errfn(err)` is never restored, so later `readkeyprm_*` calls by the caller inherit `warn`/`quiet` … |  | [ ] |

---

## EMISC-19

**state not reset** · Conf H / Sev M  
**Lines (current code):** 351-352  
**Lines at review (Oct 2026):** 351-352

**Function:** `Scale_Growth_Rate()`

**Why it's thought to be a bug**

`if (fabs(scalar - 1.0) > 1e-100) scaled_mum = mum*scalar;` When the time series returns to 1.0, `scaled_mum` keeps the last scaled value. For non-temperature-sensitive vertebrates nothing else resets it: the atq10.c:559-563 else-branch resets only `scaled_C`. Function: `Scale_Growth_Rate()`.

**Current code** (`atecology/atecologyts.c` lines 349–354)

```c
  349: 					if(scale_index != -1){
  350: 						scalar = tsEval(bm->tsGrowthRateChange, scale_index, bm->t);
  351: 						if(fabs(scalar - 1.0) > 1e-100){
  352: 							FunctGroupArray[sp].scaled_mum[cohort] = FunctGroupArray[sp].mum[cohort] * scalar;
  353:                             
  354:                             /**/
```

**Suggested fix (from review):** Always assign `scaled_mum = mum * scalar`

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

## EMISC-20

**state not reset / missing clamp** · Conf H / Sev M  
**Lines (current code):** 397-401  
**Lines at review (Oct 2026):** 397-401

**Function:** `Scale_FSPB_Rate()`

**Why it's thought to be a bug**

Same pattern for FSPB: when the addend returns to 0, `scaled_FSPB` stays at its old value. There is also no lower clamp, so a negative addend can give a negative FSPB. Function: `Scale_FSPB_Rate()`.

**Current code** (`atecology/atecologyts.c` lines 395–403)

```c
  395: 
  396: 						/* Need to check to see if this should be an addition or a scalar */
  397: 						if(fabs(scalar) > 1e-100){
  398: 							FunctGroupArray[sp].scaled_FSPB[cohort] = FunctGroupArray[sp].FSPB[cohort] + scalar;
  399: 							if(FunctGroupArray[sp].scaled_FSPB[cohort] > 1.0)
  400: 								FunctGroupArray[sp].scaled_FSPB[cohort] = 1.0;
  401: 
  402:                             /**/
  403: 							//if((bm->dayt > bm->checkstart) && (bm->debug == debug_FSPB)){
```

**Suggested fix (from review):** Always assign `FSPB + scalar`, then clamp to [0,1]

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

## EMISC-21

**global state** · Conf M / Sev L  
**Lines (current code):** 45, 91, 131, 460  
**Lines at review (Oct 2026):** 45, 91, 131, 460

**Why it's thought to be a bug**

`set_keyprm_errfn(err)` is never restored, so later `readkeyprm_*` calls by the caller inherit `warn`/`quiet` instead of `quit`. Functions: `Read_Ecology_Time_Series()`, `Ecology_Read_Recruit_TS()`, `Ecology_Read_Enviro_Forcing()`, `Ecology_Read_pCO2_TS()`.

**Current code** (`atecology/atecologyts.c` lines 43–47)

```c
   43: 	/** Get time series **/
   44: 	/* Find time series name in parameter file */
   45: 	set_keyprm_errfn(err);
   46: 	if (!readkeyprm_s(fp, key, buf)) {
   47: 		fclose(fp);
```

**Suggested fix (from review):** Restore `set_keyprm_errfn(quit)` before returning

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

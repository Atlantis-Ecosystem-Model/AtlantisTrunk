# atphysics/atbioirrig.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-06](#phy3-06) ⭐ | 60,69,78,87 | H/H | hard-coded index / out of bounds | `if (!_finite(newwc[24][0][Salinity_i]))`: leftover debug code with box 24 hard-coded, run for every box. Any … |  | [ ] |
| [PHY3-07](#phy3-07) | 195-202 | M/M | unit mismatch | Oxygen case: `pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num` is a dimensionless ratio, not a po… |  | [ ] |

---

## PHY3-06

**hard-coded index / out of bounds** · Conf H / Sev H ⭐ priority  
**Listed lines:** 60,69,78,87

**Function:** `bioirrig()`

**Why it's thought to be a bug**

`if (!_finite(newwc[24][0][Salinity_i]))`: leftover debug code with box 24 hard-coded, run for every box. Any model with `nbox <= 24` reads past the `newwc` pointer array (UB / segfault). Otherwise it checks the wrong box. Function: `bioirrig()`.

**Current code** (`atphysics/atbioirrig.c` lines 58–80)

```c
   58: 		if (bp->type != BOUNDARY && bp->type != LAND) {
   59: 
   60:             if (!_finite(newwc[24][0][Salinity_i])) {
   61:                 printf("Time: %e Salinity in bottom water is nan - while doing box %d in bioirrig\n", bm->dayt, b);
   62:                 fflush(stdout);
   63:                 fflush(stderr);
   64:             }
   65:             
   66: 			/* Diffusion */
   67: 			dissol_diff(bm, bp, sm, newwc[b], newsed[b]);
   68:             
   69:             if (!_finite(newwc[24][0][Salinity_i])) {
   70:                 printf("Time: %e Salinity in bottom water is nan - while doing bioirrig B\n", bm->dayt);
   71:                 fflush(stdout);
   72:                 fflush(stderr);
   73:             }
   74: 
   75: 			/* Exchange */
   76: 			exchange(bm, bp, sm, newwc[b], newsed[b]);
   77:             
   78:             if (!_finite(newwc[24][0][Salinity_i])) {
   79:                 printf("Time: %e Salinity in bottom water is nan - while doing bioirrig C\n", bm->dayt);
   80:                 fflush(stdout);
```

**Suggested fix (from review):** Delete these checks or use `newwc[b]`

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

## PHY3-07

**unit mismatch** · Conf M / Sev M  
**Listed lines:** 195-202

**Function:** `exchange()`

**Why it's thought to be a bug**

Oxygen case: `pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num` is a dimensionless ratio, not a pore-water volume in m3. It is then used as a volume in `ek = rate*(1/wvol+1/pvol)` and `ce = (...*wvol + ...*pvol)/(wvol+pvol)`. Oxygen exchange is therefore hugely over-damped and mass is not conserved. Compare the non-oxygen branch at line 235: `porosity*volume`. Function: `exchange()`.

**Current code** (`atphysics/atbioirrig.c` lines 193–204)

```c
  193: 				 Walker had implemented for all dissolved groups */
  194: 				/* Volume of pore water */
  195: 				double oxvol = sm->oxdepth * bp->area;
  196: 				/* Uses same form as before but adjusts volume being considered
  197: 				 to that due to oxygen depth rather than just the sediment layer
  198: 				 - FIX this is OK if only have one sediment layer in implementation
  199: 				 (as was case for PhD work) must be corrected if more than one layer used.
  200: 				 The 0.00000000001 was added so that pvol was never exactly equal to
  201: 				 zero as you must divide by it*/
  202: 				double pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num;
  203: 				/* Constants in exponential formula */
  204: 				double ek = rate[k] * (1.0 / wvol + 1.0 / pvol);
```

**Suggested fix (from review):** `pvol = sm->porosity[k] * min(oxvol, sm->volume[k]) + small_num`

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

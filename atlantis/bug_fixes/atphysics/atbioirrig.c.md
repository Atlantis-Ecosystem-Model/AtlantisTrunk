# atphysics/atbioirrig.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-06](#phy3-06) ⭐ | 60 | H/H | hard-coded index / out of bounds | `if (!_finite(newwc[24][0][Salinity_i]))`: leftover debug code with box 24 hard-coded, run for every box. Any … |  | [ ] |
| [PHY3-07](#phy3-07) | 202, 205 | M/M | unit mismatch | Oxygen case: `pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num` is a dimensionless ratio, not a po… |  | [ ] |

---

## PHY3-06

**hard-coded index / out of bounds** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atbioirrig.c` 60
**Lines at review (Oct 2026):** 60,69,78,87

**Function:** `bioirrig()`

**Why it's thought to be a bug**

`if (!_finite(newwc[24][0][Salinity_i]))`: leftover debug code with box 24 hard-coded, run for every box. Any model with `nbox <= 24` reads past the `newwc` pointer array (UB / segfault). Otherwise it checks the wrong box. Function: `bioirrig()`.

**Code at issue** — `atbioirrig.c` line 60  
Search for: `if (!_finite(newwc[24][0][Salinity_i]))`

```c
   58: 		if (bp->type != BOUNDARY && bp->type != LAND) {
   59: 
   60→             if (!_finite(newwc[24][0][Salinity_i])) {
   61:                 printf("Time: %e Salinity in bottom water is nan - while doing box %d in bioirrig\n", bm->dayt, b);
   62:                 fflush(stdout);
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
**Found at (current code):** `atbioirrig.c` 202, `atbioirrig.c` 205
**Lines at review (Oct 2026):** 195-202

**Function:** `exchange()`

**Why it's thought to be a bug**

Oxygen case: `pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num` is a dimensionless ratio, not a pore-water volume in m3. It is then used as a volume in `ek = rate*(1/wvol+1/pvol)` and `ce = (...*wvol + ...*pvol)/(wvol+pvol)`. Oxygen exchange is therefore hugely over-damped and mass is not conserved. Compare the non-oxygen branch at line 235: `porosity*volume`. Function: `exchange()`.

**Code at issue** — `atbioirrig.c` line 202  
Search for: `pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num`

```c
  200: 				 The 0.00000000001 was added so that pvol was never exactly equal to
  201: 				 zero as you must divide by it*/
  202→ 				double pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num;
  203: 				/* Constants in exponential formula */
  204: 				double ek = rate[k] * (1.0 / wvol + 1.0 / pvol);
```

**Code at issue** — `atbioirrig.c` line 205  
Search for: `* wvol +`

```c
  203: 				/* Constants in exponential formula */
  204: 				double ek = rate[k] * (1.0 / wvol + 1.0 / pvol);
  205→ 				double ce = (newwc[0][n] * wvol + newsed[k][n] * pvol) / (wvol + pvol);
  206: 				/* New water column concentration */
  207: 				double cw = ce + decay_exact(newwc[0][n] - ce, ek, bm->dt);
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

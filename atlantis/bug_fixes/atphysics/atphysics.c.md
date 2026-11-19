# atphysics/atphysics.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-20](#phy1-20) ⭐ | 700-702 | H/H | out of bounds (debug leftover) | `if (!_finite(newwctr[24][0][Salinity_i]))` hard-codes box 24 and runs every time step. `newwctr` has `nbox` r… |  | [ ] |
| [PHY1-21](#phy1-21) | 573 | H/M | wrong variable | `if (lndreclaim[b] < bm->tstop)`: `lndreclaim` is the key string `"land_reclaim"`, so this compares a characte… |  | [ ] |
| [PHY1-22](#phy1-22) | 547, 552, 561 | M/M | truncation | `bm->ship_mixing_rate[b] = (int) (ship_info[b]);` (likewise `vessel_size`) casts to int before storing into do… |  | [ ] |
| [PHY1-23](#phy1-23) | 558-562 | M/L | copy-paste duplicate | `ship_mixing_rate` is read a second time with the same key, `ship_mixing`, into the same array (counter set to… |  | [ ] |
| [PHY1-24](#phy1-24) | 610-611 | M/M | index -1 | `this_nz = nz - 1; base_cell_vol += area * dz[this_nz];`: boxes with `nz == 0` (dry/island boxes, see atvertge… |  | [ ] |
| [PHY1-25](#phy1-25) | 284 | L/L | wrong operator | `if (bm->tidal & (bm->wc_dz_tol < 1.0))` uses bitwise `&`. It works only while `tidal` is 0/1. Function: `init… |  | [ ] |

---

## PHY1-20

**out of bounds (debug leftover)** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 700-702  
**Lines at review (Oct 2026):** 700-702

**Function:** `physics()`

**Why it's thought to be a bug**

`if (!_finite(newwctr[24][0][Salinity_i]))` hard-codes box 24 and runs every time step. `newwctr` has `nbox` rows (alloc3d(ntracer,wcnz,nbox)), so any model with fewer than 25 boxes reads past the row-pointer array and will likely segfault. Function: `physics()`.

**Current code** (`atphysics/atphysics.c` lines 698–704)

```c
  698:     }
  699: 
  700:     if (!_finite(newwctr[24][0][Salinity_i])) {
  701:         printf("Time: %e Salinity in bottom water is nan - E\n", bm->dayt);
  702:     }
  703: 
  704:     /* Resuspension, sediments.
```

**Suggested fix (from review):** Delete these debug lines

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

## PHY1-21

**wrong variable** · Conf H / Sev M  
**Lines (current code):** 573  
**Lines at review (Oct 2026):** 573

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`if (lndreclaim[b] < bm->tstop)`: `lndreclaim` is the key string `"land_reclaim"`, so this compares a character code with `tstop` and reads past the literal when `b >= 13`. It should test the reclaim day just read. Function: `initPhysics()` (only with `terrestrial_on`).

**Current code** (`atphysics/atphysics.c` lines 571–575)

```c
  571:             bm->boxes[b].land.day_reclaim = dcay_scalar[b];
  572:             
  573:             if (lndreclaim[b] < bm->tstop)
  574:                 bm->flag_reclaim_land = TRUE;
  575:         }
```

**Suggested fix (from review):** `if (dcay_scalar[b] < bm->tstop)` (i.e. `land.day_reclaim`)

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

## PHY1-22

**truncation** · Conf M / Sev M  
**Lines (current code):** 547, 552, 561  
**Lines at review (Oct 2026):** 547, 552, 561

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`bm->ship_mixing_rate[b] = (int) (ship_info[b]);` (likewise `vessel_size`) casts to int before storing into double arrays (`Util_Alloc_Init_1D_Double`). Fractional mixing rates such as 0.3 become 0. Function: `initPhysics()` (simple industry model).

**Current code** (`atphysics/atphysics.c` lines 545–563)

```c
  545:         readkeyprm_darray(pfp, ship_mixing, &ship_info, &counter);
  546:         for (b = 0; b < bm->K_num_vessel_sizes; b++) {
  547:             bm->ship_mixing_rate[b] = (int) (ship_info[b]);
  548:         }
  549:         counter = bm->K_num_vessel_sizes;
  550:         readkeyprm_darray(pfp, vessel_size, &ship_info, &counter);
  551:         for (b = 0; b < bm->K_num_vessel_sizes; b++) {
  552:             bm->vessel_size[b] = (int) (ship_info[b]);
  553:         }
  554:         
  555:         /* Read in ship strike mortality scalar - weighting on likelihood of ship strike causing mortality */
  556:         readkeyprm_d(pfp, "strike_mort_scalar", &bm->strike_mort_scalar);
  557: 
  558:         counter = bm->nbox;
  559:         readkeyprm_darray(pfp, ship_mixing, &ship_info, &counter);
  560:         for (b = 0; b < bm->K_num_vessel_sizes; b++) {
  561:             bm->ship_mixing_rate[b] = (int) (ship_info[b]);
  562:         }
  563: 
```

**Suggested fix (from review):** Drop the `(int)` casts

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

## PHY1-23

**copy-paste duplicate** · Conf M / Sev L  
**Lines (current code):** 558-562  
**Lines at review (Oct 2026):** 558-562

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`ship_mixing_rate` is read a second time with the same key, `ship_mixing`, into the same array (counter set to `nbox`, loop over vessel sizes). It is redundant and was probably meant to read a different parameter. Function: `initPhysics()`.

**Current code** (`atphysics/atphysics.c` lines 556–564)

```c
  556:         readkeyprm_d(pfp, "strike_mort_scalar", &bm->strike_mort_scalar);
  557: 
  558:         counter = bm->nbox;
  559:         readkeyprm_darray(pfp, ship_mixing, &ship_info, &counter);
  560:         for (b = 0; b < bm->K_num_vessel_sizes; b++) {
  561:             bm->ship_mixing_rate[b] = (int) (ship_info[b]);
  562:         }
  563: 
  564:     }
```

**Suggested fix (from review):** Remove, or replace with the intended key

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

## PHY1-24

**index -1** · Conf M / Sev M  
**Lines (current code):** 610-611  
**Lines at review (Oct 2026):** 610-611

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`this_nz = nz - 1; base_cell_vol += area * dz[this_nz];`: boxes with `nz == 0` (dry/island boxes, see atvertgeom.c:154 and :613) read `dz[-1]`, which is the previous box's last sediment cell or memory before the allocation. The average also divides by all boxes, including those. Function: `initPhysics()`.

**Current code** (`atphysics/atphysics.c` lines 608–613)

```c
  608: 		}
  609: 
  610:         this_nz = bm->boxes[b].nz - 1;
  611:         base_cell_vol += (bm->boxes[b].area * bm->boxes[b].dz[this_nz]);
  612: 	}
  613:     bm->avg_cell_vol = base_cell_vol / ((double)bm->nbox);
```

**Suggested fix (from review):** Skip boxes with `nz < 1` and count only wet boxes

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

## PHY1-25

**wrong operator** · Conf L / Sev L  
**Lines (current code):** 284  
**Lines at review (Oct 2026):** 284

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`if (bm->tidal & (bm->wc_dz_tol < 1.0))` uses bitwise `&`. It works only while `tidal` is 0/1. Function: `initPhysics()`.

**Current code** (`atphysics/atphysics.c` lines 282–286)

```c
  282: 	 */
  283: 	readkeyprm_d(pfp, "wc_dz_tol", &bm->wc_dz_tol);
  284:     if (bm->tidal & (bm->wc_dz_tol < 1.0))
  285:         warn("Tidal model but you have not allowed for cells to completely empty or swell by more than 100%\n");
  286: 
```

**Suggested fix (from review):** `&&`

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

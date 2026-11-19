# atphysics/atphysics.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-20](#phy1-20) ⭐ | 700 | H/H | out of bounds (debug leftover) | `if (!_finite(newwctr[24][0][Salinity_i]))` hard-codes box 24 and runs every time step. `newwctr` has `nbox` r… |  | [ ] |
| [PHY1-21](#phy1-21) | 573, 200 | H/M | wrong variable | `if (lndreclaim[b] < bm->tstop)`: `lndreclaim` is the key string `"land_reclaim"`, so this compares a characte… |  | [ ] |
| [PHY1-22](#phy1-22) | 547 | M/M | truncation | `bm->ship_mixing_rate[b] = (int) (ship_info[b]);` (likewise `vessel_size`) casts to int before storing into do… |  | [ ] |
| [PHY1-23](#phy1-23) | 196 | M/L | copy-paste duplicate | `ship_mixing_rate` is read a second time with the same key, `ship_mixing`, into the same array (counter set to… |  | [ ] |
| [PHY1-24](#phy1-24) | 611 | M/M | index -1 | `this_nz = nz - 1; base_cell_vol += area * dz[this_nz];`: boxes with `nz == 0` (dry/island boxes, see atvertge… |  | [ ] |
| [PHY1-25](#phy1-25) | 284 | L/L | wrong operator | `if (bm->tidal & (bm->wc_dz_tol < 1.0))` uses bitwise `&`. It works only while `tidal` is 0/1. Function: `init… |  | [ ] |

---

## PHY1-20

**out of bounds (debug leftover)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atphysics.c` 700
**Lines at review (Oct 2026):** 700-702

**Function:** `physics()`

**Why it's thought to be a bug**

`if (!_finite(newwctr[24][0][Salinity_i]))` hard-codes box 24 and runs every time step. `newwctr` has `nbox` rows (alloc3d(ntracer,wcnz,nbox)), so any model with fewer than 25 boxes reads past the row-pointer array and will likely segfault. Function: `physics()`.

**Code at issue** — `atphysics.c` line 700  
Search for: `if (!_finite(newwctr[24][0][Salinity_i]))`

```c
  698:     }
  699: 
  700→     if (!_finite(newwctr[24][0][Salinity_i])) {
  701:         printf("Time: %e Salinity in bottom water is nan - E\n", bm->dayt);
  702:     }
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
**Found at (current code):** `atphysics.c` 573, `atphysics.c` 200
**Lines at review (Oct 2026):** 573

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`if (lndreclaim[b] < bm->tstop)`: `lndreclaim` is the key string `"land_reclaim"`, so this compares a character code with `tstop` and reads past the literal when `b >= 13`. It should test the reclaim day just read. Function: `initPhysics()` (only with `terrestrial_on`).

**Code at issue** — `atphysics.c` line 573  
Search for: `if (lndreclaim[b] < bm->tstop)`

```c
  571:             bm->boxes[b].land.day_reclaim = dcay_scalar[b];
  572:             
  573→             if (lndreclaim[b] < bm->tstop)
  574:                 bm->flag_reclaim_land = TRUE;
  575:         }
```

**Code at issue** — `atphysics.c` line 200  
Search for: `"land_reclaim"`

```c
  198:     char *dredge_path = "dredge_path";
  199:     char *dscalar = "decay_scalar";
  200→     char *lndreclaim = "land_reclaim";
  201: 
  202: 	/* Create local vectors */
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
**Found at (current code):** `atphysics.c` 547
**Lines at review (Oct 2026):** 547, 552, 561

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`bm->ship_mixing_rate[b] = (int) (ship_info[b]);` (likewise `vessel_size`) casts to int before storing into double arrays (`Util_Alloc_Init_1D_Double`). Fractional mixing rates such as 0.3 become 0. Function: `initPhysics()` (simple industry model).

**Code at issue** — `atphysics.c` line 547  
Search for: `ship_mixing_rate[b] = (int) (ship_info[b])`

```c
  545:         readkeyprm_darray(pfp, ship_mixing, &ship_info, &counter);
  546:         for (b = 0; b < bm->K_num_vessel_sizes; b++) {
  547→             bm->ship_mixing_rate[b] = (int) (ship_info[b]);
  548:         }
  549:         counter = bm->K_num_vessel_sizes;
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
**Found at (current code):** `atphysics.c` 196
**Lines at review (Oct 2026):** 558-562

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`ship_mixing_rate` is read a second time with the same key, `ship_mixing`, into the same array (counter set to `nbox`, loop over vessel sizes). It is redundant and was probably meant to read a different parameter. Function: `initPhysics()`.

**Code at issue** — `atphysics.c` line 196 (located by name only; check this is the right place)  
Search for: `char *ship_mixing = "ship_mixing_rate";`

```c
  194: 	char *eddy_S4 = "eddy_S4";
  195:     char *ship_channels = "ship_channels";
  196→     char *ship_mixing = "ship_mixing_rate";
  197:     char *vessel_size = "vessel_size";
  198:     char *dredge_path = "dredge_path";
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
**Found at (current code):** `atphysics.c` 611
**Lines at review (Oct 2026):** 610-611

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`this_nz = nz - 1; base_cell_vol += area * dz[this_nz];`: boxes with `nz == 0` (dry/island boxes, see atvertgeom.c:154 and :613) read `dz[-1]`, which is the previous box's last sediment cell or memory before the allocation. The average also divides by all boxes, including those. Function: `initPhysics()`.

**Code at issue** — `atphysics.c` line 611 (the line the review pointed to, found by its text)  
Search for: `base_cell_vol += (bm->boxes[b].area * bm->boxes[b].dz[this_nz]);`

```c
  609: 
  610:         this_nz = bm->boxes[b].nz - 1;
  611→         base_cell_vol += (bm->boxes[b].area * bm->boxes[b].dz[this_nz]);
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
**Found at (current code):** `atphysics.c` 284
**Lines at review (Oct 2026):** 284

**Function:** `initPhysics()`

**Why it's thought to be a bug**

`if (bm->tidal & (bm->wc_dz_tol < 1.0))` uses bitwise `&`. It works only while `tidal` is 0/1. Function: `initPhysics()`.

**Code at issue** — `atphysics.c` line 284  
Search for: `if (bm->tidal & (bm->wc_dz_tol < 1.0))`

```c
  282: 	 */
  283: 	readkeyprm_d(pfp, "wc_dz_tol", &bm->wc_dz_tol);
  284→     if (bm->tidal & (bm->wc_dz_tol < 1.0))
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

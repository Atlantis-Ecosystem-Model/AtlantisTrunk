# atphysics/atvmix.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-13](#phy3-13) | 135-139, 161-165 | H/M | mass conservation (updated value reused) | `c[k]` is overwritten first, then `c[k-1] = ((lvol-cvol)*c[k-1] + cvol*c[k])/lvol` uses the *new* `c[k]`. The … |  | [ ] |
| [PHY3-14](#phy3-14) | 142-143,168-169 | M/L | accumulator inside tracer loop / sign | `bp->vflux[k] += cvol` sits inside the per-tracer loop, so the water-flux diagnostic grows with the number of … |  | [ ] |

---

## PHY3-13

**mass conservation (updated value reused)** · Conf H / Sev M  
**Listed lines:** 135-139, 161-165

**Function:** `vertical_mixing()`

**Why it's thought to be a bug**

`c[k]` is overwritten first, then `c[k-1] = ((lvol-cvol)*c[k-1] + cvol*c[k])/lvol` uses the *new* `c[k]`. The total changes by `cvol*(c[k]'-c[k])`, so mass is not conserved in either branch. Function: `vertical_mixing()`.

**Current code** (`atphysics/atvmix.c` lines 133–167)

```c
  133: 						/* New concentration in current cell
  134: 						 (average of what's there and what's mixed back) */
  135: 						c[k] = ((wvol - cvol) * c[k] + c[k - 1] * cvol) / (wvol + small_num);
  136: 
  137: 						/* New concentration in sink cell
  138: 						 (average of what's there and what's transferred amount added) */
  139: 						c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);
  140: 
  141: 						/* Update vertical flux trackers */
  142: 						bp->vflux[k - 1] += cvol;
  143: 						bp->vflux[k] -= cvol;
  144: 
  145: 						/**
  146:                         //if(( b == 47 ) && (bm->newmonth)) {
  147:                             fprintf(bm->logFile,"day: %e, box: %d-%d, %s c: %e, c[%d]: %e, cvol: %e, lvol: %e, wvol: %e, mrate: %e, vflux-%d: %e\n",
  148:                                     bm->dayt, b, k, bm->tinfo[n].name, c[k], k-1, c[k-1], cvol, lvol, wvol, mrate[k], k - 1, bp->vflux[k - 1]);
  149:                             fflush(bm->logFile);
  150:                         }
  151: 						**/
  152: 					} else {
  153: 						/* Check amount to exchange is not more than in the cells */
  154: 						if (lvol < cvol)
  155: 							cvol = lvol - small_num;
  156: 						if (wvol < cvol)
  157: 							cvol = wvol - small_num;
  158: 
  159: 						/* New concentration in current cell
  160: 						 (average of what's there and what's mixed back) */
  161: 						c[k] = (cvol * c[k - 1] + (wvol - cvol) * c[k]) / (wvol + small_num);
  162: 
  163: 						/* New concentration in sink cell
  164: 						 (average of what's there and what's transferred amount added) */
  165: 						c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);
  166: 
  167: 						/* Update vertical flux trackers */
```

**Suggested fix (from review):** Save `ck = c[k], ck1 = c[k-1]` before updating and use the originals

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

## PHY3-14

**accumulator inside tracer loop / sign** · Conf M / Sev L  
**Listed lines:** 142-143,168-169

**Function:** `vertical_mixing()`

**Why it's thought to be a bug**

`bp->vflux[k] += cvol` sits inside the per-tracer loop, so the water-flux diagnostic grows with the number of dissolved tracers. The Oxygen branch also flips the sign (`vflux[k-1] += cvol`). vflux is written to output (atphysIO.c:824). Function: `vertical_mixing()`.

**Current code** (`atphysics/atvmix.c` lines 140–171)

```c
  140: 
  141: 						/* Update vertical flux trackers */
  142: 						bp->vflux[k - 1] += cvol;
  143: 						bp->vflux[k] -= cvol;
  144: 
  145: 						/**
  146:                         //if(( b == 47 ) && (bm->newmonth)) {
  147:                             fprintf(bm->logFile,"day: %e, box: %d-%d, %s c: %e, c[%d]: %e, cvol: %e, lvol: %e, wvol: %e, mrate: %e, vflux-%d: %e\n",
  148:                                     bm->dayt, b, k, bm->tinfo[n].name, c[k], k-1, c[k-1], cvol, lvol, wvol, mrate[k], k - 1, bp->vflux[k - 1]);
  149:                             fflush(bm->logFile);
  150:                         }
  151: 						**/
  152: 					} else {
  153: 						/* Check amount to exchange is not more than in the cells */
  154: 						if (lvol < cvol)
  155: 							cvol = lvol - small_num;
  156: 						if (wvol < cvol)
  157: 							cvol = wvol - small_num;
  158: 
  159: 						/* New concentration in current cell
  160: 						 (average of what's there and what's mixed back) */
  161: 						c[k] = (cvol * c[k - 1] + (wvol - cvol) * c[k]) / (wvol + small_num);
  162: 
  163: 						/* New concentration in sink cell
  164: 						 (average of what's there and what's transferred amount added) */
  165: 						c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);
  166: 
  167: 						/* Update vertical flux trackers */
  168: 						bp->vflux[k] += cvol;
  169: 						bp->vflux[k - 1] -= cvol;
  170: 
  171: 						/**
```

**Suggested fix (from review):** Accumulate vflux once per box/layer, outside the tracer loop, with a consistent sign

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

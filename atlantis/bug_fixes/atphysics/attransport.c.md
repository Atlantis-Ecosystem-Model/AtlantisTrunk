# atphysics/attransport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-16](#phy2-16) | 398-401 | H/M | leftover debug / out-of-bounds | This debug block is live: `if(bm->dayt > 1090.0 && strcmp(name,"salt")==0) fprintf(llogfp, ..., newwc[23][1][S… |  | [ ] |
| [PHY2-17](#phy2-17) | 207-216 | M/L | dead condition | `if ((bb < 0) \|\| (kk < 0))` sits inside `if (bm->hd.b[b][k][d] >= 0 ...)`, so `bb < 0` can never be true. Only… |  | [ ] |

---

## PHY2-16

**leftover debug / out-of-bounds** · Conf H / Sev M  
**Listed lines:** 398-401

**Function:** `transportBM()`

**Why it's thought to be a bug**

This debug block is live: `if(bm->dayt > 1090.0 && strcmp(name,"salt")==0) fprintf(llogfp, ..., newwc[23][1][Salinity_i], ...)`. It hard-codes box 23 / layer 1, which reads out of bounds in models with fewer than 24 boxes or 2 layers. It also writes and flushes one log line per box-layer per transport step after day 1090. Function: `transportBM()`.

**Current code** (`atphysics/attransport.c` lines 396–403)

```c
  396: 							 **/
  397:                             
  398:                             /**
  399:                             if(bm->dayt > 1090.0 && (strcmp(bm->tinfo[n].name, "salt") == 0)){
  400:                                 fprintf(llogfp,"Time %e transportBM box%d-%d for tracer %d (%s) - salinity now %e and %s was %.10g, now %.10g, oldvol = %.10g, newvol = %.10g, dtr = %.10g, dvol = %.10g\n", bm->dayt, b, k, n, bm->tinfo[n].name, newwc[23][1][Salinity_i], bm->tinfo[n].name, newwc[b][k][n], newwat, oldvol, newvol, dtr[b][k][n], dvol[b][k]);
  401:                                 fflush(llogfp);
  402:                             }
  403:                             **/
```

**Suggested fix (from review):** Delete or comment out the block, like the neighbouring debug blocks

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

## PHY2-17

**dead condition** · Conf M / Sev L  
**Listed lines:** 207-216

**Function:** `transportBM()`

**Why it's thought to be a bug**

`if ((bb < 0) || (kk < 0))` sits inside `if (bm->hd.b[b][k][d] >= 0 ...)`, so `bb < 0` can never be true. Only invalid `kk` is caught. Destinations with `kk >= boxes[bb].nz` (valid < wcnz but below the box bottom) are not caught either (line 251 only checks `kk >= wcnz`). Function: `transportBM()`.

**Current code** (`atphysics/attransport.c` lines 219–232) — code appears to have moved +14 lines since the review

```c
  219:                         if ((bp->edge_type == 3) && (bm->hd.exch[b][k][d] < 0)) {
  220:                             /* If negative base on the cell its leaving from if flow fields not symmetrical */
  221:                             k_transcale_final = bm->boxes[bb].horizmix * k_transcale;
  222:                         } else if ((bb == b) && (!bm->flag_replicated_old)) {
  223:                             k_transcale_final = bm->boxes[bb].vertmix * k_transcale;
  224:                         } else {
  225:                             /* Otherwise assume symmetrical scaling of flows */
  226:                             k_transcale_final = bp->horizmix * k_transcale;
  227:                         }
  228:                         
  229:                         /* Deal with reflective and absorptive boundaries */
  230:                         if ((bp->edge_type == 2) && (bm->hd.exch[b][k][d] < 0)) // Reflective
  231:                             exchange_amt = -bm->hd.exch[b][k][d];
  232:                         else if ((bp->edge_type == 1) && (bm->hd.exch[b][k][d] > 0)) // Absorptive
```

**Suggested fix (from review):** Check `kk >= bm->boxes[bb].nz` as well

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

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

athydromod.c `get_hydro()` (165-166) copies `nbuf` entries of `value` into `tbuf`, but only `count[0]` were read. Entries past `bufend` are never used, so this is harmless.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

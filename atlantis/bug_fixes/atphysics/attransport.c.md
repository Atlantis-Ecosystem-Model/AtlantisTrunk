# atphysics/attransport.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-16](#phy2-16) | 400-404 | H/M | leftover debug / out-of-bounds | This debug block is live: `if(bm->dayt > 1090.0 && strcmp(name,"salt")==0) fprintf(llogfp, ..., newwc[23][1][S… |  | [ ] |
| [PHY2-17](#phy2-17) | 207-216 | M/L | dead condition | `if ((bb < 0) \|\| (kk < 0))` sits inside `if (bm->hd.b[b][k][d] >= 0 ...)`, so `bb < 0` can never be true. Only… |  | [ ] |

---

## PHY2-16

**leftover debug / out-of-bounds** · Conf H / Sev M  
**Lines (current code):** 400-404  
**Lines at review (Oct 2026):** 398-401

**Function:** `transportBM()`

**Why it's thought to be a bug**

This debug block is live: `if(bm->dayt > 1090.0 && strcmp(name,"salt")==0) fprintf(llogfp, ..., newwc[23][1][Salinity_i], ...)`. It hard-codes box 23 / layer 1, which reads out of bounds in models with fewer than 24 boxes or 2 layers. It also writes and flushes one log line per box-layer per transport step after day 1090. Function: `transportBM()`.

**Current code** (`atphysics/attransport.c` lines 398–406)

```c
  398:                             /**
  399:                             if(bm->dayt > 1090.0 && (strcmp(bm->tinfo[n].name, "salt") == 0)){
  400:                                 fprintf(llogfp,"Time %e transportBM box%d-%d for tracer %d (%s) - salinity now %e and %s was %.10g, now %.10g, oldvol = %.10g, newvol = %.10g, dtr = %.10g, dvol = %.10g\n", bm->dayt, b, k, n, bm->tinfo[n].name, newwc[23][1][Salinity_i], bm->tinfo[n].name, newwc[b][k][n], newwat, oldvol, newvol, dtr[b][k][n], dvol[b][k]);
  401:                                 fflush(llogfp);
  402:                             }
  403:                             **/
  404: 						} else{
  405: 							newwat = newwc[b][k][n];
  406: 						}
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
**Lines (current code):** 207-216  
**Lines at review (Oct 2026):** 207-216

**Function:** `transportBM()`

**Why it's thought to be a bug**

`if ((bb < 0) || (kk < 0))` sits inside `if (bm->hd.b[b][k][d] >= 0 ...)`, so `bb < 0` can never be true. Only invalid `kk` is caught. Destinations with `kk >= boxes[bb].nz` (valid < wcnz but below the box bottom) are not caught either (line 251 only checks `kk >= wcnz`). Function: `transportBM()`.

**Current code** (`atphysics/attransport.c` lines 205–218) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  205:                         int kk = bm->hd.k[b][k][d];
  206:                         /* FIX - maybe should check bb and kk for validity here */
  207:                         if ((bb < 0) || (kk < 0)) {
  208:                             if(bm->mirror_invalid == true_mirror){
  209:                                 kk = k;  // out of desperation equate the source and destinaiton layers
  210:                             } else if (bm->mirror_invalid == zero_invalid){
  211:                                 kk = 0;  // or just send everything to the bottom layers
  212:                             } else {
  213:                                 quit("You have an exchange_amt of %e coming from cell box %d layer %d, destination entry %d, but you have destination box of %d and layer %d, which isn't valid\n", bm->hd.exch[b][k][d], b, k, d, bb, kk);
  214:                             }
  215: 
  216:                         }
  217: 
  218:                         /* Exchange volume */
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

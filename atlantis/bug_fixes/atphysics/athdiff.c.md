# atphysics/athdiff.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-16](#phy3-16) | 56-65 | H/M | out of bounds / misuse of diffusion1d | `diffusion1d(nnconn, hc, hx, hk, hx, ...)` needs `k` and `xk` arrays of n+1 values (sjwlib/diffusion.c:70-73).… |  | [ ] |

---

## PHY3-16

**out of bounds / misuse of diffusion1d** · Conf H / Sev M  
**Listed lines:** 56-65

**Function:** `hdiffBMwc()`

**Why it's thought to be a bug**

`diffusion1d(nnconn, hc, hx, hk, hx, ...)` needs `k` and `xk` arrays of n+1 values (sjwlib/diffusion.c:70-73). `hk`/`hx` are allocated `max_nconn` (atphysics.c:225-226), so when nconn==max_nconn they are read past the end. Also: (a) `k[0]` and `k[n]` are boundary *fluxes* but are set to `wc_kz` (atphysics.c:277), which injects spurious mass `dt*k[0]/dx`; (b) the same `hx` is passed as both cell and interface coordinates, and neighbour x-coords are not sorted, so dx can be negative; (c) box b itself is excluded and neighbours with `nz<=k` are read. Active when `horiz_diffusion==1`. Function: `hdiffBMwc()`.

**Current code** (`atphysics/athdiff.c` lines 54–67)

```c
   54: 
   55:                 /* Collect tracer values for doing diffusion for each layer in each adjoining box */
   56:                 nnconn = bm->boxes[b].nconn;
   57:                 for(k=0; k<bp->nz; k++){
   58:                     for(nb=0; nb<bm->boxes[b].nconn; nb++){
   59:                         bb = bm->boxes[b].ibox[nb];
   60:                         bm->hc[nb] = newval[bb][k][n];
   61:                         bm->hx[nb] = bm->boxes[bb].inside.x;
   62:                     }
   63: 
   64:                     /* Calculate diffusion */
   65:                     diffusion1d(nnconn,bm->hc,bm->hx,bm->hk,bm->hx,dt,bm->a_wc);
   66: 
   67:                     /* Store new values */
```

**Suggested fix (from review):** Rewrite (e.g. reuse the filler_hdiffBMwc approach); at minimum size arrays n+1 and zero the end fluxes

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

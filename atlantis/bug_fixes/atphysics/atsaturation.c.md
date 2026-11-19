# atphysics/atsaturation.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-25](#phy3-25) | 44,73,89 | H/L | uninitialised variable | `do_break++` runs on an uninitialised `int do_break` (UB). The value is reset before use at line 98, so the re… |  | [ ] |

---

## PHY3-25

**uninitialised variable** · Conf H / Sev L  
**Listed lines:** 44,73,89

**Function:** `Saturation_Check()`

**Why it's thought to be a bug**

`do_break++` runs on an uninitialised `int do_break` (UB). The value is reset before use at line 98, so the result is harmless. Function: `Saturation_Check()`.

**Current code** (`atphysics/atsaturation.c` lines 42–81 (truncated))

```c
   42: 
   43: void Saturation_Check(MSEBoxModel *bm, double ***newwc, double ***newsed) {
   44: 	int b, n, k, do_break;
   45: 	double DIN, NH = 0, NO = 0;
   46:     double rand_contrib = 0.9 + drandom(0.0, 0.2);
   47:     double DINbound = min(12500.0, 12500.0 * bm->BGC_bound_scalar * rand_contrib);
   48:     double SIbound = min(120000.0, 12500.0 * bm->BGC_bound_scalar * bm->X_SiN * rand_contrib);
   49:     
   50: 	if (verbose)
   51: 		fprintf(stderr, "Entering Saturation_Check\n");
   52: 
   53: 	/* Loop over each box */
   54: 	for (b = 0; b < bm->nbox; b++) {
   55: 		Box *bp = &bm->boxes[b];
   56: 		SedModel *sm = &bm->boxes[b].sm;
   57: 
   58: 		if (bp->type != BOUNDARY && bp->type != LAND) {
   59: 			/* Check for saturation of oxygen levels */
   60: 			/* Loop over each tracer */
   61: 			for (n = 0; n < bm->ntracer; n++) {
   62: 				/* Find oxygen */
   63: 				if (strcmp(bm->tinfo[n].name, "Oxygen") == 0) {
   64: 					/* Loop through layers and check for saturation */
   65: 					for (k = 0; k < bp->nz; k++) {
   66: 						if (newwc[b][k][n] > 8000.0)
   67: 							newwc[b][k][n] = 8000.0;
   68: 					}
   69: 					for (k = 0; k < sm->nz; k++) {
   70: 						if (newsed[b][k][n] > 8000.0)
   71: 							newsed[b][k][n] = 8000.0;
   72: 					}
   73: 					do_break++;
   74:                 } else if (strcmp(bm->tinfo[n].name, "Si") == 0) {
   75:                     for (k = 0; k < bp->nz; k++) {
   76:                         if (newwc[b][k][n] > SIbound)
   77:                             newwc[b][k][n] = SIbound;
   78:                         
   79:                         //fprintf(bm->logFile, "Time %e doing box%d-%d saturation check si now %e\n", bm->dayt, b, k, newwc[b][k][n]);
   80: 
   81:                     }
```

**Suggested fix (from review):** Initialise do_break=0

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

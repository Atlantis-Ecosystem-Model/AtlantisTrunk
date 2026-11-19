# atassess/atsolve.c, doolittle.c (triangular solvers) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 6; confidence H and severity H: 5; on the priority list: 5).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SOL-1](#sol-1) ✔ ⭐ | 137-140, 193 | H/H | 1-based port | On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old … |  | [ ] |
| [SOL-2](#sol-2) ⭐ | 97-98, 212 | H/H | algorithm | `mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased. |  | [ ] |
| [SOL-3](#sol-3) | 151, 188, 221 | H/M | double count | `nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved. |  | [ ] |
| [SOL-4](#sol-4) | 143-150 | H/L | leak | Early `return` skips `free1d(psum)`. |  | [ ] |
| [SOL-5](#sol-5) | 143 | M/M | return code | Success is inferred from `nfunk <= itmax` and is ambiguous at the limit. |  | [ ] |
| [SOL-5b](#sol-5b) | 76 | H/L | wrong arg | `AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0. |  | [ ] |
| [SOL-6](#sol-6) ✔ ⭐ | 493 (+atclassical.c:1352) | H/H | NULL deref | `PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]… |  | [ ] |
| [SOL-7](#sol-7) ⭐ | 482-484 | H/H | wrong variable | `q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`). |  | [ ] |
| [SOL-8](#sol-8) | 476 | L/M | off-by-one | Subtracts `CTData[Yr]` instead of `[Yr-1]`. |  | [ ] |
| [SOL-9](#sol-9) ⭐ | 556 | H/H | sign | `*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit. |  | [ ] |
| [SOL-10](#sol-10) | 260-262 | H/M | loop bound | Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied. |  | [ ] |
| [SOL-11](#sol-11) | 261 vs 460-462 | M/M | units | `exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6. |  | [ ] |
| [SOL-12](#sol-12) | 296-301 | H/M | ignored return | `EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0). |  | [ ] |
| [SOL-13](#sol-13) | 336-347, 367-375 | M/H | convergence | Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise … |  | [ ] |
| [SOL-14](#sol-14) | 615-622 | H/M | units | Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e. |  | [ ] |
| [DOO-1](#doo-1) | doolittle.c:93 (+atdiet.c:691-692) | M/L | singular check | Tests for an exact-zero pivot only, and the caller ignores the return value. |  | [ ] |

## Reviewer notes for this file

unit_lower_triangular.c and upper_triangular.c: no issues found.

---

## SOL-1

**1-based port** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Listed lines:** 137-140, 193

**Why it's thought to be a bug**

On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old p[1]. Callers take a non-best vertex.

**Current code** (`atassess/atsolve.c` lines 141–180 (truncated)) — code appears to have moved +6 lines since the review

```c
  141: 		 or if run out of function evaluations. Either way put best point and value in slot 0 */
  142: 		if ((rtol < ftol) || (*nfunk >= itmax)) {
  143: 			SWAP(y[0],y[ilo]);
  144: 			for (j = 0; j < ndim; j++) {
  145: 				SWAP(p[0][j],p[ilo][j]);
  146: 			}
  147: 			*ilow = 0;
  148: 			if (rtol >= ftol) {
  149: 				if (assessing)
  150: 					fprintf(ofp, "Time: %e assessment of %s has failed to converge\n ", dayt, speciesname);
  151: 				else
  152: 					fprintf(ofp, "Time %e downhill_simplex (amoeba) failed to converge\n", dayt);
  153: 			}
  154: 			break;
  155: 		}
  156: 		*nfunk += 2;
  157: 
  158: 		/* Begin a new iteration */
  159: 		/* First extrapolate by a factor alpha through the face of the simplex
  160: 		 across from the high point (i.e. reflect the simplex from the high point) */
  161: 		ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, -alpha, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  162: 		if (ytry <= y[ilo]) {
  163: 			/* Gives a result better than the best point, so try an additional extrapolation by a factor gamma */
  164: 			ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, gamma, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  165: 		} else if (ytry >= y[inhi]) {
  166: 			/* The reflected point is worse then the second highest, so look for
  167: 			 an intermediate lower point (i.e. do a 1-dimensional contraction). */
  168: 			ysave = y[ihi];
  169: 			ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, beta, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  170: 			if (ytry >= ysave) {
  171: 				/* Can't seem to get rid of that high point. Better contract
  172: 				 around the lowest (best) point */
  173: 				for (i = 0; i < mpts; i++) {
  174: 					if (i != ilo) {
  175: 						for (j = 0; j < ndim; j++) {
  176: 							psum[j] = 0.5 * (p[i][j] + p[ilo][j]);
  177: 							p[i][j] = psum[j];
  178: 						}
  179: 						y[i] = Funk(bm, funkflag, sp, nchrt, ndim, YrMax, prm_sp, psum, ofp, xpar);
  180: 					}
```

**Suggested fix (from review):** swap into p[0], set `*ilow = 0`

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

## SOL-2

**algorithm** · Conf H / Sev H ⭐ priority  
**Listed lines:** 97-98, 212

**Why it's thought to be a bug**

`mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased.

**Current code** (`atassess/atsolve.c` lines 94–99)

```c
   94: 
   95: 	 Nov 2026: corrected to match Numerical Recipes -
   96: 	   - uses all ndim+1 vertices (was mpts = ndim, so the last vertex was never used)
   97: 	   - on return (converged or not) the best point is in row 0 of p and y[0] and ilow = 0
   98: 	     (the best point was swapped into row 1 but ilow left pointing at the old row)
   99: 	   - psum recalculated correctly after a contraction
```

**Suggested fix (from review):** ndim+1 (with callers and allocations)

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

## SOL-3

**double count** · Conf H / Sev M  
**Listed lines:** 151, 188, 221

**Why it's thought to be a bug**

`nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved.

**Current code** (`atassess/atsolve.c` lines 154–158) — code appears to have moved +5 lines since the review

```c
  154: 			break;
  155: 		}
  156: 		*nfunk += 2;
  157: 
  158: 		/* Begin a new iteration */
```

**Suggested fix (from review):** remove one

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

## SOL-4

**leak** · Conf H / Sev L  
**Listed lines:** 143-150

**Why it's thought to be a bug**

Early `return` skips `free1d(psum)`.

**Current code** (`atassess/atsolve.c` lines 159–170) — code appears to have moved +18 lines since the review

```c
  159: 		/* First extrapolate by a factor alpha through the face of the simplex
  160: 		 across from the high point (i.e. reflect the simplex from the high point) */
  161: 		ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, -alpha, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  162: 		if (ytry <= y[ilo]) {
  163: 			/* Gives a result better than the best point, so try an additional extrapolation by a factor gamma */
  164: 			ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, gamma, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  165: 		} else if (ytry >= y[inhi]) {
  166: 			/* The reflected point is worse then the second highest, so look for
  167: 			 an intermediate lower point (i.e. do a 1-dimensional contraction). */
  168: 			ysave = y[ihi];
  169: 			ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, beta, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  170: 			if (ytry >= ysave) {
```

**Suggested fix (from review):** break / free

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

## SOL-5

**return code** · Conf M / Sev M  
**Listed lines:** 143

**Why it's thought to be a bug**

Success is inferred from `nfunk <= itmax` and is ambiguous at the limit.

**Current code** (`atassess/atsolve.c` lines 140–144)

```c
  140: 		/* Compute the fractional range from the highest to lowest and return if satisfactory,
  141: 		 or if run out of function evaluations. Either way put best point and value in slot 0 */
  142: 		if ((rtol < ftol) || (*nfunk >= itmax)) {
  143: 			SWAP(y[0],y[ilo]);
  144: 			for (j = 0; j < ndim; j++) {
```

**Suggested fix (from review):** explicit status

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

## SOL-5b

**wrong arg** · Conf H / Sev L  
**Listed lines:** 76

**Why it's thought to be a bug**

`AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0.

**Current code** (`atassess/atsolve.c` lines 68–72) — code appears to have moved -6 lines since the review

```c
   68: 
   69: void AmoebaL(MSEBoxModel *bm, double **p, double *y, int ndim, double ftol, int itmax, int funkflag, int nchrt, int YrMax, double prm_sp,
   70: 		int sp, int *nfunk, int *ilow, FILE *ofp, double *xpar) {
   71: 	/* Not all calls to Amoeba will have the extra info needed by the Assessment model calls
   72: 	 so reflect this in the two step call */
```

**Suggested fix (from review):** pass through

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

## SOL-6

**NULL deref** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Listed lines:** 493 (+atclassical.c:1352)

**Why it's thought to be a bug**

`PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]`, so it crashes when prm_sp is 2 or 3.

**Current code** (`atassess/atsolve.c` lines 501–505) — code appears to have moved +10 lines since the review

```c
  501: 
  502: 		// Nov 2026: regression has 2 parameters (was passed the production model ndim)
  503: 		Linear_Regression(bm, qlinear_regress, sp, XX, 2, PP, YY, YrMax, 1, ofp);
  504: 
  505: 		/* Get results of regression - need to take exponent as linear
```

**Suggested fix (from review):** allocate PP

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

## SOL-7

**wrong variable** · Conf H / Sev H ⭐ priority  
**Listed lines:** 482-484

**Why it's thought to be a bug**

`q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`).

**Current code** (`atassess/atsolve.c` lines 483–489) — code appears to have moved +3 lines since the review

```c
  483: 		step1 = NEst[Yr - 1][est_B_id] / (K + small_num);
  484: 		step2 = pow(step1, p);
  485: 		Biom_estimate = NEst[Yr - 1][est_B_id] + NEst[Yr - 1][est_B_id] * (r / p) * (1.0 - step2) - CTData[Yr];
  486: 		/* Put in constraint to prevent negative biomasses */
  487: 		NEst[Yr][est_B_id] = max(1.0, Biom_estimate);
  488: 	}
  489: 
```

**Suggested fix (from review):** use ITData

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

## SOL-8

**off-by-one** · Conf L / Sev M  
**Listed lines:** 476

**Why it's thought to be a bug**

Subtracts `CTData[Yr]` instead of `[Yr-1]`.

**Current code** (`atassess/atsolve.c` lines 483–487) — code appears to have moved +9 lines since the review

```c
  483: 		step1 = NEst[Yr - 1][est_B_id] / (K + small_num);
  484: 		step2 = pow(step1, p);
  485: 		Biom_estimate = NEst[Yr - 1][est_B_id] + NEst[Yr - 1][est_B_id] * (r / p) * (1.0 - step2) - CTData[Yr];
  486: 		/* Put in constraint to prevent negative biomasses */
  487: 		NEst[Yr][est_B_id] = max(1.0, Biom_estimate);
```

**Suggested fix (from review):** confirm alignment

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

## SOL-9

**sign** · Conf H / Sev H ⭐ priority  
**Listed lines:** 556

**Why it's thought to be a bug**

`*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit.

**Current code** (`atassess/atsolve.c` lines 554–558)

```c
  554: 
  555: 	/* Calculate SS */
  556: 	Nval = 0;
  557: 	Lval = 0;
  558: 	for (Yr = 0; Yr < YrMax + 1; Yr++) {
```

**Suggested fix (from review):** negative log-likelihood (+)

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

## SOL-10

**loop bound** · Conf H / Sev M  
**Listed lines:** 260-262

**Why it's thought to be a bug**

Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied.

**Current code** (`atassess/atsolve.c` lines 261–267) — code appears to have moved +3 lines since the review

```c
  261: 		break;
  262: 	case schafer_model: /* Schafer production model assessment function */
  263: 		for (i = 0; i < est_B0_id + 1; i++) {
  264: 			NEst[YrMax][i] = exp(X[i]);
  265: 		}
  266: 		Prod_Calc(bm, ndim, sp, nchrt, YrMax, prm_sp, ofp);
  267: 		Prod_Funk_Val(ndim, sp, nchrt, YrMax, &SS);
```

**Suggested fix (from review):** `i < ndim`

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

## SOL-11

**units** · Conf M / Sev M  
**Listed lines:** 261 vs 460-462

**Why it's thought to be a bug**

`exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6.

**Current code** (`atassess/atsolve.c` lines 259–263)

```c
  259: 	switch (funkflag) {
  260: 	case no_assess: /* No assessment */
  261: 		break;
  262: 	case schafer_model: /* Schafer production model assessment function */
  263: 		for (i = 0; i < est_B0_id + 1; i++) {
```

**Suggested fix (from review):** choose log or linear consistently

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

## SOL-12

**ignored return** · Conf H / Sev M  
**Listed lines:** 296-301

**Why it's thought to be a bug**

`EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0).

**Current code** (`atassess/atsolve.c` lines 297–306) — code appears to have moved +3 lines since the review

```c
  297: 		ans = SS;
  298: 		break;
  299: 	case EquilF_Funk_model:
  300: 		/* Evaluate at the trial point X (Nov 2026 - previously ignored X and returned 0) */
  301: 		for (i = 0; i < ndim; i++)
  302: 			xpar[i] = X[i];
  303: 		ans = EquilF_Funk(bm, sp, xpar, ofp);
  304: 		break;
  305: 	case SurplusProduction_model:
  306: 		/* Evaluate at the trial point X (Nov 2026 - previously ignored X and returned 0) */
```

**Suggested fix (from review):** `ans = ...`

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

## SOL-13

**convergence** · Conf M / Sev H  
**Listed lines:** 336-347, 367-375

**Why it's thought to be a bug**

Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise stale.

**Current code** (`atassess/atsolve.c` lines 334–373 (truncated))

```c
  334: 
  335: }
  336: 
  337: void Solve_Plus(int Yr, int nchrt, int ndim, double M_sp) {
  338: 	double Fmin, Fmax, Nmax1, Nmax2, Nproj, Nhit, FF, ZZ;
  339: 	int II;
  340: 
  341: 	Fmin = 0;
  342: 	Fmax = 3;
  343: 	Nhit = NEst[Yr + 1][nchrt - 1];
  344: 
  345: 	for (II = 0; II < ndim; II++) {
  346: 		FF = (Fmin + Fmax) / 2.0;
  347: 		ZZ = FF + M_sp;
  348: 		Nmax1 = (CData[Yr][nchrt - 2] * ZZ / FF) / (1.0 - exp(-ZZ));
  349: 		Nmax2 = (CData[Yr][nchrt - 1] * ZZ / FF) / (1.0 - exp(-ZZ));
  350: 		Nproj = (Nmax1 + Nmax2) * exp(-ZZ);
  351: 		if (fabs(Nproj - Nhit) < 0.01) {
  352: 			F[Yr][nchrt - 2] = FF;
  353: 			NEst[Yr][nchrt - 2] = Nmax1;
  354: 			F[Yr][nchrt - 1] = FF;
  355: 			NEst[Yr][nchrt - 1] = Nmax2;
  356: 		}
  357: 		if (Nproj > Nhit)
  358: 			Fmin = FF;
  359: 		else
  360: 			Fmax = FF;
  361: 	}
  362: 
  363: 	return;
  364: }
  365: 
  366: /**
  367:  * \brief Back projection
  368:  */
  369: void Solve_Back(int Yr, int age, int ndim, double M_sp) {
  370: 	double ZZ, FF, Fmin, Fmax, Nback, CTarg, CProj;
  371: 	int II;
  372: 
  373: 	Fmin = 0;
```

**Suggested fix (from review):** fixed iterations; always write

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

## SOL-14

**units** · Conf H / Sev M  
**Listed lines:** 615-622

**Why it's thought to be a bug**

Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e.

**Current code** (`atassess/atsolve.c` lines 613–624)

```c
  613: 		//SS = MAXDOUBLE;
  614: 
  615: 		for (j = 0; j < ndim; j++) {
  616: 			X[j] = P[Ilow][j];
  617: 		}
  618: 
  619: 		fprintf(
  620: 				ofp,
  621: 				"Failed to fit regression line to production model q estimates are q: %e and qinc: %e so making do or assuming constant q = 0.8 - see documentation\n",
  622: 				X[0], X[1]);
  623: 
  624: 		/* Fitting failed so take best on offer or assume q = 0.8 with no increase through time */
```

**Suggested fix (from review):** log(0.8), 0.0

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

## DOO-1

**singular check** · Conf M / Sev L  
**Listed lines:** doolittle.c:93 (+atdiet.c:691-692)

**Why it's thought to be a bug**

Tests for an exact-zero pivot only, and the caller ignores the return value.

**Current code** (`atassess/doolittle.c` lines 91–95)

```c
   91:             *(p_k + j) -= *(p_k + p) * *(p_col + j);
   92:       }
   93:       if ( *(p_k + k) == 0.0 ) return -1;
   94:       for (i = k+1, p_row = p_k + n; i < n; p_row += n, i++) {
   95:          for (p = 0, p_col = A; p < k; p_col += n, p++)
```

**Suggested fix (from review):** tolerance and check

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

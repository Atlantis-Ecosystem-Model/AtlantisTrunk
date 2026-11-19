# atassess/atsolve.c, doolittle.c (triangular solvers) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 6; confidence H and severity H: 5; on the priority list: 5).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SOL-1](#sol-1) ✔ ⭐ | 143-153, ≈147 | H/H | 1-based port | On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old … |  | [ ] |
| [SOL-2](#sol-2) ⭐ | ≈104, 215 | H/H | algorithm | `mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased. |  | [ ] |
| [SOL-3](#sol-3) | ≈156, ≈193, 224 | H/M | double count | `nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved. |  | [ ] |
| [SOL-4](#sol-4) | ≈199 | H/L | leak | Early `return` skips `free1d(psum)`. |  | [ ] |
| [SOL-5](#sol-5) | ≈142 | M/M | return code | Success is inferred from `nfunk <= itmax` and is ambiguous at the limit. |  | [ ] |
| [SOL-5b](#sol-5b) | 76 | H/L | wrong arg | `AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0. |  | [ ] |
| [SOL-6](#sol-6) ✔ ⭐ | ≈503 (+atclassical.c:1352) | H/H | NULL deref | `PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]… |  | [ ] |
| [SOL-7](#sol-7) ⭐ | 491-493 | H/H | wrong variable | `q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`). |  | [ ] |
| [SOL-8](#sol-8) | 485 | L/M | off-by-one | Subtracts `CTData[Yr]` instead of `[Yr-1]`. |  | [ ] |
| [SOL-9](#sol-9) ⭐ | 566 | H/H | sign | `*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit. |  | [ ] |
| [SOL-10](#sol-10) | 263-265 | H/M | loop bound | Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied. |  | [ ] |
| [SOL-11](#sol-11) | 264 vs 469-471 | M/M | units | `exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6. |  | [ ] |
| [SOL-12](#sol-12) | 299-310 | H/M | ignored return | `EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0). |  | [ ] |
| [SOL-13](#sol-13) | 345-356, 376-384 | M/H | convergence | Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise … |  | [ ] |
| [SOL-14](#sol-14) | 625-632 | H/M | units | Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e. |  | [ ] |
| [DOO-1](#doo-1) | doolittle.c:93 (+atdiet.c:691-692) | M/L | singular check | Tests for an exact-zero pivot only, and the caller ignores the return value. |  | [ ] |

## Reviewer notes for this file

unit_lower_triangular.c and upper_triangular.c: no issues found.

---

## SOL-1

**1-based port** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Lines (current code):** 143-153, ≈147  
**Lines at review (Oct 2026):** 137-140, 193

**Why it's thought to be a bug**

On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old p[1]. Callers take a non-best vertex.

**Current code** (`atassess/atsolve.c` lines 141–155) — ≈ the code here has changed since the review; lines are approximate

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
**Lines (current code):** ≈104, 215  
**Lines at review (Oct 2026):** 97-98, 212

**Why it's thought to be a bug**

`mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased.

**Current code** (`atassess/atsolve.c` lines 102–106) — ≈ the code here has changed since the review; lines are approximate

```c
  102: 
  103: 	int i, j, ilo, ihi, inhi;
  104: 	int mpts = ndim + 1;
  105: 	double ytry, ysave, sum, rtol, swap;
  106: 	double *psum = alloc1d(ndim); /* Temporary storage */
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
**Lines (current code):** ≈156, ≈193, 224  
**Lines at review (Oct 2026):** 151, 188, 221

**Why it's thought to be a bug**

`nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved.

**Current code** (`atassess/atsolve.c` lines 154–193 (truncated)) — ≈ the code here has changed since the review; lines are approximate

```c
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
  181: 				}
  182: 				/* Keep track of function evaluations */
  183: 				(*nfunk) += ndim;
  184: 
  185: 				/* Recompute psum */
  186: 				for (j = 0; j < ndim; j++) {
  187: 					for (sum = 0.0, i = 0; i < mpts; i++) {
  188: 						sum += p[i][j];
  189: 					}
  190: 					psum[j] = sum;
  191: 				}
  192: 			}
  193: 		} else -- (*nfunk);   // Correct the evaluation count
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
**Lines (current code):** ≈199  
**Lines at review (Oct 2026):** 143-150

**Why it's thought to be a bug**

Early `return` skips `free1d(psum)`.

**Current code** (`atassess/atsolve.c` lines 197–201) — ≈ the code here has changed since the review; lines are approximate

```c
  197: 
  198: 	/* Free local array */
  199: 	free1d(psum);
  200: 
  201: 	return;
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
**Lines (current code):** ≈142  
**Lines at review (Oct 2026):** 143

**Why it's thought to be a bug**

Success is inferred from `nfunk <= itmax` and is ambiguous at the limit.

**Current code** (`atassess/atsolve.c` lines 140–144) — ≈ the code here has changed since the review; lines are approximate

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
**Lines (current code):** 76  
**Lines at review (Oct 2026):** 76

**Why it's thought to be a bug**

`AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0.

**Current code** (`atassess/atsolve.c` lines 74–78) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
   74: 	char *dummyname = "blank";
   75: 
   76: 	Amoeba(bm, 0, sp, 1.0, dummyname, p, y, ndim, ftol, itmax, funkflag, nchrt, YrMax, prm_sp, nfunk, ilow, ofp, NULL);
   77: 
   78: 	return;
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
**Lines (current code):** ≈503 (+atclassical.c:1352)  
**Lines at review (Oct 2026):** 493 (+atclassical.c:1352)

**Why it's thought to be a bug**

`PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]`, so it crashes when prm_sp is 2 or 3.

**Current code** (`atassess/atsolve.c` lines 501–505) — ≈ the code here has changed since the review; lines are approximate

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
**Lines (current code):** 491-493  
**Lines at review (Oct 2026):** 482-484

**Why it's thought to be a bug**

`q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`).

**Current code** (`atassess/atsolve.c` lines 489–495)

```c
  489: 
  490: 	/* Calculate estimates of q */
  491: 	for (Yr = 0; Yr < YrMax + 1; Yr++) {
  492: 		NEst[Yr][est_q_id] = log(CTData[Yr] / (NEst[Yr][est_B_id] + small_num));
  493: 	}
  494: 
  495: 	if (q_dynamic) {
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
**Lines (current code):** 485  
**Lines at review (Oct 2026):** 476

**Why it's thought to be a bug**

Subtracts `CTData[Yr]` instead of `[Yr-1]`.

**Current code** (`atassess/atsolve.c` lines 483–487)

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
**Lines (current code):** 566  
**Lines at review (Oct 2026):** 556

**Why it's thought to be a bug**

`*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit.

**Current code** (`atassess/atsolve.c` lines 564–568)

```c
  564: 
  565: 	SSavg = sqrt(Lval / (Nval + small_num));
  566: 	*SS = -(Nval / 2.0) * (log(2.0 * 3.1415926) + 2.0 * log(SSavg) + 1.0);
  567: 
  568: 	/* Calculate residuals - as no age structure carried, store results in entry zero (0) */
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
**Lines (current code):** 263-265  
**Lines at review (Oct 2026):** 260-262

**Why it's thought to be a bug**

Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied.

**Current code** (`atassess/atsolve.c` lines 261–267)

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
**Lines (current code):** 264 vs 469-471  
**Lines at review (Oct 2026):** 261 vs 460-462

**Why it's thought to be a bug**

`exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6.

**Current code** (`atassess/atsolve.c` lines 262–266)

```c
  262: 	case schafer_model: /* Schafer production model assessment function */
  263: 		for (i = 0; i < est_B0_id + 1; i++) {
  264: 			NEst[YrMax][i] = exp(X[i]);
  265: 		}
  266: 		Prod_Calc(bm, ndim, sp, nchrt, YrMax, prm_sp, ofp);
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
**Lines (current code):** 299-310  
**Lines at review (Oct 2026):** 296-301

**Why it's thought to be a bug**

`EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0).

**Current code** (`atassess/atsolve.c` lines 297–312)

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
  307: 		for (i = 0; i < ndim; i++)
  308: 			xpar[i] = X[i];
  309: 		ans = SurplusProduction(bm, sp, xpar, ofp);
  310: 		break;
  311: 	default:
  312: 		quit("No such function flag defined - how did it get here?\n");
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
**Lines (current code):** 345-356, 376-384  
**Lines at review (Oct 2026):** 336-347, 367-375

**Why it's thought to be a bug**

Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise stale.

**Current code** (`atassess/atsolve.c` lines 343–382 (truncated))

```c
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
  374: 	Fmax = 3;
  375: 	CTarg = CData[Yr - 1][age - 1];
  376: 	for (II = 0; II < ndim; II++) {
  377: 		FF = (Fmin + Fmax) / 2.0;
  378: 		ZZ = M_sp + FF;
  379: 		Nback = NEst[Yr][age] * exp(ZZ);
  380: 		CProj = (FF / ZZ) * Nback * (1.0 - exp(-ZZ));
  381: 		if (fabs(CProj - CTarg) < 0.001) {
  382: 			F[Yr - 1][age - 1] = FF;
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
**Lines (current code):** 625-632  
**Lines at review (Oct 2026):** 615-622

**Why it's thought to be a bug**

Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e.

**Current code** (`atassess/atsolve.c` lines 623–634)

```c
  623: 
  624: 		/* Fitting failed so take best on offer or assume q = 0.8 with no increase through time */
  625: 		if (X[0] > 1 || X[0] < 0) {
  626: 			X[0] = 0.8;
  627: 			X[1] = 1.0;
  628: 		}
  629: 
  630: 		if (X[1] < 0.7 || X[1] > 1.5) {
  631: 			X[1] = 1.0;
  632: 		}
  633: 	}
  634: 
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
**Lines (current code):** doolittle.c:93 (+atdiet.c:691-692)  
**Lines at review (Oct 2026):** doolittle.c:93 (+atdiet.c:691-692)

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

# atassess/atsolve.c, doolittle.c (triangular solvers) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **16 findings** (severity H: 6; confidence H and severity H: 5; on the priority list: 5).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SOL-1](#sol-1) ✔ ⭐ | 143 | H/H | 1-based port | On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old … |  | [ ] |
| [SOL-2](#sol-2) ⭐ | 104 | H/H | algorithm | `mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased. |  | [ ] |
| [SOL-3](#sol-3) | 161 | H/M | double count | `nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved. |  | [ ] |
| [SOL-4](#sol-4) | 199 | H/L | leak | Early `return` skips `free1d(psum)`. |  | [ ] |
| [SOL-5](#sol-5) | 142 | M/M | return code | Success is inferred from `nfunk <= itmax` and is ambiguous at the limit. |  | [ ] |
| [SOL-5b](#sol-5b) | 69 | H/L | wrong arg | `AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0. |  | [ ] |
| [SOL-6](#sol-6) ✔ ⭐ | 592 | H/H | NULL deref | `PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]… |  | [ ] |
| [SOL-7](#sol-7) ⭐ | 492 | H/H | wrong variable | `q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`). |  | [ ] |
| [SOL-8](#sol-8) | 485 | L/M | off-by-one | Subtracts `CTData[Yr]` instead of `[Yr-1]`. |  | [ ] |
| [SOL-9](#sol-9) ⭐ | ⚠ not found | H/H | sign | `*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit. |  | [ ] |
| [SOL-10](#sol-10) | 475 | H/M | loop bound | Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied. |  | [ ] |
| [SOL-11](#sol-11) | 264 | M/M | units | `exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6. |  | [ ] |
| [SOL-12](#sol-12) | 303 | H/M | ignored return | `EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0). |  | [ ] |
| [SOL-13](#sol-13) | 345, 376 | M/H | convergence | Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise … |  | [ ] |
| [SOL-14](#sol-14) | 625 | H/M | units | Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e. |  | [ ] |
| [DOO-1](#doo-1) | ⚠ not found | M/L | singular check | Tests for an exact-zero pivot only, and the caller ignores the return value. |  | [ ] |

## Reviewer notes for this file

unit_lower_triangular.c and upper_triangular.c: no issues found.

---

## SOL-1

**1-based port** · Conf H / Sev H ✔ re-checked ⭐ priority  
**Found at (current code):** `atsolve.c` 143
**Lines at review (Oct 2026):** 137-140, 193

**Why it's thought to be a bug**

On convergence `SWAP(y[0],y[ilo])` but `SWAP(p[1][i],p[ilo][i])`, then `*ilow = ilo`, which now holds the old p[1]. Callers take a non-best vertex.

**Code at issue** — `atsolve.c` line 143  
Search for: `SWAP(y[0],y[ilo])`

```c
  141: 		 or if run out of function evaluations. Either way put best point and value in slot 0 */
  142: 		if ((rtol < ftol) || (*nfunk >= itmax)) {
  143→ 			SWAP(y[0],y[ilo]);
  144: 			for (j = 0; j < ndim; j++) {
  145: 				SWAP(p[0][j],p[ilo][j]);
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
**Found at (current code):** `atsolve.c` 104
**Lines at review (Oct 2026):** 97-98, 212

**Why it's thought to be a bug**

`mpts = ndim` (NR uses ndim+1), so the simplex is degenerate and the Amotry centroid is biased.

**Code at issue** — `atsolve.c` line 104  
Search for: `mpts = ndim`

```c
  102: 
  103: 	int i, j, ilo, ihi, inhi;
  104→ 	int mpts = ndim + 1;
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
**Found at (current code):** `atsolve.c` 161
**Lines at review (Oct 2026):** 151, 188, 221

**Why it's thought to be a bug**

`nfunk` is incremented both in Amoeba (+=2/--) and in Amotry, so the effective itmax is halved.

**Code at issue** — `atsolve.c` line 161 (located by name only; check this is the right place)  
Search for: `ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, -alpha, nchrt, YrMax, prm_sp, sp`

```c
  159: 		/* First extrapolate by a factor alpha through the face of the simplex
  160: 		 across from the high point (i.e. reflect the simplex from the high point) */
  161→ 		ytry = Amotry(bm, p, y, psum, ndim, funkflag, ihi, nfunk, -alpha, nchrt, YrMax, prm_sp, sp, ofp, xpar);
  162: 		if (ytry <= y[ilo]) {
  163: 			/* Gives a result better than the best point, so try an additional extrapolation by a factor gamma */
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
**Found at (current code):** `atsolve.c` 199
**Lines at review (Oct 2026):** 143-150

**Why it's thought to be a bug**

Early `return` skips `free1d(psum)`.

**Code at issue** — `atsolve.c` line 199  
Search for: `free1d(psum)`

```c
  197: 
  198: 	/* Free local array */
  199→ 	free1d(psum);
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
**Found at (current code):** `atsolve.c` 142
**Lines at review (Oct 2026):** 143

**Why it's thought to be a bug**

Success is inferred from `nfunk <= itmax` and is ambiguous at the limit.

**Code at issue** — `atsolve.c` line 142 (closest match to the quoted code; wording differs slightly)  
Search for: `if ((rtol < ftol) || (*nfunk >= itmax)) {`

```c
  140: 		/* Compute the fractional range from the highest to lowest and return if satisfactory,
  141: 		 or if run out of function evaluations. Either way put best point and value in slot 0 */
  142→ 		if ((rtol < ftol) || (*nfunk >= itmax)) {
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
**Found at (current code):** `atsolve.c` 69
**Lines at review (Oct 2026):** 76

**Why it's thought to be a bug**

`AmoebaL` passes NULL instead of `xpar`; time is hard-coded to 1.0.

**Code at issue** — `atsolve.c` line 69 (located by name only; check this is the right place)  
Search for: `void AmoebaL(MSEBoxModel *bm, double **p, double *y, int ndim, double ftol, int itmax, int`

```c
   67: 		int *nfunk, double fac, int nchrt, int YrMax, double prm_sp, int sp, FILE *ofp, double *xpar);
   68: 
   69→ void AmoebaL(MSEBoxModel *bm, double **p, double *y, int ndim, double ftol, int itmax, int funkflag, int nchrt, int YrMax, double prm_sp,
   70: 		int sp, int *nfunk, int *ilow, FILE *ofp, double *xpar) {
   71: 	/* Not all calls to Amoeba will have the extra info needed by the Assessment model calls
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
**Found at (current code):** `atsolve.c` 592
**Lines at review (Oct 2026):** 493 (+atclassical.c:1352)

**Why it's thought to be a bug**

`PP` is declared `= 0` (atsamplesetup.h:165) and never allocated anywhere. `Linear_Regression` writes `P[i][j]`, so it crashes when prm_sp is 2 or 3.

**Code at issue** — `atsolve.c` line 592  
Search for: `P[i][j]`

```c
  590: 	for (i = 0; i < ndim + 1; i++) {
  591: 		for (j = 0; j < ndim; j++) {
  592→ 			P[i][j] = X[j];
  593: 			if (i - 1 == j)
  594: 				P[i][j] *= GRD;
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
**Found at (current code):** `atsolve.c` 492
**Lines at review (Oct 2026):** 482-484

**Why it's thought to be a bug**

`q = log(CTData/B)` uses catch, but q = CPUE/B (`ITData`).

**Code at issue** — `atsolve.c` line 492 (the line the review pointed to, found by its text)  
Search for: `NEst[Yr][est_q_id] = log(CTData[Yr] / (NEst[Yr][est_B_id] + small_num));`

```c
  490: 	/* Calculate estimates of q */
  491: 	for (Yr = 0; Yr < YrMax + 1; Yr++) {
  492→ 		NEst[Yr][est_q_id] = log(CTData[Yr] / (NEst[Yr][est_B_id] + small_num));
  493: 	}
  494: 
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
**Found at (current code):** `atsolve.c` 485
**Lines at review (Oct 2026):** 476

**Why it's thought to be a bug**

Subtracts `CTData[Yr]` instead of `[Yr-1]`.

**Code at issue** — `atsolve.c` line 485  
Search for: `CTData[Yr]`

```c
  483: 		step1 = NEst[Yr - 1][est_B_id] / (K + small_num);
  484: 		step2 = pow(step1, p);
  485→ 		Biom_estimate = NEst[Yr - 1][est_B_id] + NEst[Yr - 1][est_B_id] * (r / p) * (1.0 - step2) - CTData[Yr];
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
**Found at (current code):** not found in current code ⚠
**Lines at review (Oct 2026):** 556

**Why it's thought to be a bug**

`*SS = -(n/2)(…)` is the log-likelihood, but Amoeba minimises it, so it seeks the worst fit.

**Code at issue** — ⚠ the code quoted in the review could not be found in the current file (`atsolve.c`). It may have been changed or already fixed.

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
**Found at (current code):** `atsolve.c` 475
**Lines at review (Oct 2026):** 260-262

**Why it's thought to be a bug**

Copies `X[0..est_B0_id]` only; `p` (est_p_id) is never copied.

**Code at issue** — `atsolve.c` line 475 (located by name only; check this is the right place)  
Search for: `step1 = max(0.1, NEst[YrMax][est_p_id]);`

```c
  473: 	if (p_dynamic) {
  474: 		/* Allow for assymetric production, but constrain so that p>0 always */
  475→ 		step1 = max(0.1, NEst[YrMax][est_p_id]);
  476: 		p = step1 / 1000000.0;
  477: 	} else
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
**Found at (current code):** `atsolve.c` 264
**Lines at review (Oct 2026):** 261 vs 460-462

**Why it's thought to be a bug**

`exp(X)` but X starts at 20.0 "linear scale", giving r ≈ 4.9e6.

**Code at issue** — `atsolve.c` line 264 (closest match to the quoted code; wording differs slightly)  
Search for: `NEst[YrMax][i] = exp(X[i]);`

```c
  262: 	case schafer_model: /* Schafer production model assessment function */
  263: 		for (i = 0; i < est_B0_id + 1; i++) {
  264→ 			NEst[YrMax][i] = exp(X[i]);
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
**Found at (current code):** `atsolve.c` 303
**Lines at review (Oct 2026):** 296-301

**Why it's thought to be a bug**

`EquilF_Funk`/`SurplusProduction` results are discarded (`ans` stays 0).

**Code at issue** — `atsolve.c` line 303 (located by name only; check this is the right place)  
Search for: `ans = EquilF_Funk(bm, sp, xpar, ofp);`

```c
  301: 		for (i = 0; i < ndim; i++)
  302: 			xpar[i] = X[i];
  303→ 		ans = EquilF_Funk(bm, sp, xpar, ofp);
  304: 		break;
  305: 	case SurplusProduction_model:
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
**Found at (current code):** `atsolve.c` 345, `atsolve.c` 376
**Lines at review (Oct 2026):** 336-347, 367-375

**Why it's thought to be a bug**

Bisection iterates `ndim` (≈10) times with an absolute tolerance, and writes results only on a hit, otherwise stale.

**Code at issue** — `atsolve.c` line 345 (the line the review pointed to, found by its text)  
Search for: `for (II = 0; II < ndim; II++) {`

```c
  343: 	Nhit = NEst[Yr + 1][nchrt - 1];
  344: 
  345→ 	for (II = 0; II < ndim; II++) {
  346: 		FF = (Fmin + Fmax) / 2.0;
  347: 		ZZ = FF + M_sp;
```

**Code at issue** — `atsolve.c` line 376 (the line the review pointed to, found by its text)  
Search for: `for (II = 0; II < ndim; II++) {`

```c
  374: 	Fmax = 3;
  375: 	CTarg = CData[Yr - 1][age - 1];
  376→ 	for (II = 0; II < ndim; II++) {
  377: 		FF = (Fmin + Fmax) / 2.0;
  378: 		ZZ = M_sp + FF;
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
**Found at (current code):** `atsolve.c` 625
**Lines at review (Oct 2026):** 615-622

**Why it's thought to be a bug**

Fallbacks X[0]=0.8 and X[1]=1.0 are linear values in log space, giving q=2.23 and qinc=e.

**Code at issue** — `atsolve.c` line 625 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `if (X[0] > 1 || X[0] < 0) {`

```c
  623: 
  624: 		/* Fitting failed so take best on offer or assume q = 0.8 with no increase through time */
  625→ 		if (X[0] > 1 || X[0] < 0) {
  626: 			X[0] = 0.8;
  627: 			X[1] = 1.0;
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
**Found at (current code):** not found in current code ⚠
**Lines at review (Oct 2026):** doolittle.c:93 (+atdiet.c:691-692)

**Why it's thought to be a bug**

Tests for an exact-zero pivot only, and the caller ignores the return value.

**Code at issue** — ⚠ the review quoted no code for this finding, and the lines it pointed to have since changed, so it can't be located automatically. Claude will find it when this finding is worked on.

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

# sjwlib/erfc.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-14](#sjw3-14) | 9-41 | M/L | precision / symbol override | File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Make… |  | [ ] |

---

## SJW3-14

**precision / symbol override** · Conf M / Sev L  
**Lines (current code):** 9-41  
**Lines at review (Oct 2026):** 9-41

**Why it's thought to be a bug**

File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Makefile.am c_sources). Because libsjwlib.a is linked before libm, any Atlantis reference to `erf`/`erfc` resolves to these approximations (A&S 7.1.26, abs. error ~1.5e-7; NR erfcc ~1.2e-7 rel.) rather than the full-precision libm versions, giving platform-dependent results. Functions: `erf()`, `erfc()`.

**Current code** (`sjwlib/erfc.c` lines 7–42)

```c
    7: #include <math.h>
    8: 
    9: double erf(double x) {
   10: 	// constants
   11: 	double t, y;
   12: 	double a1 =  0.254829592;
   13: 	double a2 = -0.284496736;
   14: 	double a3 =  1.421413741;
   15: 	double a4 = -1.453152027;
   16: 	double a5 =  1.061405429;
   17: 	double p  =  0.3275911;
   18: 	// Save the sign of x
   19: 	int sign = 1;
   20: 	if (x < 0)
   21: 		sign = -1;
   22: 	x = fabs(x);
   23: 	// A&S formula 7.1.26
   24: 
   25: 	t = 1.0/(1.0 + p*x);
   26: 	y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);
   27: 	return sign*y;
   28: }
   29: 
   30: double erfc(double x)
   31: {
   32: 	double t, z, ans;
   33: 	z = fabs(x);
   34: 	t = 1.0 / (1.0 + 0.5 * z);
   35: 
   36: 	ans=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
   37: 		t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
   38: 		t*(-0.82215223+t*0.17087277)))))))));
   39: 
   40: 	return x >= 0.0 ? ans : 2.0-ans;
   41: }
   42: 
```

**Suggested fix (from review):** Wrap the file in `#ifdef _MSC_VER` (and the nrcdflib.h prototypes likewise).

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

# sjwlib/erfc.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-14](#sjw3-14) | 9-41 | M/L | precision / symbol override | File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Make… |  | [ ] |

---

## SJW3-14

**precision / symbol override** · Conf M / Sev L  
**Listed lines:** 9-41

**Why it's thought to be a bug**

File comment says it exists only because MSVC lacks erf/erfc, but it is compiled on all platforms (sjwlib/Makefile.am c_sources). Because libsjwlib.a is linked before libm, any Atlantis reference to `erf`/`erfc` resolves to these approximations (A&S 7.1.26, abs. error ~1.5e-7; NR erfcc ~1.2e-7 rel.) rather than the full-precision libm versions, giving platform-dependent results. Functions: `erf()`, `erfc()`.

**Current code** (`sjwlib/erfc.c` lines 1–36) — code appears to have moved -7 lines since the review

```c
    1: /*
    2:  * erf() and erfc() are not included into MSVC math library while they are part of GNU libm.
    3:  * To compile STS using MSVC I had to add implementation of these functions below. Both
    4:  * are taken from Numerical recipes in C.
    5:  */
    6: 
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

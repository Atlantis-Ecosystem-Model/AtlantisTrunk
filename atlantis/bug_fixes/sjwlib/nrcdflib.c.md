# sjwlib/nrcdflib.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-08](#sjw2-08) | 103 | H/M | wrong formula (copy-paste) | `return (0.5 * erfc(-0.7071... * (x - mu )/ sigma));` is the normal CDF formula. The lognormal CDF (NR3 Lognor… |  | [ ] |
| [SJW2-09](#sjw2-09) | 141-143 | H/M | wrong formula (copy-paste) | The Cauchy CDF returns the lognormal formula `0.5*erfc(-0.707*(log(x)-mu)/sigma)` and the lognormal `x==0` gua… |  | [ ] |
| [SJW2-10](#sjw2-10) | 309 | H/M | wrong formula | `1.0 - exp(pow((- x / beta), alpha))` should be `1 - exp(-pow(x/beta, alpha))`. `pow` of a negative base with … |  | [ ] |
| [SJW2-11](#sjw2-11) | 192-194 | M/L | ignored parameters | The two-tailed branch uses the raw `t` and ignores `mu`/`sigma`, while the one-tailed branch standardises `(t-… |  | [ ] |
| [SJW2-12](#sjw2-12) | 645 | M/L | typo vs source | `if (a > 3000 && b > 300)`: NR3 uses SWITCH=3000 for both a and b. With b in (300, 3000] the code switches to … |  | [ ] |

---

## SJW2-08

**wrong formula (copy-paste)** · Conf H / Sev M  
**Listed lines:** 103

**Function:** `lognorm_cdf_values()`

**Why it's thought to be a bug**

`return (0.5 * erfc(-0.7071... * (x - mu )/ sigma));` is the normal CDF formula. The lognormal CDF (NR3 Lognormaldist) uses `log(x) - mu`. Currently no callers, so the defect is latent. Function: `lognorm_cdf_values()`.

**Current code** (`sjwlib/nrcdflib.c` lines 101–105)

```c
  101:     if (x <= 0)
  102:         quit("You can not specify a negative zero x when requesting log normal cdf value\n");
  103:     return (0.5 * erfc(-0.707106781186547524 * (x - mu )/ sigma));
  104:     
  105: }
```

**Suggested fix (from review):** Use `(log(x) - mu)`.

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

## SJW2-09

**wrong formula (copy-paste)** · Conf H / Sev M  
**Listed lines:** 141-143

**Function:** `cauchy_cdf_values()`

**Why it's thought to be a bug**

The Cauchy CDF returns the lognormal formula `0.5*erfc(-0.707*(log(x)-mu)/sigma)` and the lognormal `x==0` guard. The Cauchy CDF is `0.5 + atan2(x-mu, sigma)/M_PI` (NR3 Cauchydist), defined for negative x too. It looks like the normal and lognormal bodies were pasted into the wrong functions. Currently no callers. Function: `cauchy_cdf_values()`.

**Current code** (`sjwlib/nrcdflib.c` lines 139–145)

```c
  139:         quit("You can not specify a negative or zero sigma when requesting cauchy cdf value\n");
  140:     
  141:     if (x == 0.) return 0.0;
  142:     
  143:     return (0.5 * erfc(-0.707106781186547524 * (log(x) - mu) / sigma));
  144:     
  145: }
```

**Suggested fix (from review):** `return 0.5 + 0.318309886183790671*atan2(x-mu, sigma);` and remove the x==0 guard.

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

## SJW2-10

**wrong formula** · Conf H / Sev M  
**Listed lines:** 309

**Function:** `weibull_cdf_values()`

**Why it's thought to be a bug**

`1.0 - exp(pow((- x / beta), alpha))` should be `1 - exp(-pow(x/beta, alpha))`. `pow` of a negative base with non-integer alpha returns NaN. With alpha=2 it gives `1-exp(+(x/beta)^2)`, which is negative. It is only correct for alpha=1. Currently no callers. Function: `weibull_cdf_values()`.

**Current code** (`sjwlib/nrcdflib.c` lines 307–311)

```c
  307: {
  308:     if (x < 0.0) quit("Can not have negative exponential distribution");
  309:     return (1.0 - exp(pow((- x / beta), alpha)));
  310: }
  311:             
```

**Suggested fix (from review):** `return 1.0 - exp(-pow(x / beta, alpha));`

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

## SJW2-11

**ignored parameters** · Conf M / Sev L  
**Listed lines:** 192-194

**Function:** `studentt_cdf_values()`

**Why it's thought to be a bug**

The two-tailed branch uses the raw `t` and ignores `mu`/`sigma`, while the one-tailed branch standardises `(t-mu)/sigma`. Function: `studentt_cdf_values()`.

**Current code** (`sjwlib/nrcdflib.c` lines 187–193) — code appears to have moved -3 lines since the review

```c
  187:     if(tails == 1) {
  188:         step1 = (t - mu) / sigma;
  189:         p = 0.5 * betai(0.5 * nu, 0.5, nu/(nu + (step1 * step1)));
  190:         if (t >= mu) return 1.0 - p;
  191:         else return p;
  192:     } else if(tails == 2) {
  193:         if (t < 0.0) quit("Can not have t < 0 in two tailed student t cdf");
```

**Suggested fix (from review):** Use `step1 = (t-mu)/sigma` in the two-tailed branch as well.

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

## SJW2-12

**typo vs source** · Conf M / Sev L  
**Listed lines:** 645

**Function:** `betai()`

**Why it's thought to be a bug**

`if (a > 3000 && b > 300)`: NR3 uses SWITCH=3000 for both a and b. With b in (300, 3000] the code switches to the quadrature approximation outside its intended range, which reduces accuracy. This also affects F/beta/binomial/negbinomial CDFs, and negbinomial is live (atmanage/atManageSetup.c:1551). Function: `betai()`.

**Current code** (`sjwlib/nrcdflib.c` lines 643–647)

```c
  643:     if ((x < 0.0) || (x > 1.0)) quit("Yu can npt haev a probabilty <0 or > 1 x in routine betai");
  644:     if ((x == 0.0) || (x == 1.0)) return x;
  645:     if (a > 3000 && b > 300) return betaiapprox(a,b,x);   // Switch to quadrature method.
  646:         bt = exp(gammln(a+b)- gammln(a) - gammln(b) + a * log(x) + b * log(1.0 - x));
  647:     if (x < (a + 1.0) / (a + b + 2.0))
```

**Suggested fix (from review):** `b > 3000`

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

`negbinomial_cdf_values()` computes `1 - betai(k+1, r, p)`. Whether this matches the intended (p = success vs failure) parameterisation could not be confirmed.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

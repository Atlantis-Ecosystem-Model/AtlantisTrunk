# sjwlib/geodetic.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-24](#sjw2-24) | 199-212, 248-257 | M/L | division by zero | `c = cosbeta1*cosbeta2*sinomega/sinphi;` and the `phi*phi/sinphi`, `/tanphi` terms divide by `sinphi`, which i… |  | [ ] |

---

## SJW2-24

**division by zero** · Conf M / Sev L  
**Lines (current code):** 199-212, 248-257  
**Lines at review (Oct 2026):** 199-212, 248-257

**Function:** `inv_sodanos()`

**Why it's thought to be a bug**

`c = cosbeta1*cosbeta2*sinomega/sinphi;` and the `phi*phi/sinphi`, `/tanphi` terms divide by `sinphi`, which is 0 when the two points coincide, so the result is NaN. This is used via the `GEODESIC` macro (gridmetric.c:31) and `GANGLE` (gridangle.c:31), where degenerate or duplicated grid corners give NaN metrics. Function: `inv_sodanos()`, `inv_sodanos_angles()`.

**Current code** (`sjwlib/geodetic.c` lines 197–236 (truncated))

```c
  197:    term1 = sinomega*cosbeta2;
  198:    term2 = sinbeta2*cosbeta1 - sinbeta1*cosbeta2*cosomega;
  199:    sinphi = sqrt(term1*term1 + term2*term2);
  200:    phi = asin(sinphi);
  201:    cosphi = cos(phi);
  202:    tanphi = sinphi/cosphi;
  203:    c = cosbeta1*cosbeta2*sinomega/sinphi;
  204:    m = 1.0 - c*c;
  205: 
  206:    term1 = phi*(1+f1);
  207:    term2 = sinbeta1*sinbeta2*(f1*sinphi - f2*phi*phi/sinphi);
  208:    term3 = m*(-phi*f3 - f3*sinphi*cosphi + f2*phi*phi/tanphi);
  209:    term4 = (sinbeta1*sinbeta2)*(sinbeta1*sinbeta2)*(-f2*sinphi*cosphi);
  210:    term5 = m*m*(f5*phi + f5*sinphi*cosphi - f2*phi*phi/tanphi -
  211: 	   f4*sinphi*cosphi*cosphi*cosphi);
  212:    term6 = sinbeta1*sinbeta2*m*(f2*phi*phi/sinphi + f2*sinphi*cosphi*cosphi);
  213: 
  214:    return b*(term1 + term2 + term3 + term4 + term5 + term6);
  215: }
  216: 
  217: /** Compute the geodetic forward and reverse azimuths using the
  218:   * Sodano's algorithm
  219:   * http://www.anzlic.orig.au/icsm/gdatm/chapter4.htm
  220:   *
  221:   * @param x1 Longitude at point 1.
  222:   * @param y1 Latitude at point 1.
  223:   * @param x2 Longitude at point 2.
  224:   * @param y2 Latitude at point 2.
  225:   * @param a Semi-major axis of Earth.
  226:   * @param e2 First eccentricity squared of Earth.
  227:   */
  228: double inv_sodanos_angles(double x1, double y1, double x2, double y2,
  229: 		 double a, double e2, double *raz)
  230: {
  231: double term1, term2, term3;
  232: double sinphi, cosphi, tanphi, phi, c, m, v, sinv, cosv, az1, az2;
  233: double f = (1.0 - sqrt(1.0-e2));
  234: double f1 = f+f*f;
  235: double f2 = f*f/2;
  236: double beta1 = atan(tan(y1)*(1-f));
```

**Suggested fix (from review):** Return 0 (or a 0 angle) when `sinphi` is near 0.

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

`gc_distance()` (geodetic.c:284-285) can pass an `acos` argument slightly greater than 1 because of rounding, which gives NaN for nearly identical points.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

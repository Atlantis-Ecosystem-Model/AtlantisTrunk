# sjwlib/geodetic.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-24](#sjw2-24) | 203, 207 | M/L | division by zero | `c = cosbeta1*cosbeta2*sinomega/sinphi;` and the `phi*phi/sinphi`, `/tanphi` terms divide by `sinphi`, which i… |  | [ ] |

---

## SJW2-24

**division by zero** · Conf M / Sev L  
**Found at (current code):** `geodetic.c` 203, `geodetic.c` 207
**Lines at review (Oct 2026):** 199-212, 248-257

**Function:** `inv_sodanos()`

**Why it's thought to be a bug**

`c = cosbeta1*cosbeta2*sinomega/sinphi;` and the `phi*phi/sinphi`, `/tanphi` terms divide by `sinphi`, which is 0 when the two points coincide, so the result is NaN. This is used via the `GEODESIC` macro (gridmetric.c:31) and `GANGLE` (gridangle.c:31), where degenerate or duplicated grid corners give NaN metrics. Function: `inv_sodanos()`, `inv_sodanos_angles()`.

**Code at issue** — `geodetic.c` line 203  
Search for: `c = cosbeta1*cosbeta2*sinomega/sinphi`

```c
  201:    cosphi = cos(phi);
  202:    tanphi = sinphi/cosphi;
  203→    c = cosbeta1*cosbeta2*sinomega/sinphi;
  204:    m = 1.0 - c*c;
  205: 
```

**Code at issue** — `geodetic.c` line 207  
Search for: `phi*phi/sinphi`

```c
  205: 
  206:    term1 = phi*(1+f1);
  207→    term2 = sinbeta1*sinbeta2*(f1*sinphi - f2*phi*phi/sinphi);
  208:    term3 = m*(-phi*f3 - f3*sinphi*cosphi + f2*phi*phi/tanphi);
  209:    term4 = (sinbeta1*sinbeta2)*(sinbeta1*sinbeta2)*(-f2*sinphi*cosphi);
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

# atphysics/atvmix.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-13](#phy3-13) | 139 | H/M | mass conservation (updated value reused) | `c[k]` is overwritten first, then `c[k-1] = ((lvol-cvol)*c[k-1] + cvol*c[k])/lvol` uses the *new* `c[k]`. The … |  | [ ] |
| [PHY3-14](#phy3-14) | 168, 142 | M/L | accumulator inside tracer loop / sign | `bp->vflux[k] += cvol` sits inside the per-tracer loop, so the water-flux diagnostic grows with the number of … |  | [ ] |

---

## PHY3-13

**mass conservation (updated value reused)** · Conf H / Sev M  
**Found at (current code):** `atvmix.c` 139
**Lines at review (Oct 2026):** 135-139, 161-165

**Function:** `vertical_mixing()`

**Why it's thought to be a bug**

`c[k]` is overwritten first, then `c[k-1] = ((lvol-cvol)*c[k-1] + cvol*c[k])/lvol` uses the *new* `c[k]`. The total changes by `cvol*(c[k]'-c[k])`, so mass is not conserved in either branch. Function: `vertical_mixing()`.

**Code at issue** — `atvmix.c` line 139 (closest match to the quoted code; wording differs slightly)  
Search for: `c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);`

```c
  137: 						/* New concentration in sink cell
  138: 						 (average of what's there and what's transferred amount added) */
  139→ 						c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);
  140: 
  141: 						/* Update vertical flux trackers */
```

**Suggested fix (from review):** Save `ck = c[k], ck1 = c[k-1]` before updating and use the originals

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

## PHY3-14

**accumulator inside tracer loop / sign** · Conf M / Sev L  
**Found at (current code):** `atvmix.c` 168, `atvmix.c` 142
**Lines at review (Oct 2026):** 142-143,168-169

**Function:** `vertical_mixing()`

**Why it's thought to be a bug**

`bp->vflux[k] += cvol` sits inside the per-tracer loop, so the water-flux diagnostic grows with the number of dissolved tracers. The Oxygen branch also flips the sign (`vflux[k-1] += cvol`). vflux is written to output (atphysIO.c:824). Function: `vertical_mixing()`.

**Code at issue** — `atvmix.c` line 168  
Search for: `bp->vflux[k] += cvol`

```c
  166: 
  167: 						/* Update vertical flux trackers */
  168→ 						bp->vflux[k] += cvol;
  169: 						bp->vflux[k - 1] -= cvol;
  170: 
```

**Code at issue** — `atvmix.c` line 142  
Search for: `vflux[k - 1] += cvol`

```c
  140: 
  141: 						/* Update vertical flux trackers */
  142→ 						bp->vflux[k - 1] += cvol;
  143: 						bp->vflux[k] -= cvol;
  144: 
```

**Suggested fix (from review):** Accumulate vflux once per box/layer, outside the tracer loop, with a consistent sign

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

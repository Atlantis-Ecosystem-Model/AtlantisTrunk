# sjwlib/xytoij.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-22](#sjw2-22) | 296, 307, 317 | H/M | off-by-one / out-of-bounds read | The trim uses `ival > nce1` / `jval > nce2`, so `ival == nce1` (or `jval == nce2`) passes, giving `i = nce1`. … |  | [ ] |
| [SJW2-23](#sjw2-23) | 241 | H/L | dead cache / wrong guard | The "same cell as last time" fast path tests `partition->lasti > 0`, but `lasti`/`lastj` are never assigned (o… |  | [ ] |

---

## SJW2-22

**off-by-one / out-of-bounds read** · Conf H / Sev M  
**Found at (current code):** `xytoij.c` 296, `xytoij.c` 307, `xytoij.c` 317
**Lines at review (Oct 2026):** 296-309, 312-324

**Function:** `fijtoxy()`

**Why it's thought to be a bug**

The trim uses `ival > nce1` / `jval > nce2`, so `ival == nce1` (or `jval == nce2`) passes, giving `i = nce1`. The code then reads `gx[j][i+1]` and `gx[j+1][i]`, but the grid has only `nce1+1` x `nce2+1` points (`xytoij_init` called with size-1, dfcoords.c:1394-1395). For the last row, `gx[nce2+1]` is past the row-pointer array. This is reached from `cm_bilinear_2d_itoc` (dfcoords.c:1659) for points on the upper grid edge. Function: `fijtoxy()`.

**Code at issue** — `xytoij.c` line 296 (closest match to the quoted code; wording differs slightly)  
Search for: `if (ival > partition->nce1) {`

```c
  294:    }
  295: 
  296→    if (ival > partition->nce1) {
  297:      ival = partition->nce1-EPS;
  298:      status = 0;
```

**Code at issue** — `xytoij.c` line 307 (closest match to the quoted code; wording differs slightly)  
Search for: `if (jval > partition->nce2) {`

```c
  305:    }
  306: 
  307→    if (jval > partition->nce2) {
  308:      jval = partition->nce2-EPS;
  309:      status = 0;
```

**Code at issue** — `xytoij.c` line 317  
Search for: `gx[j][i+1]`

```c
  315:    v = jval - j;
  316: 
  317→    a = gx[j][i] - gx[j][i+1] - gx[j+1][i] + gx[j+1][i+1];
  318:    b = gx[j][i+1] - gx[j][i];
  319:    c = gx[j+1][i] - gx[j][i];
```

**Suggested fix (from review):** Use `>=` in both checks (clamp to `nce-EPS`), or clamp `i` to `nce1-1` and `j` to `nce2-1`.

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

## SJW2-23

**dead cache / wrong guard** · Conf H / Sev L  
**Found at (current code):** `xytoij.c` 241
**Lines at review (Oct 2026):** 241-246, 261-264

**Function:** `xytoij()`

**Why it's thought to be a bug**

The "same cell as last time" fast path tests `partition->lasti > 0`, but `lasti`/`lastj` are never assigned (only `lastcell` is set), so the cache never fires and every lookup does a full tree search. If they were set, `> 0` would also exclude column 0. Function: `xytoij()`.

**Code at issue** — `xytoij.c` line 241  
Search for: `partition->lasti > 0`

```c
  239: 
  240: /* check if point is in same cell as last point found */
  241→ 	if ((partition->lasti > 0)
  242: 	   && (pointinpoly(point,*partition->lastcell))) {
  243: 		*ival = partition->lasti;
```

**Suggested fix (from review):** Set `partition->lasti = *ival; partition->lastj = *jval;` and test `>= 0` (with `lastcell != NULL`).

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

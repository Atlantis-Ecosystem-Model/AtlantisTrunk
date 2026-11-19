# sjwlib/xytoij.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-22](#sjw2-22) | 296-309, 312-324 | H/M | off-by-one / out-of-bounds read | The trim uses `ival > nce1` / `jval > nce2`, so `ival == nce1` (or `jval == nce2`) passes, giving `i = nce1`. … |  | [ ] |
| [SJW2-23](#sjw2-23) | 241-246, 261-264 | H/L | dead cache / wrong guard | The "same cell as last time" fast path tests `partition->lasti > 0`, but `lasti`/`lastj` are never assigned (o… |  | [ ] |

---

## SJW2-22

**off-by-one / out-of-bounds read** · Conf H / Sev M  
**Lines (current code):** 296-309, 312-324  
**Lines at review (Oct 2026):** 296-309, 312-324

**Function:** `fijtoxy()`

**Why it's thought to be a bug**

The trim uses `ival > nce1` / `jval > nce2`, so `ival == nce1` (or `jval == nce2`) passes, giving `i = nce1`. The code then reads `gx[j][i+1]` and `gx[j+1][i]`, but the grid has only `nce1+1` x `nce2+1` points (`xytoij_init` called with size-1, dfcoords.c:1394-1395). For the last row, `gx[nce2+1]` is past the row-pointer array. This is reached from `cm_bilinear_2d_itoc` (dfcoords.c:1659) for points on the upper grid edge. Function: `fijtoxy()`.

**Current code** (`sjwlib/xytoij.c` lines 294–326)

```c
  294:    }
  295: 
  296:    if (ival > partition->nce1) {
  297:      ival = partition->nce1-EPS;
  298:      status = 0;
  299:    }
  300: 
  301:    /* Trim J to range 0 to nce2 */
  302:    if (jval < 0) {
  303:      jval = 0;
  304:      status = 0;
  305:    }
  306: 
  307:    if (jval > partition->nce2) {
  308:      jval = partition->nce2-EPS;
  309:      status = 0;
  310:    }
  311: 
  312:    i = (int)ival;
  313:    j = (int)jval;
  314:    u = ival - i;
  315:    v = jval - j;
  316: 
  317:    a = gx[j][i] - gx[j][i+1] - gx[j+1][i] + gx[j+1][i+1];
  318:    b = gx[j][i+1] - gx[j][i];
  319:    c = gx[j+1][i] - gx[j][i];
  320:    d = gx[j][i];
  321:    e = gy[j][i] - gy[j][i+1] - gy[j+1][i] + gy[j+1][i+1];
  322:    f = gy[j][i+1] - gy[j][i];
  323:    g = gy[j+1][i] - gy[j][i];
  324:    h = gy[j][i];
  325: 
  326:    *x = a*u*v + b*u + c*v + d;
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
**Lines (current code):** 241-246, 261-264  
**Lines at review (Oct 2026):** 241-246, 261-264

**Function:** `xytoij()`

**Why it's thought to be a bug**

The "same cell as last time" fast path tests `partition->lasti > 0`, but `lasti`/`lastj` are never assigned (only `lastcell` is set), so the cache never fires and every lookup does a full tree search. If they were set, `> 0` would also exclude column 0. Function: `xytoij()`.

**Current code** (`sjwlib/xytoij.c` lines 239–266)

```c
  239: 
  240: /* check if point is in same cell as last point found */
  241: 	if ((partition->lasti > 0)
  242: 	   && (pointinpoly(point,*partition->lastcell))) {
  243: 		*ival = partition->lasti;
  244: 		*jval = partition->lastj;
  245: 		return(1);
  246: 	}
  247: 
  248: /* check if point is in grid outline */
  249: 	currleaf = partition->trunk;
  250: 	if( !pointinpoly(point,*currleaf->boundary) )
  251: 		return(0);
  252: 
  253: /* do the full search */
  254: 	while (currleaf->half1 != NULL) {
  255: 		if (pointinpoly(point,*(currleaf->half1->boundary)))
  256: 			currleaf = currleaf->half1;
  257: 		else
  258: 			currleaf = currleaf->half2;
  259: 	}
  260: 
  261: 	*ival = (int)currleaf->mini;
  262: 	*jval = (int)currleaf->minj;
  263: 	partition->lastcell = currleaf->boundary;
  264: 	return(1);
  265: }
  266: 
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

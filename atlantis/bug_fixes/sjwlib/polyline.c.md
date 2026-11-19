# sjwlib/polyline.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-05](#sjw2-05) | 481 | H/L | memory leak | `ydd = alloc1d(pl->np)` is never freed (only x1, y1, d, x2 and y2 are). Function: `smoothpolyline()`. |  | [ ] |
| [SJW2-06](#sjw2-06) | 705, 710, 643 | H/L | memory leak | `new = reverseline(...)` then `clearpolyline(new)` frees the points but not the `polyline` struct from `create… |  | [ ] |
| [SJW2-07](#sjw2-07) | 967 | M/L | algorithm assumption / leak | The entry/exit test casts a ray from `(0, y)` to the point (`pt.x = 0`). This is only valid if x=0 lies outsid… |  | [ ] |

---

## SJW2-05

**memory leak** · Conf H / Sev L  
**Found at (current code):** `polyline.c` 481
**Lines at review (Oct 2026):** 481, 536-541

**Function:** `smoothpolyline()`

**Why it's thought to be a bug**

`ydd = alloc1d(pl->np)` is never freed (only x1, y1, d, x2 and y2 are). Function: `smoothpolyline()`.

**Code at issue** — `polyline.c` line 481  
Search for: `ydd = alloc1d(pl->np)`

```c
  479:     y1 = alloc1d(pl->np);
  480:     d = alloc1d(pl->np);
  481→     ydd = alloc1d(pl->np);
  482:     x2 = alloc1d(ns);
  483:     y2 = alloc1d(ns);
```

**Suggested fix (from review):** Add `free1d(ydd);`

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

## SJW2-06

**memory leak** · Conf H / Sev L  
**Found at (current code):** `polyline.c` 705, `polyline.c` 710, `polyline.c` 643
**Lines at review (Oct 2026):** 705-711, 714-720

**Function:** `joinsegments()`

**Why it's thought to be a bug**

`new = reverseline(...)` then `clearpolyline(new)` frees the points but not the `polyline` struct from `createpolyline()`, so it leaks on every reverse join. This runs in a loop from contour.c:125-127. Function: `joinsegments()`.

**Code at issue** — `polyline.c` line 705  
Search for: `new = reverseline(`

```c
  703: 	    }
  704: 	    else if( samepoint(list[j].end->p, pl->start->p, gap) ) {
  705→ 		new = reverseline(&list[j]);
  706: 		/* add points to start of pl */
  707: 		for(ptr=new->start->next; ptr!=NULL; ptr = ptr->next )
```

**Code at issue** — `polyline.c` line 710  
Search for: `clearpolyline(new)`

```c
  708: 		    addtostart(pl,ptr->p);
  709: 		clearpolyline(&list[j]);
  710→ 		clearpolyline(new);
  711: 		joins++;
  712: 	    }
```

**Code at issue** — `polyline.c` line 643  
Search for: `createpolyline()`

```c
  641: 
  642: 	/* make new polyline in reverse order */
  643→ 	rev = createpolyline();
  644: 	for(ptr=pl->start; ptr!=NULL; ptr = ptr->next )
  645: 	    addtostart(rev,ptr->p);
```

**Suggested fix (from review):** Use `freepolyline(new)` instead of `clearpolyline(new)`.

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

## SJW2-07

**algorithm assumption / leak** · Conf M / Sev L  
**Found at (current code):** `polyline.c` 967
**Lines at review (Oct 2026):** 962-972

**Function:** `wa_test()`

**Why it's thought to be a bug**

The entry/exit test casts a ray from `(0, y)` to the point (`pt.x = 0`). This is only valid if x=0 lies outside the polygon, so it gives wrong entry flags (union instead of intersection) for geometry that spans x<=0, e.g. negative longitudes. `left` is also malloc'd on every call and never freed. No current callers of `polyclip`, so the defect is latent. Function: `wa_test()`.

**Code at issue** — `polyline.c` line 967  
Search for: `pt.x = 0`

```c
  965:     int type = 0;
  966: 
  967→     pt.x = 0;
  968:     left = wa_create(pt, 0, 0, 0, 0, 0, 0, 0, 0.);
  969:     for (aux=p; aux->next; aux=aux->next)
```

**Suggested fix (from review):** Start the ray at `min(bound.x) - 1` (or reuse `pointinpoly`), and `free(left)`.

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

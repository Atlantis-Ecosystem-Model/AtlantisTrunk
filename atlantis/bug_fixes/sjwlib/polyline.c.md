# sjwlib/polyline.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-05](#sjw2-05) | 481, 536-541 | H/L | memory leak | `ydd = alloc1d(pl->np)` is never freed (only x1, y1, d, x2 and y2 are). Function: `smoothpolyline()`. |  | [ ] |
| [SJW2-06](#sjw2-06) | 705-711, 714-720 | H/L | memory leak | `new = reverseline(...)` then `clearpolyline(new)` frees the points but not the `polyline` struct from `create… |  | [ ] |
| [SJW2-07](#sjw2-07) | 962-972 | M/L | algorithm assumption / leak | The entry/exit test casts a ray from `(0, y)` to the point (`pt.x = 0`). This is only valid if x=0 lies outsid… |  | [ ] |

---

## SJW2-05

**memory leak** · Conf H / Sev L  
**Listed lines:** 481, 536-541

**Function:** `smoothpolyline()`

**Why it's thought to be a bug**

`ydd = alloc1d(pl->np)` is never freed (only x1, y1, d, x2 and y2 are). Function: `smoothpolyline()`.

**Current code** (`sjwlib/polyline.c` lines 479–518 (truncated))

```c
  479:     y1 = alloc1d(pl->np);
  480:     d = alloc1d(pl->np);
  481:     ydd = alloc1d(pl->np);
  482:     x2 = alloc1d(ns);
  483:     y2 = alloc1d(ns);
  484: 
  485:     /* load x1, y1, d */
  486:     i = 0;
  487:     prevx = pl->start->p.x;
  488:     prevy = pl->start->p.y;
  489:     for(ptr=pl->start; ptr != NULL; ptr = ptr->next ) {
  490: 	x = ptr->p.x;
  491: 	y = ptr->p.y;
  492: 	x1[i] = x;
  493: 	y1[i] = y;
  494: 	if( i > 0 )
  495:             d[i] = d[i-1] + sqrt((x-prevx)*(x-prevx)+(y-prevy)*(y-prevy));
  496: 	else
  497:             d[i] = 0.0;
  498: 	prevx = x;
  499: 	prevy = y;
  500: 	i++;
  501:     }
  502:     np = i;
  503: 
  504:     /* check if closed */
  505:     if( samepoint(pl->start->p, pl->end->p, 0.001) ) {
  506: 	/* calculate periodic boundary condition */
  507: 	/* NOT IMPLEMENTED YET */
  508: 	;
  509:     }
  510: 
  511:     /* fit spline to x */
  512:     dspline(d,x1,np,0,1e31,1e31,ydd);
  513:     /* loop to evaluate interpolated x values */
  514:     for(i=0; i<ns; i++) {
  515: 	/* calculate distance, evaluate spline */
  516: 	if( i< ns-1 ) dist = (double)i*d[pl->np -1] / (double)(ns-1);
  517: 	else dist = d[pl->np -1];
  518: 	dsplint(d,x1,ydd,np,dist,&x2[i]);
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
**Listed lines:** 705-711, 714-720

**Function:** `joinsegments()`

**Why it's thought to be a bug**

`new = reverseline(...)` then `clearpolyline(new)` frees the points but not the `polyline` struct from `createpolyline()`, so it leaks on every reverse join. This runs in a loop from contour.c:125-127. Function: `joinsegments()`.

**Current code** (`sjwlib/polyline.c` lines 703–722)

```c
  703: 	    }
  704: 	    else if( samepoint(list[j].end->p, pl->start->p, gap) ) {
  705: 		new = reverseline(&list[j]);
  706: 		/* add points to start of pl */
  707: 		for(ptr=new->start->next; ptr!=NULL; ptr = ptr->next )
  708: 		    addtostart(pl,ptr->p);
  709: 		clearpolyline(&list[j]);
  710: 		clearpolyline(new);
  711: 		joins++;
  712: 	    }
  713: 	    else if( samepoint(list[j].end->p, pl->end->p, gap) ) {
  714: 		new = reverseline(&list[j]);
  715: 		/* add points to end of pl */
  716: 		for(ptr=new->start->next; ptr!=NULL; ptr = ptr->next )
  717: 		    addtoend(pl, ptr->p);
  718: 		clearpolyline(&list[j]);
  719: 		clearpolyline(new);
  720: 		joins++;
  721: 	    }
  722: 	}
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
**Listed lines:** 962-972

**Function:** `wa_test()`

**Why it's thought to be a bug**

The entry/exit test casts a ray from `(0, y)` to the point (`pt.x = 0`). This is only valid if x=0 lies outside the polygon, so it gives wrong entry flags (union instead of intersection) for geometry that spans x<=0, e.g. negative longitudes. `left` is also malloc'd on every call and never freed. No current callers of `polyclip`, so the defect is latent. Function: `wa_test()`.

**Current code** (`sjwlib/polyline.c` lines 960–974)

```c
  960: /* Entry or exit?  (This just toggles with each intersection node) */
  961: int
  962: wa_test(waNode *point, waNode *p) {
  963:     waNode *aux, *left, i;
  964:     dpoint pt = point->p;
  965:     int type = 0;
  966: 
  967:     pt.x = 0;
  968:     left = wa_create(pt, 0, 0, 0, 0, 0, 0, 0, 0.);
  969:     for (aux=p; aux->next; aux=aux->next)
  970:         if (wa_I(left, point, aux, aux->next, &i.alpha, &i.alpha, &i.p)) type++;
  971:     return type%2;
  972: }
  973: 
  974: polyline *
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

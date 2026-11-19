# atphysics/atsettle.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-15](#phy3-15) ⭐ | 92, 112 | H/H | wrong condition (copy-paste) | `if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`: BOX_TYPE has `INTERNALBOX = 0, BOUNDARY, LAND` (atl… |  | [ ] |

---

## PHY3-15

**wrong condition (copy-paste)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atsettle.c` 92, `atsettle.c` 112
**Lines at review (Oct 2026):** 92

**Function:** `settleBMwc()`

**Why it's thought to be a bug**

`if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`: BOX_TYPE has `INTERNALBOX = 0, BOUNDARY, LAND` (atlantisboxmodel.h:3765). The test is true only for LAND boxes, so buoyant tracers (svel+xvel > 0) rise only in land boxes and never in normal water boxes. The sinking branch (line 112) correctly uses `!= LAND`. Function: `settleBMwc()`.

**Code at issue** — `atsettle.c` line 92  
Search for: `if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`

```c
   90: 			/* Loop over each box */
   91: 			for(b=0; b<bm->nbox; b++)
   92→ 			if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)
   93: 				advect_up(&bm->boxes[b],v*bm->dt,n,newwc[b]);
   94: 		}
```

**Code at issue** — `atsettle.c` line 112  
Search for: `!= LAND`

```c
  110: 			/* Loop over each box */
  111: 			for(b=0; b<bm->nbox; b++) {
  112→ 			if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type != LAND) {
  113: 				double m = advect_down(&bm->boxes[b],-v*bm->dt,n,newwc[b]);
  114: 
```

**Suggested fix (from review):** `... && bm->boxes[b].type != LAND`

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

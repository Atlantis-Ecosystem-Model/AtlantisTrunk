# atphysics/atsettle.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-15](#phy3-15) ⭐ | 92 | H/H | wrong condition (copy-paste) | `if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`: BOX_TYPE has `INTERNALBOX = 0, BOUNDARY, LAND` (atl… |  | [ ] |

---

## PHY3-15

**wrong condition (copy-paste)** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 92  
**Lines at review (Oct 2026):** 92

**Function:** `settleBMwc()`

**Why it's thought to be a bug**

`if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)`: BOX_TYPE has `INTERNALBOX = 0, BOUNDARY, LAND` (atlantisboxmodel.h:3765). The test is true only for LAND boxes, so buoyant tracers (svel+xvel > 0) rise only in land boxes and never in normal water boxes. The sinking branch (line 112) correctly uses `!= LAND`. Function: `settleBMwc()`.

**Current code** (`atphysics/atsettle.c` lines 90–94)

```c
   90: 			/* Loop over each box */
   91: 			for(b=0; b<bm->nbox; b++)
   92: 			if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)
   93: 				advect_up(&bm->boxes[b],v*bm->dt,n,newwc[b]);
   94: 		}
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

# atphysics/atindex.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-10](#phy3-10) | 72-76 | H/M | out of bounds / check order | `b = xytobox(...); kk = wczbtok(z, &this_bm->boxes[b]);` runs before the `b < 0` check. A point source outside… |  | [ ] |
| [PHY3-11](#phy3-11) | 246-259 | H/L | swapped return values | `wcsurface()` returns `gridz[0]` and `wcbottom()` returns `gridz[nz]`, but water-column `gridz[0] = botz` (atv… |  | [ ] |

---

## PHY3-10

**out of bounds / check order** · Conf H / Sev M  
**Lines (current code):** 72-76  
**Lines at review (Oct 2026):** 72-76

**Function:** `ext_xyztoijk()`

**Why it's thought to be a bug**

`b = xytobox(...); kk = wczbtok(z, &this_bm->boxes[b]);` runs before the `b < 0` check. A point source outside all boxes gives `boxes[-1]`, and `gridz` is then dereferenced. Called via sourcesink (atsourcesink.c:546). Function: `ext_xyztoijk()`.

**Current code** (`atphysics/atindex.c` lines 70–78)

```c
   70: 		quit("ext_xyztoijk: NULL box model pointer\n");
   71: 
   72:     b = xytobox(x, y, this_bm);
   73: 
   74:     kk = wczbtok(z, &this_bm->boxes[b]);
   75: 
   76: 	if( b < 0 || kk < 0 ){
   77: 		return(-1);
   78: 	}else {
```

**Suggested fix (from review):** Return -1 if b<0 before calling wczbtok

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

## PHY3-11

**swapped return values** · Conf H / Sev L  
**Lines (current code):** 246-259  
**Lines at review (Oct 2026):** 246-259

**Why it's thought to be a bug**

`wcsurface()` returns `gridz[0]` and `wcbottom()` returns `gridz[nz]`, but water-column `gridz[0] = botz` (atvertgeom.c:260), so the two are swapped. Currently unused (latent).

**Current code** (`atphysics/atindex.c` lines 244–261)

```c
  244: 
  245: *********************************************************************/
  246: double wcsurface(Box *bp)
  247: {
  248:     return(bp->gridz[0]);
  249: }
  250: 
  251: 
  252: /*******************************************************************//**
  253:  Routine that returns the wc bottom for a specified box.
  254: 
  255: *********************************************************************/
  256: double wcbottom(Box *bp)
  257: {
  258:     return(bp->gridz[bp->nz]);
  259: }
  260: 
  261: 
```

**Suggested fix (from review):** Swap the return values

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

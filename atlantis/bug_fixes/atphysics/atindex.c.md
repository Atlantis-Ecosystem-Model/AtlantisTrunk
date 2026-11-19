# atphysics/atindex.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-10](#phy3-10) | 72 | H/M | out of bounds / check order | `b = xytobox(...); kk = wczbtok(z, &this_bm->boxes[b]);` runs before the `b < 0` check. A point source outside… |  | [ ] |
| [PHY3-11](#phy3-11) | 248, 258 | H/L | swapped return values | `wcsurface()` returns `gridz[0]` and `wcbottom()` returns `gridz[nz]`, but water-column `gridz[0] = botz` (atv… |  | [ ] |

---

## PHY3-10

**out of bounds / check order** · Conf H / Sev M  
**Found at (current code):** `atindex.c` 72
**Lines at review (Oct 2026):** 72-76

**Function:** `ext_xyztoijk()`

**Why it's thought to be a bug**

`b = xytobox(...); kk = wczbtok(z, &this_bm->boxes[b]);` runs before the `b < 0` check. A point source outside all boxes gives `boxes[-1]`, and `gridz` is then dereferenced. Called via sourcesink (atsourcesink.c:546). Function: `ext_xyztoijk()`.

**Code at issue** — `atindex.c` line 72  
Search for: `b = xytobox(`

```c
   70: 		quit("ext_xyztoijk: NULL box model pointer\n");
   71: 
   72→     b = xytobox(x, y, this_bm);
   73: 
   74:     kk = wczbtok(z, &this_bm->boxes[b]);
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
**Found at (current code):** `atindex.c` 248, `atindex.c` 258
**Lines at review (Oct 2026):** 246-259

**Why it's thought to be a bug**

`wcsurface()` returns `gridz[0]` and `wcbottom()` returns `gridz[nz]`, but water-column `gridz[0] = botz` (atvertgeom.c:260), so the two are swapped. Currently unused (latent).

**Code at issue** — `atindex.c` line 248  
Search for: `gridz[0]`

```c
  246: double wcsurface(Box *bp)
  247: {
  248→     return(bp->gridz[0]);
  249: }
  250: 
```

**Code at issue** — `atindex.c` line 258 (closest match to the quoted code; wording differs slightly)  
Search for: `return(bp->gridz[bp->nz]);`

```c
  256: double wcbottom(Box *bp)
  257: {
  258→     return(bp->gridz[bp->nz]);
  259: }
  260: 
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

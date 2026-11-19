# atphysics/atphysIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-26](#phy1-26) | 538, 646 | M/L | ineffective optional check | These optional variables (`coastal`, `hdsource`, `hdsink`, `eflux`, `vflux`, `porosity`) are looked up with `n… |  | [ ] |
| [PHY1-27](#phy1-27) | 398 | L/L | condition mismatch | `coastal` is defined when `flagIncludesCoastal > 0` but written when `flagIncludesCoastal` is non-zero. A nega… |  | [ ] |

---

## PHY1-26

**ineffective optional check** · Conf M / Sev L  
**Found at (current code):** `atphysIO.c` 538, `atphysIO.c` 646
**Lines at review (Oct 2026):** 645, 656, 667, 677, 687, 697

**Function:** `readBMphysData()`

**Why it's thought to be a bug**

These optional variables (`coastal`, `hdsource`, `hdsink`, `eflux`, `vflux`, `porosity`) are looked up with `ncopts = NC_VERBOSE | NC_FATAL`, so a missing variable exits inside `ncvarid` before `if (id >= 0)` is reached. The `dz` lookup at 536-538 does this correctly by setting `ncopts = 0` around it. Function: `readBMphysData()`.

**Code at issue** — `atphysIO.c` line 538  
Search for: `ncopts = NC_VERBOSE | NC_FATAL`

```c
  536: 	ncopts = 0;
  537: 	id = ncvarid(fid, "dz");
  538→ 	ncopts = NC_VERBOSE | NC_FATAL;
  539: 	if (id >= 0) {
  540: 		ncvarread(fid, "dz", sizeof(doubleINPUT), start, count, (void *) array[0]);
```

**Code at issue** — `atphysIO.c` line 646  
Search for: `if (id >= 0)`

```c
  644:         /* Percent coastal strip */
  645:         id = ncvarid(fid, "coastal");
  646→         if (id >= 0) {
  647:             ncvarread(fid, "coastal", sizeof(doubleINPUT), start, count, (void *) fptmp);
  648:             checkNetCDFData1D("readBMphysData", "coastal", fptmp,  bm->nbox);
```

**Suggested fix (from review):** Set `ncopts = 0` around each optional `ncvarid`, then restore

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

## PHY1-27

**condition mismatch** · Conf L / Sev L  
**Found at (current code):** `atphysIO.c` 398
**Lines at review (Oct 2026):** 398 vs 917

**Why it's thought to be a bug**

`coastal` is defined when `flagIncludesCoastal > 0` but written when `flagIncludesCoastal` is non-zero. A negative value would write to an undefined variable, which is fatal. Functions: `writeBMphysInfo()`, `writeBMphysData()`.

**Code at issue** — `atphysIO.c` line 398  
Search for: `flagIncludesCoastal > 0`

```c
  396: 		ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Eddy strength") + 1, "Eddy strength");
  397: 
  398→         if(bm->flagIncludesCoastal > 0) {
  399:             /* Define the coastal strip variable */
  400:             i = ncvardef(fid, "coastal", dt, 2, dim);
```

**Suggested fix (from review):** Use the same test in both

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

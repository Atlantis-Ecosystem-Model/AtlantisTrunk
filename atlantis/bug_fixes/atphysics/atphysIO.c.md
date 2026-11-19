# atphysics/atphysIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-26](#phy1-26) | 647, 659, 670, 680, 690, 700 | M/L | ineffective optional check | These optional variables (`coastal`, `hdsource`, `hdsink`, `eflux`, `vflux`, `porosity`) are looked up with `n… |  | [ ] |
| [PHY1-27](#phy1-27) | 399 vs 921 | L/L | condition mismatch | `coastal` is defined when `flagIncludesCoastal > 0` but written when `flagIncludesCoastal` is non-zero. A nega… |  | [ ] |

---

## PHY1-26

**ineffective optional check** · Conf M / Sev L  
**Lines (current code):** 647, 659, 670, 680, 690, 700  
**Lines at review (Oct 2026):** 645, 656, 667, 677, 687, 697

**Function:** `readBMphysData()`

**Why it's thought to be a bug**

These optional variables (`coastal`, `hdsource`, `hdsink`, `eflux`, `vflux`, `porosity`) are looked up with `ncopts = NC_VERBOSE | NC_FATAL`, so a missing variable exits inside `ncvarid` before `if (id >= 0)` is reached. The `dz` lookup at 536-538 does this correctly by setting `ncopts = 0` around it. Function: `readBMphysData()`.

**Current code** (`atphysics/atphysIO.c` lines 645–672)

```c
  645:         id = ncvarid(fid, "coastal");
  646:         if (id >= 0) {
  647:             ncvarread(fid, "coastal", sizeof(doubleINPUT), start, count, (void *) fptmp);
  648:             checkNetCDFData1D("readBMphysData", "coastal", fptmp,  bm->nbox);
  649:             for (b = 0; b < bm->nbox; b++) {
  650:                 bm->boxes[b].coastal = fptmp[b];
  651:             }
  652:         }
  653:     }
  654:     
  655:     /* Read hydrodynamic sources if present */
  656: 	id = ncvarid(fid, "hdsource");
  657: 	if (id >= 0) {
  658: 		ncvarread(fid, "hdsource", sizeof(doubleINPUT), start, count, (void *) array[0]);
  659: 		checkNetCDFData2D("readBMphysData", "hdsource", array,  bm->nbox,bm->wcnz + bm->sednz);
  660: 		for (b = 0; b < bm->nbox; b++)
  661: 			for (i = 0; i < (bm->wcnz + bm->sednz); i++) {
  662: 				bm->hddrain[b][i] = (double)array[b][i];
  663: 			}
  664: 	}
  665: 
  666: 	/* Read hydrodynamic sinks if present */
  667: 	id = ncvarid(fid, "hdsink");
  668: 	if (id >= 0) {
  669: 		ncvarread(fid, "hdsink", sizeof(doubleINPUT), start, count, (void *) array[0]);
  670: 		checkNetCDFData2D("readBMphysData", "hdsink", array,  bm->nbox,bm->wcnz + bm->sednz);
  671: 		for (b = 0; b < bm->nbox; b++)
  672: 			for (i = 0; i < bm->wcnz + bm->sednz; i++)
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
**Lines (current code):** 399 vs 921  
**Lines at review (Oct 2026):** 398 vs 917

**Why it's thought to be a bug**

`coastal` is defined when `flagIncludesCoastal > 0` but written when `flagIncludesCoastal` is non-zero. A negative value would write to an undefined variable, which is fatal. Functions: `writeBMphysInfo()`, `writeBMphysData()`.

**Current code** (`atphysics/atphysIO.c` lines 397–401)

```c
  397: 
  398:         if(bm->flagIncludesCoastal > 0) {
  399:             /* Define the coastal strip variable */
  400:             i = ncvardef(fid, "coastal", dt, 2, dim);
  401:             ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("phys") + 1, "phys");
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

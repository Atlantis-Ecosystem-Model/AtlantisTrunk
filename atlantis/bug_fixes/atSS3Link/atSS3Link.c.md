# atSS3Link/atSS3Link.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-45](#ss3-45) | 236 | H/L | year value as index | `iy` runs over the calendar years `HistYrMin..HistYrMax-1` and is used directly as the year index into `SpawnB… |  | [ ] |
| [SS3-46](#ss3-46) | 241 | H/L | wrong variable | The `Nfleets` used in the catch and discard loops is left over from the *last* species of the biomass loop, no… |  | [ ] |
| [SS3-47](#ss3-47) | 240 | M/L | NULL deref | The filter `isFished \|\| isImpacted` includes fished species that are not assessed, but RBCspeciesArray arrays … |  | [ ] |
| [SS3-48](#ss3-48) | 64, 152 | L/L | error handling | `fork()` returning -1 is treated as the parent. `spawn("./child", ..., NULL)` passes a NULL argv to `execvp`, … |  | [ ] |

---

## SS3-45

**year value as index** · Conf H / Sev L  
**Found at (current code):** `atSS3Link.c` 236
**Lines at review (Oct 2026):** 236, 248, 255, 264

**Function:** `WriteHistoryFile()`

**Why it's thought to be a bug**

`iy` runs over the calendar years `HistYrMin..HistYrMax-1` and is used directly as the year index into `SpawnBio/CatchData/DiscData` (extent nYears, atAssessTierSetup.c:115/161/206), which is OOB. It also skips HistYrMax. Latent: WriteHistory is never called. Function: `WriteHistoryFile()`.

**Code at issue** — `atSS3Link.c` line 236 (closest match to the quoted code; wording differs slightly)  
Search for: `for (iy = bm->RBCestimation.HistYrMin; iy < bm->RBCestimation.HistYrMax; iy++){`

```c
  234: 	double totcatch, totdisc, sb0;
  235: 
  236→ 	for (iy = bm->RBCestimation.HistYrMin; iy < bm->RBCestimation.HistYrMax; iy++){
  237: 		for(region = 0; region < bm->K_num_reg; region++) {
  238: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
```

**Suggested fix (from review):** Loop `iy = 0..nPast` and print `HistYrMin+iy`

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

## SS3-46

**wrong variable** · Conf H / Sev L  
**Found at (current code):** `atSS3Link.c` 241
**Lines at review (Oct 2026):** 241, 254, 263

**Function:** `WriteHistoryFile()`

**Why it's thought to be a bug**

The `Nfleets` used in the catch and discard loops is left over from the *last* species of the biomass loop, not the current `groupIndex`. Latent. Function: `WriteHistoryFile()`.

**Code at issue** — `atSS3Link.c` line 241 (the line the review pointed to, found by its text)  
Search for: `Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);`

```c
  239:                 Nstocks = FunctGroupArray[groupIndex].numStocks;
  240: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  241→ 					Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
  242: 
  243: 					if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id] > 0)
```

**Suggested fix (from review):** Recompute Nfleets per groupIndex in each loop

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

## SS3-47

**NULL deref** · Conf M / Sev L  
**Found at (current code):** `atSS3Link.c` 240
**Lines at review (Oct 2026):** 240, 252, 261

**Function:** `WriteHistoryFile()`

**Why it's thought to be a bug**

The filter `isFished || isImpacted` includes fished species that are not assessed, but RBCspeciesArray arrays are only allocated when `assess_flag || isImpacted` (atAssessTierSetup.c:63). The fprintf in 248 also repeats "iy region" for every species. Latent. Function: `WriteHistoryFile()`.

**Code at issue** — `atSS3Link.c` line 240 (closest match to the quoted code; wording differs slightly)  
Search for: `if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {`

```c
  238: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  239:                 Nstocks = FunctGroupArray[groupIndex].numStocks;
  240→ 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  241: 					Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
  242: 
```

**Suggested fix (from review):** Filter on assess_flag/isImpacted. Print iy/region once per row

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

## SS3-48

**error handling** · Conf L / Sev L  
**Found at (current code):** `atSS3Link.c` 64, `atSS3Link.c` 152
**Lines at review (Oct 2026):** 64-65, 102, 152

**Why it's thought to be a bug**

`fork()` returning -1 is treated as the parent. `spawn("./child", ..., NULL)` passes a NULL argv to `execvp`, which is UB on many platforms. Functions: `spawn()`, `WriteSSFiles()`.

**Code at issue** — `atSS3Link.c` line 64  
Search for: `fork ()`

```c
   62: 
   63:   /* Duplicate this process.  */
   64→   child_pid = fork ();
   65:   if (child_pid != 0){
   66:     /* This is the parent process.  */
```

**Code at issue** — `atSS3Link.c` line 152  
Search for: `spawn("./child",`

```c
  150: 
  151: 	//abort();
  152→ 	spawn("./child", timeout, startTime, sleepTime, NULL);
  153: 
  154: 	result = chdir(folderName);
```

**Suggested fix (from review):** Handle -1. Pass `{program, NULL}` as arg_list

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

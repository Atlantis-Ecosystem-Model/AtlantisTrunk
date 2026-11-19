# atSS3Link/atSS3Link.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-45](#ss3-45) | 236, 248, 255, 264 | H/L | year value as index | `iy` runs over the calendar years `HistYrMin..HistYrMax-1` and is used directly as the year index into `SpawnB… |  | [ ] |
| [SS3-46](#ss3-46) | 241, 254, 263 | H/L | wrong variable | The `Nfleets` used in the catch and discard loops is left over from the *last* species of the biomass loop, no… |  | [ ] |
| [SS3-47](#ss3-47) | 240, 252, 261 | M/L | NULL deref | The filter `isFished \|\| isImpacted` includes fished species that are not assessed, but RBCspeciesArray arrays … |  | [ ] |
| [SS3-48](#ss3-48) | 64-65, 102, 152 | L/L | error handling | `fork()` returning -1 is treated as the parent. `spawn("./child", ..., NULL)` passes a NULL argv to `execvp`, … |  | [ ] |

---

## SS3-45

**year value as index** · Conf H / Sev L  
**Lines (current code):** 236, 248, 255, 264  
**Lines at review (Oct 2026):** 236, 248, 255, 264

**Function:** `WriteHistoryFile()`

**Why it's thought to be a bug**

`iy` runs over the calendar years `HistYrMin..HistYrMax-1` and is used directly as the year index into `SpawnBio/CatchData/DiscData` (extent nYears, atAssessTierSetup.c:115/161/206), which is OOB. It also skips HistYrMax. Latent: WriteHistory is never called. Function: `WriteHistoryFile()`.

**Current code** (`atSS3Link/atSS3Link.c` lines 234–257)

```c
  234: 	double totcatch, totdisc, sb0;
  235: 
  236: 	for (iy = bm->RBCestimation.HistYrMin; iy < bm->RBCestimation.HistYrMax; iy++){
  237: 		for(region = 0; region < bm->K_num_reg; region++) {
  238: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  239:                 Nstocks = FunctGroupArray[groupIndex].numStocks;
  240: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  241: 					Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
  242: 
  243: 					if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id] > 0)
  244: 						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][region];
  245: 					else
  246: 						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][region];
  247: 
  248: 					fprintf(fid, "%d %d %e", iy, region, (bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][region][iy] / sb0));
  249: 				}
  250: 			}
  251: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  252: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  253: 					totcatch = 0;
  254: 					for (f = 0; f < Nfleets; f++){
  255: 						totcatch += bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][region][iy];
  256: 					}
  257: 					fprintf(fid, " %e ", totcatch);
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
**Lines (current code):** 241, 254, 263  
**Lines at review (Oct 2026):** 241, 254, 263

**Function:** `WriteHistoryFile()`

**Why it's thought to be a bug**

The `Nfleets` used in the catch and discard loops is left over from the *last* species of the biomass loop, not the current `groupIndex`. Latent. Function: `WriteHistoryFile()`.

**Current code** (`atSS3Link/atSS3Link.c` lines 239–265)

```c
  239:                 Nstocks = FunctGroupArray[groupIndex].numStocks;
  240: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  241: 					Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
  242: 
  243: 					if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id] > 0)
  244: 						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][region];
  245: 					else
  246: 						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][region];
  247: 
  248: 					fprintf(fid, "%d %d %e", iy, region, (bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][region][iy] / sb0));
  249: 				}
  250: 			}
  251: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  252: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  253: 					totcatch = 0;
  254: 					for (f = 0; f < Nfleets; f++){
  255: 						totcatch += bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][region][iy];
  256: 					}
  257: 					fprintf(fid, " %e ", totcatch);
  258: 				}
  259: 			}
  260: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  261: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  262: 					totdisc = 0;
  263: 					for (f = 0; f < Nfleets; f++){
  264: 						totdisc += bm->RBCestimation.RBCspeciesArray[groupIndex].DiscData[f][region][iy];
  265: 					}
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
**Lines (current code):** 240, 252, 261  
**Lines at review (Oct 2026):** 240, 252, 261

**Function:** `WriteHistoryFile()`

**Why it's thought to be a bug**

The filter `isFished || isImpacted` includes fished species that are not assessed, but RBCspeciesArray arrays are only allocated when `assess_flag || isImpacted` (atAssessTierSetup.c:63). The fprintf in 248 also repeats "iy region" for every species. Latent. Function: `WriteHistoryFile()`.

**Current code** (`atSS3Link/atSS3Link.c` lines 238–263)

```c
  238: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  239:                 Nstocks = FunctGroupArray[groupIndex].numStocks;
  240: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  241: 					Nfleets = (int)(bm->RBCestimation.RBCspeciesParam[groupIndex][NumFisheries_id]);
  242: 
  243: 					if (bm->RBCestimation.RBCspeciesParam[groupIndex][Regime_year_id] > 0)
  244: 						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero_shift[Nstocks][region];
  245: 					else
  246: 						sb0 = bm->RBCestimation.RBCspeciesArray[groupIndex].SBioZero[Nstocks][region];
  247: 
  248: 					fprintf(fid, "%d %d %e", iy, region, (bm->RBCestimation.RBCspeciesArray[groupIndex].SpawnBio[Nstocks][region][iy] / sb0));
  249: 				}
  250: 			}
  251: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  252: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  253: 					totcatch = 0;
  254: 					for (f = 0; f < Nfleets; f++){
  255: 						totcatch += bm->RBCestimation.RBCspeciesArray[groupIndex].CatchData[f][region][iy];
  256: 					}
  257: 					fprintf(fid, " %e ", totcatch);
  258: 				}
  259: 			}
  260: 			for(groupIndex = 0; groupIndex < bm->K_num_tot_sp; groupIndex++){
  261: 				if(FunctGroupArray[groupIndex].isFished || FunctGroupArray[groupIndex].isImpacted) {
  262: 					totdisc = 0;
  263: 					for (f = 0; f < Nfleets; f++){
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
**Lines (current code):** 64-65, 102, 152  
**Lines at review (Oct 2026):** 64-65, 102, 152

**Why it's thought to be a bug**

`fork()` returning -1 is treated as the parent. `spawn("./child", ..., NULL)` passes a NULL argv to `execvp`, which is UB on many platforms. Functions: `spawn()`, `WriteSSFiles()`.

**Current code** (`atSS3Link/atSS3Link.c` lines 62–101 (truncated))

```c
   62: 
   63:   /* Duplicate this process.  */
   64:   child_pid = fork ();
   65:   if (child_pid != 0){
   66:     /* This is the parent process.  */
   67: 
   68: 	  /* First check that SS3 has started */
   69: 	  while(time < startTime){
   70: 
   71: 		  sleep(sleepTime);
   72: 		  time += sleepTime;
   73: 
   74: 		  /* Check for presence of runnumber.ss */
   75: 		  if( access( "runnumber.ss", F_OK ) != -1 ) {
   76: 			  started = TRUE;
   77: 			  printf("Successfully started SS3 model\n");
   78: 			  break;
   79: 		  }
   80: 	  }
   81: 
   82: 	  if(started == TRUE){
   83: 	  /* First check that SS3 has started */
   84: 		  while(time < timeout){
   85: 
   86: 			  sleep(sleepTime);
   87: 			  time += sleepTime;
   88: 
   89: 			  /* Check for presence of ss3.log */
   90: 			  if( access( "ss3.log", F_OK ) != -1 ) {
   91: 				  break;
   92: 			  }
   93: 		  }
   94: 	  }else{
   95: 		  printf("SS3 model failed to start\n");
   96: 	  }
   97: 
   98: 	  /* Check for presence of 'ss3.log' file */
   99:     return child_pid;
  100:   }else {
  101:     /* Now execute PROGRAM, searching for it in the path.  */
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

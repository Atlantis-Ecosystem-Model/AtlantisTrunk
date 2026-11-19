# atphysics/attempsalt.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-12](#phy1-12) ⭐ | 497-499 | H/H | invalid free | `if(propInput->input_Varname) c_free1d(propInput->input_Varname);`: `input_Varname` is declared `char input_Va… |  | [ ] |
| [PHY1-13](#phy1-13) | 996 | H/M | wrong variable (copy-paste) | The sediment loop accumulates `inputData->total_input += ((new_value - cur_value) * bp->dz[k]);`, using the wa… |  | [ ] |
| [PHY1-14](#phy1-14) | 999, 1011, 1154, 1165 | H/L | format/arg order | `quit("... %s in %d:%d at time %e ...", b, k, bm->dayt, inputData->variableName, ...)`: `%s` receives an int, … |  | [ ] |
| [PHY1-15](#phy1-15) | 1141, 1167 | M/M | wrong quantity | When a reset fires, `total_input += new_value * dz`, which adds the whole new concentration. `tracerForcingBM`… |  | [ ] |
| [PHY1-16](#phy1-16) | 64-66, 155-163, 191-193 | M/M | uninitialised sentinel | Only `tempid`, `saltid` and `pHid` are set to -1. `windid`, `noiseid` and `lightpid` (and the matching `*input… |  | [ ] |
| [PHY1-17](#phy1-17) | 471-480 | M/M | config backward-compat | The new keys `<name>_ignore_units` and `<name>_input_Varname` are read with `set_keyprm_errfn(quit)` in force … |  | [ ] |
| [PHY1-18](#phy1-18) | 888-890 | H/L | wrong array in message | The epibenthic unit check compares against `bm->einfo[tracerID].units`, but the error message prints `bm->tinf… |  | [ ] |
| [PHY1-19](#phy1-19) | 822 | L/L | buffer sizing | `bytes = sizeof(double)*nbox*wcnz`, but each buffered step holds `nbox*(wcnz+sednz)` values (line 830), so the… |  | [ ] |

---

## PHY1-12

**invalid free** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 497-499  
**Lines at review (Oct 2026):** 497-499

**Function:** `free_PhyPropertyData()`

**Why it's thought to be a bug**

`if(propInput->input_Varname) c_free1d(propInput->input_Varname);`: `input_Varname` is declared `char input_Varname[BMSLEN]` in the struct (atlantisboxmodel.h:4169). The test is always true, and `free()` gets a pointer into the middle of `bm` or the `forceTracerInput` malloc block. At shutdown (`freePhysics` -> `freeTempSalt`) this aborts or corrupts the heap for every active property input (temp, salt, pH, forced tracers, ...). This code is new in the uncommitted changes. Function: `free_PhyPropertyData()`.

**Current code** (`atphysics/attempsalt.c` lines 495–501)

```c
  495: void free_PhyPropertyData(MSEBoxModel *bm,PhyPropertyData *propInput){
  496: 	c_free2d(propInput->fname);
  497: 	c_free1d(propInput->variableName);
  498:     i_free1d(propInput->use_resets);
  499: 	close_phyprop(bm, propInput);
  500: }
  501: /**
```

**Suggested fix (from review):** Delete these 3 lines

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

## PHY1-13

**wrong variable (copy-paste)** · Conf H / Sev M  
**Lines (current code):** 996  
**Lines at review (Oct 2026):** 996

**Function:** `tracerForcingBM()`

**Why it's thought to be a bug**

The sediment loop accumulates `inputData->total_input += ((new_value - cur_value) * bp->dz[k]);`, using the water-column `dz`. It should use `bp->sm.dz[k]`, as `resetForcingBM` line 1148 does. `total_input` feeds the contaminant mass checks (atContaminants.c:2226-2303). Function: `tracerForcingBM()`.

**Current code** (`atphysics/attempsalt.c` lines 994–998)

```c
  994:                         inputData->total_input += ((new_value - cur_value) * bp->dz[k]);
  995: 
  996: 						if (!_finite(newsedtr[b][k][inputData->tracerID])) {
  997: 							quit("tracerForcingBM - %s in %d:%d at time %e sed value is not finite %e\n", b, k, bm->dayt, inputData->variableName, newsedtr[b][k][inputData->tracerID]);
  998: 						}
```

**Suggested fix (from review):** `bp->sm.dz[k]`

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

## PHY1-14

**format/arg order** · Conf H / Sev L  
**Lines (current code):** 999, 1011, 1154, 1165  
**Lines at review (Oct 2026):** 999, 1011, 1154, 1165

**Why it's thought to be a bug**

`quit("... %s in %d:%d at time %e ...", b, k, bm->dayt, inputData->variableName, ...)`: `%s` receives an int, so this crashes instead of printing the error message. In the 1-D branches (1011, 1165) `k` is also stale. Functions: `tracerForcingBM()`, `resetForcingBM()`.

**Current code** (`atphysics/attempsalt.c` lines 997–1013)

```c
  997: 							quit("tracerForcingBM - %s in %d:%d at time %e sed value is not finite %e\n", b, k, bm->dayt, inputData->variableName, newsedtr[b][k][inputData->tracerID]);
  998: 						}
  999: 						//fprintf(bm->logFile, "Time %e: %s in box %d:sediment layer %d is set to %e, inputData->tleft= %e, rewind = %d\n",
 1000: 						//		bm->dayt, inputData->variableName, b, k, newsedtr[b][k][inputData->tracerID], inputData->tleft, inputData->rewind);
 1001: 					}
 1002: 				}
 1003: 			} else {
 1004: 
 1005: 				/* We are just looking at 1D data */
 1006: 
 1007: 				/* Check the value read in is valid - otherwise quit now - non NAN values allowed ! */
 1008: 				if (!_finite(inputData->dataBuffer[b][0])) {
 1009: 					quit("tracerForcingBM - %s in %d:%d at time %e has non finite %e input value\n", b, k, bm->dayt, inputData->variableName, inputData->dataBuffer[b][0]);
 1010: 				}
 1011: 
 1012: 				if(inputData->isBoxValue == TRUE){
 1013: 					switch(inputData->boxValueIndex){
```

**Suggested fix (from review):** Pass `variableName` first: `(..., inputData->variableName, b, k, bm->dayt, value)`

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

## PHY1-15

**wrong quantity** · Conf M / Sev M  
**Lines (current code):** 1141, 1167  
**Lines at review (Oct 2026):** 1122, 1148

**Function:** `resetForcingBM()`

**Why it's thought to be a bug**

When a reset fires, `total_input += new_value * dz`, which adds the whole new concentration. `tracerForcingBM` adds the actual change `(new_value - cur_value) * dz`. The reset path therefore overstates the mass added, which inflates the contaminant budget tolerance. Function: `resetForcingBM()`.

**Current code** (`atphysics/attempsalt.c` lines 1139–1169)

```c
 1139:                         tol_valueA = (1.0 - inputData->ResetTol) * new_value;
 1140:                         tol_valueB = (1.0 + inputData->ResetTol) * new_value;
 1141:                         
 1142:                         if((cur_value < tol_valueA) || (cur_value > tol_valueB)) {
 1143:                             newsedtr[b][k][inputData->tracerID] = new_value;
 1144:                             inputData->total_input += (new_value * bp->sm.dz[k]);
 1145:                             
 1146:                            // fprintf(bm->logFile, "Time: %e box%d-%d in sed %s added = %e with total_input %e\n", bm->dayt, b, k, inputData->variableName, new_value, inputData->total_input);
 1147:                         }
 1148: 
 1149:                         if (!_finite(newsedtr[b][k][inputData->tracerID])) {
 1150:                             quit("resetForcingBM - %s in %d:%d sed value at time %e is not finite %e\n", b, k, bm->dayt, inputData->variableName, newsedtr[b][k][inputData->tracerID]);
 1151:                         }
 1152:                         //fprintf(bm->logFile, "Time %e: %s in box %d:sediment layer %d is set to %e, inputData->tleft= %e, rewind = %d, new_value: %e, cur_value: %e, tol_valueA: %e, tol_valueB: %e\n", bm->dayt, inputData->variableName, b, k, newsedtr[b][k][inputData->tracerID], inputData->tleft, inputData->rewind, new_value, cur_value, tol_valueA, tol_valueB);
 1153:                     }
 1154:                 }
 1155:             } else {
 1156: 
 1157:                 /* We are just looking at 1D data */
 1158: 
 1159:                 /* Check the value read in is valid - otherwise quit now - non NAN values allowed ! */
 1160:                 if (!_finite(inputData->dataBuffer[b][0])) {
 1161:                     quit("resetForcingBM - %s in %d:%d at time %e is non finite %e input\n", b, k, bm->dayt, inputData->variableName, inputData->dataBuffer[b][0]);
 1162:                 }
 1163: 
 1164:                 if(inputData->isBoxValue == TRUE){
 1165:                     switch(inputData->boxValueIndex){
 1166:                         case EDDY_ID:
 1167:                             bm->boxes[b].eddy = inputData->dataBuffer[b][0];
 1168:                             break;
 1169:                         case VERTMIX_ID:
```

**Suggested fix (from review):** `(new_value - cur_value) * dz`

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

## PHY1-16

**uninitialised sentinel** · Conf M / Sev M  
**Lines (current code):** 64-66, 155-163, 191-193  
**Lines at review (Oct 2026):** 64-66, 155-163, 191-193

**Function:** `tempsalt_init()`

**Why it's thought to be a bug**

Only `tempid`, `saltid` and `pHid` are set to -1. `windid`, `noiseid` and `lightpid` (and the matching `*input.tracerID` checked in `init_PhyPropertyData` line 303) never are, so the "no windid/noiseid/lightpid set" quits never fire. A missing tracer is silently treated as tracer 0 (or garbage). Function: `tempsalt_init()`.

**Current code** (`atphysics/attempsalt.c` lines 62–68)

```c
   62: 	int tracerIndex;
   63: 	int i = 0;
   64: 	bm->tempid = -1;
   65: 	bm->saltid = -1;
   66: 	bm->pHid = -1;
   67: 
   68: 
```

**Suggested fix (from review):** Set `bm->windid = bm->noiseid = bm->lightpid = -1;` and the `tracerID`s to -1 at the start

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

## PHY1-17

**config backward-compat** · Conf M / Sev M  
**Lines (current code):** 471-480  
**Lines at review (Oct 2026):** 471-480

**Function:** `init_forceTracers()`

**Why it's thought to be a bug**

The new keys `<name>_ignore_units` and `<name>_input_Varname` are read with `set_keyprm_errfn(quit)` in force (set at line 356), so any existing forcing prm file without them now aborts. The new code comment calls `ignore_units` optional behaviour. Function: `init_forceTracers()`.

**Current code** (`atphysics/attempsalt.c` lines 469–482)

```c
  469:             }
  470: 
  471:             snprintf(key, sizeof(key), "%s_ignore_units", bm->forceTracerInput[tracerIndex].variableName);
  472:             readkeyprm_i(fp, key, &tmp_value);
  473:             if(tmp_value > 0){
  474:                 bm->forceTracerInput[tracerIndex].ignoreUnits = TRUE; // So can load Si values and resacale from N values for Diatoms
  475:             }
  476: 
  477:             /* Name in forcing file */
  478:             
  479:             snprintf(key, sizeof(key), "%s_input_Varname", bm->forceTracerInput[tracerIndex].variableName);
  480:             readkeyprm_s(fp, key, bm->forceTracerInput[tracerIndex].input_Varname);
  481:             strcpy(bm->forceTracerInput[tracerIndex].variableName, bm->forceTracerInput[tracerIndex].input_Varname); // Relace Atlantis name with input tracer name
  482:             
```

**Suggested fix (from review):** Wrap these reads with `set_keyprm_errfn(warn)` and default `input_Varname` to the Atlantis name

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

## PHY1-18

**wrong array in message** · Conf H / Sev L  
**Lines (current code):** 888-890  
**Lines at review (Oct 2026):** 888-890

**Function:** `open_phyprop()`

**Why it's thought to be a bug**

The epibenthic unit check compares against `bm->einfo[tracerID].units`, but the error message prints `bm->tinfo[tracerID].units`. Function: `open_phyprop()`.

**Current code** (`atphysics/attempsalt.c` lines 886–892)

```c
  886:                 if(strcmp(propInput->tracer_units, bm->einfo[propInput->tracerID].units) != 0){
  887:                     quit("open_phyprop: Incompatible units in tracer forcing file %s for tracer %s. Units for tracer are '%s' and units in forcing file are '%s'.",
  888:                          propInput->fname[propInput->curFile], propInput->variableName, bm->tinfo[propInput->tracerID].units, propInput->tracer_units);
  889:                 }
  890:             } else {
  891:                 if(strcmp(propInput->tracer_units, bm->tinfo[propInput->tracerID].units) != 0){
  892:                     quit("open_phyprop: Incompatible units in tracer forcing file %s for tracer %s. Units for tracer are '%s' and units in forcing file are '%s'.",
```

**Suggested fix (from review):** Print `bm->einfo[...]` in that branch

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

## PHY1-19

**buffer sizing** · Conf L / Sev L  
**Lines (current code):** 822  
**Lines at review (Oct 2026):** 822

**Function:** `open_phyprop()`

**Why it's thought to be a bug**

`bytes = sizeof(double)*nbox*wcnz`, but each buffered step holds `nbox*(wcnz+sednz)` values (line 830), so the cache can exceed MAXBUFMEM. Function: `open_phyprop()`.

**Current code** (`atphysics/attempsalt.c` lines 820–824)

```c
  820: 	bytes = (long int)sizeof(double) * (long int)bm->nbox * (long int)bm->wcnz;
  821: 	propInput->nbuf = min(MAXBUFMEM/bytes, propInput->nstep);
  822: 	if (propInput->nbuf < 1)
  823: 		propInput->nbuf = 1;
  824: 
```

**Suggested fix (from review):** Use `wcnz + sednz`

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

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

attempsalt.c 1032/1186: 2-D (non-z) water-column forcing is written to `newwc[b][0]`, which is the bottom layer in Atlantis. If surface forcing was intended, it should be `nz-1`.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

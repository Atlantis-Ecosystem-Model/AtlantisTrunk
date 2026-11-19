# atphysics/attempsalt.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-12](#phy1-12) ⭐ | 497 | H/H | invalid free | `if(propInput->input_Varname) c_free1d(propInput->input_Varname);`: `input_Varname` is declared `char input_Va… |  | [ ] |
| [PHY1-13](#phy1-13) | 994, 1144 | H/M | wrong variable (copy-paste) | The sediment loop accumulates `inputData->total_input += ((new_value - cur_value) * bp->dz[k]);`, using the wa… |  | [ ] |
| [PHY1-14](#phy1-14) | 997 | H/L | format/arg order | `quit("... %s in %d:%d at time %e ...", b, k, bm->dayt, inputData->variableName, ...)`: `%s` receives an int, … |  | [ ] |
| [PHY1-15](#phy1-15) | 1144, 994 | M/M | wrong quantity | When a reset fires, `total_input += new_value * dz`, which adds the whole new concentration. `tracerForcingBM`… |  | [ ] |
| [PHY1-16](#phy1-16) | 160 | M/M | uninitialised sentinel | Only `tempid`, `saltid` and `pHid` are set to -1. `windid`, `noiseid` and `lightpid` (and the matching `*input… |  | [ ] |
| [PHY1-17](#phy1-17) | 356 | M/M | config backward-compat | The new keys `<name>_ignore_units` and `<name>_input_Varname` are read with `set_keyprm_errfn(quit)` in force … |  | [ ] |
| [PHY1-18](#phy1-18) | 886 | H/L | wrong array in message | The epibenthic unit check compares against `bm->einfo[tracerID].units`, but the error message prints `bm->tinf… |  | [ ] |
| [PHY1-19](#phy1-19) | 820 | L/L | buffer sizing | `bytes = sizeof(double)*nbox*wcnz`, but each buffered step holds `nbox*(wcnz+sednz)` values (line 830), so the… |  | [ ] |

---

## PHY1-12

**invalid free** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `attempsalt.c` 497
**Lines at review (Oct 2026):** 497-499

**Function:** `free_PhyPropertyData()`

**Why it's thought to be a bug**

`if(propInput->input_Varname) c_free1d(propInput->input_Varname);`: `input_Varname` is declared `char input_Varname[BMSLEN]` in the struct (atlantisboxmodel.h:4169). The test is always true, and `free()` gets a pointer into the middle of `bm` or the `forceTracerInput` malloc block. At shutdown (`freePhysics` -> `freeTempSalt`) this aborts or corrupts the heap for every active property input (temp, salt, pH, forced tracers, ...). This code is new in the uncommitted changes. Function: `free_PhyPropertyData()`.

**Code at issue** — `attempsalt.c` line 497 (located by name only; check this is the right place)  
Search for: `c_free1d(propInput->variableName);`

```c
  495: void free_PhyPropertyData(MSEBoxModel *bm,PhyPropertyData *propInput){
  496: 	c_free2d(propInput->fname);
  497→ 	c_free1d(propInput->variableName);
  498:     i_free1d(propInput->use_resets);
  499: 	close_phyprop(bm, propInput);
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
**Found at (current code):** `attempsalt.c` 994, `attempsalt.c` 1144
**Lines at review (Oct 2026):** 996

**Function:** `tracerForcingBM()`

**Why it's thought to be a bug**

The sediment loop accumulates `inputData->total_input += ((new_value - cur_value) * bp->dz[k]);`, using the water-column `dz`. It should use `bp->sm.dz[k]`, as `resetForcingBM` line 1148 does. `total_input` feeds the contaminant mass checks (atContaminants.c:2226-2303). Function: `tracerForcingBM()`.

**Code at issue** — `attempsalt.c` line 994  
Search for: `inputData->total_input += ((new_value - cur_value) * bp->dz[k])`

```c
  992:                         newsedtr[b][k][inputData->tracerID] = new_value;
  993:                         
  994→                         inputData->total_input += ((new_value - cur_value) * bp->dz[k]);
  995: 
  996: 						if (!_finite(newsedtr[b][k][inputData->tracerID])) {
```

**Code at issue** — `attempsalt.c` line 1144  
Search for: `bp->sm.dz[k]`

```c
 1142:                         if((cur_value < tol_valueA) || (cur_value > tol_valueB)) {
 1143:                             newsedtr[b][k][inputData->tracerID] = new_value;
 1144→                             inputData->total_input += (new_value * bp->sm.dz[k]);
 1145:                             
 1146:                            // fprintf(bm->logFile, "Time: %e box%d-%d in sed %s added = %e with total_input %e\n", bm->dayt, b, k, inputData->variableName, new_value, inputData->total_input);
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
**Found at (current code):** `attempsalt.c` 997
**Lines at review (Oct 2026):** 999, 1011, 1154, 1165

**Why it's thought to be a bug**

`quit("... %s in %d:%d at time %e ...", b, k, bm->dayt, inputData->variableName, ...)`: `%s` receives an int, so this crashes instead of printing the error message. In the 1-D branches (1011, 1165) `k` is also stale. Functions: `tracerForcingBM()`, `resetForcingBM()`.

**Code at issue** — `attempsalt.c` line 997  
Search for: `quit("`

```c
  995: 
  996: 						if (!_finite(newsedtr[b][k][inputData->tracerID])) {
  997→ 							quit("tracerForcingBM - %s in %d:%d at time %e sed value is not finite %e\n", b, k, bm->dayt, inputData->variableName, newsedtr[b][k][inputData->tracerID]);
  998: 						}
  999: 						//fprintf(bm->logFile, "Time %e: %s in box %d:sediment layer %d is set to %e, inputData->tleft= %e, rewind = %d\n",
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
**Found at (current code):** `attempsalt.c` 1144, `attempsalt.c` 994
**Lines at review (Oct 2026):** 1122, 1148

**Function:** `resetForcingBM()`

**Why it's thought to be a bug**

When a reset fires, `total_input += new_value * dz`, which adds the whole new concentration. `tracerForcingBM` adds the actual change `(new_value - cur_value) * dz`. The reset path therefore overstates the mass added, which inflates the contaminant budget tolerance. Function: `resetForcingBM()`.

**Code at issue** — `attempsalt.c` line 1144 (closest match to the quoted code; wording differs slightly)  
Search for: `inputData->total_input += (new_value * bp->sm.dz[k]);`

```c
 1142:                         if((cur_value < tol_valueA) || (cur_value > tol_valueB)) {
 1143:                             newsedtr[b][k][inputData->tracerID] = new_value;
 1144→                             inputData->total_input += (new_value * bp->sm.dz[k]);
 1145:                             
 1146:                            // fprintf(bm->logFile, "Time: %e box%d-%d in sed %s added = %e with total_input %e\n", bm->dayt, b, k, inputData->variableName, new_value, inputData->total_input);
```

**Code at issue** — `attempsalt.c` line 994 (closest match to the quoted code; wording differs slightly)  
Search for: `inputData->total_input += ((new_value - cur_value) * bp->dz[k]);`

```c
  992:                         newsedtr[b][k][inputData->tracerID] = new_value;
  993:                         
  994→                         inputData->total_input += ((new_value - cur_value) * bp->dz[k]);
  995: 
  996: 						if (!_finite(newsedtr[b][k][inputData->tracerID])) {
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
**Found at (current code):** `attempsalt.c` 160
**Lines at review (Oct 2026):** 64-66, 155-163, 191-193

**Function:** `tempsalt_init()`

**Why it's thought to be a bug**

Only `tempid`, `saltid` and `pHid` are set to -1. `windid`, `noiseid` and `lightpid` (and the matching `*input.tracerID` checked in `init_PhyPropertyData` line 303) never are, so the "no windid/noiseid/lightpid set" quits never fire. A missing tracer is silently treated as tracer 0 (or garbage). Function: `tempsalt_init()`.

**Code at issue** — `attempsalt.c` line 160 (the line the review pointed to, found by its text)  
Search for: `if (bm->noiseid == -1)`

```c
  158:     }
  159:     if(bm->use_pollutantfiles){
  160→         if (bm->noiseid == -1)
  161:             quit("tempsalt_init: no noiseid set\n");
  162:         if (bm->lightpid == -1)
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
**Found at (current code):** `attempsalt.c` 356
**Lines at review (Oct 2026):** 471-480

**Function:** `init_forceTracers()`

**Why it's thought to be a bug**

The new keys `<name>_ignore_units` and `<name>_input_Varname` are read with `set_keyprm_errfn(quit)` in force (set at line 356), so any existing forcing prm file without them now aborts. The new code comment calls `ignore_units` optional behaviour. Function: `init_forceTracers()`.

**Code at issue** — `attempsalt.c` line 356  
Search for: `set_keyprm_errfn(quit)`

```c
  354: 	set_keyprm_errfn(warn);
  355: 	readkeyprm_i(fp, "use_force_tracers", &bm->use_forceTracers);
  356→ 	set_keyprm_errfn(quit);
  357: 
  358: 	if (bm->use_forceTracers) {
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
**Found at (current code):** `attempsalt.c` 886
**Lines at review (Oct 2026):** 888-890

**Function:** `open_phyprop()`

**Why it's thought to be a bug**

The epibenthic unit check compares against `bm->einfo[tracerID].units`, but the error message prints `bm->tinfo[tracerID].units`. Function: `open_phyprop()`.

**Code at issue** — `attempsalt.c` line 886 (closest match to the quoted code; wording differs slightly)  
Search for: `if(strcmp(propInput->tracer_units, bm->einfo[propInput->tracerID].units) != 0){`

```c
  884:                 }
  885:             } else if(propInput->isEpi == TRUE) {
  886→                 if(strcmp(propInput->tracer_units, bm->einfo[propInput->tracerID].units) != 0){
  887:                     quit("open_phyprop: Incompatible units in tracer forcing file %s for tracer %s. Units for tracer are '%s' and units in forcing file are '%s'.",
  888:                          propInput->fname[propInput->curFile], propInput->variableName, bm->tinfo[propInput->tracerID].units, propInput->tracer_units);
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
**Found at (current code):** `attempsalt.c` 820
**Lines at review (Oct 2026):** 822

**Function:** `open_phyprop()`

**Why it's thought to be a bug**

`bytes = sizeof(double)*nbox*wcnz`, but each buffered step holds `nbox*(wcnz+sednz)` values (line 830), so the cache can exceed MAXBUFMEM. Function: `open_phyprop()`.

**Code at issue** — `attempsalt.c` line 820 (closest match to the quoted code; wording differs slightly)  
Search for: `bytes = (long int)sizeof(double) * (long int)bm->nbox * (long int)bm->wcnz;`

```c
  818: 	 * to store in buffers in memory
  819: 	 */
  820→ 	bytes = (long int)sizeof(double) * (long int)bm->nbox * (long int)bm->wcnz;
  821: 	propInput->nbuf = min(MAXBUFMEM/bytes, propInput->nstep);
  822: 	if (propInput->nbuf < 1)
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

# atphysics/aticeIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **11 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 5 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-01](#phy1-01) ⭐ | 68, 744, 707 | H/H | NULL dereference | `int *tsiceid;` is a file-global never allocated anywhere in the repo (grep: only uses are in this file). `Loa… |  | [ ] |
| [PHY1-02](#phy1-02) ⭐ | 730, 746 | H/H | uninitialised variable / wrong index | `char key[BMSLEN];` is never set, then `strcmp(key, this_ts->ts.varname[b])` compares garbage. It also indexes… |  | [ ] |
| [PHY1-03](#phy1-03) | 771, 1011 | H/M | logic / divide-by-zero | `get_ice_property(bm, &(bm->iceinput))` runs unconditionally at the end. (a) For `cdf_file`, `ice_cdf_init()` … |  | [ ] |
| [PHY1-04](#phy1-04) | 454 | H/M | type mismatch | `value = bm->tinfo[i].inice; ncattput(fid, vid, "inice", NC_LONG, 1, &value);`: `value` is a `doubleINPUT`, bu… |  | [ ] |
| [PHY1-05](#phy1-05) | 1325 | M/M | logic / reset in wrong loop | `ice->wc_layer_match[i] = bm->boxes[b].nz - 1;` (the default) runs inside the outer water-layer loop `z`. On e… |  | [ ] |
| [PHY1-06](#phy1-06) | 1258, 1329 | M/M | stale state | When ice is present, only layers `0..z-1` of `ice->ice_classes` are written. Layers `z..icenz-1` keep their va… |  | [ ] |
| [PHY1-07](#phy1-07) | 809 | H/L | format/arg mismatch | `quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);` has two `%s` but only one… |  | [ ] |
| [PHY1-08](#phy1-08) | 819 | H/L | uninitialised read | `fprintf(..., fts[i].iceclass)` prints a field of a `malloc`ed array that is never set, here or anywhere else.… |  | [ ] |
| [PHY1-09](#phy1-09) | 629 | H/L | format specifier | `quit("... code compiled with %d\n", ..., sizeof(doubleINPUT))`: `%d` is given a `size_t`. Function: `writeBMI… |  | [ ] |
| [PHY1-10](#phy1-10) | 1204 | M/L | dangling pointer | `close_iceprop` frees `dzvaluebuf` but does not NULL it. `tbuf` and `valuebuf` are NULLed. A second close (for… |  | [ ] |
| [PHY1-11](#phy1-11) | 536 | H/L | dead branch / debug output | `icestart[]` and `start[]` are identical, so the `insed\|\|inwc` if/else branches do the same thing. Leftover un… |  | [ ] |

---

## PHY1-01

**NULL dereference** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `aticeIO.c` 68, `aticeIO.c` 744, `aticeIO.c` 707
**Lines at review (Oct 2026):** 68, 707, 744, 747

**Why it's thought to be a bug**

`int *tsiceid;` is a file-global never allocated anywhere in the repo (grep: only uses are in this file). `Load_Ice_Timeseries()` writes `tsiceid[b] = -1` and `get_Loaded_Ice()` reads `tsiceid[box_id]`, so these write and read through a NULL pointer. Any run with `kind_ice_model = ts_file` segfaults. Functions: `Load_Ice_Timeseries()`, `get_Loaded_Ice()`.

**Code at issue** — `aticeIO.c` line 68  
Search for: `int *tsiceid`

```c
   66: int ntsIce; /**< Number of ice time series (should match one per box at most, but keep this int as check) */
   67: int tsIcetype; /**< Whether to use interpolated or last valid entries from time-series */
   68→ int *tsiceid; /**< Array matching box ids to ice time series entry ids */
   69: IceTimeSeries *tsIce; /**< List of ice time series (one per ice class) */
   70: 
```

**Code at issue** — `aticeIO.c` line 744  
Search for: `tsiceid[b] = -1`

```c
  742: 				/* Assign box ids */
  743: 				for (b = 0; b < bm->nbox; b++) {
  744→ 					tsiceid[b] = -1;
  745: 					for (i = 0; i < this_ts->ts.nv; i++) {
  746: 						if (strcmp(key, this_ts->ts.varname[b]) == 0) {
```

**Code at issue** — `aticeIO.c` line 707  
Search for: `tsiceid[box_id]`

```c
  705: 	double tsEvaled = 0.0;
  706: 	IceTimeSeries *this_tsIce = &tsIce[iceclass_id];
  707→ 	int ts_id = tsiceid[box_id];
  708: 
  709: 	if (tsIcetype == interpolate_id) {
```

**Suggested fix (from review):** `tsiceid = i_alloc1d(bm->nbox);` before the loop (free it in `freeIceFiles`)

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

## PHY1-02

**uninitialised variable / wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `aticeIO.c` 730, `aticeIO.c` 746
**Lines at review (Oct 2026):** 730, 746

**Function:** `Load_Ice_Timeseries()`

**Why it's thought to be a bug**

`char key[BMSLEN];` is never set, then `strcmp(key, this_ts->ts.varname[b])` compares garbage. It also indexes `varname[b]` (box) where it should be `varname[i]` (the series variable being looped over, `i < ts.nv`), so it can read past `nv`. The box-to-column mapping is wrong in every case. Function: `Load_Ice_Timeseries()`.

**Code at issue** — `aticeIO.c` line 730  
Search for: `char key[BMSLEN]`

```c
  728: int Load_Ice_Timeseries(MSEBoxModel *bm) {
  729: 	int b, i;
  730→ 	char key[BMSLEN];
  731: 
  732: 	/* Load catch time series */
```

**Code at issue** — `aticeIO.c` line 746  
Search for: `strcmp(key, this_ts->ts.varname[b])`

```c
  744: 					tsiceid[b] = -1;
  745: 					for (i = 0; i < this_ts->ts.nv; i++) {
  746→ 						if (strcmp(key, this_ts->ts.varname[b]) == 0) {
  747: 							tsiceid[b] = i;
  748: 							break;
```

**Suggested fix (from review):** Build `key` for box b (e.g. `snprintf(key,...,"%d",b)` or whatever column naming the ts uses) and compare to `varname[i]`

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

## PHY1-03

**logic / divide-by-zero** · Conf H / Sev M  
**Found at (current code):** `aticeIO.c` 771, `aticeIO.c` 1011
**Lines at review (Oct 2026):** 771 (with 944)

**Function:** `Load_Ice_Timeseries()`

**Why it's thought to be a bug**

`get_ice_property(bm, &(bm->iceinput))` runs unconditionally at the end. (a) For `cdf_file`, `ice_cdf_init()` already called it at line 944, so the second call increments `nextrec` again and the model starts on the 2nd ice record, one step out of sync. (b) For `ts_file` (and if ice is off), `iceinput` was never opened: `nstep==0` triggers `close_iceprop` on an unopened fid, then `% propInput->nFiles` with `nFiles==0` is an integer divide by zero. Function: `Load_Ice_Timeseries()`.

**Code at issue** — `aticeIO.c` line 771  
Search for: `get_ice_property(bm, &(bm->iceinput))`

```c
  769: 	}
  770: 
  771→ 	get_ice_property(bm, &(bm->iceinput));
  772: 
  773: 	return 0;
```

**Code at issue** — `aticeIO.c` line 1011  
Search for: `% propInput->nFiles`

```c
 1009: 		close_iceprop(bm, propInput);
 1010: 
 1011→ 		propInput->curFile = (propInput->curFile + 1) % propInput->nFiles;
 1012: 		open_iceprop(bm, propInput);
 1013: 		tstepnum = propInput->nextrec;
```

**Suggested fix (from review):** Delete line 771 (or move it into the `cdf_file` case and drop the call in `ice_cdf_init`)

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

## PHY1-04

**type mismatch** · Conf H / Sev M  
**Found at (current code):** `aticeIO.c` 454
**Lines at review (Oct 2026):** 454-455

**Function:** `writeBMIceInfo()`

**Why it's thought to be a bug**

`value = bm->tinfo[i].inice; ncattput(fid, vid, "inice", NC_LONG, 1, &value);`: `value` is a `doubleINPUT`, but NC_LONG makes netCDF read the buffer as a 4-byte int. That writes the low half of the double's bit pattern, which is 0 for 1.0 on little-endian. Ice-only tracers get `inice=0` in output and restart files, and `readBMIceInfo` (line 352) reads that back for particulate tracers. Every other int attribute here passes `&bm->tinfo[i].<field>`. Function: `writeBMIceInfo()`.

**Code at issue** — `aticeIO.c` line 454  
Search for: `value = bm->tinfo[i].inice;`

```c
  452: 				ncattput(fid, vid, "i_conc", NC_DOUBLE, 1, &value);
  453: 			}
  454→ 			value = bm->tinfo[i].inice;
  455: 			ncattput(fid, vid, "inice", NC_LONG, 1, &value);
  456: 
```

**Suggested fix (from review):** `ncattput(fid, vid, "inice", NC_LONG, 1, &bm->tinfo[i].inice);`

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

## PHY1-05

**logic / reset in wrong loop** · Conf M / Sev M  
**Found at (current code):** `aticeIO.c` 1325
**Lines at review (Oct 2026):** 1319-1337

**Function:** `Enter_IceBM()`

**Why it's thought to be a bug**

`ice->wc_layer_match[i] = bm->boxes[b].nz - 1;` (the default) runs inside the outer water-layer loop `z`. On each new `z` the inner loop goes over ice layers from 0 again and resets the layers already matched to a higher `z` back to `nz-1`, because their `last_depth < icedepth` test now fails. So any ice layer that maps to a water layer between 1 and nz-2 ends up recorded as the surface layer. Function: `Enter_IceBM()`.

**Code at issue** — `aticeIO.c` line 1325  
Search for: `ice->wc_layer_match[i] = bm->boxes[b].nz - 1`

```c
 1323: 				icedepth += ice->dz[i];
 1324: 
 1325→ 				ice->wc_layer_match[i] = bm->boxes[b].nz - 1;  // As a default say its at the surface
 1326: 				if((last_depth < icedepth) && (icedepth <= tot_depth)){
 1327: 					bm->boxes[b].has_ice = 1;
```

**Suggested fix (from review):** Set the default once per box, before the `z` loop

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

## PHY1-06

**stale state** · Conf M / Sev M  
**Found at (current code):** `aticeIO.c` 1258, `aticeIO.c` 1329
**Lines at review (Oct 2026):** 1249-1295, 1322-1330

**Function:** `Enter_IceBM()`

**Why it's thought to be a bug**

When ice is present, only layers `0..z-1` of `ice->ice_classes` are written. Layers `z..icenz-1` keep their values from the previous time step when ice thins, and nothing zeroes them. The mapping loop then adds `ice_classes[i][k]` over all `icenz` layers into `ice_prop`, so stale ice fractions get counted. Function: `Enter_IceBM()`.

**Code at issue** — `aticeIO.c` line 1258  
Search for: `ice->ice_classes`

```c
 1256: 				// Sort out the ice entry
 1257: 				for(i=0; i<bm->K_num_ice_classes; i++){
 1258→ 					ice->ice_classes[z][i] = ice_data_array[b][i] * depth_prop;
 1259: 				}
 1260: 
```

**Code at issue** — `aticeIO.c` line 1329  
Search for: `ice_classes[i][k]`

```c
 1327: 					bm->boxes[b].has_ice = 1;
 1328: 					for(k=0; k<bm->K_num_ice_classes; k++){
 1329→ 						bm->boxes[b].ice_prop[z][k] += ice->ice_classes[i][k];
 1330: 						ice->wc_layer_match[i] = z;
 1331: 					}
```

**Suggested fix (from review):** After filling, zero `ice_classes[zz][*]` for `zz >= z`

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

## PHY1-07

**format/arg mismatch** · Conf H / Sev L  
**Found at (current code):** `aticeIO.c` 809
**Lines at review (Oct 2026):** 809

**Function:** `Ice_Read_Time_Series()`

**Why it's thought to be a bug**

`quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);` has two `%s` but only one argument, which is undefined behaviour on this error path. Function: `Ice_Read_Time_Series()`.

**Code at issue** — `aticeIO.c` line 809  
Search for: `quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname)`

```c
  807: 	/* Allocate memory for list of time series */
  808: 	if ((fts = (IceTimeSeries *) malloc((size_t)nnts * sizeof(IceTimeSeries))) == NULL)
  809→ 		quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);
  810: 
  811: 	/* Read each fisheries time series input */
```

**Suggested fix (from review):** Add a second `tsname` argument

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

## PHY1-08

**uninitialised read** · Conf H / Sev L  
**Found at (current code):** `aticeIO.c` 819
**Lines at review (Oct 2026):** 819

**Function:** `Ice_Read_Time_Series()`

**Why it's thought to be a bug**

`fprintf(..., fts[i].iceclass)` prints a field of a `malloc`ed array that is never set, here or anywhere else. Function: `Ice_Read_Time_Series()`.

**Code at issue** — `aticeIO.c` line 819  
Search for: `fprintf(`

```c
  817: 		snprintf(key, sizeof(key), "%sts%d.data", tsname, i);
  818: 
  819→ 		fprintf(stderr, "Loading %s for class %d\n", key, fts[i].iceclass);
  820: 
  821: 		readkeyprm_s(fp, key, buf);
```

**Suggested fix (from review):** Set `fts[i].iceclass = i;` before printing

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

## PHY1-09

**format specifier** · Conf H / Sev L  
**Found at (current code):** `aticeIO.c` 629
**Lines at review (Oct 2026):** 629

**Function:** `writeBMIceData()`

**Why it's thought to be a bug**

`quit("... code compiled with %d\n", ..., sizeof(doubleINPUT))`: `%d` is given a `size_t`. Function: `writeBMIceData()`.

**Code at issue** — `aticeIO.c` line 629  
Search for: `quit("`

```c
  627: 			/* Check double compatibility */
  628: 			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
  629→ 				quit("writeBMIceData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, n, sizeof(doubleINPUT));
  630: 
  631: 			/* Get ice data from model storage */
```

**Suggested fix (from review):** `%zu` or cast to `(int)`

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

## PHY1-10

**dangling pointer** · Conf M / Sev L  
**Found at (current code):** `aticeIO.c` 1204
**Lines at review (Oct 2026):** 1203-1207

**Function:** `close_iceprop()`

**Why it's thought to be a bug**

`close_iceprop` frees `dzvaluebuf` but does not NULL it. `tbuf` and `valuebuf` are NULLed. A second close (for example `freeCDFIceFiles` after a failed reopen) would double-free. Function: `close_iceprop()`.

**Code at issue** — `aticeIO.c` line 1204 (located by name only; check this is the right place)  
Search for: `free2d((double **)propInput->dzvaluebuf);`

```c
 1202: 	free1d(propInput->tbuf);
 1203: 	free3d((double ***) propInput->valuebuf);
 1204→ 	free2d((double **)propInput->dzvaluebuf);
 1205: 
 1206: 	propInput->tbuf = NULL;
```

**Suggested fix (from review):** Add `propInput->dzvaluebuf = NULL;`

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

## PHY1-11

**dead branch / debug output** · Conf H / Sev L  
**Found at (current code):** `aticeIO.c` 536
**Lines at review (Oct 2026):** 536-547, 370, 516, 534, 537, 542

**Why it's thought to be a bug**

`icestart[]` and `start[]` are identical, so the `insed||inwc` if/else branches do the same thing. Leftover unconditional `printf` debug lines ("in gere", every ice dz, currentnz per box) flood stdout. Functions: `readBMIceData()`, `readBMIceInfo()`.

**Code at issue** — `aticeIO.c` line 536 (closest match to the quoted code; wording differs slightly)  
Search for: `if(bm->tinfo[i].insed || bm->tinfo[i].inwc){`

```c
  534: 		printf(" bm->tinfo[i].name, = %s\n", bm->tinfo[i].name);
  535: 
  536→ 		if(bm->tinfo[i].insed || bm->tinfo[i].inwc){
  537: 			printf("in gere\n");
  538: 			/* Read data */
```

**Suggested fix (from review):** Drop one branch; guard or remove the printfs

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

aticeIO.c `Get_Ice()` (ts_file): inside the class loop, `ts_ice_dz_data[b]` is overwritten for every class, so the ice depth comes from the last class's depth series only.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

aticeIO.c `readBMIceInfo()` 346-352: for particulate ice-only tracers, `inice` (set to 1 "by definition") is overwritten from the file attribute. Combined with PHY1-04, a restart file could switch ice tracers off.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

aticeIO.c `Enter_IceBM()` 1267-1270: if `tot_depth > maxicedz*icenz`, `z` is set to `icenz-1` and layer `icenz-2` is then overwritten. This looks unintended, but the intent is unclear.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

aticeIO.c 903: the parameter key is spelled `"nInceFiles"`, so user files must use that typo. The same goes for `"temp_ampltiude"` at atphysics.c:376.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-5

aticeIO.c `open_iceprop()` 1173: buffer size is computed from `icenz`, but the buffers hold `K_num_ice_classes` values per box.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

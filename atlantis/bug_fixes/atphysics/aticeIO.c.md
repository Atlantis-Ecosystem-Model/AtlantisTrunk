# atphysics/aticeIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **11 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2), plus 5 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY1-01](#phy1-01) ⭐ | 68, 707, 744, 747 | H/H | NULL dereference | `int *tsiceid;` is a file-global never allocated anywhere in the repo (grep: only uses are in this file). `Loa… |  | [ ] |
| [PHY1-02](#phy1-02) ⭐ | 730, 746 | H/H | uninitialised variable / wrong index | `char key[BMSLEN];` is never set, then `strcmp(key, this_ts->ts.varname[b])` compares garbage. It also indexes… |  | [ ] |
| [PHY1-03](#phy1-03) | 771 (with 944) | H/M | logic / divide-by-zero | `get_ice_property(bm, &(bm->iceinput))` runs unconditionally at the end. (a) For `cdf_file`, `ice_cdf_init()` … |  | [ ] |
| [PHY1-04](#phy1-04) | 454-455 | H/M | type mismatch | `value = bm->tinfo[i].inice; ncattput(fid, vid, "inice", NC_LONG, 1, &value);`: `value` is a `doubleINPUT`, bu… |  | [ ] |
| [PHY1-05](#phy1-05) | 1319-1337 | M/M | logic / reset in wrong loop | `ice->wc_layer_match[i] = bm->boxes[b].nz - 1;` (the default) runs inside the outer water-layer loop `z`. On e… |  | [ ] |
| [PHY1-06](#phy1-06) | 1249-1295, 1322-1330 | M/M | stale state | When ice is present, only layers `0..z-1` of `ice->ice_classes` are written. Layers `z..icenz-1` keep their va… |  | [ ] |
| [PHY1-07](#phy1-07) | 809 | H/L | format/arg mismatch | `quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);` has two `%s` but only one… |  | [ ] |
| [PHY1-08](#phy1-08) | 819 | H/L | uninitialised read | `fprintf(..., fts[i].iceclass)` prints a field of a `malloc`ed array that is never set, here or anywhere else.… |  | [ ] |
| [PHY1-09](#phy1-09) | 629 | H/L | format specifier | `quit("... code compiled with %d\n", ..., sizeof(doubleINPUT))`: `%d` is given a `size_t`. Function: `writeBMI… |  | [ ] |
| [PHY1-10](#phy1-10) | 1203-1207 | M/L | dangling pointer | `close_iceprop` frees `dzvaluebuf` but does not NULL it. `tbuf` and `valuebuf` are NULLed. A second close (for… |  | [ ] |
| [PHY1-11](#phy1-11) | 536-547, 370, 516, 534, 537, 542 | H/L | dead branch / debug output | `icestart[]` and `start[]` are identical, so the `insed\|\|inwc` if/else branches do the same thing. Leftover un… |  | [ ] |

---

## PHY1-01

**NULL dereference** · Conf H / Sev H ⭐ priority  
**Listed lines:** 68, 707, 744, 747

**Why it's thought to be a bug**

`int *tsiceid;` is a file-global never allocated anywhere in the repo (grep: only uses are in this file). `Load_Ice_Timeseries()` writes `tsiceid[b] = -1` and `get_Loaded_Ice()` reads `tsiceid[box_id]`, so these write and read through a NULL pointer. Any run with `kind_ice_model = ts_file` segfaults. Functions: `Load_Ice_Timeseries()`, `get_Loaded_Ice()`.

**Current code** (`atphysics/aticeIO.c` lines 66–70)

```c
   66: int ntsIce; /**< Number of ice time series (should match one per box at most, but keep this int as check) */
   67: int tsIcetype; /**< Whether to use interpolated or last valid entries from time-series */
   68: int *tsiceid; /**< Array matching box ids to ice time series entry ids */
   69: IceTimeSeries *tsIce; /**< List of ice time series (one per ice class) */
   70: 
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
**Listed lines:** 730, 746

**Function:** `Load_Ice_Timeseries()`

**Why it's thought to be a bug**

`char key[BMSLEN];` is never set, then `strcmp(key, this_ts->ts.varname[b])` compares garbage. It also indexes `varname[b]` (box) where it should be `varname[i]` (the series variable being looped over, `i < ts.nv`), so it can read past `nv`. The box-to-column mapping is wrong in every case. Function: `Load_Ice_Timeseries()`.

**Current code** (`atphysics/aticeIO.c` lines 728–748)

```c
  728: int Load_Ice_Timeseries(MSEBoxModel *bm) {
  729: 	int b, i;
  730: 	char key[BMSLEN];
  731: 
  732: 	/* Load catch time series */
  733: 	if (bm->ice_on) {
  734: 		switch(bm->kind_ice_model) {
  735: 		case ts_file:
  736: 			tsIceWarned = 0;
  737: 			Ice_Read_Time_Series(bm, bm->forceIfname, "Ice", bm->t_units, &tsIce, &ntsIce, &tsIcetype);
  738: 
  739: 			if (ntsIce) {
  740: 				IceTimeSeries *this_ts = &tsIce[0];
  741: 
  742: 				/* Assign box ids */
  743: 				for (b = 0; b < bm->nbox; b++) {
  744: 					tsiceid[b] = -1;
  745: 					for (i = 0; i < this_ts->ts.nv; i++) {
  746: 						if (strcmp(key, this_ts->ts.varname[b]) == 0) {
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
**Listed lines:** 771 (with 944)

**Function:** `Load_Ice_Timeseries()`

**Why it's thought to be a bug**

`get_ice_property(bm, &(bm->iceinput))` runs unconditionally at the end. (a) For `cdf_file`, `ice_cdf_init()` already called it at line 944, so the second call increments `nextrec` again and the model starts on the 2nd ice record, one step out of sync. (b) For `ts_file` (and if ice is off), `iceinput` was never opened: `nstep==0` triggers `close_iceprop` on an unopened fid, then `% propInput->nFiles` with `nFiles==0` is an integer divide by zero. Function: `Load_Ice_Timeseries()`.

**Current code** (`atphysics/aticeIO.c` lines 769–773)

```c
  769: 	}
  770: 
  771: 	get_ice_property(bm, &(bm->iceinput));
  772: 
  773: 	return 0;
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
**Listed lines:** 454-455

**Function:** `writeBMIceInfo()`

**Why it's thought to be a bug**

`value = bm->tinfo[i].inice; ncattput(fid, vid, "inice", NC_LONG, 1, &value);`: `value` is a `doubleINPUT`, but NC_LONG makes netCDF read the buffer as a 4-byte int. That writes the low half of the double's bit pattern, which is 0 for 1.0 on little-endian. Ice-only tracers get `inice=0` in output and restart files, and `readBMIceInfo` (line 352) reads that back for particulate tracers. Every other int attribute here passes `&bm->tinfo[i].<field>`. Function: `writeBMIceInfo()`.

**Current code** (`atphysics/aticeIO.c` lines 452–457)

```c
  452: 				ncattput(fid, vid, "i_conc", NC_DOUBLE, 1, &value);
  453: 			}
  454: 			value = bm->tinfo[i].inice;
  455: 			ncattput(fid, vid, "inice", NC_LONG, 1, &value);
  456: 
  457: 			if (verbose > 1)
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
**Listed lines:** 1319-1337

**Function:** `Enter_IceBM()`

**Why it's thought to be a bug**

`ice->wc_layer_match[i] = bm->boxes[b].nz - 1;` (the default) runs inside the outer water-layer loop `z`. On each new `z` the inner loop goes over ice layers from 0 again and resets the layers already matched to a higher `z` back to `nz-1`, because their `last_depth < icedepth` test now fails. So any ice layer that maps to a water layer between 1 and nz-2 ends up recorded as the surface layer. Function: `Enter_IceBM()`.

**Current code** (`atphysics/aticeIO.c` lines 1317–1339)

```c
 1317: 
 1318: 		// Assign the actual value
 1319: 		for(z=bm->boxes[b].nz - 1; z>= 0; z--){
 1320: 			tot_depth += bm->boxes[b].dz[z];
 1321: 			icedepth = 0.0;
 1322: 			for(i=0; i<bm->icenz; i++){
 1323: 				icedepth += ice->dz[i];
 1324: 
 1325: 				ice->wc_layer_match[i] = bm->boxes[b].nz - 1;  // As a default say its at the surface
 1326: 				if((last_depth < icedepth) && (icedepth <= tot_depth)){
 1327: 					bm->boxes[b].has_ice = 1;
 1328: 					for(k=0; k<bm->K_num_ice_classes; k++){
 1329: 						bm->boxes[b].ice_prop[z][k] += ice->ice_classes[i][k];
 1330: 						ice->wc_layer_match[i] = z;
 1331: 					}
 1332: 				} else if(icedepth > tot_depth) {
 1333: 					break;
 1334: 				}
 1335: 			}
 1336: 			last_depth = tot_depth;
 1337: 		}
 1338: 
 1339: 		// Find total ice cover
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
**Listed lines:** 1249-1295, 1322-1330

**Function:** `Enter_IceBM()`

**Why it's thought to be a bug**

When ice is present, only layers `0..z-1` of `ice->ice_classes` are written. Layers `z..icenz-1` keep their values from the previous time step when ice thins, and nothing zeroes them. The mapping loop then adds `ice_classes[i][k]` over all `icenz` layers into `ice_prop`, so stale ice fractions get counted. Function: `Enter_IceBM()`.

**Current code** (`atphysics/aticeIO.c` lines 1251–1290 (truncated)) — code appears to have moved +4 lines since the review

```c
 1251: 			slush_allowed = 1;
 1252: 
 1253: 			while(ice_depth > tot_depth && z < bm->icenz){
 1254: 				last_depth = tot_depth;
 1255: 
 1256: 				// Sort out the ice entry
 1257: 				for(i=0; i<bm->K_num_ice_classes; i++){
 1258: 					ice->ice_classes[z][i] = ice_data_array[b][i] * depth_prop;
 1259: 				}
 1260: 
 1261: 				// Now update
 1262: 				tot_depth += ice->dz[z];
 1263: 
 1264: 				z++;
 1265: 			}
 1266: 			// Make sure don't exceed max possible depth
 1267: 			if(tot_depth > (max_possible_ice)){
 1268: 				tot_depth = ice_depth - 1.0;
 1269: 				z = bm->icenz - 1;
 1270: 				slush_allowed = 0;
 1271: 			}
 1272: 			if(z >= bm->icenz) {
 1273: 				// Nothing else to do as all ice full
 1274: 			} else {
 1275: 				// Final layer - all slush
 1276: 				// TODO: Handle this more effectively
 1277: 				if( slush_allowed ) {
 1278: 					for(i=0; i<bm->K_num_ice_classes; i++){
 1279: 						if ( i != bm->slush_id ){
 1280: 							ice->ice_classes[z-1][i] = 0.0;
 1281: 						} else {
 1282: 							ice->ice_classes[z-1][i] = ice_data_array[b][i] * depth_prop;
 1283: 						}
 1284: 					}
 1285: 				} else {
 1286: 				// Final layer is same as other layers all way through
 1287: 					for(i=0; i<bm->K_num_ice_classes; i++){
 1288: 						ice->ice_classes[z-1][i] = ice_data_array[b][i] * depth_prop;
 1289: 					}
 1290: 				}
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
**Listed lines:** 809

**Function:** `Ice_Read_Time_Series()`

**Why it's thought to be a bug**

`quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);` has two `%s` but only one argument, which is undefined behaviour on this error path. Function: `Ice_Read_Time_Series()`.

**Current code** (`atphysics/aticeIO.c` lines 807–811)

```c
  807: 	/* Allocate memory for list of time series */
  808: 	if ((fts = (IceTimeSeries *) malloc((size_t)nnts * sizeof(IceTimeSeries))) == NULL)
  809: 		quit("read%sTimeSeries: Can't allocate memory for %s time series list\n", tsname);
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
**Listed lines:** 819

**Function:** `Ice_Read_Time_Series()`

**Why it's thought to be a bug**

`fprintf(..., fts[i].iceclass)` prints a field of a `malloc`ed array that is never set, here or anywhere else. Function: `Ice_Read_Time_Series()`.

**Current code** (`atphysics/aticeIO.c` lines 817–821)

```c
  817: 		snprintf(key, sizeof(key), "%sts%d.data", tsname, i);
  818: 
  819: 		fprintf(stderr, "Loading %s for class %d\n", key, fts[i].iceclass);
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
**Listed lines:** 629

**Function:** `writeBMIceData()`

**Why it's thought to be a bug**

`quit("... code compiled with %d\n", ..., sizeof(doubleINPUT))`: `%d` is given a `size_t`. Function: `writeBMIceData()`.

**Current code** (`atphysics/aticeIO.c` lines 627–631)

```c
  627: 			/* Check double compatibility */
  628: 			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
  629: 				quit("writeBMIceData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, n, sizeof(doubleINPUT));
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
**Listed lines:** 1203-1207

**Function:** `close_iceprop()`

**Why it's thought to be a bug**

`close_iceprop` frees `dzvaluebuf` but does not NULL it. `tbuf` and `valuebuf` are NULLed. A second close (for example `freeCDFIceFiles` after a failed reopen) would double-free. Function: `close_iceprop()`.

**Current code** (`atphysics/aticeIO.c` lines 1201–1209)

```c
 1201: 	/* Free buffers */
 1202: 	free1d(propInput->tbuf);
 1203: 	free3d((double ***) propInput->valuebuf);
 1204: 	free2d((double **)propInput->dzvaluebuf);
 1205: 
 1206: 	propInput->tbuf = NULL;
 1207: 	propInput->valuebuf = NULL;
 1208: }
 1209: 
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
**Listed lines:** 536-547, 370, 516, 534, 537, 542

**Why it's thought to be a bug**

`icestart[]` and `start[]` are identical, so the `insed||inwc` if/else branches do the same thing. Leftover unconditional `printf` debug lines ("in gere", every ice dz, currentnz per box) flood stdout. Functions: `readBMIceData()`, `readBMIceInfo()`.

**Current code** (`atphysics/aticeIO.c` lines 534–549)

```c
  534: 		printf(" bm->tinfo[i].name, = %s\n", bm->tinfo[i].name);
  535: 
  536: 		if(bm->tinfo[i].insed || bm->tinfo[i].inwc){
  537: 			printf("in gere\n");
  538: 			/* Read data */
  539: 			ncvarread(fid, bm->tinfo[i].name, sizeof(doubleINPUT), icestart, count, val[0]);
  540: 
  541: 		}else{
  542: 			printf("in gere 2\n");
  543: 
  544: 			/* Read data */
  545: 			ncvarread(fid, bm->tinfo[i].name, sizeof(doubleINPUT), start, count, val[0]);
  546: 
  547: 		}
  548: 
  549: 
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

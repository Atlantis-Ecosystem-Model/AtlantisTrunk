# atlantisUtil/atUtil.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-25](#util-25) | 893-897 | H/M | unit/reference mismatch (relative vs absolute day) | Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarves… |  | [ ] |
| [UTIL-26](#util-26) | 1050-1061 | M/M | wrong algorithm | The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and return… |  | [ ] |
| [UTIL-27](#util-27) | 861 | M/L | wrong index (copy-paste) | Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[ch… |  | [ ] |
| [UTIL-28](#util-28) | 359 | H/L | wrong parameter key string | `paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atU… |  | [ ] |
| [UTIL-29](#util-29) | 1237-1238, 1254-1255, 1249 | M/L | NULL dereference / wrong message | `*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileN… |  | [ ] |
| [UTIL-30](#util-30) | 1039, 1177 | L/L | log(0) | `log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Res… |  | [ ] |

---

## UTIL-25

**unit/reference mismatch (relative vs absolute day)** · Conf H / Sev M  
**Lines (current code):** 893-897  
**Lines at review (Oct 2026):** 841-845

**Function:** `Util_Copy_Change_Values()`

**Why it's thought to be a bug**

Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarvest.c:1933. But the insertion point is chosen with `originalArray[i][start_id] > start`, using the relative `start`. Once `dayt > 0`, the new change goes in the wrong place, and `Util_Get_Change_Scale` (which assumes ascending start dates) picks the wrong active change. Callers: atHarvest.c:1943/1984, atbiolUtil.c:149. Function: `Util_Copy_Change_Values()`.

**Current code** (`atlantisUtil/atUtil.c` lines 891–899)

```c
  891: 					newArray[arrayIndex][mult_id] = mult;
  892: 					arrayIndex++;
  893: 					beenInserted = TRUE;
  894: 				}
  895: 			}
  896: 			newArray[arrayIndex][start_id] = originalArray[i][start_id];
  897: 			newArray[arrayIndex][period_id] = originalArray[i][period_id];
  898: 			newArray[arrayIndex][mult_id] = originalArray[i][mult_id];
  899: 
```

**Suggested fix (from review):** Compare against `start + bm->dayt`.

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

## UTIL-26

**wrong algorithm** · Conf M / Sev M  
**Lines (current code):** 1050-1061  
**Lines at review (Oct 2026):** 998-1009

**Function:** `Util_Logx_Result()`

**Why it's thought to be a bug**

The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and returns the lognormal density f(x_b), not a lognormal variate. The values are mostly not lognormally distributed, and `x_b = 0` gives division by zero / log(0). Used as a recruitment multiplier at atecology/atannualbiology.c:1121 and 1206. Function: `Util_Logx_Result()`.

**Current code** (`atlantisUtil/atUtil.c` lines 1048–1063)

```c
 1048: 	step_a = 1.0 / (x_b * sigma * sqrt(2.0 * 3.141592654));
 1049: 	step_b = (log(x_b) - mu) * (log(x_b) - mu);
 1050: 	step_c = 2.0 * sigma * sigma;
 1051: 	result = step_a * exp(-step_b / step_c);
 1052: 
 1053: 	return (result);
 1054: 
 1055: }
 1056: 
 1057: /**
 1058:  *	\brief lognormal distribution
 1059:  *
 1060:  */
 1061: double Util_Lognorm_Distrib(double mu, double sigma, double x_b) {
 1062: 	double result, step_a, step_b, step_c;
 1063: 
```

**Suggested fix (from review):** Return `exp(mu + sigma * Util_Normx_Result(0,1))` (confirm intent with author).

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

## UTIL-27

**wrong index (copy-paste)** · Conf M / Sev L  
**Lines (current code):** 861  
**Lines at review (Oct 2026):** 809

**Function:** `Util_Get_Accumulative_Change_Scale()`

**Why it's thought to be a bug**

Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[changeIndex][start_id]`. This was copied from `Util_Get_Change_Scale`, which has no loop. Function: `Util_Get_Accumulative_Change_Scale()`.

**Current code** (`atlantisUtil/atUtil.c` lines 859–863)

```c
  859: 			if (bm->dayt >= end_date) {
  860:                 /* If the increase has finished then simply apply */
  861: 				scale = scale * changeArray[changeIndex][mult_id];
  862: 			} else {
  863:                 /* Else apply partial outcome */
```

**Suggested fix (from review):** Use `changeArray[changeIndex][start_id]`, if the per-entry test was intended.

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

## UTIL-28

**wrong parameter key string** · Conf H / Sev L  
**Lines (current code):** 359  
**Lines at review (Oct 2026):** 358

**Function:** `Util_Setup_Species_Param_Strings()`

**Why it's thought to be a bug**

`paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atUtilXML.c:219). It's the same length, so ConvertAtlantis prefix stripping still works, but error messages (atCreateXML.c:326) name the wrong parameter. Similarly, `"_threshdepth_id"` and `"_depmum_scalar_id"` (471-472) have a stray `_id` compared with the real `_threshdepth` / `_depmum_scalar` keys. Function: `Util_Setup_Species_Param_Strings()`.

**Current code** (`atlantisUtil/atUtil.c` lines 357–361)

```c
  357:     snprintf(paramStrings[SNcost_id], paramlen, "%s", "SNcost_");
  358:     snprintf(paramStrings[RNcost_id], paramlen, "%s", "RNcost_");
  359:     snprintf(paramStrings[RSstarve_id], paramlen, "%s", "RNstarve_");
  360:     snprintf(paramStrings[shrinkStarve_id], paramlen, "%s", "shrinkStarve_");
  361:     snprintf(paramStrings[shrinkProp_id], paramlen, "%s", "shrinkProp_");
```

**Suggested fix (from review):** Use `"RSstarve_"`, `"_threshdepth"`, `"_depmum_scalar"`.

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

## UTIL-29

**NULL dereference / wrong message** · Conf M / Sev L  
**Lines (current code):** 1237-1238, 1254-1255, 1249  
**Lines at review (Oct 2026):** 1185-1186, 1202-1203, 1197

**Function:** `Util_Check_NetCDF_Size()`

**Why it's thought to be a bug**

`*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileName` rather than the `fname` actually checked. Function: `Util_Check_NetCDF_Size()`.

**Current code** (`atlantisUtil/atUtil.c` lines 1235–1257)

```c
 1235: 	}else{
 1236: 		snprintf(fname, sizeof(fname), "%s%s", bm->destFolder, fileName);
 1237: 	}
 1238: 
 1239: 
 1240: 	/* Might need to close the file and then open it after we have checked its size */
 1241: 	/* Check the size of the diff file. */
 1242: 	if(stat(fname, &buffer) < 0){
 1243: 		quit("Unable to get information about file %s\n", fileName);
 1244: 	}
 1245: 
 1246: 	if (buffer.st_size > MAX_NETCDF_FILE_SIZE) {
 1247: 		strcpy(tempStr, fileName);
 1248: 		pdest = strstr(tempStr, endname);
 1249: 		*pdest = '\0';
 1250: 
 1251: 		(*index)++;
 1252: 		snprintf(fname, sizeof(fname), "%s%d.nc", tempStr, *index);
 1253: 		ncclose(fid);
 1254: 
 1255: 		/* Create file anew */
 1256: 		if(type == -1){
 1257: 			newFid = createBMSummaryDataFile(bm->destFolder, fname, bm);
```

**Suggested fix (from review):** Check strstr for NULL; print `fname`.

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

## UTIL-30

**log(0)** · Conf L / Sev L  
**Lines (current code):** 1039, 1177  
**Lines at review (Oct 2026):** 987, 1125

**Why it's thought to be a bug**

`log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Result()`, `Util_xnorm()`.

**Current code** (`atlantisUtil/atUtil.c` lines 1037–1041) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1037: 
 1038: }
 1039: 
 1040: /**
 1041:  *	\brief random number from lognormal distribution
```

**Suggested fix (from review):** Clamp the draw away from 0.

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

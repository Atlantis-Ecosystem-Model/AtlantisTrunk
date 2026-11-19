# atlantisUtil/atUtil.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-25](#util-25) | 891, 887 | H/M | unit/reference mismatch (relative vs absolute day) | Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarves… |  | [ ] |
| [UTIL-26](#util-26) | 1028 | M/M | wrong algorithm | The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and return… |  | [ ] |
| [UTIL-27](#util-27) | 855 | M/L | wrong index (copy-paste) | Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[ch… |  | [ ] |
| [UTIL-28](#util-28) | 359, 465, 143 | H/L | wrong parameter key string | `paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atU… |  | [ ] |
| [UTIL-29](#util-29) | 1231 | M/L | NULL dereference / wrong message | `*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileN… |  | [ ] |
| [UTIL-30](#util-30) | 1033, 1171 | L/L | log(0) | `log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Res… |  | [ ] |

---

## UTIL-25

**unit/reference mismatch (relative vs absolute day)** · Conf H / Sev M  
**Found at (current code):** `atUtil.c` 891, `atUtil.c` 887
**Lines at review (Oct 2026):** 841-845

**Function:** `Util_Copy_Change_Values()`

**Why it's thought to be a bug**

Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarvest.c:1933. But the insertion point is chosen with `originalArray[i][start_id] > start`, using the relative `start`. Once `dayt > 0`, the new change goes in the wrong place, and `Util_Get_Change_Scale` (which assumes ascending start dates) picks the wrong active change. Callers: atHarvest.c:1943/1984, atbiolUtil.c:149. Function: `Util_Copy_Change_Values()`.

**Code at issue** — `atUtil.c` line 891  
Search for: `newArray[`

```c
  889: 					newArray[arrayIndex][start_id] = start + bm->dayt;
  890: 					newArray[arrayIndex][period_id] = period;
  891→ 					newArray[arrayIndex][mult_id] = mult;
  892: 					arrayIndex++;
  893: 					beenInserted = TRUE;
```

**Code at issue** — `atUtil.c` line 887  
Search for: `originalArray[i][start_id] > start`

```c
  885: 		for (i = 0; i < size - 1; i++) {
  886: 			if (beenInserted == FALSE) {
  887→ 				if (originalArray[i][start_id] > start) {
  888: 					/* insert the new values into the array here */
  889: 					newArray[arrayIndex][start_id] = start + bm->dayt;
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
**Found at (current code):** `atUtil.c` 1028
**Lines at review (Oct 2026):** 998-1009

**Function:** `Util_Logx_Result()`

**Why it's thought to be a bug**

The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and returns the lognormal density f(x_b), not a lognormal variate. The values are mostly not lognormally distributed, and `x_b = 0` gives division by zero / log(0). Used as a recruitment multiplier at atecology/atannualbiology.c:1121 and 1206. Function: `Util_Logx_Result()`.

**Code at issue** — `atUtil.c` line 1028 (located by name only; check this is the right place)  
Search for: `double Util_Normx_Result(double mu, double sigma) {`

```c
 1026:  *
 1027:  */
 1028→ double Util_Normx_Result(double mu, double sigma) {
 1029:     double result, step_a, step_b, step_c;
 1030: 
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
**Found at (current code):** `atUtil.c` 855
**Lines at review (Oct 2026):** 809

**Function:** `Util_Get_Accumulative_Change_Scale()`

**Why it's thought to be a bug**

Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[changeIndex][start_id]`. This was copied from `Util_Get_Change_Scale`, which has no loop. Function: `Util_Get_Accumulative_Change_Scale()`.

**Code at issue** — `atUtil.c` line 855  
Search for: `changeArray[0][start_id] != 0`

```c
  853: 	for (changeIndex = 0; changeIndex < num_changes; changeIndex++) {
  854: 
  855→ 		if ((bm->dayt >= changeArray[changeIndex][start_id]) && (changeArray[0][start_id] != 0)) {
  856: 
  857: 			end_date = changeArray[changeIndex][start_id] + changeArray[changeIndex][period_id];
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
**Found at (current code):** `atUtil.c` 359, `atUtil.c` 465, `atUtil.c` 143
**Lines at review (Oct 2026):** 358

**Function:** `Util_Setup_Species_Param_Strings()`

**Why it's thought to be a bug**

`paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atUtilXML.c:219). It's the same length, so ConvertAtlantis prefix stripping still works, but error messages (atCreateXML.c:326) name the wrong parameter. Similarly, `"_threshdepth_id"` and `"_depmum_scalar_id"` (471-472) have a stray `_id` compared with the real `_threshdepth` / `_depmum_scalar` keys. Function: `Util_Setup_Species_Param_Strings()`.

**Code at issue** — `atUtil.c` line 359  
Search for: `paramStrings[RSstarve_id]`

```c
  357:     snprintf(paramStrings[SNcost_id], paramlen, "%s", "SNcost_");
  358:     snprintf(paramStrings[RNcost_id], paramlen, "%s", "RNcost_");
  359→     snprintf(paramStrings[RSstarve_id], paramlen, "%s", "RNstarve_");
  360:     snprintf(paramStrings[shrinkStarve_id], paramlen, "%s", "shrinkStarve_");
  361:     snprintf(paramStrings[shrinkProp_id], paramlen, "%s", "shrinkProp_");
```

**Code at issue** — `atUtil.c` line 465  
Search for: `"_threshdepth_id"`

```c
  463:     snprintf(paramStrings[prop_zooxanth_id], paramlen, "%s", "_prop_zooxanth");
  464: 	snprintf(paramStrings[DHW_thresh_id], paramlen, "%s", "_DHW_thresh");
  465→     snprintf(paramStrings[threshdepth_id], paramlen, "%s", "_threshdepth_id");
  466:     snprintf(paramStrings[depmum_scalar_id], paramlen, "%s", "_depmum_scalar_id");
  467: 
```

**Code at issue** — `atUtil.c` line 143  
Search for: `Util_Setup_Species_Param_Strings()`

```c
  141: 
  142:  05-05-2010 Bec Gorton
  143→  Added code to Util_Setup_Species_Param_Strings() that will set up the paramString values
  144:  for the new Q10 algorithm.
  145: 
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
**Found at (current code):** `atUtil.c` 1231
**Lines at review (Oct 2026):** 1185-1186, 1202-1203, 1197

**Function:** `Util_Check_NetCDF_Size()`

**Why it's thought to be a bug**

`*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileName` rather than the `fname` actually checked. Function: `Util_Check_NetCDF_Size()`.

**Code at issue** — `atUtil.c` line 1231 (located by name only; check this is the right place)  
Search for: `pdest = strstr(tempStr, endname);`

```c
 1229: 	if((*index) > 0){
 1230: 		strcpy(tempStr, fileName);
 1231→ 		pdest = strstr(tempStr, endname);
 1232: 		*pdest = '\0';
 1233: 
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
**Found at (current code):** `atUtil.c` 1033, `atUtil.c` 1171
**Lines at review (Oct 2026):** 987, 1125

**Why it's thought to be a bug**

`log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Result()`, `Util_xnorm()`.

**Code at issue** — `atUtil.c` line 1033  
Search for: `log(step_a)`

```c
 1031:     step_a = drandom(0.0, 1.0);
 1032:     step_b = drandom(0.0, 1.0);
 1033→     step_c = sqrt(-2.0 * log(step_a)) * cos(2.0 * 3.1415926 * step_b);
 1034:     result = step_c * sigma + mu;
 1035: 
```

**Code at issue** — `atUtil.c` line 1171  
Search for: `log(z2)`

```c
 1169: 	z2 = ran3(iiseed);
 1170: 
 1171→ 	xn = sin(6.283185*z1) * sqrt(-2.0*log(z2)) * sigg + mean;
 1172: 
 1173: 	return xn;
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

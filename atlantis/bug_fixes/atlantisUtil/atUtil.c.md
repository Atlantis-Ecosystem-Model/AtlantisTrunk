# atlantisUtil/atUtil.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTIL-25](#util-25) | 841-845 | H/M | unit/reference mismatch (relative vs absolute day) | Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarves… |  | [ ] |
| [UTIL-26](#util-26) | 998-1009 | M/M | wrong algorithm | The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and return… |  | [ ] |
| [UTIL-27](#util-27) | 809 | M/L | wrong index (copy-paste) | Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[ch… |  | [ ] |
| [UTIL-28](#util-28) | 358 | H/L | wrong parameter key string | `paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atU… |  | [ ] |
| [UTIL-29](#util-29) | 1185-1186, 1202-1203, 1197 | M/L | NULL dereference / wrong message | `*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileN… |  | [ ] |
| [UTIL-30](#util-30) | 987, 1125 | L/L | log(0) | `log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Res… |  | [ ] |

---

## UTIL-25

**unit/reference mismatch (relative vs absolute day)** · Conf H / Sev M  
**Listed lines:** 841-845

**Function:** `Util_Copy_Change_Values()`

**Why it's thought to be a bug**

Existing entries store absolute days: `newArray[...][start_id] = start + bm->dayt` here and atharvest/atHarvest.c:1933. But the insertion point is chosen with `originalArray[i][start_id] > start`, using the relative `start`. Once `dayt > 0`, the new change goes in the wrong place, and `Util_Get_Change_Scale` (which assumes ascending start dates) picks the wrong active change. Callers: atHarvest.c:1943/1984, atbiolUtil.c:149. Function: `Util_Copy_Change_Values()`.

**Current code** (`atlantisUtil/atUtil.c` lines 875–883) — code appears to have moved +36 lines since the review

```c
  875: }
  876: 
  877: void Util_Copy_Change_Values(MSEBoxModel *bm, double **originalArray, double **newArray, int size, double start, double period, double mult) {
  878: 
  879: 	int arrayIndex, i;
  880: 	int beenInserted = FALSE;
  881: 
  882: 	/* Now copy the values across inserting the new value in the correct place.*/
  883: 	arrayIndex = 0;
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
**Listed lines:** 998-1009

**Function:** `Util_Logx_Result()`

**Why it's thought to be a bug**

The comment says "random number from lognormal distribution", but the function draws `x_b ~ U(0,1)` and returns the lognormal density f(x_b), not a lognormal variate. The values are mostly not lognormally distributed, and `x_b = 0` gives division by zero / log(0). Used as a recruitment multiplier at atecology/atannualbiology.c:1116 and 1206. Function: `Util_Logx_Result()`.

**Current code** (`atlantisUtil/atUtil.c` lines 1042–1057) — code appears to have moved +46 lines since the review

```c
 1042:  *
 1043:  */
 1044: double Util_Logx_Result(double mu, double sigma) {
 1045: 	double x_b, result, step_a, step_b, step_c;
 1046: 
 1047: 	x_b = drandom(0.0, 1.0);
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
**Listed lines:** 809

**Function:** `Util_Get_Accumulative_Change_Scale()`

**Why it's thought to be a bug**

Inside the loop over `changeIndex` the guard tests `changeArray[0][start_id] != 0` rather than `changeArray[changeIndex][start_id]`. This was copied from `Util_Get_Change_Scale`, which has no loop. Function: `Util_Get_Accumulative_Change_Scale()`.

**Current code** (`atlantisUtil/atUtil.c` lines 846–850) — code appears to have moved +39 lines since the review

```c
  846:  *
  847:  */
  848: double Util_Get_Accumulative_Change_Scale(MSEBoxModel *bm, int num_changes, double **changeArray) {
  849: 
  850: 	int changeIndex;
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
**Listed lines:** 358

**Function:** `Util_Setup_Species_Param_Strings()`

**Why it's thought to be a bug**

`paramStrings[RSstarve_id]` is set to `"RNstarve_"`, but the parameter is `RSstarve_` (regex `^RSstarve_`, atUtilXML.c:215). It's the same length, so ConvertAtlantis prefix stripping still works, but error messages (atCreateXML.c:326) name the wrong parameter. Similarly, `"_threshdepth_id"` and `"_depmum_scalar_id"` (462-463) have a stray `_id` compared with the real `_threshdepth` / `_depmum_scalar` keys. Function: `Util_Setup_Species_Param_Strings()`.

**Current code** (`atlantisUtil/atUtil.c` lines 356–360)

```c
  356:     snprintf(paramStrings[RSprop_id], paramlen, "%s", "RSprop_");
  357:     snprintf(paramStrings[SNcost_id], paramlen, "%s", "SNcost_");
  358:     snprintf(paramStrings[RNcost_id], paramlen, "%s", "RNcost_");
  359:     snprintf(paramStrings[RSstarve_id], paramlen, "%s", "RNstarve_");
  360:     snprintf(paramStrings[shrinkStarve_id], paramlen, "%s", "shrinkStarve_");
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
**Listed lines:** 1185-1186, 1202-1203, 1197

**Function:** `Util_Check_NetCDF_Size()`

**Why it's thought to be a bug**

`*strstr(tempStr, ".nc") = '\0'` crashes if the file name has no ".nc". The stat-failure message prints `fileName` rather than the `fname` actually checked. Function: `Util_Check_NetCDF_Size()`.

**Current code** (`atlantisUtil/atUtil.c` lines 1218–1240) — code appears to have moved +35 lines since the review

```c
 1218: int Util_Check_NetCDF_Size(MSEBoxModel *bm, int fid, int *dump, char *fileName, int *index, int type){
 1219: 	struct stat buffer;
 1220: 	char fname[STRLEN*2];
 1221: 	char endname[] = ".nc";
 1222: 	char tempStr[STRLEN];
 1223: 	char *pdest;
 1224: 	int newFid;
 1225: 
 1226: 	if(fid < 0)
 1227: 		return -1;
 1228: 
 1229: 	if((*index) > 0){
 1230: 		strcpy(tempStr, fileName);
 1231: 		pdest = strstr(tempStr, endname);
 1232: 		*pdest = '\0';
 1233: 
 1234: 		snprintf(fname, sizeof(fname), "%s%s%d.nc", bm->destFolder, tempStr, *index);
 1235: 	}else{
 1236: 		snprintf(fname, sizeof(fname), "%s%s", bm->destFolder, fileName);
 1237: 	}
 1238: 
 1239: 
 1240: 	/* Might need to close the file and then open it after we have checked its size */
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
**Listed lines:** 987, 1125

**Why it's thought to be a bug**

`log(step_a)` / `log(z2)` with a uniform draw that can be exactly 0 gives -inf/NaN. Functions: `Util_Normx_Result()`, `Util_xnorm()`.

**Current code** (`atlantisUtil/atUtil.c` lines 1026–1030) — code appears to have moved +41 lines since the review

```c
 1026:  *
 1027:  */
 1028: double Util_Normx_Result(double mu, double sigma) {
 1029:     double result, step_a, step_b, step_c;
 1030: 
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

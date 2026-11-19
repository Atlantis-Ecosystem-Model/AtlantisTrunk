# ateconomic/ateconomicsetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECON-01](#econ-01) ⭐ | 350, 361, 104 | H/H | wrong variable / out-of-bounds | `bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;` sits after the `sp` lo… |  | [ ] |
| [ECON-02](#econ-02) | 974, 789 | H/L | swapped dims (alloc vs use) | `bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0)` gives `[2][12][K_num_fisheries]`, b… |  | [ ] |
| [ECON-03](#econ-03) | 1271, 1274, 1307 | M/M | buffer overrun (input-driven) | `columnIndexValues` is `[numExpectedTokens][2]` (1232), but `count++` writes `columnIndexValues[count]` in the… |  | [ ] |
| [ECON-04](#econ-04) | 1043, 1033 | M/L | copy-paste / truncation | `EconIndicatorInputNames[down_time_id]` is set twice ("DownTime" then "minDownTime"). "triplength(days)" (16 c… |  | [ ] |
| [ECON-05](#econ-05) | 1143 | H/L | buffer truncation | "AnnualExpectedProfitInd" (23 chars) goes into a 20-byte name buffer (atlantismain.c:2101), so the output head… |  | [ ] |
| [ECON-06](#econ-06) | 137, 104 | M/L | NULL deref | `*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the econ file name has no ".prm". Function: `Econ… |  | [ ] |

---

## ECON-01

**wrong variable / out-of-bounds** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `ateconomicsetup.c` 350, `ateconomicsetup.c` 361, `ateconomicsetup.c` 104
**Lines at review (Oct 2026):** 349-351

**Function:** `Economic_Init()`

**Why it's thought to be a bug**

`bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;` sits after the `sp` loop, so `mth` is whatever the earlier `for (mth=0; mth<12; ...)` loops left: 12 (one past the end of the 12-month dimension, allocated at ateconomicsetup.c:968/970 as `[fish][sub][12][nbox]`). If no species is fished, `mth` is uninitialised on the first pass. Starting discards for months 0-11 are never set. Function: `Economic_Init()`.

**Code at issue** — `ateconomicsetup.c` line 350  
Search for: `SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC`

```c
  348:             }
  349:             for(b=0; b<bm->nbox; b++){
  350→                 bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;
  351:             }
  352: 
```

**Code at issue** — `ateconomicsetup.c` line 361  
Search for: `for (mth = 0; mth < 12`

```c
  359: 			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  360: 				for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
  361→ 					for (mth = 0; mth < 12; mth++) {
  362: 						bm->SpatialCPUE[nf][ns][mth][b] /= 2.2;
  363: 						bm->SpatialDisPUE[nf][ns][mth][b] /= 2.2;
```

**Code at issue** — `ateconomicsetup.c` line 104  
Search for: `Economic_Init()`

```c
  102:  * Initialisation.
  103:  *
  104→  * The routine Economic_Init() is called once at the beginning of
  105:  * the run. It carries out the following steps.
  106:  *
```

**Suggested fix (from review):** Wrap it in `for (mth = 0; mth < 12; mth++) { for (b...) ... }`

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

## ECON-02

**swapped dims (alloc vs use)** · Conf H / Sev L  
**Found at (current code):** `ateconomicsetup.c` 974, `ateconomicsetup.c` 789
**Lines at review (Oct 2026):** 789-790, 974

**Function:** `Pre_Load_Array_Alloc()`

**Why it's thought to be a bug**

`bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0)` gives `[2][12][K_num_fisheries]`, but it is indexed `EffortTrades[nf][i][source_id]` (fishery first). With more than 2 fisheries (or only 1) that writes past the end. Latent: the only use is in `InitialiseEconArrays()`, which nothing calls (grep finds no caller). Function: `Pre_Load_Array_Alloc()` / `InitialiseEconArrays()`.

**Code at issue** — `ateconomicsetup.c` line 974  
Search for: `EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0)`

```c
  972: 	bm->SUBFLEET_ECONprms = Util_Alloc_Init_3D_Double(K_sub_fleet_params, bm->K_max_num_subfleet, bm->K_num_fisheries, 0.0);
  973: 	bm->Trades = Util_Alloc_Init_2D_Int(bm->K_num_tot_sp, 12, 0);
  974→ 	bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0);
  975: 	friendship_netwk = Util_Alloc_Init_4D_Double(bm->K_max_num_subfleet, bm->K_max_num_subfleet, bm->K_num_fisheries, bm->K_num_fisheries, 1.0);
  976: 	prop_tow_time = Util_Alloc_Init_3D_Double(bm->nbox, bm->K_max_num_subfleet, bm->K_num_fisheries, 1.0);
```

**Code at issue** — `ateconomicsetup.c` line 789  
Search for: `EffortTrades[nf][i][source_id]`

```c
  787: 					bm->SpatialCPUE[nf][ns][i][b] = 0;
  788: 
  789→ 					bm->EffortTrades[nf][i][source_id] = 0;
  790: 					bm->EffortTrades[nf][i][sink_id] = 0;
  791: 				}
```

**Suggested fix (from review):** Allocate `Util_Alloc_Init_3D_Int(2, 12, bm->K_num_fisheries, 0)`

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

## ECON-03

**buffer overrun (input-driven)** · Conf M / Sev M  
**Found at (current code):** `ateconomicsetup.c` 1271, `ateconomicsetup.c` 1274, `ateconomicsetup.c` 1307
**Lines at review (Oct 2026):** 1231-1316

**Function:** `Read_FriendShip_Network()`

**Why it's thought to be a bug**

`columnIndexValues` is `[numExpectedTokens][2]` (1232), but `count++` writes `columnIndexValues[count]` in the header loop before the `count != numExpectedTokens` check at 1274. Each data row likewise reads `columnIndexValues[rowIndex]` before the 1315 check. A CSV with extra columns therefore overruns the heap. Also, only the last character is parsed as the subfleet index (1259/1288), so subfleet 10 or higher is misread, and `index` is never bounds-checked before `friendship_netwk[..][..][index][..]` (1310). Function: `Read_FriendShip_Network()`.

**Code at issue** — `ateconomicsetup.c` line 1271  
Search for: `count++`

```c
 1269: 		columnIndexValues[count][0] = fisheryIndex;
 1270: 		columnIndexValues[count][1] = index;
 1271→ 		count++;
 1272: 	}
 1273: 
```

**Code at issue** — `ateconomicsetup.c` line 1274  
Search for: `count != numExpectedTokens`

```c
 1272: 	}
 1273: 
 1274→ 	if(count != numExpectedTokens){
 1275: 		quit("ERROR: The friendship network input file %s does not have the correct number of columns %d it has %d columns\n\n", filePath, numExpectedTokens, count);
 1276: 	}
```

**Code at issue** — `ateconomicsetup.c` line 1307  
Search for: `columnIndexValues[rowIndex]`

```c
 1305: 			if(value <= 1.0){
 1306: 				warn("Read_FriendShip_Network. You have tried to set the friendship index between %s:%d and %s:%d to less than the min value 1.0. It will default back to 1.0\n",
 1307→ 						FisheryArray[fisheryIndex].fisheryCode, index, FisheryArray[columnIndexValues[rowIndex][0]].fisheryCode, columnIndexValues[rowIndex][1]);
 1308: 				value = 1.0;
 1309: 			}
```

**Suggested fix (from review):** Check `count < numExpectedTokens` / `rowIndex < numExpectedTokens` inside the loops and bound-check `index < K_max_num_subfleet`

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

## ECON-04

**copy-paste / truncation** · Conf M / Sev L  
**Found at (current code):** `ateconomicsetup.c` 1043, `ateconomicsetup.c` 1033
**Lines at review (Oct 2026):** 1043, 1067, 1041

**Function:** `Init_Econ_Indicator_Names()`

**Why it's thought to be a bug**

`EconIndicatorInputNames[down_time_id]` is set twice ("DownTime" then "minDownTime"). "triplength(days)" (16 chars) is cut off by `paramLen = 15`. The array is never read (only freed), so this is cosmetic. Function: `Init_Econ_Indicator_Names()`.

**Code at issue** — `ateconomicsetup.c` line 1043  
Search for: `EconIndicatorInputNames[down_time_id]`

```c
 1041: 	snprintf(EconIndicatorInputNames[max_trip_length_id], paramLen, "%s", "triplength(days)");
 1042: 	snprintf(EconIndicatorInputNames[FishableLength_id], paramLen, "%s", "FishableLength");
 1043→ 	snprintf(EconIndicatorInputNames[down_time_id], paramLen, "%s", "DownTime");
 1044: 	snprintf(EconIndicatorInputNames[choicebuffer_id], paramLen, "%s", "choicebuffer");
 1045: 	snprintf(EconIndicatorInputNames[effortDiscount_id], paramLen, "%s", "EffortDiscount");
```

**Code at issue** — `ateconomicsetup.c` line 1033  
Search for: `paramLen = 15`

```c
 1031: 
 1032: void Init_Econ_Indicator_Names(MSEBoxModel *bm) {
 1033→     int paramLen = 15;
 1034: 	EconIndicatorInputNames = (char **) c_alloc2d(paramLen, K_sub_fleet_params);
 1035: 
```

**Suggested fix (from review):** Remove the duplicate and enlarge paramLen

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

## ECON-05

**buffer truncation** · Conf H / Sev L  
**Found at (current code):** `ateconomicsetup.c` 1143
**Lines at review (Oct 2026):** 1143

**Function:** `Init_Econ_Index_Names()`

**Why it's thought to be a bug**

"AnnualExpectedProfitInd" (23 chars) goes into a 20-byte name buffer (atlantismain.c:2248), so the output header is truncated. Function: `Init_Econ_Index_Names()`.

**Code at issue** — `ateconomicsetup.c` line 1143 (located by name only; check this is the right place)  
Search for: `snprintf(bm->SUBFLEETeconindxNAME[AnnualExpectPI_id], paramLen, "%s", "AnnualExpectedProfi`

```c
 1141: 	snprintf(bm->SUBFLEETeconindxNAME[tax_ind_id], paramLen, "%s", "TaxInd");
 1142: 	snprintf(bm->SUBFLEETeconindxNAME[dv_ind_id], paramLen, "%s", "DVInd");
 1143→ 	snprintf(bm->SUBFLEETeconindxNAME[AnnualExpectPI_id], paramLen, "%s", "AnnualExpectedProfitInd");
 1144: 	snprintf(bm->SUBFLEETeconindxNAME[rev_land_ind_id], paramLen, "%s", "RevLandInd");
 1145: 	snprintf(bm->SUBFLEETeconindxNAME[rev_effort_ind_id], paramLen, "%s", "RevEffortInd");
```

**Suggested fix (from review):** Shorten the label or widen the buffer

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

## ECON-06

**NULL deref** · Conf M / Sev L  
**Found at (current code):** `ateconomicsetup.c` 137, `ateconomicsetup.c` 104
**Lines at review (Oct 2026):** 137

**Function:** `Economic_Init()`

**Why it's thought to be a bug**

`*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the econ file name has no ".prm". Function: `Economic_Init()`.

**Code at issue** — `ateconomicsetup.c` line 137  
Search for: `*(strstr(convertedXMLFileName, ".prm")) = '\0'`

```c
  135: 	/* Build the converted filename */
  136: 	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->econprmIfname);
  137→ 	*(strstr(convertedXMLFileName, ".prm")) = '\0';
  138: 	strcat(convertedXMLFileName, ".xml");
  139: 
```

**Code at issue** — `ateconomicsetup.c` line 104  
Search for: `Economic_Init()`

```c
  102:  * Initialisation.
  103:  *
  104→  * The routine Economic_Init() is called once at the beginning of
  105:  * the run. It carries out the following steps.
  106:  *
```

**Suggested fix (from review):** Check the strstr result and quit with a message

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

ateconomicsetup.c:225: when `!UseEconQuotaDefine`, `TACamt[sp][nf][now_id]` is added to owned quota even if it holds the `no_quota` sentinel (1e12), which would inflate owned quota.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

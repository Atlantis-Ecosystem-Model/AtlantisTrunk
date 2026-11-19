# ateconomic/ateconomicsetup.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECON-01](#econ-01) ⭐ | 349-351 | H/H | wrong variable / out-of-bounds | `bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;` sits after the `sp` lo… |  | [ ] |
| [ECON-02](#econ-02) | 789-790, 974 | H/L | swapped dims (alloc vs use) | `bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0)` gives `[2][12][K_num_fisheries]`, b… |  | [ ] |
| [ECON-03](#econ-03) | 1231-1316 | M/M | buffer overrun (input-driven) | `columnIndexValues` is `[numExpectedTokens][2]` (1232), but `count++` writes `columnIndexValues[count]` in the… |  | [ ] |
| [ECON-04](#econ-04) | 1043, 1067, 1041 | M/L | copy-paste / truncation | `EconIndicatorInputNames[down_time_id]` is set twice ("DownTime" then "minDownTime"). "triplength(days)" (16 c… |  | [ ] |
| [ECON-05](#econ-05) | 1143 | H/L | buffer truncation | "AnnualExpectedProfitInd" (23 chars) goes into a 20-byte name buffer (atlantismain.c:2101), so the output head… |  | [ ] |
| [ECON-06](#econ-06) | 137 | M/L | NULL deref | `*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the econ file name has no ".prm". Function: `Econ… |  | [ ] |

---

## ECON-01

**wrong variable / out-of-bounds** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 349-351  
**Lines at review (Oct 2026):** 349-351

**Function:** `Economic_Init()`

**Why it's thought to be a bug**

`bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;` sits after the `sp` loop, so `mth` is whatever the earlier `for (mth=0; mth<12; ...)` loops left: 12 (one past the end of the 12-month dimension, allocated at ateconomicsetup.c:968/970 as `[fish][sub][12][nbox]`). If no species is fished, `mth` is uninitialised on the first pass. Starting discards for months 0-11 are never set. Function: `Economic_Init()`.

**Current code** (`ateconomic/ateconomicsetup.c` lines 347–353)

```c
  347:                 avg_loadDetFC = 0.0;
  348:             }
  349:             for(b=0; b<bm->nbox; b++){
  350:                 bm->SpatialDisPUE[nf][ns][mth][b] += bm->SpatialCPUE[nf][ns][mth][b] * avg_loadDetFC;
  351:             }
  352: 
  353: 		}
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
**Lines (current code):** 789-790, 974  
**Lines at review (Oct 2026):** 789-790, 974

**Function:** `Pre_Load_Array_Alloc()`

**Why it's thought to be a bug**

`bm->EffortTrades = Util_Alloc_Init_3D_Int(bm->K_num_fisheries, 12, 2, 0)` gives `[2][12][K_num_fisheries]`, but it is indexed `EffortTrades[nf][i][source_id]` (fishery first). With more than 2 fisheries (or only 1) that writes past the end. Latent: the only use is in `InitialiseEconArrays()`, which nothing calls (grep finds no caller). Function: `Pre_Load_Array_Alloc()` / `InitialiseEconArrays()`.

**Current code** (`ateconomic/ateconomicsetup.c` lines 787–792)

```c
  787: 					bm->SpatialCPUE[nf][ns][i][b] = 0;
  788: 
  789: 					bm->EffortTrades[nf][i][source_id] = 0;
  790: 					bm->EffortTrades[nf][i][sink_id] = 0;
  791: 				}
  792: 			}
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
**Lines (current code):** 1231-1316  
**Lines at review (Oct 2026):** 1231-1316

**Function:** `Read_FriendShip_Network()`

**Why it's thought to be a bug**

`columnIndexValues` is `[numExpectedTokens][2]` (1232), but `count++` writes `columnIndexValues[count]` in the header loop before the `count != numExpectedTokens` check at 1274. Each data row likewise reads `columnIndexValues[rowIndex]` before the 1315 check. A CSV with extra columns therefore overruns the heap. Also, only the last character is parsed as the subfleet index (1259/1288), so subfleet 10 or higher is misread, and `index` is never bounds-checked before `friendship_netwk[..][..][index][..]` (1310). Function: `Read_FriendShip_Network()`.

**Current code** (`ateconomic/ateconomicsetup.c` lines 1229–1268 (truncated))

```c
 1229: 	double value;
 1230: 
 1231: 	int numExpectedTokens = (bm->K_num_fisheries * bm->K_max_num_subfleet);
 1232: 	int **columnIndexValues = Util_Alloc_Init_2D_Int(2, numExpectedTokens, 0);
 1233: 
 1234: 
 1235: 	if( (inputFP=Open_Input_File(bm->inputFolder, filePath, "r")) == NULL )
 1236: 		quit("Read_FriendShip_Network: Can't open %s%s\n",bm->inputFolder,filePath);
 1237: 
 1238: 	/* This is a wee bit hard coded for this file structure due to time but would be
 1239: 	* easy to generalise if required later.
 1240: 	*
 1241: 	* So we know this file structure is ( K_num_fisheries * K_num_subfleets) * ( K_num_fisheries * K_num_subfleets)
 1242: 	*/
 1243: 
 1244: 	fseek ( inputFP, 0L, 0 );
 1245: 
 1246: 	/* Get the first line */
 1247: 	fgets ( buf, buflen, inputFP );
 1248: 
 1249: 	/* First we need to check that the number of tokens is correct */
 1250: 	varStr = strtok ( buf, seps );
 1251: 	count = 0;
 1252: 	while((varStr = strtok ( NULL, seps )) != NULL){
 1253: 
 1254: 		/* get rid of a \n in case there is one */
 1255: 		if(strchr(varStr, '\n') != NULL)
 1256: 			*(strchr(varStr, '\n')) = '\0';
 1257: 		/* Parse the values */
 1258: 		/* grab the fleet index off the end */
 1259: 		indexStr = varStr + strlen(varStr) - 1;
 1260: 
 1261: 		index = atoi(indexStr);
 1262: 		*(indexStr) = '\0';
 1263: 
 1264: 		/* get the index of the fishery */
 1265: 		fisheryIndex = Util_Get_Fishery_Index(bm, varStr);
 1266: 		if(fisheryIndex == -1)
 1267: 			quit("ERROR: The friendship network input file %s contains an unrecognised fishery name %s\n\n", filePath, varStr);
 1268: 
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
**Lines (current code):** 1043, 1067, 1041  
**Lines at review (Oct 2026):** 1043, 1067, 1041

**Function:** `Init_Econ_Indicator_Names()`

**Why it's thought to be a bug**

`EconIndicatorInputNames[down_time_id]` is set twice ("DownTime" then "minDownTime"). "triplength(days)" (16 chars) is cut off by `paramLen = 15`. The array is never read (only freed), so this is cosmetic. Function: `Init_Econ_Indicator_Names()`.

**Current code** (`ateconomic/ateconomicsetup.c` lines 1041–1069)

```c
 1041: 	snprintf(EconIndicatorInputNames[max_trip_length_id], paramLen, "%s", "triplength(days)");
 1042: 	snprintf(EconIndicatorInputNames[FishableLength_id], paramLen, "%s", "FishableLength");
 1043: 	snprintf(EconIndicatorInputNames[down_time_id], paramLen, "%s", "DownTime");
 1044: 	snprintf(EconIndicatorInputNames[choicebuffer_id], paramLen, "%s", "choicebuffer");
 1045: 	snprintf(EconIndicatorInputNames[effortDiscount_id], paramLen, "%s", "EffortDiscount");
 1046: 	snprintf(EconIndicatorInputNames[bycatchDiscount_id], paramLen, "%s", "BycatchDiscount");
 1047: 	snprintf(EconIndicatorInputNames[var_cost_id], paramLen, "%s", "varcost");
 1048: 	snprintf(EconIndicatorInputNames[supp_cost_id], paramLen, "%s", "suppcost");
 1049: 	snprintf(EconIndicatorInputNames[switch_cost_id], paramLen, "%s", "switchcost");
 1050: 	snprintf(EconIndicatorInputNames[newboat_cost_id], paramLen, "%s", "newcost");
 1051: 	snprintf(EconIndicatorInputNames[tol_debt_id], paramLen, "%s", "TolDebt");
 1052: 	snprintf(EconIndicatorInputNames[fuel_cost_id], paramLen, "%s", "fuelcost");
 1053: 	snprintf(EconIndicatorInputNames[gear_cost_id], paramLen, "%s", "gearcost");
 1054: 	snprintf(EconIndicatorInputNames[unload_cost_id], paramLen, "%s", "unloadcost");
 1055: 	snprintf(EconIndicatorInputNames[fixed_cost_id], paramLen, "%s", "fixedcost");
 1056: 	snprintf(EconIndicatorInputNames[capital_cost_id], paramLen, "%s", "capitalcost");
 1057: 	snprintf(EconIndicatorInputNames[crewshare_id], paramLen, "%s", "crewshare");
 1058: 	snprintf(EconIndicatorInputNames[mgmt_cost_id], paramLen, "%s", "mgmtcost");
 1059: 	snprintf(EconIndicatorInputNames[flexweight_orig_id], paramLen, "%s", "flexweight");
 1060: 	snprintf(EconIndicatorInputNames[resale_id], paramLen, "%s", "resale");
 1061: 	snprintf(EconIndicatorInputNames[buyout_id], paramLen, "%s", "buyout");
 1062: 	snprintf(EconIndicatorInputNames[propbuyback_id], paramLen, "%s", "propbuyback");
 1063: 	snprintf(EconIndicatorInputNames[flag_econind_id], paramLen, "%s", "flageconind");
 1064: 	snprintf(EconIndicatorInputNames[flag_indtype_id], paramLen, "%s", "flagtypeind");
 1065: 	snprintf(EconIndicatorInputNames[betarev_id], paramLen, "%s", "betarev");
 1066: 	snprintf(EconIndicatorInputNames[minprob_id], paramLen, "%s", "minprob");
 1067: 	snprintf(EconIndicatorInputNames[down_time_id], paramLen, "%s", "minDownTime");
 1068: 	snprintf(EconIndicatorInputNames[hold_capacity_id], paramLen, "%s", "hold_capacity");
 1069: 
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
**Lines (current code):** 1143  
**Lines at review (Oct 2026):** 1143

**Function:** `Init_Econ_Index_Names()`

**Why it's thought to be a bug**

"AnnualExpectedProfitInd" (23 chars) goes into a 20-byte name buffer (atlantismain.c:2248), so the output header is truncated. Function: `Init_Econ_Index_Names()`.

**Current code** (`ateconomic/ateconomicsetup.c` lines 1141–1145)

```c
 1141: 	snprintf(bm->SUBFLEETeconindxNAME[tax_ind_id], paramLen, "%s", "TaxInd");
 1142: 	snprintf(bm->SUBFLEETeconindxNAME[dv_ind_id], paramLen, "%s", "DVInd");
 1143: 	snprintf(bm->SUBFLEETeconindxNAME[AnnualExpectPI_id], paramLen, "%s", "AnnualExpectedProfitInd");
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
**Lines (current code):** 137  
**Lines at review (Oct 2026):** 137

**Function:** `Economic_Init()`

**Why it's thought to be a bug**

`*(strstr(convertedXMLFileName, ".prm")) = '\0';` crashes if the econ file name has no ".prm". Function: `Economic_Init()`.

**Current code** (`ateconomic/ateconomicsetup.c` lines 135–139)

```c
  135: 	/* Build the converted filename */
  136: 	snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", bm->econprmIfname);
  137: 	*(strstr(convertedXMLFileName, ".prm")) = '\0';
  138: 	strcat(convertedXMLFileName, ".xml");
  139: 
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

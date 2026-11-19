# ateconomic/atquota.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **15 findings** (severity H: 3; confidence H and severity H: 2; on the priority list: 2).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [QUOT-01](#quot-01) ⭐ | 130-133 | H/H | buffer overrun | `&bm->tsResid[nmarket]` — residual series indexed by market, but atmanage/atManageSetup.c:676-679 states "only… |  | [ ] |
| [QUOT-02](#quot-02) | 576-578 | H/M | static state not reset / copy-paste | `price_coefft = 0; bind_coefft = 0;` — these are the loop-wide coefficient copies set once at 531/538. Once an… |  | [ ] |
| [QUOT-03](#quot-03) | 664 | H/L | printf format | `quit("No default (%d) flagLease defined...\n")` — `%d` with no argument (UB). Function: `Total_Quota_Price()`… |  | [ ] |
| [QUOT-04](#quot-04) ⭐ | 825-827, 871-872, 915-916 | H/H | NULL dereference | `linearPI[sp][linPI_id][ns] = 0;` — `linearPI` is defined `= 0` in include/atEconomicSetup.h:30 and is never a… |  | [ ] |
| [QUOT-05](#quot-05) | 870, 931, 968, 1098 | H/M | wrong index (offset vs absolute) | `idd = ns*tnf*nm + nm*nf + month - bm->MofY;` so decoding with `% nm` gives `month - MofY`, an offset rather t… |  | [ ] |
| [QUOT-06](#quot-06) | 920, 960, 962-965, 1055-1064 | H/M | off-by-one | `s = nsort;` then `linearPI[sp][idd_id][s]` — the sorted entries are 0..nsort-1. Index nsort holds a zeroed sl… |  | [ ] |
| [QUOT-07](#quot-07) | 1101 | H/M | stale variable | Step 2 demand test uses `max_month_effort`, which is still set for the previous buyer (from 1000/1130 or 933).… |  | [ ] |
| [QUOT-08](#quot-08) | 1064-1066, 1131 | H/M | uninitialised/stale variable | Step 2 decodes `sellerns/sellernf` but never `sellermonth`, so `Trade()` gets the stale value from step 1, or … |  | [ ] |
| [QUOT-09](#quot-09) | 1013, 1023 | H/M | division by zero | `scheduledEffort * ExpCatch / ExpEffort` — no `+ small_num`, unlike every other use of this expression (952, 9… |  | [ ] |
| [QUOT-10](#quot-10) | 866 | M/M | wrong index | Inside the `month` loop, `expectedCPUE` uses `BlackBook[..][bm->MofY]` and `EffortSchedule[..][bm->MofY][hist_… |  | [ ] |
| [QUOT-11](#quot-11) | 1356 | H/M | wrong variable | `Quicksort_Dir(ax, bx, cx, dx, ex, nsort, ascendflag);` — in the package path `nsort` is still 0 (set at 808/8… |  | [ ] |
| [QUOT-12](#quot-12) | 1327-1332 | M/H | buffer overrun | `ax[idp] = ...; idp++` runs once per (buyer subfleet, seller subfleet) pair, so idp can reach ~(nf*ns)^2. But … |  | [ ] |
| [QUOT-13](#quot-13) | 1166, 1171-1172, 1955 | H/M | wrong id constant (array aliasing) | `bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id]` — `within_id` (#define 15, a QuotaAlloc index, atlantisbo… |  | [ ] |
| [QUOT-14](#quot-14) | 1806-1807, 1651-1655 | M/M | missing cap | Bought quota: in the supply>demand branch there is no cap, so `owned_id -= demand` can push seller owned below… |  | [ ] |
| [QUOT-15](#quot-15) | 1449 vs 1271-1272 | L/M | inconsistent formula | spareend is `p+(1-p)*M/12` with threshold `(1-spareend)*Q` when matching, but `p*(1-M/12)` (algebraically) wit… |  | [ ] |

---

## QUOT-01

**buffer overrun** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 130-133  
**Lines at review (Oct 2026):** 130-133

**Function:** `Get_Fish_Prices()`

**Why it's thought to be a bug**

`&bm->tsResid[nmarket]` — residual series indexed by market, but atmanage/atManageSetup.c:676-679 states "only one time series allowed" and builds `tseconid` from `tsResid[0]` only. With `K_num_markets`=2 (header comment says two markets are used) and `ntsResid`=1, `tsResid[1]` is read past the end of the array. Function: `Get_Fish_Prices()`.

**Current code** (`ateconomic/atquota.c` lines 128–135)

```c
  128: 					auto_coefft = bm->MARKETprms[nmarket][sp][auto_coefft_id];
  129: 
  130: 					if(bm->ntsResid){
  131: 						/* Residual times series for use with autoregressive price model */
  132: 						FisheryTimeSeries *this_residual = &bm->tsResid[nmarket];
  133: 						resid = tsEval(&this_residual->ts, ts_id, bm->t);
  134: 					} else
  135: 						resid = 0.0;
```

**Suggested fix (from review):** Use `bm->tsResid[min(nmarket, bm->ntsResid-1)]` or require `ntsResid >= K_num_markets` at setup

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

## QUOT-02

**static state not reset / copy-paste** · Conf H / Sev M  
**Lines (current code):** 576-578  
**Lines at review (Oct 2026):** 576-578

**Function:** `Total_Quota_Price()`

**Why it's thought to be a bug**

`price_coefft = 0; bind_coefft = 0;` — these are the loop-wide coefficient copies set once at 531/538. Once any species has price < minValue (and UseMinValue off), price and bind terms are switched off for every later species, subfleet and fishery in that call. Function: `Total_Quota_Price()` (NZmodel).

**Current code** (`ateconomic/atquota.c` lines 574–580)

```c
  574: 							} else {
  575: 								/*Ignore price related terms if the fish has no landed value */
  576: 								price_coefft = 0;
  577: 								bind_coefft = 0;
  578: 								price = small_num;
  579: 							}
  580: 						}
```

**Suggested fix (from review):** Use per-species locals (e.g. `pc = bm->price_Coefft; bc = bm->bind_Coefft;` inside the sp loop)

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

## QUOT-03

**printf format** · Conf H / Sev L  
**Lines (current code):** 664  
**Lines at review (Oct 2026):** 664

**Function:** `Total_Quota_Price()`

**Why it's thought to be a bug**

`quit("No default (%d) flagLease defined...\n")` — `%d` with no argument (UB). Function: `Total_Quota_Price()`.

**Current code** (`ateconomic/atquota.c` lines 662–666)

```c
  662: 		break;
  663: 	default:
  664: 		quit("No default (%d) flagLease defined, please reset to an appropriate option\n");
  665: 		break;
  666: 	}
```

**Suggested fix (from review):** Add `, bm->flagLease`

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

## QUOT-04

**NULL dereference** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 825-827, 871-872, 915-916  
**Lines at review (Oct 2026):** 825-827, 871-872, 915-916

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

`linearPI[sp][linPI_id][ns] = 0;` — `linearPI` is defined `= 0` in include/atEconomicSetup.h:30 and is never allocated anywhere in the repo (grep finds only atquota.c, the commented-out ateffortquota.c, and the headers). Setting `sp_by_sp`=1 (a documented XML option) crashes straight away. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 823–829)

```c
  823: 
  824: 				/* Initialise linearPI */
  825: 				for (ns = 0; ns < totpi; ns++) {
  826: 					linearPI[sp][linPI_id][ns] = 0;
  827: 					linearPI[sp][idd_id][ns] = 0;
  828: 					ax[ns] = 0;
  829: 					bx[ns] = 0;
```

**Suggested fix (from review):** Allocate `linearPI` (e.g. `Util_Alloc_Init_3D_Double(totpi, 2, K_num_tot_sp, 0)`) in Post_Load_Array_Alloc and free it

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

## QUOT-05

**wrong index (offset vs absolute)** · Conf H / Sev M  
**Lines (current code):** 870, 931, 968, 1098  
**Lines at review (Oct 2026):** 870, 931, 968, 1098

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

`idd = ns*tnf*nm + nm*nf + month - bm->MofY;` so decoding with `% nm` gives `month - MofY`, an offset rather than a month. `buyermonth`/`sellermonth` are then used as absolute month indices into EffortSchedule/BlackBook/MargRent and passed to `Trade()`, so after January the wrong month is used. Function: `Quota_trade()` (sp_by_sp).

**Current code** (`ateconomic/atquota.c` lines 868–872)

```c
  868: 									pi[nf][ns][month][sp] -= priceQuota * (cumCatch + expectedCatch - ownQuota - leaseQuota);
  869: 
  870: 									idd = ns * tnf * nm + nm * nf + month - bm->MofY;
  871: 									linearPI[sp][linPI_id][idd] = bm->QuotaAlloc[nf][ns][sp][finalutility_id];
  872: 									linearPI[sp][idd_id][idd] = idd;
```

**Suggested fix (from review):** `buyermonth = ((int)... % nm) + bm->MofY;` (same for sellermonth)

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

## QUOT-06

**off-by-one** · Conf H / Sev M  
**Lines (current code):** 920, 960, 962-965, 1055-1064  
**Lines at review (Oct 2026):** 920, 960, 962-965, 1055-1064

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

`s = nsort;` then `linearPI[sp][idd_id][s]` — the sorted entries are 0..nsort-1. Index nsort holds a zeroed slot (decodes to fishery 0, subfleet 0, month 0), and when nsort==totpi it reads past the end. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 918–957 (truncated))

```c
  918: 
  919: 				b = 0;
  920: 				s = nsort;
  921: 				while ((s >= 0) && (b < nsort && b >= 0)) {
  922: 					b = 0;
  923: 					demand = 1;
  924: 					while ((demand > 0) && (b < nsort && b >= 0)) {
  925: 						demand = 0;
  926: 						/* Deconstructing ids from single sorted array - assuming ids in
  927: 						 the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
  928: 						step1 = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][b] / (nm + small_num))); /* First step in finding fishery_id and subfleet_id */
  929: 						buyerns = (int) floor(ROUNDGUARD + (step1 / (tnf + small_num))); /* Finding subfleet_id */
  930: 						buyernf = step1 % tnf; /* Finding the fishery_id */
  931: 						buyermonth = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][b])) % nm; /* Finding the month_id */
  932: 						down_time = bm->SUBFLEET_ECONprms[buyernf][buyerns][down_time_id];
  933: 						max_month_effort = bm->month_scalar * (1 - down_time) * bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id];
  934: 
  935: 						/* The expected increase in catch by filling the current month */
  936: 						if ((max_month_effort - bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id] >= 1)
  937: 								&& (bm->MargRent[buyernf][buyerns][sp][buyermonth] >= 0)) {
  938: 							scheduledEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id];
  939: 							ExpCatch = bm->BlackBook[buyernf][buyerns][sp][buyermonth][expect_id];
  940: 							ExpEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][hist_id];
  941: 							expectedIncreasedCatch = (max_month_effort - scheduledEffort) * ExpCatch / (ExpEffort + small_num);
  942: 							expectedCatch = 0;
  943: 							prop_olease = bm->QuotaAlloc[buyernf][buyerns][sp][permolease_id];
  944: 							prop_tlease = bm->QuotaAlloc[buyernf][buyerns][sp][templease_id];
  945: 							ownQuota = (1.0 - prop_olease - prop_tlease) * bm->QuotaAlloc[buyernf][buyerns][sp][owned_id];
  946: 							leaseQuota = bm->QuotaAlloc[buyernf][buyerns][sp][leased_id];
  947: 							cumCatch = bm->QuotaAlloc[buyernf][buyerns][sp][cumboatcatch_id];
  948: 							for (m = bm->MofY; m < 12; m++) {
  949: 								scheduledEffort = bm->EffortSchedule[buyernf][buyerns][m][expect_id];
  950: 								ExpCatch = bm->BlackBook[buyernf][buyerns][sp][m][expect_id];
  951: 								ExpEffort = bm->EffortSchedule[buyernf][buyerns][m][hist_id];
  952: 								expectedCatch += scheduledEffort * ExpCatch / (ExpEffort + small_num);
  953: 							}
  954: 							demand = cumCatch + expectedCatch + expectedIncreasedCatch - ownQuota - leaseQuota;
  955: 						}
  956: 						if (demand <= 0)
  957: 							b++;
```

**Suggested fix (from review):** Start at `s = nsort - 1`

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

## QUOT-07

**stale variable** · Conf H / Sev M  
**Lines (current code):** 1101  
**Lines at review (Oct 2026):** 1101

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

Step 2 demand test uses `max_month_effort`, which is still set for the previous buyer (from 1000/1130 or 933). Unlike step 1 (932-933), it is not recomputed for the current `buyernf/buyerns`. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 1099–1103)

```c
 1099: 
 1100: 						/* The expected increase in catch by filling the current month */
 1101: 						if ((max_month_effort - bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id] >= 1)
 1102: 								&& (bm->MargRent[buyernf][buyerns][sp][buyermonth] >= 0)) {
 1103: 							scheduledEffort = bm->EffortSchedule[buyernf][buyerns][buyermonth][expect_id];
```

**Suggested fix (from review):** Recompute down_time/max_month_effort for the buyer inside the loop before line 1101

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

## QUOT-08

**uninitialised/stale variable** · Conf H / Sev M  
**Lines (current code):** 1064-1066, 1131  
**Lines at review (Oct 2026):** 1064-1066, 1131

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

Step 2 decodes `sellerns/sellernf` but never `sellermonth`, so `Trade()` gets the stale value from step 1, or 0. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 1062–1068)

```c
 1062: 						/* Deconstructing ids from single sorted array - assuming ids in
 1063: 						 the sorted array are of the form ns*tnf*(12 - month) + (12 - month)*nf + month - bm->MofY */
 1064: 						step1 = (int) floor(ROUNDGUARD + (linearPI[sp][idd_id][s] / (nm + small_num))); /* First step in finding fishery_id and subfleet_id */
 1065: 						sellerns = (int) floor(ROUNDGUARD + (step1 / (tnf + small_num))); /* Finding subfleet_id */
 1066: 						sellernf = step1 % tnf; /* Finding the fishery_id */
 1067: 
 1068: 						/* Use totPlanEffort and totPlanCatch calculated elsewhere */
```

**Suggested fix (from review):** Decode sellermonth as at 968 (plus the QUOT-05 fix)

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

## QUOT-09

**division by zero** · Conf H / Sev M  
**Lines (current code):** 1013, 1023  
**Lines at review (Oct 2026):** 1013, 1023

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

`scheduledEffort * ExpCatch / ExpEffort` — no `+ small_num`, unlike every other use of this expression (952, 987). Any month with zero historical effort makes totPlanCatch Inf/NaN. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 1011–1025)

```c
 1011: 						ExpEffort = bm->EffortSchedule[sellernf][sellerns][m][hist_id];
 1012: 						bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanEffort_id] += scheduledEffort;
 1013: 						bm->SUBFLEET_ECONprms[sellernf][sellerns][totPlanCatch_id] += scheduledEffort * ExpCatch / ExpEffort;
 1014: 					}
 1015: 
 1016: 					bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanEffort_id] = 0;
 1017: 					bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanCatch_id] = 0;
 1018: 					for (m = bm->MofY; m < 12; m++) {
 1019: 						scheduledEffort = bm->EffortSchedule[buyernf][buyerns][m][expect_id];
 1020: 						ExpCatch = bm->BlackBook[buyernf][buyerns][sp][m][expect_id];
 1021: 						ExpEffort = bm->EffortSchedule[buyernf][buyerns][m][hist_id];
 1022: 						bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanEffort_id] += scheduledEffort;
 1023: 						bm->SUBFLEET_ECONprms[buyernf][buyerns][totPlanCatch_id] += scheduledEffort * ExpCatch / ExpEffort;
 1024: 					}
 1025: 
```

**Suggested fix (from review):** Divide by `(ExpEffort + small_num)`

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

## QUOT-10

**wrong index** · Conf M / Sev M  
**Lines (current code):** 866  
**Lines at review (Oct 2026):** 866

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

Inside the `month` loop, `expectedCPUE` uses `BlackBook[..][bm->MofY]` and `EffortSchedule[..][bm->MofY][hist_id]`, not `[month]`, so pi for every future month uses the current month's CPUE. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 864–868)

```c
  864: 									scheduledEffort = bm->EffortSchedule[nf][ns][month][expect_id];
  865: 									fishprice = bm->SP_FISHERYprms[sp][nf][saleprice_id];
  866: 									expectedCPUE = bm->BlackBook[nf][ns][sp][bm->MofY][expect_id] / (bm->EffortSchedule[nf][ns][bm->MofY][hist_id] + small_num);
  867: 									pi[nf][ns][month][sp] = scheduledEffort * ((fishprice * expectedCPUE) - (TripCost / (TripLength + small_num)));
  868: 									pi[nf][ns][month][sp] -= priceQuota * (cumCatch + expectedCatch - ownQuota - leaseQuota);
```

**Suggested fix (from review):** Use `[month]`

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

## QUOT-11

**wrong variable** · Conf H / Sev M  
**Lines (current code):** 1356  
**Lines at review (Oct 2026):** 1356

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

`Quicksort_Dir(ax, bx, cx, dx, ex, nsort, ascendflag);` — in the package path `nsort` is still 0 (set at 808/811 and only reset in the sp_by_sp branch). The match list (`idp` entries) is never sorted, so trades go in loop order, not best match first. The comment also says matches should be minimised, but `ascendflag`=0 gives descending order. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 1354–1358)

```c
 1354: 			 what is wanted and what is available)
 1355: 			 */
 1356: 			Quicksort_Dir(ax, bx, cx, dx, ex, nsort, ascendflag);
 1357: 
 1358: 			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
```

**Suggested fix (from review):** Pass `idp`; check whether ascendflag should be 1 here

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

## QUOT-12

**buffer overrun** · Conf M / Sev H  
**Lines (current code):** 1327-1332  
**Lines at review (Oct 2026):** 1327-1332

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

`ax[idp] = ...; idp++` runs once per (buyer subfleet, seller subfleet) pair, so idp can reach ~(nf*ns)^2. But ax..ex are sized `totpi = K_max_num_subfleet*K_num_fisheries*12` (ateconomicsetup.c:990-1004). With more than about 13 active fishery-subfleets that all pass the filters, this writes past the end of the arrays. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 1325–1334) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
 1325: 										continue;
 1326: 
 1327: 									ax[idp] = tot_match;
 1328: 									bx[idp] = buyernf;
 1329: 									cx[idp] = buyerns; // buyer id
 1330: 									dx[idp] = sellernf;
 1331: 									ex[idp] = sellerns; // seller id
 1332: 									idp++;
 1333: 								}
 1334: 							}
```

**Suggested fix (from review):** Size ax..ex as `(nf*ns)^2`, or bound-check idp

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

## QUOT-13

**wrong id constant (array aliasing)** · Conf H / Sev M  
**Lines (current code):** 1166, 1171-1172, 1955  
**Lines at review (Oct 2026):** 1166, 1171-1172, 1955

**Why it's thought to be a bug**

`bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id]` — `within_id` (#define 15, a QuotaAlloc index, atlantisboxmodel.h:1577) is used to index SUBFLEET_ECONprms, where slot 15 is `res_cost_ind_id` (enum at atlantisboxmodel.h:1409-1423). ateconindicator.c:533 writes reswork there, so the "within" filter really tests research cost, and Trade()/ateconresponse.c:1078/atEconomicAnnual.c:241 overwrite the ResCost indicator with 0/1. The code's own TODO comment admits this.

**Current code** (`ateconomic/atquota.c` lines 1164–1174)

```c
 1164: 						if (do_debug) {
 1165: 							fprintf(llogfp, "Time: %e, itc: %d, buyer = %s-%d nboat: %e, within: %e\n", bm->dayt, itc, FisheryArray[buyernf].fisheryCode,
 1166: 									buyerns, bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id], bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id]);
 1167: 						}
 1168: 
 1169: 						/* TODO: create a seperate # define for the within_id in the subfleet econ params */
 1170: 						/* if not close to exceeding quota they are not looking to buy stuff in */
 1171: 						if((!bm->SUBFLEET_ECONprms[buyernf][buyerns][nboat_id]) ||
 1172: 						   (!bm->SUBFLEET_ECONprms[buyernf][buyerns][within_id])){
 1173: 							continue;
 1174: 						}
```

**Suggested fix (from review):** Add a dedicated subfleet enum entry (e.g. `subfleet_within_id`) and use it at all five sites

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

## QUOT-14

**missing cap** · Conf M / Sev M  
**Lines (current code):** 1806-1807, 1651-1655  
**Lines at review (Oct 2026):** 1806-1807, 1651-1655

**Function:** `Trade()`

**Why it's thought to be a bug**

Bought quota: in the supply>demand branch there is no cap, so `owned_id -= demand` can push seller owned below 0 and trigger `quit("Negative quota")`. The demand>supply branch caps at total owned, not the un-leased part (1-po-pt)*owned. Latent today, because `keep_trading`=1e9 gives p_buy of about 4e-44. Function: `Trade()`.

**Current code** (`ateconomic/atquota.c` lines 1804–1809)

```c
 1804: 		if (rndnum < p_buy) {
 1805: 			/* Bought quota */
 1806: 			bm->QuotaAlloc[buyernf][buyerns][sp][owned_id] += demand;
 1807: 			bm->QuotaAlloc[sellernf][sellerns][sp][owned_id] -= demand;
 1808: 			demand_used = demand;
 1809: 
```

**Suggested fix (from review):** Cap at `(1-po-pt)*owned` in both branches

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

## QUOT-15

**inconsistent formula** · Conf L / Sev M  
**Lines (current code):** 1449 vs 1271-1272  
**Lines at review (Oct 2026):** 1449 vs 1271-1272

**Function:** `Quota_trade()`

**Why it's thought to be a bug**

spareend is `p+(1-p)*M/12` with threshold `(1-spareend)*Q` when matching, but `p*(1-M/12)` (algebraically) with threshold `spareend*Q` when trading. The matcher and the trader disagree on what is available to sell. Function: `Quota_trade()`.

**Current code** (`ateconomic/atquota.c` lines 1447–1451)

```c
 1447: 
 1448: 						/* Closer to end of year then more likely to sell what extra remains - intentionally keep less so happy to sell so don't zero out supply as easily */
 1449: 						spareend = bm->prop_spareend + (((1.0 / bm->prop_spareend) - 1.0) * bm->prop_spareend - 1.0) * (bm->MofY / 12.0);
 1450: 						if (supply < spareend * (SELLERownQuota + SELLERleaseQuota)) {
 1451: 							/* Want to hold on to their last little bit of quota.
```

**Suggested fix (from review):** Use one helper for both

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

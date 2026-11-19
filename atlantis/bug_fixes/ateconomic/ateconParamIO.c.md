# ateconomic/ateconParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-08](#ecio-08) | 735 | M/M | missing subfleet fill | `bm->ECONexprice[fisheryIndex][0][sp][i][hist_id] = values[i];` fills subfleet 0 only. ateconomicsetup.c:392 t… |  | [ ] |
| [ECIO-09](#ecio-09) | 678 | H/M | wrong cast | `bm->high_price = (int) Util_XML_Read_Value(..., no_checking, "high_price");`, but `high_price` is a `double` … |  | [ ] |
| [ECIO-10](#ecio-10) | 158 | M/M | duplicate read / validation bypass | `bm->minValue` is read from Economic_Flags (l.158) and validated (l.160, must be non-zero for the NZ log model… |  | [ ] |
| [ECIO-11](#ecio-11) | 291 | H/L | copy-paste (messages) | Error strings are copied from other blocks. `Read_Market_Value()` reports "Market_Intercept" for Market_Trend … |  | [ ] |
| [ECIO-12](#ecio-12) | 751, 757 | M/L | NULL dereference | `inputDoc = xmlReadFileDestFolder(...)` is used as `inputDoc->children` without a NULL check, so a missing or … |  | [ ] |

---

## ECIO-08

**missing subfleet fill** · Conf M / Sev M  
**Found at (current code):** `ateconParamIO.c` 735
**Lines at review (Oct 2026):** 733-736

**Function:** `Read_Dan_Holland_XML()`

**Why it's thought to be a bug**

`bm->ECONexprice[fisheryIndex][0][sp][i][hist_id] = values[i];` fills subfleet 0 only. ateconomicsetup.c:392 then copies `[nf][ns]..[hist_id]` into `expect_id` for every ns (the copy-from-subfleet-0 line at :391 is commented out). ateconindicator.c:438 and ateconeffort.c:1031/1114 read `[nf][ns]..[hist_id]` for all ns. Result: every subfleet with ns>0 sees an ex-vessel price of 0 in the Holland model (zero gvp and zero marginal rent). Function: `Read_Dan_Holland_XML()`.

**Code at issue** — `ateconParamIO.c` line 735  
Search for: `ECONexprice[fisheryIndex][0][sp][i][hist_id] = values[i]`

```c
  733: 			for (i = 0; i < bm->nbox; i++) {
  734: 				for (fisheryIndex = 0; fisheryIndex < bm->K_num_fisheries; fisheryIndex++) {
  735→ 					bm->ECONexprice[fisheryIndex][0][sp][i][hist_id] = values[i];
  736: 				}
  737: 			}
```

**Suggested fix (from review):** Loop `ns < nsubfleets` and set `ECONexprice[fisheryIndex][ns][sp][i][hist_id]`.

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

## ECIO-09

**wrong cast** · Conf H / Sev M  
**Found at (current code):** `ateconParamIO.c` 678
**Lines at review (Oct 2026):** 678

**Function:** `Read_Dan_Holland_XML()`

**Why it's thought to be a bug**

`bm->high_price = (int) Util_XML_Read_Value(..., no_checking, "high_price");`, but `high_price` is a `double` (atlantisboxmodel.h:6062). The cast truncates the fraction, and a value above INT_MAX (plausible for an "arbitrarily high price") is undefined behaviour. Function: `Read_Dan_Holland_XML()`.

**Code at issue** — `ateconParamIO.c` line 678  
Search for: `high_price = (int) Util_XML_Read_Value(`

```c
  676: 
  677: 	bm->ITQ_think_ahead = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, integer_check, "ITQ_think_ahead");
  678→ 	bm->high_price = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "high_price");
  679: 
  680: 	/* Penatly tax */
```

**Suggested fix (from review):** Remove the `(int)` cast.

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

## ECIO-10

**duplicate read / validation bypass** · Conf M / Sev M  
**Found at (current code):** `ateconParamIO.c` 158
**Lines at review (Oct 2026):** 158-161 vs 345

**Why it's thought to be a bug**

`bm->minValue` is read from Economic_Flags (l.158) and validated (l.160, must be non-zero for the NZ log model). It is then read again from Quota_Reconciliation/LeaseValues (l.345), which silently overwrites it without re-validating, so a 0 there leads to log(0) in the Newell model. Functions: `Read_Economic_Flags()`, `Read_Quota_XML()`.

**Code at issue** — `ateconParamIO.c` line 158  
Search for: `minValue`

```c
  156: 	fflush(stdout);
  157: 
  158→ 	bm->minValue = Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, groupingNode, no_checking, "minValue");
  159: 
  160: 	if (!bm->minValue && bm->UseMinValue && (bm->flagLease == NZmodel))
```

**Suggested fix (from review):** Read it in one place, or repeat the check after l.345.

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

## ECIO-11

**copy-paste (messages)** · Conf H / Sev L  
**Found at (current code):** `ateconParamIO.c` 291
**Lines at review (Oct 2026):** 234, 238, 262, 174, 180, 484, 599, 627, 675

**Why it's thought to be a bug**

Error strings are copied from other blocks. `Read_Market_Value()` reports "Market_Intercept" for Market_Trend and Market_Autocoefft. `Read_Monthly_Market_Value()` reports path "Market_Intercept/…" for MonthlyMarket. Rescale messages say 'ReScale_Effort'/'ReScale_Catch' but the tags are ReScale_Eff/ReScale_Cat. FuelCosts path is given as "SubfleetEffort/" (it lives under SubfleetCosts). The SpatialEffort and SpatialCPUE checks both say "TemporalBlackBook". `Read_Dan_Holland_XML()` says "Read_Management_Costs_XML".

**Code at issue** — `ateconParamIO.c` line 291 (located by name only; check this is the right place)  
Search for: `Read_Market_Value(bm, fileName, groupingNode, "Market_Trend", trend_coefft_id);`

```c
  289: 
  290: 	Read_Market_Value(bm, fileName, groupingNode, "Market_Intercept", market_intercept_id);
  291→ 	Read_Market_Value(bm, fileName, groupingNode, "Market_Trend", trend_coefft_id);
  292: 	Read_Market_Value(bm, fileName, groupingNode, "Market_Autocoefft", auto_coefft_id);
  293: }
```

**Suggested fix (from review):** Correct each message.

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

## ECIO-12

**NULL dereference** · Conf M / Sev L  
**Found at (current code):** `ateconParamIO.c` 751, `ateconParamIO.c` 757
**Lines at review (Oct 2026):** 751-757

**Function:** `Read_Economic_Parameters()`

**Why it's thought to be a bug**

`inputDoc = xmlReadFileDestFolder(...)` is used as `inputDoc->children` without a NULL check, so a missing or malformed econ XML segfaults instead of quitting with a message. Function: `Read_Economic_Parameters()`.

**Code at issue** — `ateconParamIO.c` line 751  
Search for: `inputDoc = xmlReadFileDestFolder(`

```c
  749: int Read_Economic_Parameters(MSEBoxModel *bm, char *filename) {
  750: 	xmlDocPtr inputDoc;
  751→ 	inputDoc = xmlReadFileDestFolder(bm->destFolder, filename, NULL, 0);
  752: 
  753: 	printf("Economic parameter file loading\n");
```

**Code at issue** — `ateconParamIO.c` line 757  
Search for: `inputDoc->children`

```c
  755: 	printf("***");
  756: 
  757→ 	Read_Economic_Flags(bm, filename, inputDoc->children);
  758: 	Read_ByCatch_Incentives_XML(bm, filename, inputDoc->children);
  759: 	Read_Market_Characteristics_XML(bm, filename, inputDoc->children);
```

**Suggested fix (from review):** `if (!inputDoc) quit(...)`.

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

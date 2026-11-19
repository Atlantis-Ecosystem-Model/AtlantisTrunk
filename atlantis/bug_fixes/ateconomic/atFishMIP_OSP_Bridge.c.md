# ateconomic/atFishMIP_OSP_Bridge.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **10 findings** (severity H: 6; confidence H and severity H: 5; on the priority list: 5), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-13](#ecio-13) ⭐ | 518, 90 | H/H | index packing / overlap | `catchBySpFishery[k * nfishery + nf]` uses a different `nfishery` for each species. Rows for different k overl… |  | [ ] |
| [ECIO-14](#ecio-14) ⭐ | 542 | H/H | stale variable | In Step 3, `totalCatch += catchBySpFishery[k * nfishery + nf]` uses `nfishery` left over from the last species… |  | [ ] |
| [ECIO-15](#ecio-15) ⭐ | 670, 673 | H/H | stale variable / accumulator not reset / wrong index | The Step 4 per-fishery loop has three faults. (a) `sp` is stale (the last species from the earlier loops), so … |  | [ ] |
| [ECIO-16](#ecio-16) ⭐ | 730 | H/H | stale variable | In Mode C, `c = catchBySpFishery[k * nfishery + nf]` again uses the stale `nfishery` from Step 2 rather than s… |  | [ ] |
| [ECIO-17](#ecio-17) ⭐ | 577 | H/H | type truncation | `gp` is declared `int` (l.480), but `FishMIP_Adapter_GetGlobalPriceForYear()` returns `double` (adapter .h:112… |  | [ ] |
| [ECIO-18](#ecio-18) | 387 | H/M | uninitialised variable | `g_catch_n_effort_History` is `malloc`'d and `firstcall` is never set, so `if (g_catch_n_effort_History[sp].fi… |  | [ ] |
| [ECIO-19](#ecio-19) | 33, 428, 447 | M/H | unit mismatch | Catch comes from `Harvest_Get_TotCumCatch()` (TotCumCatch, in mg N: ateconindicator.c:454-456 compares it to `… |  | [ ] |
| [ECIO-20](#ecio-20) | 550 | M/M | defaults overwritten with 0 | `ApplyLoadedDrivers()` guards catchability with `> 0.0` but copies `compliance`, `invest_driver_U` and `invest… |  | [ ] |
| [ECIO-21](#ecio-21) | 138 | M/L | empty-dimension call | `RunPIMAssessment()` runs for every fished species even when `nfishery == 0` (no fishery targets it per `Colle… |  | [ ] |
| [ECIO-22](#ecio-22) | 148 | L/L | memory leak / dead code | `FreeOSPStruct()` is static and never called, and no public free exists, so all OSP arrays leak; it will also … |  | [ ] |

## Reviewer notes for this file

(This file IS listed in libateconomic_a_SOURCES (ateconomic/Makefile.am:23), even though its header comment says it is not. It compiles but is not called from anywhere.)

---

## ECIO-13

**index packing / overlap** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 518, `atFishMIP_OSP_Bridge.c` 90
**Lines at review (Oct 2026):** 513-520 (and 542, 673, 730)

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

`catchBySpFishery[k * nfishery + nf]` uses a different `nfishery` for each species. Rows for different k overlap: for example, species k=0 with nfishery=3 uses slots 0-2, and species k=2 with nfishery=1 writes slot 2, overwriting k=0's third fishery. The buffer is allocated `K_num_fisheries*K_num_tot_sp` (l.90). Function: `FishMIP_OSP_Bridge_Annual()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 518  
Search for: `catchBySpFishery[k * nfishery + nf]`

```c
  516:         for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  517:             this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  518→             OSP_PARAMS->catchBySpFishery[k * nfishery + nf] = Harvest_Get_TotCumCatch(OSP_PARAMS->fishedSp[k], this_nf, bm->thisyear);
  519:         }
  520:     }
```

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 90  
Search for: `K_num_fisheries * bm->K_num_tot_sp`

```c
   88:     OSP_PARAMS->price_by_sp =  Util_Alloc_Init_1D_Double(bm->K_num_tot_sp, 0.0);
   89:     
   90→     OSP_PARAMS->catchBySpFishery = Util_Alloc_Init_1D_Double(bm->K_num_fisheries * bm->K_num_tot_sp, 0.0);
   91:     
   92:     OSP_PARAMS->thisYearEffort = Util_Alloc_Init_1D_Double(bm->K_num_fisheries, 0.0);
```

**Suggested fix (from review):** Index as `k * bm->K_num_fisheries + nf` everywhere (or `+ this_nf`).

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

## ECIO-14

**stale variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 542
**Lines at review (Oct 2026):** 538-544

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

In Step 3, `totalCatch += catchBySpFishery[k * nfishery + nf]` uses `nfishery` left over from the last species of the Step 2 loop, not species k's own value, so `prod[k]` (and the price) is summed from the wrong slots. Function: `FishMIP_OSP_Bridge_Annual()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 542 (closest match to the quoted code; wording differs slightly)  
Search for: `totalCatch += OSP_PARAMS->catchBySpFishery[k * nfishery + nf];`

```c
  540:             sp = OSP_PARAMS->fishedSp[k];
  541:             for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  542→                 totalCatch += OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
  543:             }
  544:             OSP_PARAMS->prod[k] = totalCatch;
```

**Suggested fix (from review):** Set `nfishery = g_catch_n_effort_History[sp].nfishery;` inside the loop (plus the ECIO-13 fix).

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

## ECIO-15

**stale variable / accumulator not reset / wrong index** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 670, `atFishMIP_OSP_Bridge.c` 673
**Lines at review (Oct 2026):** 669-691

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

The Step 4 per-fishery loop has three faults. (a) `sp` is stale (the last species from the earlier loops), so `this_nf = g_catch_n_effort_History[sp].targetFishery[nf]` maps every fishery through one species' target list. For nf >= that species' nfishery the entries are 0, so q/compliance/effort all come from fishery 0. (b) `totalCatch`, `revenue` and `ftargetWeighted` are never reset per fishery; `totalCatch` even carries over from Step 3. Y_arr, price_arr and Ftarget_arr are therefore running totals over fisheries. (c) `c = catchBySpFishery[k*nfishery+nf]` reads with nf running up to K_num_fisheries, crossing into other species' rows. Function: `FishMIP_OSP_Bridge_Annual()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 670  
Search for: `this_nf = g_catch_n_effort_History[sp].targetFishery[nf]`

```c
  668: 
  669:         for (nf = 0; nf < bm->K_num_fisheries; ++nf) {
  670→             this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  671:             for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  672:                 nfishery = g_catch_n_effort_History[sp].nfishery;
```

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 673 (closest match to the quoted code; wording differs slightly)  
Search for: `c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];`

```c
  671:             for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  672:                 nfishery = g_catch_n_effort_History[sp].nfishery;
  673→                 c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
  674:                 if (c <= 0.0) {
  675:                     continue;
```

**Suggested fix (from review):** Reset the three accumulators at the top of each nf iteration, use `this_nf = nf`, and index catch as `[k*K_num_fisheries + nf]`.

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

## ECIO-16

**stale variable** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 730
**Lines at review (Oct 2026):** 728-736

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

In Mode C, `c = catchBySpFishery[k * nfishery + nf]` again uses the stale `nfishery` from Step 2 rather than species sp's value, so the catch weights for origprice are taken from the wrong slots. Function: `FishMIP_OSP_Bridge_Annual()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 730 (closest match to the quoted code; wording differs slightly)  
Search for: `c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];`

```c
  728:             for (nf = 0; nf < g_catch_n_effort_History[sp].nfishery; ++nf) {
  729:                 this_nf = g_catch_n_effort_History[sp].targetFishery[nf];
  730→                 c = OSP_PARAMS->catchBySpFishery[k * nfishery + nf];
  731:                 if (c <= 0.0) {
  732:                     continue;
```

**Suggested fix (from review):** Use the per-species nfishery (with the ECIO-13 indexing fix).

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

## ECIO-17

**type truncation** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 577
**Lines at review (Oct 2026):** 480, 576-578

**Function:** `FishMIP_OSP_Bridge_Annual()`

**Why it's thought to be a bug**

`gp` is declared `int` (l.480), but `FishMIP_Adapter_GetGlobalPriceForYear()` returns `double` (adapter .h:112). External prices are truncated to whole units, and any price below 1.0 becomes 0, which still passes `gp >= 0.0` and is written as the species price. Function: `FishMIP_OSP_Bridge_Annual()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 577  
Search for: `gp >= 0.0`

```c
  575:             for (k = 0; k < OSP_PARAMS->fished_nsp; ++k) {
  576:                 gp = FishMIP_Adapter_GetGlobalPriceForYear(FunctGroupArray[OSP_PARAMS->fishedSp[k]].groupCode, calendarYear);
  577→                 if (gp >= 0.0) {
  578:                     OSP_PARAMS->price_by_sp[k] = gp;
  579:                     ++overridden;
```

**Suggested fix (from review):** Declare `double gp;`.

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

## ECIO-18

**uninitialised variable** · Conf H / Sev M  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 387
**Lines at review (Oct 2026):** 134-142, 387

**Why it's thought to be a bug**

`g_catch_n_effort_History` is `malloc`'d and `firstcall` is never set, so `if (g_catch_n_effort_History[sp].firstcall > 0)` reads garbage. Either the back-fill never runs (past years stay 0 and the assessment sees an effort/catch collapse) or it runs unpredictably. Functions: `Create_OSP_Arrays()`, `RunPIMAssessment()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 387  
Search for: `if(g_catch_n_effort_History[sp].firstcall > 0)`

```c
  385:     }
  386: 
  387→     if(g_catch_n_effort_History[sp].firstcall > 0){
  388:         /* If the bridge only started running mid-simulation, earlier years have
  389:          * no recorded entry -- back-fill (permanently -- recorded[iy] is set so
```

**Suggested fix (from review):** Set `firstcall = 1` in Create_OSP_Arrays (or use calloc).

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

## ECIO-19

**unit mismatch** · Conf M / Sev H  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 33, `atFishMIP_OSP_Bridge.c` 428, `atFishMIP_OSP_Bridge.c` 447
**Lines at review (Oct 2026):** 400, 410, 435-448, 518, 602

**Why it's thought to be a bug**

Catch comes from `Harvest_Get_TotCumCatch()` (TotCumCatch, in mg N: ateconindicator.c:454-456 compares it to `TACamt*kg_2_mg/X_CN`). That catch feeds the PIM assessment, and the resulting `MSY` is written straight into `bm->TACamt[sp][nf][now_id]` and `SP_FISHERYprms[..][TAC_id]`, which are in tonnes/kg. The mg N catch is also used as "prod" for the price model and for revenue weighting. Functions: `RunPIMAssessment()`, `FeedPIMAssessmentIntoAtlantisTAC()`, `FishMIP_OSP_Bridge_Annual()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 33  
Search for: `Harvest_Get_TotCumCatch()`

```c
   31:  * catch, update effort and fishing mortality from the priced catch -- but
   32:  * every quantity that step reads or writes is a real Atlantis array
   33→  * (bm->totfishpop, Harvest_Get_TotCumCatch(), bm->Effort,
   34:  * bm->SP_FISHERYprms[...][saleprice_id]) instead of the toy model's own
   35:  * logistic-population arrays; every call into the plug-in model itself goes
```

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 428  
Search for: `TACamt[sp][nf][now_id]`

```c
  426: /** The "feed back to TACs" half of option (a): writes the plug-in's MSY
  427:  *  estimate into exactly the fields Atlantis's own TAC machinery reads
  428→  *  (bm->TACamt[sp][nf][now_id], consulted by atharvest/atHarvestIndex.c and
  429:  *  atimplementation/atImplementationAnnual.c; bm->SP_FISHERYprms[sp][nf]
  430:  *  [TAC_id], the "origin" TAC atimplementation/atImplementationAnnual.c's
```

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 447 (closest match to the quoted code; wording differs slightly)  
Search for: `bm->SP_FISHERYprms[sp][nf][TAC_id] = TACnf;`

```c
  445:         bm->TACamt[sp][nf][old_id] = bm->TACamt[sp][nf][now_id];
  446:         bm->TACamt[sp][nf][now_id] = TACnf;
  447→         bm->SP_FISHERYprms[sp][nf][TAC_id] = TACnf;
  448:     }
  449:     /*
```

**Suggested fix (from review):** Convert catch with `* X_CN * mg_2_tonne` (and back) at the bridge boundary.

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

## ECIO-20

**defaults overwritten with 0** · Conf M / Sev M  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 550
**Lines at review (Oct 2026):** 321-331

**Function:** `ApplyLoadedDrivers()`

**Why it's thought to be a bug**

`ApplyLoadedDrivers()` guards catchability with `> 0.0` but copies `compliance`, `invest_driver_U` and `invest_driver_V` unconditionally. For a reference country with no EEZ, the adapter returns `ok=1` with these fields left at 0 (adapter .cpp:158-165). Compliance then becomes 0 and investment drivers 0 for every fishery. Function: `ApplyLoadedDrivers()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 550  
Search for: `ApplyLoadedDrivers()`

```c
  548:              * model itself would read for the loaded reference
  549:              * country/commodity/year (applied uniformly across every fished
  550→              * species -- see ApplyLoadedDrivers()'s doc comment for why).
  551:              * Otherwise falls back to the original placeholder: intercept
  552:              * 50% above current supply, unit slope -- guarantees a
```

**Suggested fix (from review):** Only overwrite when the driver set has EEZ data (expose hasEezPos) or the value is > 0.

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

## ECIO-21

**empty-dimension call** · Conf M / Sev L  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 138
**Lines at review (Oct 2026):** 370-416, 651-656

**Function:** `RunPIMAssessment()`

**Why it's thought to be a bug**

`RunPIMAssessment()` runs for every fished species even when `nfishery == 0` (no fishery targets it per `CollectFleets()`), so `fit_one_stock` gets a [nyears x 0] C/E matrix. Function: `RunPIMAssessment()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 138 (located by name only; check this is the right place)  
Search for: `g_catch_n_effort_History[sp].nfishery =  CollectFleets(bm, sp) ;`

```c
  136:     for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  137:         g_catch_n_effort_History[sp].targetFishery = Util_Alloc_Init_1D_Int(bm->K_num_fisheries, 0.0);
  138→         g_catch_n_effort_History[sp].nfishery =  CollectFleets(bm, sp) ;
  139:         g_catch_n_effort_History[sp].effortFlat = Util_Alloc_Init_1D_Double(arr_lengtth, 0.0);
  140:         g_catch_n_effort_History[sp].catchFlat = Util_Alloc_Init_1D_Double(arr_lengtth, 0.0);
```

**Suggested fix (from review):** `if (nfishery == 0) return 0;`.

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

## ECIO-22

**memory leak / dead code** · Conf L / Sev L  
**Found at (current code):** `atFishMIP_OSP_Bridge.c` 148
**Lines at review (Oct 2026):** 148-187

**Function:** `FreeOSPStruct()`

**Why it's thought to be a bug**

`FreeOSPStruct()` is static and never called, and no public free exists, so all OSP arrays leak; it will also trigger an unused-function warning. The unused `done` parameter suggests it was meant to be wired up. Function: `FreeOSPStruct()`.

**Code at issue** — `atFishMIP_OSP_Bridge.c` line 148 (the line the review pointed to, found by its text)  
Search for: `static void FreeOSPStruct(MSEBoxModel *bm, int done) {`

```c
  146: 
  147: 
  148→ static void FreeOSPStruct(MSEBoxModel *bm, int done) {
  149:     int sp;
  150:     
```

**Suggested fix (from review):** Expose it (e.g. `Free_OSP_Arrays`) and call it from Economic_Free.

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

atFishMIP_OSP_Bridge.c:67/141: `SimLength` is in days, but `recorded`/`effortFlat` are indexed by year. That is oversized (no bug), but the naming suggests a years-vs-days mix-up.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atFishMIP_OSP_Bridge.c:443: when there is no prior TAC, MSY is split evenly across all fisheries, including ones that cannot catch the species.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

# atlantismain/include/atlantisboxmodel.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MAIN-09](#main-09) | 408-414 | H/L | copy-paste / wrong guard | `#ifndef min` / `#define min` is followed by `#if !defined(min)` / `#define max`. Because min was just defined… |  | [ ] |
| [MAIN-10](#main-10) | 333-340, 153 | M/L | preprocessor logic | `#ifndef SMALL_LOG_FILE_SIZE ... #elif BETH_LOG_FILE_SIZE ... #else`: the BETH option is only checked when SMA… |  | [ ] |
| [MAIN-11](#main-11) | 389-402 | L/M | conflicting enum/define values | `nlevel_id` and `growth_id` both equal `ICE_BASED+1` (5), which is also `MIXED`. `death_id` (6) equals `nTotHa… |  | [ ] |
| [BMH-01](#bmh-01) | 6302, 6321, 6323, 6347, 6365, 6366, 6367, 6237? | H/L | dead declarations | Eight prototypes have no definition anywhere in the source tree (externalLibs/netcdf excluded) and no call sit… |  | [ ] |
| [BMH-02](#bmh-02) | 6280-6281? (defs atmanage/atRlink.c:298, 359; atmanage/atRlinkRedus.c:269, 323) | H/L | duplicate definition (latent) | `freeRedus()` and `REDUS_management()` are each defined twice, in atRlink.c and in atRlinkRedus.c. Both copies… |  | [ ] |

## Reviewer notes for this file

Prototype spot-checks (no mismatch found): createBMDataFile, readBMTracerInfo, boundaries, physics, External_Box_Ecology, Util_Check_NetCDF_Size, Ecology_Test_Fish_Total. The header's prototypes were not checked exhaustively.
Allocation and index checks (all consistent after applying the reversal convention): newwctr/newsedtr/newicetr/newlandtr, vvdistrib vs set_up_vvdistrib, stock_struct, v_stock_struct, coveramt, refuge_status, turbid_effect, FCtsCarryOver, Effort_hdistrib, EffortRecord, CatchRecord, totagepop, CumDisplaceEffort, TAC_trigger (triggered_scalar_id = 2 < 3).
Prototype check summary: all 145 prototypes in the header's prototype section (lines 6093-6307, including the 8 under `#ifdef RASSESS_LINK_ENABLED`) were checked against every definition in the repo's .c/.cpp files. There are no other function prototypes in the header, only the `void(*err)(...)` parameter types. The check was done with a helper script (scratchpad/bmh_protos.py, which strips comments and string literals), followed by a manual pass over its full signature dump (scratchpad/bmh_sigs.txt).
- 134 match exactly: same return type, same number of parameters, and the same parameter types in the same order. This includes `physics()`, whose definition in atphysics.c:666 also uses the reduced 4-argument form that matches the header comment, and the only call (atlantismain.c:550) follows it.
- No return-type, parameter-count, parameter-type or order-swap mismatches were found. Only 4 prototypes name their parameters differently from the definition, and none of these is a swap: readBMIceInfo `fielName`/`fileName` (a typo), Util_Read_Run_XML `name`/`fileName`, Determine_Stock_Prop `sp`/`species`, and Get_Length `i`/`sp`.
- 8 have no definition (BMH-01).
- 3 are defined in two places with identical signatures: freeRedus and REDUS_management (BMH-02), and `Quicksort_Dir` (ateconhelp.c:42 and atUtilhelp.c:43). Quicksort_Dir is already logged as ECIO-26 and is not logged again here.
- None of the prototype names is declared in another header with a different signature. The only other declaration is Quicksort_Dir in ateconomic/include/atEconomic.h:111, which is identical. No `static` definition or file-local redeclaration conflicts with any of these prototypes.
- `fuel_intercept_id` and `market_intercept_id` are both 0 and index different arrays.
- `tier_orig` and `tier0` are both 0.
- The "changing conditions" group (1220-1231) overlays several change schemas. For example `end_id`, `period_id`, `selcurvechange_id` and `thresh_mult_id` are all 1, and `pulse_id` and `discard_mult_id` are both 3. Usage shows each array uses only one schema; the `end_id` and `period_id` overlap appears only in comments (atbiolUtil.c:124-125).
No `K_num_*`, `num_*` or `tot_*` count is smaller than or equal to an id in its own group. The `HABITAT_TYPES` versus `nlevel_id`/`growth_id`/`death_id` overlap is already logged as MAIN-11, and `within_id` aliasing into SUBFLEET_ECONprms is already logged as QUOT-13.

---

## MAIN-09

**copy-paste / wrong guard** · Conf H / Sev L  
**Lines (current code):** 408-414  
**Lines at review (Oct 2026):** 408-414

**Why it's thought to be a bug**

`#ifndef min` / `#define min` is followed by `#if !defined(min)` / `#define max`. Because min was just defined, the max guard is always false, so this header never defines `max`. It only compiles because sjwlib/include/sjwlib.h:71 defines max. The code is latent and fragile, and with the `>=` version the two definitions also differ.

**Current code** (`sjwlib/include/sjwlib.h` lines 86–96) — code appears to have moved -320 lines since the review

```c
   86: 
   87: /* Maximum length of a file name */
   88: #define MAXFNAMELEN (200)
   89: 
   90: #include "sjwalloc.h"
   91: #include "txt_param.h"
   92: #include "polyline.h"
   93: #include "colourtable.h"
   94: #include "mapproj.h"
   95: #include "datafile.h"
   96: #include "pointsourcesink.h"
```

**Suggested fix (from review):** Change the guard to `#ifndef max`

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

## MAIN-10

**preprocessor logic** · Conf M / Sev L  
**Lines (current code):** 333-340, 153  
**Lines at review (Oct 2026):** 333-340, 153

**Why it's thought to be a bug**

`#ifndef SMALL_LOG_FILE_SIZE ... #elif BETH_LOG_FILE_SIZE ... #else`: the BETH option is only checked when SMALL_LOG_FILE_SIZE is defined, so defining BETH_LOG_FILE_SIZE alone has no effect. The revision note at line 153 also gives stale sizes (1.024e9/1.024e3, against the actual 2.024e8/1.024e7).

**Suggested fix (from review):** Restructure as `#if defined(BETH_LOG_FILE_SIZE) ... #elif defined(SMALL_LOG_FILE_SIZE) ... #else`

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

## MAIN-11

**conflicting enum/define values** · Conf L / Sev M  
**Lines (current code):** 389-402  
**Lines at review (Oct 2026):** 389-402

**Why it's thought to be a bug**

`nlevel_id` and `growth_id` both equal `ICE_BASED+1` (5), which is also `MIXED`. `death_id` (6) equals `nTotHabTypes`. Arrays sized `nlevel_id` cannot take a `growth_id`/`death_id` index, and a `fluxnum` of MIXED cannot be told apart from growth_id (atecology/atfluxbreakdown.c:80, atecology.c:1350). There is no compile error only because no switch also has `case MIXED`. Arrays indexed by fluxnum were not traced.

**Current code** (`atecology/atfluxbreakdown.c` lines 216–233) — code appears to have moved -171 lines since the review

```c
  216: 								maxflux = valcheck;
  217: 								flux_sp = fgIndex;
  218: 								fluxnum = death_id;
  219: 								fcnum = n;
  220: 							}
  221: 						}
  222: 					}
  223: 					break;
  224: 				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
  225: 				case BIOMASS:
  226: 					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
  227: 						if (FunctGroupArray[fgIndex].habitatType == EPIFAUNA){
  228: 							if (fabs(localEPIFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
  229: 								ans = FunctGroupArray[fgIndex].totNTracers[n];
  230: 								maxflux = localEPIFlux[FunctGroupArray[fgIndex].totNTracers[n]];
  231: 								flux_sp = fgIndex;
  232: 								fluxnum = EPIFAUNA;
  233: 								fcnum = n;
```

**Suggested fix (from review):** Define growth_id/death_id as `nTotHabTypes`, `nTotHabTypes+1` and size the flux arrays to match

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

## BMH-01

**dead declarations** · Conf H / Sev L  
**Lines (current code):** 6302, 6321, 6323, 6347, 6365, 6366, 6367, 6237?  
**Lines at review (Oct 2026):** 6110, 6129, 6131, 6155, 6173, 6174, 6175, 6237

**Why it's thought to be a bug**

Eight prototypes have no definition anywhere in the source tree (externalLibs/netcdf excluded) and no call sites: `alloc_names`, `readRunParams`, `readSourceSink`, `writeBMSummaryEpiInfo`, `setfisheryNameID`, `setfisheryParamNameID`, `setspNameID`, `setSPid`. The code says some were removed on purpose: atmanage/atManageSetup.c:157 "Deleted the setSPid function", and atphysics/atparamIO.c:32 says readRunParams was replaced (now `Util_Read_Run_XML`). They are harmless today, but if someone adds a call, the build fails at link time instead of at compile time. Functions: as listed.

**Current code** (`atmanage/atManageSetup.c` lines 6108–1605) — ⚠ the quoted code was not found in the current file (changed, or already fixed?). These are just the listed line numbers, which may now hold unrelated code

```c

```

**Suggested fix (from review):** Delete the 8 prototypes

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

## BMH-02

**duplicate definition (latent)** · Conf H / Sev L  
**Lines (current code):** 6280-6281? (defs atmanage/atRlink.c:298, 359; atmanage/atRlinkRedus.c:269, 323)  
**Lines at review (Oct 2026):** 6280-6281 (defs atmanage/atRlink.c:298, 359; atmanage/atRlinkRedus.c:269, 323)

**Why it's thought to be a bug**

`freeRedus()` and `REDUS_management()` are each defined twice, in atRlink.c and in atRlinkRedus.c. Both copies have identical signatures and both sit under `#ifdef RASSESS_LINK_ENABLED`. atRlink.c is not listed in atmanage/Makefile.am (line 11 lists only atRlinkRedus.c, atRlinkRBC.c and atRlinkRAssess.c) or in any other build file, so it is a stale copy that does not link today. If it is ever added back to the sources, the build gets duplicate-symbol link errors, and in the meantime the two copies can drift apart. Callers (atlantismain.c:880, atmanage/atManageAnnual.c:2176) bind to the atRlinkRedus.c versions. Functions: `freeRedus()`, `REDUS_management()`.

**Current code** (`atmanage/atRlink.c` lines 296–300)

```c
  296: }
  297: 
  298: int freeRedus()
  299: {
  300:  /*
```

**Suggested fix (from review):** Delete atmanage/atRlink.c, or move it to an archive folder

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

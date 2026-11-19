# atlantisUtil/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTH-01](#uth-01) | 776 | M/M | fixed-size array vs unbounded index | `int spMinMax[4];` holds the min/max SN tracer id per stage, for juvenile and adult only. atbiolsetup.c:3600-3… |  | [ ] |
| [UTH-02](#uth-02) | 835, 2392, 2378 | M/M | allocation size vs index (stage vs habitat) | The comment says `pSP_SG_eat` is "diet available for each cohort of seagrass". It is allocated with a fixed si… |  | [ ] |
| [UTH-03](#uth-03) | 266 | M/L | buffer overrun (unbounded strcpy) | `char groupCode[20]`, `name[200]`, `fullName[200]` are filled by `Util_XML_Get_Value_String(..., "GroupCode"/"… |  | [ ] |
| [UTH-04](#uth-04) | 44 | H/L | enum vs name table (converter lookup) | The GROUP_TYPES enum has 33 real values, and the runtime table in atGroupTypes.h covers all of them. The conve… |  | [ ] |

## Reviewer notes for this file

The prototype/definition mismatches are logged above (UTL2-08 for atUtilLib.h:102-103, UTL2-09 for atUtilLib.h:112). No other issues found. Checked: the atUtilLib.h, atXMLUtil.h, atFisheryXMLUtil.h and atlantisutil.h prototypes against the definitions in the audited .c files, and the atlantisMem.h alloc/free macros. atFunctGroup.h, atTracer.h, atGroupTypes.h and atFisheryStruct.h were only skimmed (atFisheryStruct.h: `fisheryCode[20]`, `name[200]` are filled via strcpy in Util_XML_Get_Value_String with no length check, so an overlong code in the XML would overflow; see the note below).
No issues found. Checked: every `extern int` tracer index against its definition in atecology/include/atnamelist.h:87-145 (only included by atbiolsetup.c:373, so there is no multiple definition) and against its registration in atPhysChemIO.c / atBuildTracer.c. Also checked that optional tracers (SED_i, Wind_i, Rugosity_i, AragoniteSat_i, Noise/Light_Pollution_i, P/C/TOP and the phosphorus set) are only used under the same flag that registers them (flagIsEstuary, track_wind, track_rugosity_arag, flag_pollutant_impacts / isNoiseEffected, track_atomic_ratio). pH_i is explicitly set to -1 when not tracked.
No issues found. The `groupTypes[]` table has 33 entries covering every GROUP_TYPES value except the NUM_GROUP_TYPES sentinel. Its order differs from the enum (MICROPHTYBENTHOS sits before CORAL), but the table is only searched by string (atUtilGroupIO.c:345-347), so order doesn't matter. FISH_INVERT is remapped to FISH right afterwards (atUtilGroupIO.c:357-360). All GROUP_TYPES switches (atecology.c:301, atUtilGroupIO.c:1443, atassess/atindices.c, atnetwork.c, atdatareading.c) and the `spErrorStructure[NUM_GROUP_TYPES]` initialisation in atassess/atsamplesetup.c:733-777 cover every invertebrate type that can reach them.
No new issues. `fisheryCode[20]`/`name[200]` unbounded strcpy was already noted in 28_UTL2 (see UTH-03 for the same fix). `isRec`, `index`, `totalEffortTracer`, `averageCatchSizeTracer` are all set (atUtilFisheryIO.c:119-121, atHarvestSetup.c:391/412). The AssessProjectionStruct allocation, indexing and free are covered by MNGIO-08..17 (atmanage/atPGMSY.c).
Lower-confidence notes:
- Cross-checked and already logged elsewhere, so not re-logged here: HowFar int (MOVE-04); habitatCoeffs sizing (UTIL-19); contaminant arrays including `agingContam` sized numCohorts (UTIL-17/CONT-01); ratioLost*/ratioGained* habitat=3 and numCohorts (UTIL-18); isCloseKin/isCollisionRisk uninitialised (UTIL-20/21); containsTurf (UTIL-22); `cohortSpeciesParams` indexed by cohort (NUTR-26); `grow` reset loop bound (ECOL-16); `distrib_VERTICAL`/`distrib_OVERWINTER`/`distrib` stage dimension 2 (BXML-05); `co_sp`/`co_sp_catch` indexing (TIER-38, MNGA-02, MNGIO-05).
- Header comments that disagree with the allocations (cosmetic): `pSPEat` says `[cohort][prey][habitat]`, but the first dimension is `numStages` (atUtilGroupIO.c:832) and every caller loops stages, so it's correct. `ratioGainedPred` says "one value per cohort, element and habitat", but it is 2-D `[nCxG][num_atomic_id]` (atUtilGroupIO.c:896). The `FisheryStruct` doc comment says "each functional group", and the `AssessProjectionStruct` comment says "evolution relevant information" (copy-paste).
- atTracer.h declares `Diatom_S_i` and `MicroPB_S_i` extern, but they are never defined or used anywhere. Any future use would be a link error. atTracer.h also has no include guard, which is harmless for extern declarations.
- `isWCActiveOnly` (atFunctGroup.h:130) is set (atUtilGroupIO.c:1509-1512) but never read. `index` (line 84), `MaxNumMig`, `SpawnSizeTracer` and `tempINVpopratio` are likewise set/allocated but never used (dead members, no bug).
- atGroupTypes.h defines a non-static, initialised object (`GroupTypesStruct groupTypes[]`) in a header. It is only safe while exactly one translation unit includes it (currently atUtilGroupIO.c). Making it `static const` would prevent a future duplicate-symbol error.

---

## UTH-01

**fixed-size array vs unbounded index** · Conf M / Sev M  
**Found at (current code):** `atbiolsetup.c` 776
**Lines at review (Oct 2026):** 306

**Why it's thought to be a bug**

`int spMinMax[4];` holds the min/max SN tracer id per stage, for juvenile and adult only. atbiolsetup.c:3828-3831 (`Set_spMinMax_Values()`) fills just [0..3]. It is read as `spMinMax[(preystage * 2)]` / `[(preystage * 2 + 1)]` with `preystage < FunctGroupArray[prey].numStages` (atmovement.c:283, 299-300), and the same with `predstage < numStages` (atmovement.c:472, 481-482). `numStages` is only checked to be >= 1 (atUtilGroupIO.c:294). An AGE_STRUCTURED group with 3+ stages therefore reads past the array into the next struct members (`AGEnewden` pointer bytes), and the garbage value is then used as a tracer index. This is the same root cause as BXML-05 (no `numStages <= 2` check), but that finding doesn't list `spMinMax`. Struct: `FunctionalGroupStruct`.

**Code at issue** — `atbiolsetup.c` line 776 (closest match to the quoted code; wording differs slightly)  
Search for: `for (preyage = 0; preyage < FunctGroupArray[prey].numStages; preyage++) {`

```c
  774: 			for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
  775: 				for (chrtstage = 0; chrtstage < FunctGroupArray[pred].numStages; chrtstage++) {
  776→ 					for (preyage = 0; preyage < FunctGroupArray[prey].numStages; preyage++) {
  777: 
  778: 						if (!FunctGroupArray[pred].pSPEat[chrtstage][prey][WC] && bm->pSPVERTeat[pred][prey][chrtstage][preyage] > 0) {
```

**Suggested fix (from review):** Add a group-load check that `numStages <= 2` for vertebrates (covers BXML-05 too), or make `spMinMax` `int *` sized `2*numStages` and fill per stage.

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

## UTH-02

**allocation size vs index (stage vs habitat)** · Conf M / Sev M  
**Found at (current code):** `atBiologyXMLParamIO.c` 835, `atBiologyXMLParamIO.c` 2392, `atBiologyXMLParamIO.c` 2378
**Lines at review (Oct 2026):** 351-353

**Why it's thought to be a bug**

The comment says `pSP_SG_eat` is "diet available for each cohort of seagrass". It is allocated with a fixed size of 3 (atBiologyXMLParamIO.c:833, `Util_Alloc_Init_1D_Double(3, 0.0)`). It is filled in `Read_Seagrass_Diet_Avail_XML()` (atBiologyXMLParamIO.c:2375-2390) as `pSP_SG_eat[cohort] = values[b]` with `cohort < FunctGroupArray[sp].numStages` of the seagrass group, but read in `Get_Prey_Avail` code as `pSP_SG_eat[habitat]` (atprocess.c:368). If the seagrass group has more than 3 stages, every predator's array is overrun. If it has fewer than 3, the epiphyte/root entries stay 0. Either way the fill index (stage) and the read index (habitat) are not the same thing. The in-code comment (line 2375) admits the loop "is actually habitats ... rather than cohorts".

**Code at issue** — `atBiologyXMLParamIO.c` line 835  
Search for: `Util_Alloc_Init_1D_Double(3, 0.0)`

```c
  833: 		}
  834: 		if(bm->flag_macro_model == TRUE){
  835→ 			FunctGroupArray[sp].pSP_SG_eat = Util_Alloc_Init_1D_Double(3, 0.0);
  836: 		}
  837: 	}
```

**Code at issue** — `atBiologyXMLParamIO.c` line 2392  
Search for: `pSP_SG_eat[cohort] = values[b]`

```c
 2390:                 // TODO: Allow for multiple Seagrass species - currently overwrites
 2391:                 for (b = 0; b < bm->K_num_tot_sp; b++){
 2392→                     FunctGroupArray[b].pSP_SG_eat[cohort] = values[b];
 2393:                 }
 2394:                 free(values);
```

**Code at issue** — `atBiologyXMLParamIO.c` line 2378  
Search for: `cohort < FunctGroupArray[sp].numStages`

```c
 2376:                 quit("%s species attribute group not found.\n", errorString);
 2377:             
 2378→             for (cohort = 0; cohort < FunctGroupArray[sp].numStages; cohort++) { // This is actually habitats - sed, wc, epiphytes rather than cohorts
 2379:                 snprintf(stageString, sizeof(stageString), "stage%d", cohort);
 2380:                 snprintf(errorString, sizeof(errorString), "Diet/SeagrassFoodAvail/%s/stage%d", FunctGroupArray[sp].groupCode, cohort);
```

**Suggested fix (from review):** Loop the read over the 3 seagrass components (or `FunctGroupArray[sp].numCohorts`, and allocate to match). Allocate `pSP_SG_eat` from the same constant used to read it, and fix the header comment.

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

## UTH-03

**buffer overrun (unbounded strcpy)** · Conf M / Sev L  
**Found at (current code):** `atUtilGroupIO.c` 266
**Lines at review (Oct 2026):** 80-82

**Why it's thought to be a bug**

`char groupCode[20]`, `name[200]`, `fullName[200]` are filled by `Util_XML_Get_Value_String(..., "GroupCode"/"Name"/"LongName", FunctGroupArray[groupIndex].groupCode)` (atUtilGroupIO.c:264, 274-275). That function does `strcpy(returnValue, value)` with no length (atUtilXML.c:1363). `Is_Valid_Group_Code()` (2-3 letters) only runs after the copy (line 269). An over-long code or name in the groups XML/CSV therefore overflows into the following int fields before validation. The earlier UTL2 note only covered `FisheryStruct.fisheryCode[20]`/`name[200]` (same mechanism, atUtilFisheryIO.c).

**Code at issue** — `atUtilGroupIO.c` line 266  
Search for: `Util_XML_Get_Value_String(`

```c
  264: 			groupNode = groupList->nodesetval->nodeTab[groupIndex];
  265: 
  266→ 			Util_XML_Get_Value_String(convertedXMLFileName, ATLANTIS_ATTRIBUTE, TRUE, groupNode, "GroupCode", FunctGroupArray[groupIndex].groupCode);
  267: 			if (verbose > 0)
  268: 				printf("Reading in definition of %s\n", FunctGroupArray[groupIndex].groupCode);
```

**Suggested fix (from review):** Add a size argument to `Util_XML_Get_Value_String` and use a bounded copy (`snprintf(dst, size, "%s", value)`), or check `strlen(value)` before copying.

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

## UTH-04

**enum vs name table (converter lookup)** · Conf H / Sev L  
**Found at (current code):** `atGroupTypes.h` 44
**Lines at review (Oct 2026):** 36-72

**Why it's thought to be a bug**

The GROUP_TYPES enum has 33 real values, and the runtime table in atGroupTypes.h covers all of them. The converter's `GroupType` Lookup list (ConvertAtlantis/atGroupsXML.c:181-206) is missing `BIRD`, `MICROPHTYBENTHOS`, `CORAL`, `SPONGE`, `ICE_DIATOMS`, `ICE_MIXOTROPHS` and `ICE_ZOOBIOTA`. The Lookup nodes are documentation only (nothing validates against them), so this doesn't change results, but the generated XML advertises an incomplete set of legal group types.

**Code at issue** — `atGroupTypes.h` line 44 (located by name only; check this is the right place)  
Search for: `{ MICROPHTYBENTHOS, "MICROPHTYBENTHOS" },`

```c
   42: 		{ SED_EP_OTHER, "SED_EP_OTHER" },
   43: 		{ MOB_EP_OTHER, "MOB_EP_OTHER" },
   44→ 		{ MICROPHTYBENTHOS, "MICROPHTYBENTHOS" },
   45: 		{ CORAL, "CORAL" },
   46:         { SPONGE, "SPONGE" },
```

**Suggested fix (from review):** Add the missing `Util_XML_Create_Lookup_Value_Node` entries.

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

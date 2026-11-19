# Cross-references, duplicates and fix-together groups

File:line references were re-mapped to the current code on 2026-11-19 (≈ approximate, ? untraced).

Copied from `to-do-fixt-list.md` (Part A §3 and Part B §B.4). Check here before fixing a bug, because some must be fixed together across files.

## Part A notes (atassess)


- **Mirrored bugs:** several bugs appear in both atsurvey.c (sampled) and atattribute.c (true values), e.g. SRV-08/ATT-08, ATT-13/atsurvey.c:600, SRV-11/ATT-14, ATT-11/atsurvey.c:535/699. Fix both together so true and sampled values stay comparable.
- **Fix together:** SOL-1, SOL-2 and CLS-01 all concern the Nelder-Mead implementation and should be fixed as one change (Amoeba, both callers in atclassical.c, Linear_Regression, and the P/Y allocations).
- **Earlier reviews:** the earlier `*_bug_analysis.md` files in this directory (April 2026) overlap with some entries here (e.g. CLS-03/CLS-04). This list was produced independently from the current source; line numbers refer to the files as of today.

---


## Part B cross-references


**ID prefixes:**

| Prefix | Files |
|---|---|
| DEMO | atdemography |
| MOVE | atmovement |
| BXML | atBiologyXMLParamIO |
| ECOL | atecology.c |
| BSET | atbiolsetup |
| BMSG | atbiolmessage, atfluxbreakdown, atLandProcess, atImposeRecruit, atPhysChemIO |
| PROC | atprocess, atGroupProcesses |
| CONT | atContaminants, atvertprocesses, atexternalpop |
| BIOL | atbiology, additionalTracer, atannualbiology, atmacrophytes |
| NUTR | atNutrient, atdiversity, atbiophysics, atcoral |
| EINIT | init, atmigration, atIceProcesses, atBuildTracer, atq10, atcalibtools |
| EMISC | other atecology files and headers |
| QUOT | atquota, ateconeffort, ateffortquota, ateconts |
| ECON | ateconomicsetup, ateconresponse, atEconomicAnnual, misc |
| ECIO | ateconio, ateconindicator, ateconParamIO, FishMIP bridge |
| HARV | atHarvest, atHarvestParamIO, atHarvestCatch |
| HRV2 | other atharvest files and atimplementation |
| TIER | atManageTier |
| MNG | atManage, atManageIndex, atSS3assess |
| MNGA | atManageAnnual, atManageSetup |
| MNGIO | atManageParamIO, atPGMSY, atRlink*, atManageIO, atManageMPATS |
| SS3IO | atSS3LinkIO |
| SS3 | other atSS3Link files |
| PHY1 | aticeIO, attempsalt, atphysics, atphysIO |
| PHY2 | geometry, tracer IO, source/sink, transport |
| PHY3 | other atphysics files |
| UTIL | atUtilXML, atUtilGroupIO, atUtil |
| UTL2 | other atlantisUtil files and headers |
| MAIN | atlantismain |
| CNV1 / CNV2 | ConvertAtlantis |
| LINK | atCLAMLink, atlink, atbrokerlink |
| SJW1 / SJW2 / SJW3 | sjwlib |

**Same bug reported twice. Fix once and tick both entries:**
- **EMISC-26 = CLS-17 (Part A):** `Ecology_Spawn` gets X_RS and FSPB in swapped order in atclassical.c.
- **UTIL-17 ↔ CONT-01:** contaminant arrays are allocated per `numCohorts` (atUtilGroupIO.c:909-913) but looped to `numCohortsXnumGenes` (atContaminants.c). Fix either the allocation or the loops.
- **API-06 (Part A) ↔ CNV2-06:** `Num_Sexes` is read and written as a binary flag, in both the reader and the converter.
- **SJW1-06:** the `dfFindTime` rewind formula. The SJW3 reviewer found the same bug independently.
- **BIOL-02 ↔ PHY3-12:** `totDiet` is sized by `K_num_max_cohort`, but both of these loop to `numCohortsXnumGenes*numStocks`.
- **PHY3-06 ↔ PHY1-20:** leftover debug checks hard-code box index 24 (`newwc[24]` / `newwctr[24]`).
- **SRV-08 / ATT-08 (Part A) ↔ SJW3-15:** the `round()` macro in sjwlib.h evaluates its argument three times. Where that argument contains `drandom()`, three different random numbers feed one age assignment, which makes the double-`+0.5` age bug worse.

**Bugs reviewers noticed in files outside their own assignment.** Check these against the owning file's entries:
- `atmanage/atManageSetup.c:1551`: `cpue_cdf_poor_p_id` is used where `cpue_cdf_top_p_id` is meant. Found by the SJW2 reviewer.
- `atmanage/atManageParamIO.c:662`: `CPUEshotavgdis` is read from the key `"CPUEshotfdis"`. Found by the CNV2 reviewer.
- **MOVE-04:** the fix also needs `int HowFar` changed to `double` in `atlantisUtil/include/atFunctGroup.h:94`.
- `atlink/Makefile.am` lists `atlinkconversion.o` instead of `.c`. Found by the LINK reviewer.

**Code that isn't compiled or isn't called (findings there are latent):**
- Not compiled: `atecology/dump.c`, `atmanage/atRlink.c`, `atCLAMLink/atCLAMLink.c`, `atSS3Link/atSS3Test.c`.
- Compiled but never called: the FishMIP bridge (`ateconomic/atFishMIP_OSP_*`), `atPGMSY.c`'s `Initialise_PGMSY` (so `Do_Atlantis_PGMSY` crashes if it is ever reached; see MNGIO-08), and `nrcdflib` lognormal/Cauchy/Weibull.
- `init.c` is fully commented out.
- The atlink EwE code only builds with LINK_ENABLED.

**Fix together:**
- **TIER-35, TIER-41, SS3-34:** several calls pass (year, species) in swapped order. Check every caller of `DoCPUE`, `DoPrager`, `Call_Tier3`, `TierFiveSAFE` and `writeCloseKinNcompsfiles`.
- **SS3-26 ✔:** every local close-kin array in atCloseKin.c is allocated in index order. The code's own TODO asks for this check, and the reviewer notes the same pattern at atAssessTierSetup.c:541-554.
- **SS3IO-14, SS3IO-16, SS3IO-17, SS3IO-20, SS3IO-21, SS3IO-25, SS3IO-28, SS3IO-31:** all corrupt the generated SS3 input files. Fix them as one pass, then check the result against an SS3.30 example file.

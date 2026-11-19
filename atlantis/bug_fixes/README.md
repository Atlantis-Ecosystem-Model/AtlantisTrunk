# Atlantis bug-fix workflow

This folder holds the working review files for `atlantis/to-do-fixt-list.md`: **1143 findings** in **188 review files**. 202 findings are rated high confidence and high severity, 207 are on the priority lists and there are 180 lower-confidence notes. The review files were generated on 2026-11-19.

## Step 0 — merge main first

Before starting on the bug files, work through **[00_merge_from_main.md](00_merge_from_main.md)**. Main has 137 commits that this branch doesn't, and several of them touch code in the to-do list. **Done 2026-11-19** (merge commit `1c3b874`; follow-up batch 001). The review files have been regenerated against the merged code.

## How it works

1. **Pick a file.** Choose a review file from the index below. The "Suggested order" table lists the files with the most high-priority bugs first.
2. **Respond per bug.** Each bug in the review file has the review's reasoning, the current code, the suggested fix, and options A–E to tick. Write any detail under **Your comments**. You can also fill in the **Decision** column of the summary table at the top (e.g. A, C, "A but use isImpacted") if that is quicker.
   - A = apply the suggested fix · B = fix differently (say how) · C = not a bug · D = Claude investigates and reports back · E = defer.
3. **Fix a batch.** Tell Claude which file and which IDs to work on. Claude will:
   - re-read the current code (line numbers have drifted since the review) and check `CROSS_REFERENCES.md` for bugs that must be fixed together;
   - make the code changes for that batch only. A batch is a few bugs from one file, unless the bug itself spans several files;
   - fill in **Outcome** for each bug and tick **Done** in the summary table;
   - write a commit message to `commit_messages/NNN_<file>_<IDs>.txt` and show it in chat.
4. **You review and commit.** Claude never stages, commits or pushes.
5. Bugs marked D get investigated and written up in that bug's Outcome, and come back to you for a decision before any code changes.

**Notes**
- **Finding the code:** each bug's **Code at issue** block is located by searching the current code for what the review quoted, not by line number. Paste the **Search for** text into your editor's Find; line numbers change as fixes go in. → marks the matching line, and the block says how it was found: exact (700), a close match (135), the line the review pointed to found by its text (134), or by name only (137; check these). In 35 cases the review quoted no code, and the block shows the line it pointed to (also check these). ⚠ means it couldn't be found (2). **Lines at review** keeps the October numbers.
- **Conf/Sev**: H/M/L confidence it's a bug / severity if it is. ✔ = the reviewer re-checked it by hand. ⭐ = on the priority list. 🔀 = the merge of main changed this code (21 findings), so check whether it's already fixed before acting.
- `_lower_confidence_<GROUP>.md` files hold speculative notes (from §B.3 of the list) that could not be tied to one file. Other lower-confidence notes are at the end of the relevant file's review.
- The original list's Status column is not being updated. These files are the record.

## Progress

| Batch | Date | Review file | IDs | Commit message file |
|---|---|---|---|---|
| 000 | 2026-11-19 | [00_merge_from_main.md](00_merge_from_main.md) | merge origin/main — committed as `1c3b874` | [000_merge_origin_main.txt](commit_messages/000_merge_origin_main.txt) (not used — default merge message) |
| 001 | 2026-11-19 | atdemography.c, outamps, bug_fixes/ | A-01 tempPopRatio index; remove outamps; review files | [001_post_merge_fixup.txt](commit_messages/001_post_merge_fixup.txt) |
| 002 | 2026-11-19 | [atassess/atAssessTierSetup.c](atassess/atAssessTierSetup.c.md) | ATS-01–13 done except ATS-08 (awaiting your choice); code-located snippets in all review files | [002_atAssessTierSetup_ATS.txt](commit_messages/002_atAssessTierSetup_ATS.txt) |
|  |  |  |  |  |

## Suggested order (most high-confidence + high-severity first)

| Review file | Findings | Conf H & Sev H | ⭐ Priority | Sev H |
|---|---|---|---|---|
| [atmanage/atManageTier.c](atmanage/atManageTier.c.md) | 56 | 23 | 23 | 24 |
| [atSS3Link/atSS3LinkIO.c](atSS3Link/atSS3LinkIO.c.md) | 33 | 17 | 17 | 19 |
| [atmanage/atManageAnnual.c](atmanage/atManageAnnual.c.md) | 23 | 11 | 11 | 11 |
| [atmanage/atPGMSY.c](atmanage/atPGMSY.c.md) | 18 | 10 | 10 | 11 |
| [atSS3Link/atSS3DataGen.c](atSS3Link/atSS3DataGen.c.md) | 24 | 8 | 8 | 10 |
| [atassess/atindices.c](atassess/atindices.c.md) | 22 | 8 | 8 | 9 |
| [atassess/atclassical.c](atassess/atclassical.c.md) | 24 | 7 | 8 | 8 |
| [atmanage/atManage.c](atmanage/atManage.c.md) | 29 | 6 | 6 | 8 |
| [atSS3Link/atCloseKin.c](atSS3Link/atCloseKin.c.md) | 15 | 6 | 6 | 7 |
| [atassess/atsolve_doolittle](atassess/atsolve_doolittle.md) | 16 | 5 | 5 | 6 |
| [ateconomic/atFishMIP_OSP_Bridge.c](ateconomic/atFishMIP_OSP_Bridge.c.md) | 10 | 5 | 5 | 6 |
| [atmanage/atRlinkRBC.c](atmanage/atRlinkRBC.c.md) | 8 | 5 | 5 | 5 |
| [atassess/atsurvey.c](atassess/atsurvey.c.md) | 18 | 4 | 5 | 6 |
| [atecology/atContaminants.c](atecology/atContaminants.c.md) | 19 | 3 | 3 | 6 |
| [atecology/atdemography.c](atecology/atdemography.c.md) | 30 | 3 | 3 | 4 |
| [atassess/atdiet.c](atassess/atdiet.c.md) | 7 | 3 | 3 | 3 |
| [atCLAMLink/atCLAMLinkIO.c](atCLAMLink/atCLAMLinkIO.c.md) | 8 | 3 | 3 | 3 |
| [atlink/atlinkconversion.c](atlink/atlinkconversion.c.md) | 6 | 3 | 3 | 3 |
| [atlink/atlinkexport.c](atlink/atlinkexport.c.md) | 7 | 3 | 3 | 3 |
| [atassess/atassessParamIO.c](atassess/atassessParamIO.c.md) | 17 | 2 | 2 | 6 |
| [atassess/atAssessTierSetup.c](atassess/atAssessTierSetup.c.md) | 13 | 2 | 2 | 3 |
| [atecology/atmovement.c](atecology/atmovement.c.md) | 30 | 2 | 2 | 3 |
| [atecology/atprocess.c](atecology/atprocess.c.md) | 18 | 2 | 2 | 3 |
| [ateconomic/atquota.c](ateconomic/atquota.c.md) | 15 | 2 | 2 | 3 |
| [atbrokerlink/ImportExportData.c](atbrokerlink/ImportExportData.c.md) | 7 | 2 | 2 | 3 |

## Full index

### atassess/ — 166 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atAssessTierSetup.c](atassess/atAssessTierSetup.c.md) | 13 | 2 | 2 | 3 | 0 | not started |
| [atassessParamIO.c](atassess/atassessParamIO.c.md) | 17 | 2 | 2 | 6 | 0 | not started |
| [atclassical.c](atassess/atclassical.c.md) | 24 | 7 | 8 | 8 | 0 | not started |
| [atindices.c](atassess/atindices.c.md) | 22 | 8 | 8 | 9 | 0 | not started |
| [atsurvey.c](atassess/atsurvey.c.md) | 18 | 4 | 5 | 6 | 0 | not started |
| [atwriting.c](atassess/atwriting.c.md) | 5 | 1 | 1 | 1 | 0 | not started |
| [atattribute.c](atassess/atattribute.c.md) | 16 | 0 | 1 | 2 | 0 | not started |
| [atnetwork.c](atassess/atnetwork.c.md) | 3 | 0 | 1 | 0 | 0 | not started |
| [atsample.c](atassess/atsample.c.md) | 7 | 0 | 0 | 1 | 0 | not started |
| [atdiet.c](atassess/atdiet.c.md) | 7 | 3 | 3 | 3 | 0 | not started |
| [atdatareading.c](atassess/atdatareading.c.md) | 6 | 2 | 2 | 2 | 0 | not started |
| [atsamplesetup.c](atassess/atsamplesetup.c.md) | 7 | 0 | 0 | 1 | 0 | not started |
| [atsolve_doolittle](atassess/atsolve_doolittle.md) | 16 | 5 | 5 | 6 | 0 | not started |
| [include_headers](atassess/include_headers.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [outside_atassess](atassess/outside_atassess.md) | 1 | 0 | 1 | 0 | 0 | not started |

### atecology/ — 295 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atdemography.c](atecology/atdemography.c.md) | 30 | 3 | 3 | 4 | 7 | not started |
| [atmovement.c](atecology/atmovement.c.md) | 30 | 2 | 2 | 3 | 11 | not started |
| [atBiologyXMLParamIO.c](atecology/atBiologyXMLParamIO.c.md) | 13 | 0 | 0 | 1 | 5 | not started |
| [atecology.c](atecology/atecology.c.md) | 23 | 0 | 0 | 0 | 6 | not started |
| [atbiolsetup.c](atecology/atbiolsetup.c.md) | 16 | 2 | 2 | 2 | 0 | not started |
| [atbiolmessage.c](atecology/atbiolmessage.c.md) | 11 | 0 | 0 | 0 | 2 | not started |
| [atfluxbreakdown.c](atecology/atfluxbreakdown.c.md) | 6 | 0 | 0 | 0 | 0 | not started |
| [atImposeRecruit.c](atecology/atImposeRecruit.c.md) | 2 | 0 | 0 | 0 | 1 | not started |
| [atPhysChemIO.c](atecology/atPhysChemIO.c.md) | 1 | 0 | 0 | 0 | 1 | not started |
| [atprocess.c](atecology/atprocess.c.md) | 18 | 2 | 2 | 3 | 5 | not started |
| [atGroupProcesses.c](atecology/atGroupProcesses.c.md) | 10 | 0 | 0 | 0 | 4 | not started |
| [atContaminants.c](atecology/atContaminants.c.md) | 19 | 3 | 3 | 6 | 0 | not started |
| [atvertprocesses.c](atecology/atvertprocesses.c.md) | 5 | 0 | 0 | 0 | 0 | not started |
| [atexternalpop.c](atecology/atexternalpop.c.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [atbiology.c](atecology/atbiology.c.md) | 4 | 2 | 2 | 2 | 1 | not started |
| [additionalTracer.c](atecology/additionalTracer.c.md) | 11 | 1 | 1 | 1 | 2 | not started |
| [atannualbiology.c](atecology/atannualbiology.c.md) | 9 | 0 | 0 | 0 | 2 | not started |
| [atmacrophytes.c](atecology/atmacrophytes.c.md) | 2 | 0 | 0 | 0 | 3 | not started |
| [atNutrient.c](atecology/atNutrient.c.md) | 7 | 0 | 0 | 0 | 2 | not started |
| [atdiversity.c](atecology/atdiversity.c.md) | 9 | 1 | 1 | 1 | 1 | not started |
| [atbiophysics.c](atecology/atbiophysics.c.md) | 4 | 0 | 0 | 0 | 2 | not started |
| [atcoral.c](atecology/atcoral.c.md) | 8 | 1 | 1 | 2 | 1 | not started |
| [atmigration.c](atecology/atmigration.c.md) | 12 | 2 | 2 | 2 | 3 | not started |
| [atIceProcesses.c](atecology/atIceProcesses.c.md) | 6 | 0 | 0 | 0 | 2 | not started |
| [atBuildTracer.c](atecology/atBuildTracer.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atq10.c](atecology/atq10.c.md) | 4 | 1 | 1 | 1 | 1 | not started |
| [atcalibtools.c](atecology/atcalibtools.c.md) | 2 | 0 | 0 | 0 | 1 | not started |
| [atForcedMovement.c](atecology/atForcedMovement.c.md) | 5 | 0 | 0 | 0 | 0 | not started |
| [attime.c](atecology/attime.c.md) | 3 | 1 | 1 | 1 | 0 | not started |
| [dump.c](atecology/dump.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [atExternalScalar.c](atecology/atExternalScalar.c.md) | 7 | 0 | 0 | 0 | 0 | not started |
| [atecologyts.c](atecology/atecologyts.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [atbiolUtil.c](atecology/atbiolUtil.c.md) | 4 | 2 | 2 | 2 | 0 | not started |
| [include_headers](atecology/include_headers.md) | 2 | 1 | 1 | 1 | 0 | not started |
| [_lower_confidence_PROC](atecology/_lower_confidence_PROC.md) | 0 | 0 | 0 | 0 | 1 | not started |

### ateconomic/ — 84 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atquota.c](ateconomic/atquota.c.md) | 15 | 2 | 2 | 3 | 0 | not started |
| [ateconeffort.c](ateconomic/ateconeffort.c.md) | 12 | 1 | 1 | 1 | 0 | not started |
| [ateconts.c](ateconomic/ateconts.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [ateconomicsetup.c](ateconomic/ateconomicsetup.c.md) | 6 | 1 | 1 | 1 | 1 | not started |
| [ateconresponse.c](ateconomic/ateconresponse.c.md) | 11 | 2 | 2 | 2 | 3 | not started |
| [atEconomicAnnual.c](ateconomic/atEconomicAnnual.c.md) | 10 | 1 | 1 | 1 | 1 | not started |
| [ateconhelp.c](ateconomic/ateconhelp.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [ateconio.c](ateconomic/ateconio.c.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [ateconindicator.c](ateconomic/ateconindicator.c.md) | 3 | 0 | 0 | 0 | 2 | not started |
| [ateconParamIO.c](ateconomic/ateconParamIO.c.md) | 5 | 0 | 0 | 0 | 0 | not started |
| [atFishMIP_OSP_Bridge.c](ateconomic/atFishMIP_OSP_Bridge.c.md) | 10 | 5 | 5 | 6 | 2 | not started |
| [atFishMIP_OSP_PluginAdapter.cpp](ateconomic/atFishMIP_OSP_PluginAdapter.cpp.md) | 2 | 0 | 0 | 1 | 1 | not started |
| [include_headers](ateconomic/include_headers.md) | 3 | 0 | 0 | 1 | 0 | not started |
| [_lower_confidence_ECON](ateconomic/_lower_confidence_ECON.md) | 0 | 0 | 0 | 0 | 1 | not started |
| [_lower_confidence_ECIO](ateconomic/_lower_confidence_ECIO.md) | 0 | 0 | 0 | 0 | 1 | not started |

### atharvest/ — 32 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atHarvest.c](atharvest/atHarvest.c.md) | 7 | 0 | 0 | 0 | 4 | not started |
| [atHarvestParamIO.c](atharvest/atHarvestParamIO.c.md) | 3 | 0 | 0 | 1 | 0 | not started |
| [atHarvestCatch.c](atharvest/atHarvestCatch.c.md) | 3 | 1 | 1 | 1 | 2 | not started |
| [atHarvestDiscards.c](atharvest/atHarvestDiscards.c.md) | 6 | 0 | 0 | 2 | 1 | not started |
| [atHarvestSetup.c](atharvest/atHarvestSetup.c.md) | 4 | 0 | 0 | 0 | 1 | not started |
| [atHarvestImposedCatch.c](atharvest/atHarvestImposedCatch.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atHarvestTS.c](atharvest/atHarvestTS.c.md) | 3 | 0 | 0 | 0 | 1 | not started |
| [atHarvestIndex.c](atharvest/atHarvestIndex.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atHarvestAnnual.c](atharvest/atHarvestAnnual.c.md) | 3 | 0 | 0 | 1 | 0 | not started |
| [include_headers](atharvest/include_headers.md) | 1 | 0 | 0 | 0 | 0 | not started |

### atimplementation/ — 8 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atImplementationAnnual.c](atimplementation/atImplementationAnnual.c.md) | 4 | 1 | 1 | 1 | 2 | not started |
| [atImplementationSetup.c](atimplementation/atImplementationSetup.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atImplementationParamIO.c](atimplementation/atImplementationParamIO.c.md) | 3 | 1 | 1 | 1 | 0 | not started |

### atmanage/ — 164 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atManageTier.c](atmanage/atManageTier.c.md) | 56 | 23 | 23 | 24 | 8 | not started |
| [atManage.c](atmanage/atManage.c.md) | 29 | 6 | 6 | 8 | 5 | not started |
| [atManageIndex.c](atmanage/atManageIndex.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atSS3assess.c](atmanage/atSS3assess.c.md) | 3 | 1 | 1 | 1 | 1 | not started |
| [atManageAnnual.c](atmanage/atManageAnnual.c.md) | 23 | 11 | 11 | 11 | 3 | not started |
| [atManageSetup.c](atmanage/atManageSetup.c.md) | 9 | 1 | 1 | 1 | 4 | not started |
| [atManageParamIO.c](atmanage/atManageParamIO.c.md) | 7 | 1 | 1 | 2 | 2 | not started |
| [atPGMSY.c](atmanage/atPGMSY.c.md) | 18 | 10 | 10 | 11 | 0 | not started |
| [atRlink.c](atmanage/atRlink.c.md) | 1 | 0 | 0 | 0 | 1 | not started |
| [atRlinkRBC.c](atmanage/atRlinkRBC.c.md) | 8 | 5 | 5 | 5 | 1 | not started |
| [atRlinkRAssess.c](atmanage/atRlinkRAssess.c.md) | 5 | 2 | 2 | 2 | 1 | not started |
| [atRlinkRedus.c](atmanage/atRlinkRedus.c.md) | 2 | 0 | 0 | 0 | 1 | not started |
| [atManageIO.c](atmanage/atManageIO.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [_lower_confidence_MNGIO](atmanage/_lower_confidence_MNGIO.md) | 0 | 0 | 0 | 0 | 1 | not started |

### atSS3Link/ — 81 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atSS3LinkIO.c](atSS3Link/atSS3LinkIO.c.md) | 33 | 17 | 17 | 19 | 5 | not started |
| [atSS3Test.c](atSS3Link/atSS3Test.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atSS3DataGen.c](atSS3Link/atSS3DataGen.c.md) | 24 | 8 | 8 | 10 | 0 | not started |
| [atCloseKin.c](atSS3Link/atCloseKin.c.md) | 15 | 6 | 6 | 7 | 0 | not started |
| [atSS3Util.c](atSS3Link/atSS3Util.c.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [atSS3Link.c](atSS3Link/atSS3Link.c.md) | 4 | 0 | 0 | 0 | 0 | not started |

### atphysics/ — 71 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [aticeIO.c](atphysics/aticeIO.c.md) | 11 | 2 | 2 | 2 | 5 | not started |
| [attempsalt.c](atphysics/attempsalt.c.md) | 8 | 1 | 1 | 1 | 1 | not started |
| [atphysics.c](atphysics/atphysics.c.md) | 6 | 1 | 1 | 1 | 0 | not started |
| [atphysIO.c](atphysics/atphysIO.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atgeomIO.c](atphysics/atgeomIO.c.md) | 3 | 0 | 0 | 0 | 2 | not started |
| [attracerIO.c](atphysics/attracerIO.c.md) | 3 | 0 | 0 | 0 | 2 | not started |
| [atsourcesink.c](atphysics/atsourcesink.c.md) | 3 | 1 | 1 | 1 | 2 | not started |
| [atvertgeom.c](atphysics/atvertgeom.c.md) | 2 | 1 | 1 | 1 | 0 | not started |
| [atagetracerIO.c](atphysics/atagetracerIO.c.md) | 4 | 1 | 1 | 1 | 0 | not started |
| [attransport.c](atphysics/attransport.c.md) | 2 | 0 | 0 | 0 | 1 | not started |
| [atparamIO.c](atphysics/atparamIO.c.md) | 4 | 1 | 1 | 1 | 0 | not started |
| [atepiIO.c](atphysics/atepiIO.c.md) | 1 | 0 | 0 | 0 | 2 | not started |
| [atbioirrig.c](atphysics/atbioirrig.c.md) | 2 | 1 | 1 | 1 | 0 | not started |
| [atLandIO.c](atphysics/atLandIO.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atindex.c](atphysics/atindex.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atdietIO.c](atphysics/atdietIO.c.md) | 1 | 1 | 1 | 1 | 0 | not started |
| [atvmix.c](atphysics/atvmix.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atsettle.c](atphysics/atsettle.c.md) | 1 | 1 | 1 | 1 | 0 | not started |
| [athdiff.c](atphysics/athdiff.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atbioturb.c](atphysics/atbioturb.c.md) | 4 | 2 | 2 | 2 | 0 | not started |
| [atsuspension.c](atphysics/atsuspension.c.md) | 4 | 1 | 1 | 2 | 1 | not started |
| [atsaturation.c](atphysics/atsaturation.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [atswr.c](atphysics/atswr.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [_lower_confidence_PHY3](atphysics/_lower_confidence_PHY3.md) | 0 | 0 | 0 | 0 | 2 | not started |

### atlantisUtil/ — 48 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atUtilXML.c](atlantisUtil/atUtilXML.c.md) | 16 | 1 | 1 | 1 | 0 | not started |
| [atUtilGroupIO.c](atlantisUtil/atUtilGroupIO.c.md) | 8 | 2 | 2 | 2 | 0 | not started |
| [atUtil.c](atlantisUtil/atUtil.c.md) | 6 | 0 | 0 | 0 | 0 | not started |
| [atUtilFisheryXML.c](atlantisUtil/atUtilFisheryXML.c.md) | 3 | 0 | 0 | 0 | 3 | not started |
| [atUtilIO.c](atlantisUtil/atUtilIO.c.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [atUtilArray.c](atlantisUtil/atUtilArray.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [atUtilhelp.c](atlantisUtil/atUtilhelp.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atUtilFisheryIO.c](atlantisUtil/atUtilFisheryIO.c.md) | 2 | 0 | 0 | 0 | 2 | not started |
| [include_headers](atlantisUtil/include_headers.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [_lower_confidence_UTL2](atlantisUtil/_lower_confidence_UTL2.md) | 0 | 0 | 0 | 0 | 1 | not started |

### atlantismain/ — 13 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atlantismain.c](atlantismain/atlantismain.c.md) | 8 | 0 | 0 | 0 | 3 | not started |
| [include_headers](atlantismain/include_headers.md) | 5 | 0 | 0 | 0 | 0 | not started |
| [_lower_confidence_MAIN](atlantismain/_lower_confidence_MAIN.md) | 0 | 0 | 0 | 0 | 1 | not started |

### ConvertAtlantis/ — 48 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atBioltoXML.c](ConvertAtlantis/atBioltoXML.c.md) | 17 | 1 | 1 | 2 | 4 | not started |
| [atCreateXML.c](ConvertAtlantis/atCreateXML.c.md) | 7 | 0 | 0 | 0 | 1 | not started |
| [atManagetoXML.c](ConvertAtlantis/atManagetoXML.c.md) | 5 | 0 | 0 | 0 | 0 | not started |
| [atAssesstoXML.c](ConvertAtlantis/atAssesstoXML.c.md) | 4 | 1 | 1 | 1 | 1 | not started |
| [atHarvesttoXML.c](ConvertAtlantis/atHarvesttoXML.c.md) | 5 | 0 | 0 | 0 | 1 | not started |
| [atEconomicXML.c](ConvertAtlantis/atEconomicXML.c.md) | 4 | 0 | 0 | 0 | 0 | not started |
| [atGroupsXML.c](ConvertAtlantis/atGroupsXML.c.md) | 1 | 1 | 1 | 1 | 0 | not started |
| [atRunXML.c](ConvertAtlantis/atRunXML.c.md) | 4 | 1 | 1 | 1 | 0 | not started |
| [atImplementationtoXML.c](ConvertAtlantis/atImplementationtoXML.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [_lower_confidence_CNV1](ConvertAtlantis/_lower_confidence_CNV1.md) | 0 | 0 | 0 | 0 | 2 | not started |
| [_lower_confidence_CNV2](ConvertAtlantis/_lower_confidence_CNV2.md) | 0 | 0 | 0 | 0 | 1 | not started |

### atCLAMLink/ — 22 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atCLAMComm.c](atCLAMLink/atCLAMComm.c.md) | 5 | 1 | 1 | 1 | 1 | not started |
| [atCLAMImplement.c](atCLAMLink/atCLAMImplement.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [atCLAMLinkIO.c](atCLAMLink/atCLAMLinkIO.c.md) | 8 | 3 | 3 | 3 | 0 | not started |
| [atCLAMManage.c](atCLAMLink/atCLAMManage.c.md) | 7 | 1 | 1 | 1 | 2 | not started |
| [_lower_confidence_LINK](atCLAMLink/_lower_confidence_LINK.md) | 0 | 0 | 0 | 0 | 1 | not started |

### atlink/ — 22 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atComms.c](atlink/atComms.c.md) | 4 | 1 | 1 | 1 | 0 | not started |
| [atlinkconversion.c](atlink/atlinkconversion.c.md) | 6 | 3 | 3 | 3 | 1 | not started |
| [atlinkexport.c](atlink/atlinkexport.c.md) | 7 | 3 | 3 | 3 | 0 | not started |
| [atlinkimport.c](atlink/atlinkimport.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [include_headers](atlink/include_headers.md) | 2 | 0 | 0 | 0 | 0 | not started |

### atbrokerlink/ — 25 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [atBrokerLinkInit.c](atbrokerlink/atBrokerLinkInit.c.md) | 3 | 0 | 0 | 1 | 0 | not started |
| [LinkageInterface.c](atbrokerlink/LinkageInterface.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [Deserialiser.c](atbrokerlink/Deserialiser.c.md) | 8 | 1 | 1 | 1 | 0 | not started |
| [ImportExportData.c](atbrokerlink/ImportExportData.c.md) | 7 | 2 | 2 | 3 | 0 | not started |
| [network.c](atbrokerlink/network.c.md) | 3 | 0 | 0 | 0 | 1 | not started |
| [include_headers](atbrokerlink/include_headers.md) | 2 | 0 | 0 | 0 | 0 | not started |

### sjwlib/ — 64 findings

| Review file | Findings | Conf H & Sev H | ⭐ | Sev H | Low-conf notes | Status |
|---|---|---|---|---|---|---|
| [memory.c](sjwlib/memory.c.md) | 4 | 0 | 0 | 0 | 1 | not started |
| [datafile.c](sjwlib/datafile.c.md) | 9 | 1 | 1 | 1 | 2 | not started |
| [dfcoords.c](sjwlib/dfcoords.c.md) | 9 | 0 | 0 | 0 | 1 | not started |
| [mapproj.c](sjwlib/mapproj.c.md) | 4 | 1 | 1 | 1 | 0 | not started |
| [polyline.c](sjwlib/polyline.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [nrcdflib.c](sjwlib/nrcdflib.c.md) | 5 | 0 | 0 | 0 | 1 | not started |
| [txt_param.c](sjwlib/txt_param.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [timeseries.c](sjwlib/timeseries.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [keyfile.c](sjwlib/keyfile.c.md) | 3 | 0 | 0 | 0 | 1 | not started |
| [convertFileFormats.c](sjwlib/convertFileFormats.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [xytoij.c](sjwlib/xytoij.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [geodetic.c](sjwlib/geodetic.c.md) | 1 | 0 | 0 | 0 | 1 | not started |
| [time.c](sjwlib/time.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [dfeval.c](sjwlib/dfeval.c.md) | 2 | 0 | 0 | 0 | 1 | not started |
| [netcdf.c](sjwlib/netcdf.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [ptrack.c](sjwlib/ptrack.c.md) | 3 | 0 | 0 | 0 | 0 | not started |
| [diffusion.c](sjwlib/diffusion.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [weight_fn.c](sjwlib/weight_fn.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [vgrid.c](sjwlib/vgrid.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [win32.c](sjwlib/win32.c.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [quit.c](sjwlib/quit.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [erfc.c](sjwlib/erfc.c.md) | 1 | 0 | 0 | 0 | 0 | not started |
| [include_headers](sjwlib/include_headers.md) | 2 | 0 | 0 | 0 | 0 | not started |
| [_lower_confidence_SJW1](sjwlib/_lower_confidence_SJW1.md) | 0 | 0 | 0 | 0 | 1 | not started |
| [_lower_confidence_SJW2](sjwlib/_lower_confidence_SJW2.md) | 0 | 0 | 0 | 0 | 3 | not started |
| [_lower_confidence_SJW3](sjwlib/_lower_confidence_SJW3.md) | 0 | 0 | 0 | 0 | 4 | not started |

## Files reviewed with no findings

The review checked these and logged nothing. No review file was created for them.

- **atecology/atLandProcess.c** — No issues found. Functions checked: Land_PrimaryProduction, Land_HabitatState, Ecology_Land_Biology_Process (body is all commented out; unused locals `k`, `maxdeep`, `totaldeep` only).
- **atecology/init.c** — No issues found. The entire file (lines 1-935) is commented out (every line starts with `//`), so it contains no live code. Functions present but inactive: init_tr2ex, build_box, set_boxes, set_cells,…
- **ateconomic/ateffortquota.c** — No issues found. Functions checked: Total_Effort_Quota_Price, Effort_Quota_trade (body entirely commented out; it repeats the QUOT-05/06/07/08/09 patterns, so fix those too if it is ever re-enabled).
- **ateconomic/atEconomicUtil.c** — No issues found. Functions checked: Get_Deemed_Value_Scale, Get_Expected_Profit.
- **ateconomic/ateconomic.c** — No issues found. Functions checked: Economic_Annual, Economics.
- **atharvest/atHarvestIO.c** — No issues found. Functions checked: Harvest_Report_Annual_Stats, Harvest_Report_Monthly_Stats, Open_Harvest_Output_Files, Close_Harvest_Output_Files, initAnnCatchFile, initAnnRecCatchFile, initAnnDisc…
- **atimplementation/include/*.h** — No issues found. Checked atImplementation.h, atImplementationLib.h.
- **atmanage/atManageMPATS.c** — No issues found. Functions checked: Setup_MPA_Lists, Load_MPAlist_From_File.
- **atmanage/include/*.h** — No issues found. Files checked: atManage.h, atManageLib.h, atManagePrivate.h, atRlink.h. (`EFFORTchange` is defined twice in atManagePrivate.h:54 and :66. This is legal because the second is a tentati…
- **atSS3Link/include/*.h** — No issues found. Headers checked: atSS3LinkLib.h, atSS3LinkPrivate.h, atSS3Util.h. Minor: `GenMnorm` is declared in atSS3Util.h but not defined in atSS3Util.c (the real one is `Util_GenMnorm`). It is …
- **atphysics/athydromod.c** — No issues found. Functions checked: hydro_init, get_hydro, open_hydro, close_hydro, closeHydroFinal.
- **atphysics/atsummaryIO.c** — No issues found. Functions checked: createBMSummaryDataFile, writeBMSummaryInfo, writeBMSummaryData, writeBMSummaryEpiData. (Only the `%d`-for-`sizeof` format nit at lines 310/408, covered in Lower-co…
- **atphysics/atdiagIO.c** — No issues found. Functions checked: freeBMDiagInfo, writeBMDiagInfo, writeBMDiagData (readBMDiagInfo is commented out).
- **atphysics/atfishstatIO.c** — No issues found. Functions checked: readBMFisheriesInfo, freeBMFisheriesInfo, writeBMFisheriesInfo, writeBMFisheriesData.
- **atphysics/atdeposition.c** — No issues found. Functions checked: deposit. (See Lower-confidence notes on the maintain_sedlayer clamp.)
- **atphysics/atreadbm.c** — No issues found that affect live code. Functions checked: readMSEBoxModel, freeMSEBoxModel (both unused; see notes).
- **atphysics/atgas.c** — No issues found. Functions checked: gasExchange.
- **atphysics/atsedprops.c** — No issues found. Functions checked: sedprops, calc_por, calc_css, calc_er.
- **atphysics/atvdiff.c** — No issues found. Functions checked: vdiffBMwc.
- **atphysics/atboundary.c** — No issues found. Functions checked: boundaries.
- **atphysics/atdecay.c** — No issues found. Functions checked: decayBM.
- **atphysics/atprofile.c** — No issues found. Functions checked: bioprofile.
- **atphysics/atPhysicsModule.c** — No issues found. Functions checked: allocatePhysicsModule, closePhysicsFile, freePhysicsStruct.
- **atphysics/include/*.h** — No issues found. Files checked: atPhysics.h, atPhysicsModule.h.
- **atlantisUtil/atUtilUnix.c** — No issues found. Functions checked: _spawnlp.
- **atlantismain/process.h** — No issues found. Functions checked: `_spawnlp` prototype (fixed 3-arg declaration of a normally variadic Windows API; not called in the audited code).
- **ConvertAtlantis/atFisheriesXML.c** — No issues found. Functions checked: Convert_Fisheries_To_XML.
- **ConvertAtlantis/include/convertXML.h** — No issues found. Functions checked: all prototypes against their definitions (Parse_File, Parse_File_Single_Line, Create_*_ParamXML, Convert_*_To_XML, Fishery_Last_XMLFunction, Species_NodeXML, create…
- **atCLAMLink/atCLAMLink.c** — No issues found. Not compiled: it is absent from Makefile.am `c_sources`, its header comment says "This page is not used", and it references the undefined `numOutputOptions`. Functions checked: none (…
- **atCLAMLink/atCLAMLinkSetup.c** — No issues found. Allocation orders were checked against the Init calls in atCLAMManage.c:199-203 and are consistent ([ind][scen][strat][state], [scen][strat][ind], [scen][strat]). Functions checked: C…
- **atCLAMLink/include/atCLAMLink.h** — No issues found. It defines (rather than declares) the module globals and initialised tables, which is acceptable because it is included only by atCLAMLinkIO.c. Functions checked: none (data definitio…
- **atbrokerlink/NetworkError.c** — No issues found. Functions checked: printErrorMessage.
- **sjwlib/contour.c** — No issues found. Functions checked: lencmp, contour, conbox, contri, con_between, labelcontour.
- **sjwlib/pointsourcesink.c** — No issues found. Functions checked: readPointSourceSink, locatePointSourceSink, parse_location.
- **sjwlib/drandom.c** — No issues found. Functions checked: drandom, ran1, ran2, ran3.
- **sjwlib/waterprops.c** — No issues found. Functions checked: water_vapourp (table has 101 entries, indices 0..100 valid).
- **sjwlib/colourtable.c** — No issues found. Functions checked: readCT, writeCT, valcolour.
- **sjwlib/gridmetric.c** — No issues found. Functions checked: gridmetric, geog_gridmetric.
- **sjwlib/gridangle.c** — No issues found. Functions checked: gridangle, geog_gridangle.
- **sjwlib/spline.c** — No issues found. Functions checked: dspline, dsplint.
- **sjwlib/text_input.c** — No issues found. Functions checked: nextline, iscommentline, isblankline, parseline, flushline.
- **sjwlib/ellipt_coord.c** — No issues found. Functions checked: ellipt_coord.
- **sjwlib/decay.c** — No issues found. Functions checked: decay_forward, decay_centered, decay_backward, decay_exact.
- **sjwlib/polar_coord.c** — No issues found. Functions checked: polar_coord.
- **sjwlib/cfft.c** — No issues found. Functions checked: cfft.
- **sjwlib/rect_coord.c** — No issues found. Functions checked: rect_coord.
- **sjwlib/gridmisc.c** — No issues found. Functions checked: CentreToCorner.
- **sjwlib/ppbfetch.c** — No issues found. Functions checked: ppbfetch (hard-coded path `/home/walker/...` but function is unused).
- **sjwlib/warn.c** — No issues found. Functions checked: warn, quiet.

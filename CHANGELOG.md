# Changelog

All notable changes to this repository will be documented in this file.
As changed, removed, or added.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.6722] - 2026-05-22 - Commit 94ccf9b

### Added
- Created md file with a table listing Atlantis models in use, location, and contact person
- GitHub actions to build and run SETAS when a pull request is triggered
- Templates for bug reporting, feature requests and pull requests that standardize content for maintainers and reviewers. 
- Parameter min_pool_cont for the Contaminant module, which sets the threshold for minimum contaminant concentration. This addition decouples min_pool a global parameter from the contaminant submodule. Need to add min_pool_cont as a parameter if using track_contaminants 
- Instructions to add new features or bug fixes to the user manual. The documentation should be updated directly after the feature or bug is incorporated in a new release
- Code to identify the Git release and return the release version
- mFC fisheries produce no harvest when flagdisplace is on. The Effort array is used as a scalar multiplier for mFC catch but was initialized to 0.0 instead of 1.0. Added flagMFCdisplace_id to identify mFC fisheries and initialize their Effort to 1.0. Modified the fishery activation check to prevent mFC fisheries with displacement from being skipped. Files: atlantisboxmodel.h, atManageSetup.c, atManage.c


### Changed
- In atlantis/PreRules.am‎ refactors the code to detect the underlying OS and then "turn" on the appropriate flag
- Readme and CHANGELOG are updated
- CumDisplaceEffort accumulation was in dead code path. Accumulation was in the if(!bm->EffortModelsActive) block, but EffortModelsActive is true when any fishery has flageffortmodel > 0 (including forced effort = 11). Placed accumulation in the EffortModelsActive code path instead.
File: atManage.c
- Wrong box index for MPA scaling in readts_effort branch. MPA scale was read from bm->MPA[bm->current_box][fishery_id] instead of bm->MPA[ij][fishery_id]. current_box was always 0, so every box got box 0's MPA value. Changed to bm->MPA[ij][fishery_id].
File: atManage.c
- orig_FCpressure set after MPA scaling instead of before. In the readts_effort branch, orig_FCpressure was assigned after FCpressure *= mpa_scale, so Effort_Displacement computed orig_FCpressure - FCpressure = 0. Set orig_FCpressure before MPA scaling.
File: atManage.c
- All displacement logic was gated by TempCPUE < mEff_thresh. For readts_effort fisheries, TempCPUE is never calculated (stays 0.0), and with mEff_thresh = 0.0 the condition 0.0 < 0.0 is false. The manual (Section 15.6.1) states displacement should occur both for low CPUE and when MPAs are imposed. Added MPA as alternative trigger: if ((TempCPUE < mEff_thresh) || (MPA[ij][fishery_id] < 1.0)). File: atManage.c (Effort_Displacement function)
- Inverted MPA check in displacement destination scoring. Adjacent boxes were scored using MPA_check = 1.0 - bm->MPA[nb][fishery_id]. For open boxes (MPA=1.0) this gives 0.0, zeroing out their biomass attractiveness. Changed to MPA_check = bm->MPA[nb][fishery_id] so open boxes retain full attractiveness.
File: atManage.c (Effort_Displacement function)
- Biomass comparison blocked MPA-imposed displacement. Displacement was conditional on maxfishthere > fishhere. Closed MPA boxes can have high target biomass, causing this condition to fail. The manual states displacement "will still occur" when MPAs are imposed. Separated MPA-imposed displacement (unconditional, find best open neighbour) from CPUE-triggered displacement (conditional on better biomass elsewhere). File: atManage.c (Effort_Displacement function)
- Renamed output file from OutDisplaceEffort.txt to OutDetailedEffort.txt (atHarvestIO.c)
- Detailed in the following Google group thread: https://groups.google.com/g/atlantis-ecosystem-model/c/gNJN6AXaM88, the way the allocation is done does not match the way BiTAC_sp is used. For instance:
AtlantisTrunk/atlantis/atmanage/atManage.c. Line 2382 in 9f99af0, bm->BiTAC_sp[bim][nreg][sp][now_id] = 0.0; shows that the second dimension is nreg, but the allocation used the number of species for the second dimension.
- SVN release number was printed in log file. This has been replaced by GitHub version release in an earlier PR. For this to work git is required as a system dependency. This will not work for the SETAS workflow. This version number also needs to be printed to standard out. This PR fixes the standard out printing

### Removed 
- Extra printfs in atUtils.c used for debugging 
- Extra printfs in atbiology.c that was creating a memory error
- atbiology.c
	//printf("printing tracer\n");
	//printf("WC - %d, box: %d, layer: %d, %s - bm->atEcologyModule->localTracer[i] = %.20e, fluxFlag = %d\n",i, bm->current_box, bm->current_layer, Varname[i], localWCTracers[i], Fluxflag[i]);
- Rremove .DS_Store files and .Rhistory. .DS_Store are in the .gitignore but once they are tracked they need to be removed explicitly (https://docs.github.com/en/get-started/git-basics/ignoring-files).
- Existing quit statement in at_demography.c for when maternal transfer was on and had externally reproducing species , but the necessary code has been added and the quit statement is no longer necessary

## [3.6721] - 2026-01-12 - Commit 51f186d

### Added
- Missing lines in Free_Contaminants in atContaminant.c 
	    free(bm->contaminantStructure[cIndex]->sp_avoid);
        free(bm->contaminantStructure[cIndex]->sp_K_avoid);
        free(bm->contaminantStructure[cIndex]->sp_L);
        free(bm->contaminantStructure[cIndex]->sp_A);
        free(bm->contaminantStructure[cIndex]->sp_B);
        free(bm->contaminantStructure[cIndex]->sp_L_r);
        free(bm->contaminantStructure[cIndex]->sp_A_r);
        free(bm->contaminantStructure[cIndex]->sp_B_r);
- this_habitat definition to atContaminants.c that was triggering a memory error
- Alloc for ReprodThresh 
- Midway adding maternal transfer of contaminants - have it in recruits and migrants.
- Migrant return for contaminants. Using average contaminants not a distribution as yet
- Cases to Get_ContamReproductionEffects: No effect (0) Equation 13 from Lovindeer et al. (1) In Vitro (2) Salmon paper effect (3) activated by flag_contamReprodModel
- Core base code for ice-based fish recruitment in lakes. 
- Hooks for contaminant transfer in migrants
- Manual within the code base and also getting git to ignore some of the mac specific DS_store and workspace files.
- Maternal transfer of contaminants during recruitment and suckling (if have maternal care). Code added to atdemography during reproduction and to movement. Currently only for vertebrates.
- StingArray to AtlantisXMLAttributeTypeStrings so contaminants don't cause odd symbol.
- Code to stop Groups with flag_recruit set to 11 or 12 getting environmental amplification of recruits even when norm_larval_distrib is set to 0. To turn this new code off and have the amplification on for all species you set flag_replicated_old to 1
- Check to make sure only called on min_wgt if age structured and not biomass (was causing memory sed fault)
- Made sure effort displacement was possible when using forced effort time series
- Fixed a ContamClosed free data leak issue and added a compile flag to make file build rules to allow address checking when using gcc compiled code on MacOSX
- Species specific half life
- Global flag
- flag_contam_halflife_spbased - set to 1 for species specific half life
- Species-specific parameter - decay_half_life, formated the same way as LD50
- targ_refE and forage_refE to get Albi Rovellini's changes to work completely on read-in
- New ecocap params to setas harvest.prm
- childgroupingnode for ecocap params in the reading part
- Holly's code to use SSB for HCR calculations
- Clause for tier0 which should now trigger no mFC scaling
- tier14 for truncated HCR shape, Alaska pollock style
- gitignore for build files
     
### Changed
- Corrected integer check in atManageparamIO.c Line 1183 
- Fixed some memory alloc and free around contaminants
- Displaced effort for F and maternal contam transfer
- This has displaced effort in place for mFC forced harvest (overloading Effort matrix to store the displaced F).
- Misplaced parentheses in attime.c
- Missing ";" Line 1752 to atmovement.c to fix compilation error
- Made change to spawnmove, so spawnmove = 1.0 initially
- Corrected HowFar which returned 0
- Fixed a typo in rec_var
- Merge pull request #7 from Atlantis-Ecosystem-Model/alaia-contam-newoptions
- Update gitignore for mac only files
- Updated fisheries part of the manual for the code update - flaginfringe, flagdisplace, flagmpa
- Effort displacement and MPAs when have forced effort. 
- Move mFCchange definition. To help with compile errors on linux server brute force changed where mFCchange is defined 
- Moved from atHaevestLib.h to atlantismain.c with external definition included in atlantisboxmodel.h to pass to other sublibraries where its used). Compils fine on Mac lets see if it fixes the linux issue.
- Updated the ice read so working for lake ice read
- Brought across all the changes from Albi Rovellini's branch as of 17th Dec - to do with using SSB in HCR and getting the ecosystem catch cap to work for him
- Relaxed sanity checks for max and min weight and biomass
- For ecosystem cap calculations updated how rolling is done (not cumulative average but queue average)
- Fixed mixup between syst_cap_calc_method and M_est_method in atdemography.c
- Fixed output file management for ecosystem cap
- Fixed Ecosystem cap initial code
- Fixed some missing +=, reinitialised min_wgt, max_wgt, min_B, max_B and sorted a missing loop on mpa_scale and tje rescale of mFC

### Removed
- printfs in atContaminants that were causing too large log files
- .Proj folder from last commit
- Free() as had some duplicates
- Contaminants subnode as was causing a segfault as somehow getting mixed with a fisheries node so removed the subnode and left it with the Scenario Options node. Works on tests with Salish Sea, Puget Sound and Iceland
- Debugging fprints causing issues once contaminants turned off. 


## [3.6720] - 2024-11-28 - Commit 9b3fa4c

### Changed
- First commit to GitHub, moving from CSIRO SVN
- Updated how fixed M can be read in for system caps
- Overloaded the assess_nat_mort species param. id no assess.prm included in the run command then will look for FixedAssessMort in the harvest.prm otherwise it will overwrite with the assess_nat_mort vector values from assess.prm.

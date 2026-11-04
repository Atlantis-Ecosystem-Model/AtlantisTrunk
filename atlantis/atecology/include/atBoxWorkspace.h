/**
 \file
 \brief Per-box working values that each thread needs its own copy of.

 Every water box overwrites these values before it uses them (layer properties,
 Q10-corrected rates, adaptive step counter). When boxes run on several threads
 (OpenMP build), each thread keeps its own copy. In a normal build
 BOX_THREAD_LOCAL is empty and these are plain globals, exactly as before.

 These names must only be declared here, so every file sees the same storage class.
 */
#ifndef ATBOXWORKSPACE_H_
#define ATBOXWORKSPACE_H_

#ifdef _OPENMP
#define BOX_THREAD_LOCAL __thread
#else
#define BOX_THREAD_LOCAL
#endif

/* Layer and box properties set by Box_Bio_Process and Properties_At_Depth */
extern BOX_THREAD_LOCAL double sporosity, surf_stress, wcLayerThick, smLayerThick, iceLayerThick;
extern BOX_THREAD_LOCAL double Susp_Sed, DRdepth, O2depth, newO2depth, current_layer_sed, eddy_strength,
	BioirrigEnh, BioturbEnh, Turbatn_contribs, Irrig_contribs, cell_depth,
	H2Otemp, current_SALT, current_PH, Bact_stim, current_depth, area_reef, area_flat, LocalRugosity,
	area_canyon, area_box, area_soft, current_ARAG, current_WIND;

/* Parameters after Q10 adjustments (rewritten for every box by Apply_Q10_Corrections) */
extern BOX_THREAD_LOCAL double r_DL, r_DR, r_DC, r_DON, r_DSi, K_nit, R_0, R_D;

/* Tracking values for one step per day animals that impact on multi-dt per day groups */
extern BOX_THREAD_LOCAL double RecycledNHglobal, wcFlux2global, wcFlux3global, wcFlux2aglobal, wcFlux3aglobal,
	wcFlux4global, smFlux2global, epiFlux2global, wcFishingGlobal, epiFishingGlobal;

/* Adaptive step counter and boundary flag of the box being processed */
extern BOX_THREAD_LOCAL int it_count, waterboundary;

#endif /* ATBOXWORKSPACE_H_ */

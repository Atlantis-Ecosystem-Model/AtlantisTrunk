/*********************************************************************

 File:           atbioirrig.c

 Created:        Wed May 17 13:45:02 EST 1995

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        This file contains routines which represent the bioirrigation processes.
 These processes primarily move dissolved tracers within the sediments
 and between the sediments and lowest water column layer.

 Arguments:      See below

 Returns:        void

 Revisions:      8/8/2004 EA Fulton
 Ported across the code from the southeast (sephys) model

 17/11/2004 EA Fulton
 Converted original routine definitions from
 void
 routine_name(blah,blahblah)
 int blah;
 double blahblah;

 to

 void routine_name(int blah, double blahblah)
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Prototypes for local routines */
void dissol_diff(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed);
void exchange(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed);
void injection(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed);

/* Overall routine called by physics() */
void bioirrig(MSEBoxModel *bm, double ***newwc, double ***newsed) {
	int b = 0;

	if (verbose)
		fprintf(stderr, "Entering bioirrig\n");

	/* Loop over each box */
	for (b = 0; b < bm->nbox; b++) {
		Box *bp = &bm->boxes[b];
		SedModel *sm = &bm->boxes[b].sm;

		if (bp->type != BOUNDARY && bp->type != LAND) {

            if (!_finite(newwc[24][0][Salinity_i])) {
                printf("Time: %e Salinity in bottom water is nan - while doing box %d in bioirrig\n", bm->dayt, b);
                fflush(stdout);
                fflush(stderr);
            }
            
			/* Diffusion */
			dissol_diff(bm, bp, sm, newwc[b], newsed[b]);
            
            if (!_finite(newwc[24][0][Salinity_i])) {
                printf("Time: %e Salinity in bottom water is nan - while doing bioirrig B\n", bm->dayt);
                fflush(stdout);
                fflush(stderr);
            }

			/* Exchange */
			exchange(bm, bp, sm, newwc[b], newsed[b]);
            
            if (!_finite(newwc[24][0][Salinity_i])) {
                printf("Time: %e Salinity in bottom water is nan - while doing bioirrig C\n", bm->dayt);
                fflush(stdout);
                fflush(stderr);
            }

			/* Injection */
			injection(bm, bp, sm, newwc[b], newsed[b]);
            
            if (!_finite(newwc[24][0][Salinity_i])) {
                printf("Time: %e Salinity in bottom water is nan - while doing bioirrig D\n", bm->dayt);
                fflush(stdout);
                fflush(stderr);
            }

		}
	}
}

/* This routine implements diffusion of dissolved tracers within the
 * sediments, for one sediment model (1 box).
 */
void dissol_diff(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed) {
	int k = 0;
	int n = 0;
	int tk = sm->topk; /* Index of top sediment layer */
	double *c = alloc1d(sm->nz); /* Temporary storage */

	/* Nothing to do if less than 2 layers */
	if (tk >= sm->nz - 1) {
		free1d(c);
		return;
	}

	/* Calculate diffusion coefficients at layer interfaces */
	for (k = tk + 1; k < sm->nz; k++) {
		sm->dissol_kz[k] = bm->bi_dissol_kz * sm->biodens * sm->irrigenh * bioprofile(bm, sm->gridz[k], sm->biodepth);
	}
	sm->dissol_kz[tk] = sm->dissol_kz[sm->nz] = 0.0;

	/* Loop over tracers */
	for (n = 0; n < bm->ntracer; n++) {

		/* Skip tracers which are not dissolved and or
		 * not allowed in sediment. Also skip the water tracer
		 */
		if (!(bm->tinfo[n].dissol) || !(bm->tinfo[n].insed) || (n == bm->waterid) || !bm->tinfo[n].can_be_moved)
			continue;

		/* Collect tracer values for each layer */
		/* FIX - does porosity matter here? */
        for (k = tk; k < bp->sm.nz; k++) {
            c[k] = newsed[k][n];
        }

		/* Calculate diffusion */
		diffusion1d(sm->nz - tk, &c[tk], &sm->cellz[tk], &sm->dissol_kz[tk], &sm->gridz[tk], bm->dt, bm->a_sed);

		/* Store new values */
        for (k = tk; k < bp->sm.nz; k++) {
            newsed[k][n] = c[k];
        }

	}

	free1d(c);
}

void exchange(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed) {
	int k = 0;
	int n = 0;
	double sum;
	int tk = sm->topk; /* Index of top sediment layer */
	double wvol = bp->volume[0]; /* Volume of bottom water column layer */
	double *rate = alloc1d(sm->nz); /* Temporary storage */

	/* Total exchange rate (m3 s-1)
	 * for bottom water column layer
	 */
	double erate = bm->bi_exchange * bp->area * sm->biodens * sm->irrigenh;

	/* Calculate activity for each layer and integrate */
	sum = 0.0;
	for (k = tk; k < sm->nz; k++) {
		rate[k] = sm->dz[k] * bioprofile(bm, sm->cellz[k], sm->biodepth);
		sum += rate[k];
	}
	if (sum <= 0.0) {
		free1d(rate);
		return;
	}

	/* Distribute exchange over sediment layers */
    for (k = tk; k < sm->nz; k++) {
        rate[k] *= erate / sum;
    }

	/* Loop over tracers */
	for (n = 0; n < bm->ntracer; n++) {

		/* Skip tracers which are not dissolved and or
		 * not allowed in sediment. Also skip the water tracer
		 */
		if (!(bm->tinfo[n].dissol) || !(bm->tinfo[n].insed) || (n == bm->waterid) || !bm->tinfo[n].can_be_moved) {
			//printf("skipping over %s\n", bm->tinfo[n].name);
			continue;
		}

		/* Loop over each sediment layer, starting at the
		 * uppermost one and working down
		 */
		if (strcmp(bm->tinfo[n].name, "Oxygen") == 0) { /* Oxygen case */
			for (k = tk; k < sm->nz && rate[k] > 0; k++) {
				/* Calculate new concentrations of oxygen in sediment and water
				 column as per method outlined by Parslow rather than the one
				 Walker had implemented for all dissolved groups */
				/* Volume of pore water */
				double oxvol = sm->oxdepth * bp->area;
				/* Uses same form as before but adjusts volume being considered
				 to that due to oxygen depth rather than just the sediment layer
				 - FIX this is OK if only have one sediment layer in implementation
				 (as was case for PhD work) must be corrected if more than one layer used.
				 The 0.00000000001 was added so that pvol was never exactly equal to
				 zero as you must divide by it*/
				double pvol = sm->porosity[k] * (oxvol / sm->volume[k]) + small_num;
				/* Constants in exponential formula */
				double ek = rate[k] * (1.0 / wvol + 1.0 / pvol);
				double ce = (newwc[0][n] * wvol + newsed[k][n] * pvol) / (wvol + pvol);
				/* New water column concentration */
				double cw = ce + decay_exact(newwc[0][n] - ce, ek, bm->dt);
				/* New sediment value  - part changed here */
				double cs = newsed[k][n] - (wvol / pvol) * (cw - newwc[0][n]);

				/* FIX - may have to modify this further so not just modifying
				 depth considered to the oxygen depth instead of sediment depth,
				 rather may also have to modify such that sediment value = 0 (as
				 it does at the oxygen depth) rather than just using the sediment's
				 oxygen value above the sediment depth */

				/* Store results */
				newwc[0][n] = cw;
				newsed[k][n] = cs;
			}
		} else {
			for (k = tk; k < sm->nz && rate[k] > 0; k++) {
				/* Calculate new concentrations in both water column
				 * and this sediment layer using analytic solution
				 * for two boxes exchanging at a given rate. This
				 * ignores the fact that other layers will also be
				 * simultaneously exchanging with the water column.
				 * The full solution is a system of coupled differential
				 * equations, requiring a matrix inversion to solve -
				 * effort which does not seem warranted here.
				 */
                double orig_wc = newwc[0][n];
                double orig_sed = newsed[k][n];
				/* Volume of pore water */
				double pvol = sm->porosity[k] * sm->volume[k] + small_num;
				/* Constants in exponential formula */
				double ek = rate[k] * (1.0 / wvol + 1.0 / pvol);
				double ce = (orig_wc * wvol + orig_sed * pvol) / (wvol + pvol);
				/* New water column concentration */
                double decay_amt = decay_exact(newwc[0][n] - ce, ek, bm->dt);
				double cw = ce + decay_amt;
				/* New sediment value */
				double cs = newsed[k][n] - (wvol / pvol) * (cw - newwc[0][n]);

				/* Store results */
				newwc[0][n] = cw;
				newsed[k][n] = cs;

				/**
				 //if((bm->current_box == bm->checkbox) && (!strcmp(bm->tinfo[n].name, "NH3") || !strcmp(bm->tinfo[n].name, "NO3")))
                if (!_finite(newwc[0][Salinity_i])) {
                    fprintf(bm->logFile, "Time: %e new bioirrig2 %s newwc: %e sed layer %d newsed: %e as cw: %e ce: %e decay_amt: %e orig_wc: %e wvol: %e orig_sed: %e pvol: %e rate: %e, ek: %e\n", bm->dayt, bm->tinfo[n].name, newwc[0][n], k, newsed[k][n], cw, ce, decay_amt, orig_wc, wvol, orig_sed, pvol, rate[k], ek);
                }
                /*/

			}
		}
	}

	free1d(rate);
}

void injection(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed) {
	int k = 0;
	int n = 0;
	double sum, step1;
	int tk = sm->topk; /* Index of top sediment layer */
	double *rate = alloc1d(sm->nz); /* Temporary storage */

	/* Total injection rate (m3 s-1)
	 * from bottom water column layer
	 */
	double irate = bm->bi_injection * bp->area * sm->biodens * sm->irrigenh;

	/* Calculate activity for each layer and integrate */
	sum = 0.0;
	for (k = tk; k < sm->nz; k++) {
        step1 = bioprofile(bm, sm->cellz[k], sm->biodepth);
        rate[k] = sm->dz[k] * step1;
		sum += rate[k];
        
        if((!(_finite(sum))) || (sum && (sum < small_num))) {
            fprintf(bm->logFile,"Time %e box %d-sedk%d rate: %e dz: %e bioprofile: %e cellz: %e biodepth: %e\n", bm->dayt, bp->n, k, rate[k], sm->dz[k], step1, sm->cellz[k], sm->biodepth);
        }
	}
    
	//if (sum <= 0.0) {
    if (sum < small_num) {
		free1d(rate);
		return;
    }

	/* Distribute injection over sediment layers */
    for (k = tk; k < sm->nz; k++) {
		rate[k] *= irate / sum;
        
    }

	/* Loop over tracers */
	for (n = 0; n < bm->ntracer; n++) {
		/* Storage for volume and concentration entering
		 * each layer from layer below. These are initially
		 * set to zero, as the loop below starts at the deepest
		 * sediment layer.
		 */
		double bvol = 0.0;
		double bconc = 0.0;

		/* Skip tracers which are not dissolved and or
		 * not allowed in sediment. Also skip the water tracer
		 */
		if (!(bm->tinfo[n].dissol) || !(bm->tinfo[n].insed) || (n == bm->waterid) || !bm->tinfo[n].can_be_moved)
			continue;

		/* Loop up through each sediment layer */
		for (k = sm->nz - 1; k >= tk; k--) {
			/* Pore water volume in this layer */
			double pvol = sm->porosity[k] * sm->volume[k];
			/* Injected volume from bottom water layer */
			double wvol = rate[k] * bm->dt;
			/* New concentration - this is made up from
			 * stuff already here, stuff coming from injected
			 * water, and stuff from layer below. This assumes that
			 * everything somehow gets well mixed within each
			 * layer.
			 */
			newsed[k][n] = (pvol * newsed[k][n] + wvol * newwc[0][n] + bvol * bconc) / (pvol + wvol + bvol);

			/* The pore volume doesn't change, so the rest
			 * goes into the next layer up
			 */
			bvol = bvol + wvol;
			bconc = newsed[k][n];
            
            if(!(_finite(bvol))) {
                fprintf(bm->logFile, "Time: %e box %d-sedk%d bvol: %e wvol: %e rate: %e dt: %e\n", bm->dayt, bp->n, k, bvol, wvol, rate[k], bm->dt);
            }
            
		}

		/* After doing all the sediment layers, we have to
		 * adjust the concentration in the bottom water
		 * column layer. We have injected irate*dt volume
		 * of water, and have got back bvol (hopefully
		 * these are equal!).
		 */
		/* DEBUGGING  check volumes */
        
		if (fabs(irate * bm->dt / bvol - 1.0) > 1e-10)
            quit("Time: %e nox %d, injection: Injected and returned volumes don't match (%s t=%f injected=%f as irate: %e, returned bvol=%f\n", bm->dayt, bp->n, bp->label, bm->t, irate * bm->dt, irate, bvol);

		newwc[0][n] += (bconc * bvol - newwc[0][n] * irate * bm->dt) / bp->volume[0];

		/**
		 if((bm->current_box == bm->checkbox) && (!strcmp(bm->tinfo[n].name, "NH3") || !strcmp(bm->tinfo[n].name, "NO3")))
		 fprintf(bm->logFile, "Time %e box %d new bioirrig3 %s, newwc[0]: %e\n", bm->dayt, bm->current_box, bm->tinfo[n].name, newwc[0][n]);
		 **/
	}

	free1d(rate);

}

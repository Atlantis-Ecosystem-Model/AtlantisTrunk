/*********************************************************************

 File:           atvmix.c

 Created:        Thur Oct 23 12:09:31 EST 2003

 Author:         Beth Fulton
 CSIRO Division of Marine Research

 Purpose:        This file contains routines which
 perform forced vertical mixing (to represenmt upwelling).
 vertical_mixing - forced water column mixing.

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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Forced vertical mixing in the water column -
 to simulate enhanced mixing due to upwelling */
void vertical_mixing(MSEBoxModel *bm, double ***newval) {
	int b = 0;
	int k = 0;
	int n = 0;
	double eddy_scale = 0;
	double *c = alloc1d(bm->wcnz);
	double *mrate = alloc1d(bm->wcnz); /* Temporary storage of rates */
	double cvol, wvol, lvol, seasonal_scale, vertmix_scale;
	double nowtime = bm->t / 86400.0;

	if (verbose)
		fprintf(stderr, "Entering vertical_mixing\n");

	seasonal_scale = max(1.0, bm->mix_season_kz*sin(2.0*3.1415926*(nowtime-31.0)/365.0));

	/* Initialise vertical flux tracker */
	for (b = 0; b < bm->nbox; b++) {
		for (k = 0; k < bm->wcnz; k++) {
			bm->boxes[b].vflux[k] = 0;
		}
	}

	/* Loop over each box */
	for (b = 0; b < bm->nbox; b++) {
		Box *bp = &bm->boxes[b];

		/* Eddy scalar - including scaling the eddy contribution by the
		 * scalar reflecting the weighting of eddy contribution
		 * to vertical mixing*/
		if (bm->eddy_vertmix)
			eddy_scale = bp->eddy * bm->eddy_scale * (1.0 - bm->eddy_mixscale);
		else
			eddy_scale = 1.0;

		if(eddy_scale < 0.0)
			eddy_scale = 0.0;

		
        /* Get vertical mixing scalar - if variable through time */
        vertmix_scale = 1.0;
        if (bm->use_VertMixfiles) {
            vertmix_scale = bp->vmix_scale;
        }
        
        /* Calculate total mixing rate (m3 s-1) from lower layer up */
        for (k = 0; k < bp->nz; k++){
			mrate[k] = bm->mix_injection * bp->vertmix * vertmix_scale * eddy_scale * seasonal_scale * bp->volume[k];
		}

		/* Reset bottom rate to zero as can't upwell from the sediments here */
		mrate[0] = 0.0;

		if (bp->type != BOUNDARY && bp->type != LAND) {
			/* Loop over each tracer */
			for (n = 0; n < bm->ntracer; n++) {
				/* Skip tracers not in water column and not dissolved */
				if (!(bm->tinfo[n].dissol) || !(bm->tinfo[n].inwc) || (n == bm->waterid))
					continue;

				/* Collect tracer values for each layer */
				for (k = 0; k < bp->nz; k++) {
					c[k] = newval[b][k][n];

                    /**
                    if(bm->dayt > 1092.0) {
                        fprintf(bm->logFile, "day: %e, box: %d-%d starting c: %e\n", bm->dayt, b, k, c[k]);
                        fflush(bm->logFile);
                    }
                    **/
				}
				/* Calculate exchanges  - ignoring bottom most water column
				 layer as obviously can't have upwelling through the seafloor */
				for (k = 1; k < bp->nz; k++) {
					/* Volume of current cell */
					wvol = bp->volume[k];
					/* Volume of cell below */
					lvol = bp->volume[k - 1];
					/* Volume injected */
					cvol = mrate[k] * bm->dt;

					/* Slab mixing - crude but necessary unless
					 refined vertical fluxes are available */
					if ((strcmp(bm->tinfo[n].name, "Oxygen") == 0) && (c[k] > c[k - 1])) {
						/* Check amount to exchange is not more than in the cells */
						if (wvol < cvol)
							cvol = wvol - small_num;
						if (lvol < cvol)
							cvol = lvol - small_num;

						/* New concentration in current cell
						 (average of what's there and what's mixed back) */
						c[k] = ((wvol - cvol) * c[k] + c[k - 1] * cvol) / (wvol + small_num);

						/* New concentration in sink cell
						 (average of what's there and what's transferred amount added) */
						c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);

						/* Update vertical flux trackers */
						bp->vflux[k - 1] += cvol;
						bp->vflux[k] -= cvol;

						/**
                        //if(( b == 47 ) && (bm->newmonth)) {
                            fprintf(bm->logFile,"day: %e, box: %d-%d, %s c: %e, c[%d]: %e, cvol: %e, lvol: %e, wvol: %e, mrate: %e, vflux-%d: %e\n",
                                    bm->dayt, b, k, bm->tinfo[n].name, c[k], k-1, c[k-1], cvol, lvol, wvol, mrate[k], k - 1, bp->vflux[k - 1]);
                            fflush(bm->logFile);
                        }
						**/
					} else {
						/* Check amount to exchange is not more than in the cells */
						if (lvol < cvol)
							cvol = lvol - small_num;
						if (wvol < cvol)
							cvol = wvol - small_num;

						/* New concentration in current cell
						 (average of what's there and what's mixed back) */
						c[k] = (cvol * c[k - 1] + (wvol - cvol) * c[k]) / (wvol + small_num);

						/* New concentration in sink cell
						 (average of what's there and what's transferred amount added) */
						c[k - 1] = ((lvol - cvol) * c[k - 1] + cvol * c[k]) / (lvol + small_num);

						/* Update vertical flux trackers */
						bp->vflux[k] += cvol;
						bp->vflux[k - 1] -= cvol;

						/**
                        //if(!strcmp(bm->tinfo[n].name, "NH3"))
                        //if(b == bm->checkbox)
                        //if(( b == 47 ) && (bm->newmonth))
                        //if(bm->dayt > 1092.0) {
                            fprintf(bm->logFile, "day: %e, box: %d-%d, %s c: %e, c[%d]: %e, cvol: %e, lvol: %e, wvol: %e, mrate: %e, vflux-%d: %e\n",
                                    bm->dayt, b, k, bm->tinfo[n].name, c[k], k-1, c[k-1], cvol, lvol, wvol, mrate[k], k, bp->vflux[k]);
                            fflush(bm->logFile);
                        }
                         **/
						 
					}

					/**
					 if(b == bm->checkbox)
                         fprintf(bm->logFile, "day: %e, box: %d, %s conc[%d]: %e, conc[%d]: %e, cvol: %e, mrate: %e, \n",
                              bm->dayt, b, bm->tinfo[n].name, k, c[k], k-1, c[k-1], cvol, mrate[k]);
					 **/
                    
                    if (!_finite(c[k])) {
                        fprintf(bm->logFile, "day: %e, box: %d-%d, %s c starts: %e c: %e, c[%d]: %e, cvol: %e, lvol: %e, wvol: %e, mrate: %e, vflux-%d: %e\n",
                                bm->dayt, b, k, bm->tinfo[n].name, newval[b][k][n], c[k], k-1, c[k-1], cvol, lvol, wvol, mrate[k], k, bp->vflux[k]);
                        fflush(bm->logFile);
                        printf("day: %e, box: %d-%d, %s c starts: %e c: %e, c[%d]: %e, cvol: %e, lvol: %e, wvol: %e, mrate: %e, vflux-%d: %e\n",
                                bm->dayt, b, k, bm->tinfo[n].name, newval[b][k][n], c[k], k-1, c[k-1], cvol, lvol, wvol, mrate[k], k, bp->vflux[k]);
                        fflush(stdout);
                        fflush(stderr);
                        quit("vertical_mixing - %s in %d:%d at time %e is non finite\n", bm->tinfo[n].name, b, k, bm->dayt);
                    }


				}

				/* If mixing with deep ocean allow replenishment lowest water column box */
				if (bm->mix_deep && (bm->mix_deep_depth > bm->boxes[b].botz)) {
					if (!strcmp(bm->tinfo[n].name, "NH3"))
						c[0] = bp->bottNH;
					else if (!strcmp(bm->tinfo[n].name, "NO3"))
						c[0] = bp->bottNO;
                    else if ((!strcmp(bm->tinfo[n].name, "Oxygen")) && bm->mix_deep_O2)
                        c[0] = bp->bottO2;
                    else if (!strcmp(bm->tinfo[n].name, "Si"))
						c[0] = bp->bottSi;
					else if (!strcmp(bm->tinfo[n].name, "MicroNut"))
						c[0] = bp->bottFe;
					else if (bm->track_atomic_ratio && (strcmp(bm->tinfo[n].name, "Phosphorus") == 0))
						c[0] = bp->bottP;
					else if (bm->track_atomic_ratio && (strcmp(bm->tinfo[n].name, "Carbon") == 0))
						c[0] = bp->bottC;

					/* Old code - used to just reset the value without mixing, could lead to overaccumulation
					 c[0] = newval[b][0][n];
					 */
				}

				/* Store new values */
                for (k = 0; k < bp->nz; k++) {
                    newval[b][k][n] = c[k];
                    
                    if (!_finite(newval[b][k][n])) {
                        fflush(bm->logFile);
                        fflush(stdout);
                        fflush(stderr);
                        quit("vertical_mixing - %s in %d:%d at time %e is non finite\n", bm->tinfo[n].name, b, k, bm->dayt);
                    }
                }
			}
		}
	}
    
    fflush(stdout);
    fflush(stderr);

	free1d(c);
	free1d(mrate);
}

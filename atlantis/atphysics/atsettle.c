/*********************************************************************

    File:           atsettle.c

    Created:        Mon Feb 20 16:58:01 EST 1995

    Author:         Stephen Walker
                    CSIRO Division of Oceanography

    Purpose:        Implement settling for Box Model

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


					28/Nov/2008 Bec Gorton
					Moved the static variable masstosed into the atPhysicsModule
					so it can be freed at the end of the model run.
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Prototypes for routines used here */
void    advect_up(Box *bp, double dist, int n, double **newwc);
double  advect_down(Box *bp, double dist, int n, double **newwc);

void settleBMwc(MSEBoxModel *bm, double ***newwc, double ***newsed, FILE *llogfp)
{
    int b = 0;
    int n = 0;

	if( verbose )
		fprintf(stderr,"Entering settleBMk\n");

    /* Allocate memory for mass to be deposited to sediments.
     * WARNING - this is only done once, so may fail
     * if several different MSEBoxModels are active
     */
    if( bm->atPhysicsModule->masstosed == NULL ){
    	bm->atPhysicsModule->masstosed = (double **)alloc2d(bm->ntracer,bm->nbox);
	}

    /* Set mass to be deposited to sediments to zero */
    for(b=0; b<bm->nbox; b++)
    	for(n=0; n<bm->ntracer; n++)
    		bm->atPhysicsModule->masstosed[b][n] = 0.0;

    /* Loop over each tracer */
    for(n=0; n<bm->ntracer; n++) {
		double v = bm->tinfo[n].svel + bm->tinfo[n].xvel;

        //fprintf(bm->logFile,"Time: %e doing %s with v: %e\n", bm->dayt, bm->tinfo[n].name, v);
        
        
		/* Skip diagnostic tracers */
		if( (!bm->tinfo[n].inwc && !bm->tinfo[n].insed) || !bm->tinfo[n].can_be_moved)
			continue;

		/* Skip tracers with zero settling velocity */
        if( v == 0.0 )
			continue;

			/* If settling velocity is positive,
			 * particles are buoyant and rise up through
			 * the water column. There is no exchange with
			 * the sediments in this case
			 */
		else if( v > 0.0 ) {
			/* Loop over each box */
			for(b=0; b<bm->nbox; b++)
			if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type)
				advect_up(&bm->boxes[b],v*bm->dt,n,newwc[b]);
		}

			/* If settling velocity is negative,
			 * particles sink towards the bottom.
			 * The flux of tracer through the bottom
			 * in the lowest water column layer is added to the
			 * uppermost sediment layer if the tracer is allowed
		 * to exist in the sediments. Otherwise it is retained
		 * in the lowest water column layer.
		 *
		 * FIX - may need to be some weighted average of both
		 * these outcomes if we implement a shear dependent
		 * deposition probability - see Teisson (1991)
		 * J Hydraulic Res.
			 */
		else if( v < 0.0 ) {  /* Sinking particles */
			/* Loop over each box */
			for(b=0; b<bm->nbox; b++) {
			if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type != LAND) {
				double m = advect_down(&bm->boxes[b],-v*bm->dt,n,newwc[b]);


				if((bm->debug == debug_deposit) && (bm->dayt > bm->checkstart)){
					fprintf(llogfp,"Time: %e, depositing %e of %s which settles as %e\n", bm->dayt, m, bm->tinfo[n].name, v);
				}

				if( bm->tinfo[n].insed ){
					/* Store mass to be deposited to sediment layer */
					bm->atPhysicsModule->masstosed[b][n] = m;
				}
				else
				/* Retain in lowest water column layer */
				newwc[b][0][n] += m/bm->boxes[b].volume[0];
			}
			}
		}
    }

	 /* Loop over each box, depositing material to sediments */
    for(b=0; b<bm->nbox; b++) {
        if( bm->boxes[b].type != BOUNDARY && bm->boxes[b].type != LAND) {
            deposit(bm,&bm->boxes[b],bm->atPhysicsModule->masstosed[b],newwc[b],newsed[b],llogfp);
        }
    }

    /* Re-calculate layer coordinates */
    layer_coords(bm, llogfp);
}

/* Routine to calculate upward motion of a buoyant tracer
 * for 1 box. This routine allows for the fact that the
 * distance traversed in a single time step may be greater
 * than a single layer thickness
 */
void advect_up(Box *bp, double dist, int n, double **newwc)
{
    double tm = 0;    /* Total mass through top of each layer */
    double bm = 0;    /* Total mass through bottom of each layer */
    double botz = bp->botz;
    int k = 0;

    /* Loop down through water column */
    tm = 0.0;
    for(k=bp->nz-1; k>0; k--) {
	/* Maximum distance from which particles can come */
	double d = min(dist, bp->gridz[k]-botz);
	int kk;

	/* Calculate total mass through bottom of this layer */
	bm = 0.0;
	for(kk=k-1; d>0.0 && kk>=0; kk--) {
	    double dz = min(d, bp->gridz[kk+1]-bp->gridz[kk]);
	    bm += newwc[kk][n]*dz*bp->area;
	    d -= dz;
	}
	/* Change in tracer concentration */
	newwc[k][n] += (bm-tm)/bp->volume[k];
	/* Transfer bottom flux to top of next cell down */
	tm = bm;
    }
    /* Calculation for bottom layer */
    newwc[0][n] += (0.0-tm)/bp->volume[0];
}

/* Routine to calculate downward motion of a sinking tracer
 * for 1 box. This routine allows for the fact that the
 * distance traversed in a single time step may be greater
 * than a single layer thickness. This routine returns the
 * mass of tracer which should be deposited to the
 * uppermost sediment layer.
 */
double advect_down(Box *bp, double dist, int n, double **newwc)
{
    double tm = 0;    /* Total mass through top of each layer */
    double bm = 0;    /* Total mass through bottom of each layer */
    double topz = bp->gridz[bp->nz];
    double botz = bp->gridz[0];
    double masstosed = 0;
    double d = 0;
    int k = 0;

    /* Calculate mass through bottom of water column */
    d = min(dist, topz-botz);
    bm = 0.0;
    for(k=0; d>0.0 && k<bp->nz; k++) {
        double dz = min(d, bp->dz[k]);
        bm -= newwc[k][n]*dz*bp->area;
        d -= dz;
    }
    /* Store return value */
    masstosed = -bm;

    /* Loop up through water column */
    for(k=0; k<bp->nz; k++) {
	/* Maximum distance from which particles can come */
	double d = min(dist, topz-bp->gridz[k+1]);
	int kk;

	/* Calculate total mass through top of this layer */
	tm = 0.0;
	for(kk=k+1; d>0.0 && kk<bp->nz; kk++) {
	    double dz = min(d, bp->dz[kk]);
	    tm -= newwc[kk][n]*dz*bp->area;
	    d -= dz;
	}
	/* Change in tracer concentration */
	newwc[k][n] += (bm-tm)/bp->volume[k];

	/* FIX - Avoid rounding error */
	if( newwc[k][n] < 0.0 ) newwc[k][n] = 0;

	/* Transfer top flux to bottom of next cell up */
	bm = tm;
    }

    /* Return mass of tracer which should be added to top sediment layer */
    return(masstosed);
}

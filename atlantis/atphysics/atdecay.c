/*********************************************************************

    File:           atdecay.c
    
    Created:        Tue Dec 6 11:24:32 EST 1994
    
    Author:         Stephen Walker
                    CSIRO Division of Oceanography
    
    Purpose:        Implement decay process for box model.
    
    Arguments:      

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
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>

/* Pointer to decay routine */
double (*decay)(double c, double k, double dt);

void decayBM(MSEBoxModel *bm, double ***newwc, double ***newsed)
{
    int b = 0;
    int k = 0;
    int n = 0;
	int allowsed = bm->decay_sed;
    int allowwc = bm->decay_wc;
	double dt = bm->dt;
    double dks, seddks;

	if( verbose )
		fprintf(stderr,"Entering decayBM\n");

    /* Loop over each tracer */
    for(n=0; n<bm->ntracer; n++) {
		double dk = bm->tinfo[n].decay;
		double seddk = bm->tinfo[n].decay * bm->decay_sed_scale;
		int inwc = bm->tinfo[n].inwc;
		int insed = bm->tinfo[n].insed;

		if( dk > 0.0 ) {
			/* Loop over each box */
			for(b=0; b<bm->nbox; b++) {
				Box *bp = &bm->boxes[b];
				SedModel *sm = &bm->boxes[b].sm;

				if( bp->type != BOUNDARY && bp->type != LAND ) {
					/* Loop through water column */
                    dks = dk * bm->boxes[b].decay_scalar;
					if( inwc && allowwc ){
						for(k=0; k<bp->nz; k++)
							newwc[b][k][n] = (*decay)(newwc[b][k][n],dks,dt);
					}
					/* Loop through sediment layers */
                    seddks = seddk * bm->boxes[b].decay_scalar;
					if( insed && allowsed){
						for(k=sm->topk; k<sm->nz; k++){
							newsed[b][k][n] = (*decay)(newsed[b][k][n],seddks,dt);
						}
					}
				}	
			}
		}
    }
}

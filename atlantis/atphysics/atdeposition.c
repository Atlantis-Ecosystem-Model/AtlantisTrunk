/*********************************************************************

    File:           atdeposition.c
    
    Created:        Tue May 9 15:06:33 EST 1995
    
    Author:         Stephen Walker
                    CSIRO Division of Oceanography
    
    Purpose:        Routine to deal with deposition of material
		            from water column onto surface of sediments
		            for a single box.
    
    Arguments:
                    bm            Box model pointer
                    bp            Box pointer
		            m[n]          Mass of tracer n to be deposited
		            newwc[k][n]   New water colum tracer values
		            newsed[k][n]  New sediment tracer values


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

void deposit(MSEBoxModel *bm, Box *bp, double *m, double **newwc, double **newsed, FILE *llogfp)
{
    int n = 0;
    double depvol = 0;
    double solidvol = 0;
    double depdz = 0;
    double newpor = 0;
    SedModel *sm = &bp->sm;

    /* Loop over each tracer to calculate total volume,
     * total thickness and porosity of material to be
     * deposited to this box. Total volume is calculated
     * as the sum of the volumes of each tracer deposited.
     * These volumes are calculated from the mass deposited
     * using a specified concentration for new deposits (the
     * i_conc attribute). This thus ignores the fact that
     * where mixtures occur, fine material may fit
     * between coarse particles. Porosity of the new deposit
     * is calculated from the concentrations of each tracer and the
     * bulk density of the tracer particles.
     */
    depvol = solidvol = 0.0;
    for(n=0; n<bm->ntracer; n++) {
		if( m[n] > 0 ) {
			depvol += m[n]/bm->tinfo[n].i_conc;
			solidvol += m[n]/bm->tinfo[n].b_dens;

			if((bm->debug == debug_deposit) && (bm->dayt > bm->checkstart)){
				fprintf(llogfp, "Time: %e, box%d depvol: %e (m: %e, i_conc: %e) and solidvol: %e, b_dens: %e\n",
					bm->dayt, bp->n, depvol, m[n], bm->tinfo[n].i_conc, solidvol, bm->tinfo[n].b_dens);
			}
		}
    }

    /* If no deposits, nothing else to do */
    if( depvol <= 0.0 )
	return;

    depdz = depvol/bp->area;
    newpor = 1.0 - solidvol/depvol;

	if((bm->debug == debug_deposit) && (bm->dayt > bm->checkstart))
		fprintf(llogfp, "Time: %e, box%d newpor: %e\n", bm->dayt, bp->n, newpor);

    /* Check for sensible porosity value - this should not be a problem
     * as the bulk density (b_dens) of a tracer should be greater than
     * its initial deposit concentration (i_conc)
     */
    if( newpor < 0.0 || newpor > 1.0 )
        quit("deposit: porosity outside range 0.0 to 1.0 (box %d, time %.10g)\n",bp->n, bm->t);

    /* Add particulate material to sediment,
     * creating new layers as necessary
     */
    while( depdz > 0.0 ) {
	/* Top sediment layer index */
	int tk = sm->topk;
	/* Old volume of top layer */
	double oldvol = sm->volume[tk];
	/* Old water volume of top layer */
	double oldwatervol = oldvol*sm->porosity[tk];
	/* Thickness of material to deposit in this layer */
	double thisdz = min(depdz, sm->maxdz - sm->dz[tk]);
	double thisdepvol;
	double frac;
	double newvol;
	double newwatervol;

	/* If this layer is already too thick, don't deposit any material in it
	 * The code at the end of the loop will then form a new layer
	 */
	if( thisdz < 0 )
	    thisdz = 0;

	/* Deposit all material if no empty layers are
	 * left, or if less than the minimum thickness
	 * would be left over. I could just deposit all the material
	 * in the current surface layer and let later physics code split
	 * the layers if necessary, but there is less mixing of material
	 * between layers if new layers are created as needed here.
	 */
	if( tk == 0 || depdz-thisdz < sm->mindz )
	    thisdz = depdz;

	/* Volume of new deposit in this layer */
	thisdepvol = thisdz*bp->area;
	/* Fraction of total material */
	frac = thisdepvol/depvol;
	/* New volume of top layer */
	newvol = oldvol + thisdepvol;
	/* New water volume of top layer */
	newwatervol = oldwatervol + thisdepvol*newpor;

	/* Update tracer concentrations */
    for(n=0; n<bm->ntracer; n++) {
	    if( bm->tinfo[n].insed ) {
		/* Calculate new concentration for particulates.
		 * The terms here (in order of appearance) are:
		 *    -  the exsting mass of tracer in the sediment
		 *    -  the mass to be deposited
		 *    -  the mass associated with pore water entrained from
		 *       the overlying water column. The volume of entrained
		 *       water is the new deposit volume times the
		 *       porosity of the new deposit.
		 * Note that adding the last term (the particles associated
		 * with entrained pore water) will slightly change the concentration
		 * of tracer in the sediment, and should cause a further slight
		 * change in the deposit volume. The volume effect is ignored here,
		 * so that the initial deposit density may be slightly higher than
		 * specified. This should not otherwise cause a problem.
		 */
		if( bm->tinfo[n].partic )
		    newsed[tk][n] = ( newsed[tk][n]*oldvol
				     +frac*m[n]
				     +newwc[0][n]*thisdepvol*newpor )/newvol;
		/* Calculate new concentration for dissolved tracers,
		 * including entrained water from the bottom of
		 * the water column.
		 */
		else if( bm->tinfo[n].dissol )
		    newsed[tk][n] = (newsed[tk][n]*oldwatervol
			      +newwc[0][n]*thisdepvol*newpor)/newwatervol;
	    }
	}

	/* Update layer volume, thickness, fill time and porosity */
	sm->dz[tk] += thisdz;
	sm->volume[tk] = newvol;
	sm->filltime[tk] = bm->t;
	sm->porosity[tk] = newwatervol/newvol;

    if(bm->maintain_sedlayer) {
        if (sm->dz[tk] < sm->mindz)
            sm->dz[tk] = sm->mindz;
        if (sm->dz[tk] > sm->maxdz)
            sm->dz[tk] = sm->maxdz;
    }

    /* Form new layer if necessary */
	depdz -= thisdz;
	if( depdz > 0.0 ) {
	    if( tk > 0 )
		tk -= 1;
	    else
		/* As the code above stands, this should never happen */
		/* If space calculation above did not test for tk>0,
		 * this could happen, and we really should merge deeper layers
		 * to allow the creation of a new surface layer
		 */
		quit("deposit: Cannot create new sediment layer, box %d, time %.10g\n",bp->n, bm->t);
	    sm->topk = tk;
	    sm->volume[tk] = 0.0;
	    sm->dz[tk] = 0.0;
	    sm->filltime[tk] = bm->t;
	    for(n=0; n<bm->ntracer; n++)
		newsed[tk][n] = 0.0;
	}
    }

    /* Calculate updated sediment layer coordinates */
    //fprintf(bm->logFile,"Doing deposition in box %d \n");
    sed_layer_coords(sm, bm->logFile);

    /* Remove water from bottom water column layer
     * which has been entrained into new sediment deposit.
     * This alters the volume of the bottom water column layer,
     * This will change the concentrations of any tracers
     * which are not transferred to the sediments
     * (and those not allowed in the sediments).
     * Note that if the porosity of the new deposit is zero, 
     * no water is entrained, and the volume of the bottom
     * water column layer does not change. However, the z-coordinate
     * of the bottom should change, by the thickness of the deposit,
     * independently of the porosity.  The code which follows
     * only adjusts the volume (and hence by implication the bottom
     * z-coordinate) in proportion to the porosity.
     * This is consistent with the view that the
     * volume values in the water column refer to the
     * volume of water present, not the total volume
     * of water plus particulate tracers. If the volume was to
     * be the total volume, then we should adjust the water column
     * volumes in settleBMwc(), as particles leave each layer.
     * This is a minor effect, and is neglected in this version
     * of the model.
     */
    for(n=0; n<bm->ntracer; n++) {
	if( bm->tinfo[n].inwc && !bm->tinfo[n].insed )
	    newwc[0][n] *= bp->volume[0]/(bp->volume[0]-newpor*depvol);
        
 //       fprintf(bm->logFile, "Time: %e %s has new newwc: %e vol: %e newpor: %e depvol: %e scalar: %e\n",
 //               bm->dayt, bm->tinfo[n].name, newwc[0][n], bp->volume[0], newpor, depvol, bp->volume[0]/(bp->volume[0]-newpor*depvol));
        
    }
    bp->volume[0] -= newpor*depvol;
    if( bp->volume[0] <= 0.0 )
        quit("deposit: Bottom water column layer filled with sediment, box %d, t = %.10g\n",bp->n, bm->t);
    bp->dz[0] = bp->volume[0]/bp->area;
}

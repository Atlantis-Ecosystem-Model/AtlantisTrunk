/*********************************************************************

 File:           attransport.c

 Created:        Tue Feb 28 15:14:16 EST 1995

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routines to implement transports
 associated with the hydrodynamic exchange data.

 Arguments:      See below

 Returns:        void

 Revisions:      8/8/2004 EA Fulton
 Ported across the code from the southeast (sephys) model

 17/11/2004 EA Fulton
 Converted original routine definitions from
 void
 routine_name(blah,blah,blah)
 int blah;
 double blahblah;

 to

 void routine_name(int blah, double blahblah)

 28/3/2005 Added checks using bm->do_availflag (flag indicating
 whether to do the check at all) and bm->tinfo[n].flagid (flag
 saying whether the group is active) so that only active groups
 are advected.

 22-04-2009 Bec Gorton
 Moved the static file pointers and arrays into the atPhysicsModule
 structure so they can be freed at the end of the model run.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Prototypes */
void get_hydro(MSEBoxModel *bm);
FILE *initExportFile(MSEBoxModel *bm);
void writeExports(FILE *fp, MSEBoxModel *bm, double ***dtr);

/*********************************************************************/
void transportBM(MSEBoxModel *bm, double ***newwc, FILE *llogfp) {
	int totnz, diffnz;
	int b;
	int k, startk;
	long d;
	int n;
	double k_transcale, k_transcale_final, exchange_amt, e, orig_sink, orig_source;
	double tleft = bm->dt;

	if (verbose)
		printf("Entering transportBM\n");
    
    fflush(stdout);     /* in case stdout and stderr are the same so can track soem frees*/
    fflush(stderr);

	/* Initialise export file if necessary */
	if (!bm->atPhysicsModule->expfp)
		bm->atPhysicsModule->expfp = initExportFile(bm);

	/* Loop while more time remains in this transport time step */
	while (tleft > 0) {
		double dt;

		/* Get more hydro data if necessary */
		if (bm->hd.tleft <= 0)
			get_hydro(bm);

		/* Calculate time step allowed */
		dt = min(tleft, bm->hd.tleft);

		/* Set tracer and volume changes to zero and reset source sink info */
		for (b = 0; b < bm->nbox; b++) {
			for (k = 0; k < bm->wcnz; k++) {
				dvol[b][k] = 0.0;
				bm->boxes[b].hdsource[k] = 0.0;
				bm->boxes[b].hdsink[k] = 0.0;
				bm->boxes[b].eflux[k] = 0.0;
                if (bm->vert_mix != 1)
                    bm->boxes[b].vflux[k] = 0.0;  // Note zeroed in vertical_mixing() if vert_mix == 1
				for (n = 0; n < bm->ntracer; n++)
					dtr[b][k][n] = 0.0;
			}
		}

		/* Loop over each box to check exchanges
		 * FIX - This shouldn't necessarily be done
		 * each time through the loop. This really needs
		 * more thought, but is ok if the Hydrodynamic and
		 * Transport time steps are the same
		 */
		for (b = 0; b < bm->nbox; b++) {
			Box *bp = &bm->boxes[b];
			if (bp->numlayers < bp->nz) {
				warn("Increasing numlayers in box %d to %d (from %d)\n", b, bp->nz, bp->numlayers);
				bp->numlayers = bp->nz;
			} else if (bp->numlayers > bp->nz) {
				warn("Decreasing numlayers in box %d to %d (from %d)\n", b, bp->nz, bp->numlayers);
				bp->numlayers = bp->nz;
			}

			/* Loop through water column */
			if (!bm->cascade_flows)
				totnz = bp->nz;
			else
				totnz = bm->wcnz;

			for (k = 0; k < totnz; k++) {
				double dv = 0.0;
				/* Loop over destination cells */
				for (d = 0; d < bm->hd.dsize; d++) {
					if ((bp->edge_type == 2) && (bm->hd.exch[b][k][d] < 0)) // Reflective
						exchange_amt = -bm->hd.exch[b][k][d];
					else if ((bp->edge_type == 1) && (bm->hd.exch[b][k][d] > 0)) // Absorptive
						exchange_amt = -bm->hd.exch[b][k][d];
					else
						exchange_amt = bm->hd.exch[b][k][d]; // Standard (everything else)

					/* correct any difference in timesteps */
					if (bm->hd.b[b][k][d] >= 0)
						dv += dt * exchange_amt / bm->hd.dt;
				}
				/* Identify layer to consider - assume cascade happens in bottom water-layer*/
				if (totnz > bp->nz) {
					diffnz = totnz - bp->nz;
					startk = k - diffnz;
					if (startk < 0)
						startk = 0;
				} else
					startk = k;
				if (dv > bp->volume[startk]) {
					/* More water going out of box than was in it!! *
					if (verbose > 1){
						fprintf(stderr, "transport: Exchange %f greater than volume %f at box %d k = %d\n", dv, bp->volume[startk], b, startk);
						//abort();
					}
					*/
					/* Scale the exchanges to correct the problem. */
					for (d = 0; d < bm->hd.dsize; d++) {
						if (bm->hd.b[b][k][d] >= 0)
							bm->hd.exch[b][k][d] *= (0.999999 * bp->volume[startk]) / dv;
					}
				}
			}
		}

		/* Determine coefficient of scaling for exchanges (to correct for hyperdiffusion problem) */
		if (bm->scale_transport)
			k_transcale = bm->prcnt_exchange;
		else
			k_transcale = 1.0;

		/* Loop over each box to implement exchanges */
		for (b = 0; b < bm->nbox; b++) {
			Box *bp = &bm->boxes[b];
            
			/* If area correcting exchanges to avoid hyperdiffusion problem and
			 scale appropriately */
			if (bm->scale_transport == 2)
				k_transcale = bm->ka_exchange / bp->area;

			/* Loop through water column */
			if (!bm->cascade_flows)
				totnz = bp->nz;
			else
				totnz = bm->wcnz;

			for (k = 0; k < totnz; k++) {
				/* Identify layer to consider - assume cascade happens in bottom water-layer*/
				if (totnz > bp->nz) {
					diffnz = totnz - bp->nz;
					startk = k - diffnz;
					if (startk < 0)
						startk = 0;
				} else
					startk = k;
				/* Loop over destination cells */
				for (d = 0; d < bm->hd.dsize; d++) {

					/*
					 if ((bm->dayt < 10) || (bm->dayt > 580)){
					 fprintf(llogfp,"Time: %e, amt: %e, b: %d, k: %d, d: %d, bb: %d, kk: %d\n",
					 bm->dayt, bm->hd.exch[b][k][d], b, k, d, bm->hd.b[b][k][d], bm->hd.k[b][k][d]);
					 }
					 */

					if ((bm->hd.b[b][k][d] >= 0) && (bm->hd.exch[b][k][d])) {
                        
                        /* Indices for destination cell */
                        int bb = bm->hd.b[b][k][d];
                        int kk = bm->hd.k[b][k][d];
                        /* FIX - maybe should check bb and kk for validity here */
                        if ((bb < 0) || (kk < 0)) {
                            if(bm->mirror_invalid == true_mirror){
                                kk = k;  // out of desperation equate the source and destinaiton layers
                            } else if (bm->mirror_invalid == zero_invalid){
                                kk = 0;  // or just send everything to the bottom layers
                            } else {
                                quit("You have an exchange_amt of %e coming from cell box %d layer %d, destination entry %d, but you have destination box of %d and layer %d, which isn't valid\n", bm->hd.exch[b][k][d], b, k, d, bb, kk);
                            }

                        }

                        /* Exchange volume */
                        if ((bp->edge_type == 3) && (bm->hd.exch[b][k][d] < 0)) {
                            /* If negative base on the cell its leaving from if flow fields not symmetrical */
                            k_transcale_final = bm->boxes[bb].horizmix * k_transcale;
                        } else if ((bb == b) && (!bm->flag_replicated_old)) {
                            k_transcale_final = bm->boxes[bb].vertmix * k_transcale;
                        } else {
                            /* Otherwise assume symmetrical scaling of flows */
                            k_transcale_final = bp->horizmix * k_transcale;
                        }
                        
                        /* Deal with reflective and absorptive boundaries */
                        if ((bp->edge_type == 2) && (bm->hd.exch[b][k][d] < 0)) // Reflective
                            exchange_amt = -bm->hd.exch[b][k][d];
                        else if ((bp->edge_type == 1) && (bm->hd.exch[b][k][d] > 0)) // Absorptive
                            exchange_amt = -bm->hd.exch[b][k][d];
                        else
                            exchange_amt = bm->hd.exch[b][k][d]; // Standard (everything else)


                        if (isnan(exchange_amt)){
                            warn("Exchange from box %d, layer %d to box %d, layer %d is nan, Update your hydro forcing file. \n", b, k, bb, kk);
                            continue;
                        }
                        
                        e = k_transcale_final * dt * exchange_amt / bm->hd.dt;

                        if (e) {
                            /*
                            if(b == 14 || bb == 14)
                                fprintf(bm->logFile, "transport: exchange %f from box%d-%d to box %d-%d (nz: %d) with k_transcale: %e\n", e, b, k, bb, kk, bm->boxes[bb].nz, k_transcale_final);
                            */
                            /* Add water to destination cell and subtract from source cell */
                            if ((bb >= bm->nbox) || (kk >= bm->wcnz)) {
                                /* This destination cell doesn't exist - log an error and continue */
                                fprintf(llogfp,"WARNING - trying to advect water (amt: %e) to nonexistent box-layer combination (%d, %d) from box-layer (%d, %d) at time: %e (TofY: %d)\n", exchange_amt, bb, kk, b, k, bm->dayt, bm->TofY);
                            } else {
                                dvol[bb][kk] += e;
                                dvol[b][startk] -= e;

                                /**
                                if((bb == bm->checkbox || b == bm->checkbox))
                                    fprintf(llogfp,"Time: %e box%d-%d sending %.10g to box%d-%d so source vol %.10g and sink vol %.10g exchange_amt: %.10g k_transcale_final: %.10g dt: %.10g bm->hd.dt: %.10g\n", bm->dayt, b, startk, e, bb, kk, dvol[b][startk], dvol[bb][kk], exchange_amt, k_transcale_final, dt, bm->hd.dt);
                                **/
                            
                                /* Do the same for each tracer */
                                for (n = 0; n < bm->ntracer; n++) {
                                    if (bm->tinfo[n].partic && !bm->tinfo[n].passive) {
                                        /* Do nothng */
                                    } else {
                                        orig_sink = dtr[bb][kk][n];
                                        orig_source = dtr[b][startk][n];
                                        
                                        dtr[bb][kk][n] += e * newwc[b][startk][n];
                                        dtr[b][startk][n] -= e * newwc[b][startk][n];
                                    }

                                    /**/
                                    //if((bb == bm->checkbox || b == bm->checkbox)) {
                                    if ((!_finite(dtr[bb][kk][n])) || (!_finite(dtr[b][startk][n]))) {
                                        printf("Hit a nan, telling you what went wrong\n");                                        
                                        fflush(stdout);
                                        fflush(stderr);
                                        
                                        if (strcmp(bm->tinfo[n].name, "salt") == 0)
                                            fprintf(bm->logFile, "Time: %e dtr sink: %e, dtr source: %e, with orig_sink: %e and orig_source: %e - exchanging %e (e*newec) with source newwc %e source vol %e %s from box%d-%d to box%d-%d with e %e, k_transcale_final: %e, dt: %e, exchange_amt: %e, hd.dt: %e\n",
                                                bm->dayt, dtr[bb][kk][n], dtr[b][startk][n], orig_sink, orig_source, e*newwc[b][startk][n], newwc[b][startk][n], bp->volume[startk], bm->tinfo[n].name, b, startk, bb, kk, e, k_transcale_final, dt, exchange_amt, bm->hd.dt);
                                        fflush(bm->logFile);
                                        quit("Time: %e dtr sink: %e, dtr source: %e, with orig_sink: %e and orig_source: %e - exchanging %e (e*newec) with source newwc %e source vol %e %s from box%d-%d to box%d-%d with e %e, k_transcale_final: %e, dt: %e, exchange_amt: %e, hd.dt: %e\n",
                                             bm->dayt, dtr[bb][kk][n], dtr[b][startk][n], orig_sink, orig_source, e*newwc[b][startk][n], newwc[b][startk][n], bp->volume[startk], bm->tinfo[n].name, b, startk, bb, kk, e, k_transcale_final, dt, exchange_amt, bm->hd.dt);

                                    }
                                    /**/

                                }
                                /* Update source and sink counter */
                                if (e > 0) {
                                    bm->boxes[b].hdsource[startk]++;
                                    bm->boxes[bb].hdsink[kk]++;
                                    
                                    // Work out flush_index here
                                    bm->boxes[b].flush_in[startk] = bm->dayt;
                                    bm->boxes[bb].flush_out[kk] = bm->dayt;

                                } else {
                                    bm->boxes[bb].hdsource[kk]++;
                                    bm->boxes[b].hdsink[startk]++;
                                    
                                    // Work out flush_index here
                                    bm->boxes[bb].flush_in[kk] = bm->dayt;
                                    bm->boxes[b].flush_out[startk] = bm->dayt;

                                }
                                if ((bb == b) && (kk != k)) {
                                    /* Actually vertical fluxes */
                                    bm->boxes[bb].vflux[kk] += e;
                                    bm->boxes[b].vflux[startk] -= e;
                                } else {
                                    /* Horizontal fluxes */
                                    bm->boxes[bb].eflux[kk] += e;
                                    bm->boxes[b].eflux[startk] -= e;
                                }
                            }
                        }
					}
				}
			}
		}
		bm->hd.tleft -= dt;
		tleft -= dt;

		/* Continuity - adjust tracer values and cell volumes */
		for (b = 0; b < bm->nbox; b++) {
			Box *bp = &bm->boxes[b];
			/* Skip boundary boxes */
			if (bp->type == BOUNDARY || bp->type == LAND)
				continue;
			/* Otherwise loop over each water column layer */
			if (bm->tidal)
				bp->current_botz = 0.0;
			for (k = 0; k < bp->nz; k++) {
				/* Old and new volume values */
				double oldvol = bp->volume[k];
				double newvol = bp->volume[k] + dvol[b][k];

                /*
                if(b == bm->checkbox) {
                    fprintf(bm->logFile, "bp->volume[k] = %.20e, dvol[b][k] = %.20e\n", bp->volume[k], dvol[b][k]);
				    fprintf(llogfp, "box:%d:%d, oldvol = %e, newvol = %e, volumeChange = %e\n", b, k, oldvol, newvol, dvol[b][k]);
                }
                */

				/* New concentration for each water column tracer */
				for (n = 0; n < bm->ntracer; n++) {
					if (bm->tinfo[n].inwc) {
						int can_be_moved = 1;
						double newwat = 0;

						/* Check if flag sensitive and then if actually active */
						if ((bm->do_availflag && !bm->tinfo[n].flagid) || (bm->tinfo[n].partic && !bm->tinfo[n].passive) || !bm->tinfo[n].can_be_moved || !bm->tinfo[n].isUsed){
							can_be_moved = 0;
						}
                        
                        if(bm->dayt > 1090.0 && (strcmp(bm->tinfo[n].name, "salt") == 0)){
                            fprintf(llogfp,"Time %e transportBM box%d-%d for tracer %d (%s) - salinity %e starts %s starts %e\n", bm->dayt, b, k, n, bm->tinfo[n].name, newwc[23][1][Salinity_i], bm->tinfo[n].name, newwc[b][k][n]);
                            fflush(llogfp);
                        }

						//if(strcmp(bm->tinfo[n].name, "Rugosity") == 0)
                        //    fprintf(bm->logFile, "Time %e %d-%d Rugosity starts %e ", bm->dayt, b, k, newwc[b][k][n]);
						//	printf("trying to transport %s with flagid %d\n", bm->tinfo[n].name, bm->tinfo[n].flagid);

						if (can_be_moved) {
							//if(strcmp(bm->tinfo[n].name, "MicroNut") == 0)
							//	printf("transporting %s\n", bm->tinfo[n].name);

							newwat = (newwc[b][k][n] * oldvol + dtr[b][k][n]) / newvol;
							if (newwat < 0 && newwc[b][k][n] != bm->min_pool) {
								//warn("transport: time: %e %s negative (was %.10g, now %.10g so zeroing out) at box %d layer %d, oldvol = %.10g, newvol = %.10g, dtr = %.10g\n", bm->dayt, bm->tinfo[n].name, newwc[b][k][n], newwat, b, k, oldvol, newvol, dtr[b][k][n]);                                
                                if(!lots_warn || (lots_warn && bm->newmonth)){
                                    fprintf(llogfp, "transport: time: %e %s negative (was %.10g, now %.10g so zeroing out) at box %d layer %d, oldvol = %.10g, newvol = %.10g, dtr = %.10g\n", bm->dayt, bm->tinfo[n].name, newwc[b][k][n], newwat, b, k, oldvol, newvol, dtr[b][k][n]);
                                }
                                track_warn++;

                                newwat = 0.0;
							}
                            
                            /**
                            if((b == bm->checkbox) && (strcmp(bm->tinfo[n].name, "SED") == 0)) {
                                fprintf(llogfp, "Time: %e box %d-%d tracer: %d transport of %s was %f, now %f at oldvol = %f, newvol = %f, dtr = %f, dvol = %f\n", bm->dayt, b, k, n, bm->tinfo[n].name, newwc[b][k][n], newwat, oldvol, newvol, dtr[b][k][n], dvol[b][k]);
                            }
                            **/

							/**
                            if(b == bm->checkbox)
                                fprintf(bm->logFile, "transport: %s was %.10g, now %.10g at box %d layer %d\noldvol = %.10g, newvol = %.10g, dtr = %.10g (can_be_moved: %d)\n",
                                       bm->tinfo[n].name,newwc[b][k][n],newwat,b,k,oldvol,newvol,dtr[b][k][n], bm->tinfo[n].can_be_moved);
							 **/
                            
                            if(bm->dayt > 1090.0 && (strcmp(bm->tinfo[n].name, "salt") == 0)){
                                fprintf(llogfp,"Time %e transportBM box%d-%d for tracer %d (%s) - salinity now %e and %s was %.10g, now %.10g, oldvol = %.10g, newvol = %.10g, dtr = %.10g, dvol = %.10g\n", bm->dayt, b, k, n, bm->tinfo[n].name, newwc[23][1][Salinity_i], bm->tinfo[n].name, newwc[b][k][n], newwat, oldvol, newvol, dtr[b][k][n], dvol[b][k]);
                                fflush(llogfp);
                            }
						} else{
							newwat = newwc[b][k][n];
						}
                        
                        if (!_finite(newwat)) {
                            fprintf(llogfp,"Time %e transportBM box%d-%d for tracer %d (%s) - was %.10g, now %.10g, oldvol = %.10g, newvol = %.10g, dtr = %.10g, dvol = %.10g\n", bm->dayt, b, k, n, bm->tinfo[n].name, newwc[b][k][n], newwat, oldvol, newvol, dtr[b][k][n], dvol[b][k]);
                            fflush(llogfp);
                            quit("NAN value for %s - was %.10g, now %.10g, oldvol = %.10g, newvol = %.10g, dtr = %.10g, dvol = %.10g\n", bm->tinfo[n].name, newwc[b][k][n], newwat, oldvol, newvol, dtr[b][k][n], dvol[b][k]);
                        }

						newwc[b][k][n] = newwat;
                        
					}
				}
				/* Store new volume and dz values */
				bp->volume[k] = newvol;
				bp->dz[k] = newvol / bp->area;

				if (isnan(bp->dz[k])){
					printf("bp->dz[k] = %e\n", bp->dz[k]);
					quit("NAN value for dz in box %d, layer %d, newvol = %e, area = %e\n", bp->n, k, newvol, bp->area);
				}



				/* For tidal systems don't completely empty cell */
				if (bm->tidal) {
					if (bp->dz[k] == 0)
						bp->dz[k] = bm->min_pool;
					bp->current_botz += bp->dz[k];
				}
			}
		}

		/* Write exports */
		if (bm->t >= bm->inputs_toutNext){
			writeExports(bm->atPhysicsModule->expfp, bm, dtr);
		}
	}
	// fclose(expfp);
    
    //if(verbose)
    //    printf("Leaving transport\n");
    
    fflush(stdout);     /* in case stdout and stderr are the same so can track soem frees*/
    fflush(stderr);

	/* Get more hydro data if necessary, so that hydro time
	 * is updated appropriately
	 */
	if (bm->hd.tleft <= 0)
		get_hydro(bm);
    
    return;

}

/* Routine to initialise export file */
FILE * initExportFile(MSEBoxModel *bm) {
	FILE *fp;
	char *fname = "export.ts";
	int n = 0;

    /* Create file */
    if( (fp=Util_fopen(bm, fname, "w")) == NULL )
    	quit("initExportFile: Can't open %s\n",fname);

    /* File title */
	fprintf(fp, "# Net mass changes for box 0, level 0 due to exchange\n#\n");

	/* Time column header */
	fprintf(fp, "## COLUMNS %d\n##\n", bm->ntracer + 1);
	fprintf(fp, "## COLUMN1.name time\n");
	fprintf(fp, "## COLUMN1.long_name Time\n");
	fprintf(fp, "## COLUMN1.units %s\n", bm->t_units);
	fprintf(fp, "## COLUMN1.missing_value -999\n##\n");

	/* Header for variables */
	for (n = 0; n < bm->ntracer; n++) {
		int c = n + 2;
		fprintf(fp, "## COLUMN%d.name %s\n", c, bm->tinfo[n].name);
		fprintf(fp, "## COLUMN%d.long_name %s\n", c, bm->tinfo[n].long_name);
		fprintf(fp, "## COLUMN%d.units mass per timestep\n", c);
		fprintf(fp, "## COLUMN%d.missing_value -99999\n##\n", c);
	}

	/* Return file pointer */
	return (fp);
}

void writeExports(FILE *fp, MSEBoxModel *bm, double ***dtr) {
	int n = 0;

	/* Write time */
	fprintf(fp, "%.10g", bm->t);

	/* Write values */
	for (n = 0; n < bm->ntracer; n++)
		fprintf(fp, " %.8g", dtr[0][0][n]);

	/* New line */
	fprintf(fp, "\n");
}

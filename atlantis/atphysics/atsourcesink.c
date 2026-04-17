/*******************************************************************//**
 \file
 \brief Routines which deal with box model tracer sources and sinks.

 File:           atsourcesink.c

 Created:        Fri Dec 9 14:44:28 EST 1994

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routines which deal with box model tracer
 sources and sinks. These are specified as
 time series of inputs (sources) and outputs
 (sinks). My standard time series routines
 (sjwlib) are used here.

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


 2-02-2009 Bec Gorton
 Added a check before tsFree is called for bm->precip
 and bm->eval. The init code prints a warning if these
 input values are not found so we should check if they
 were read before we try to free them.
 Also changed the call to read in the Solar_radiation to produce an
 error if these parameters are not found.

 22-04-2009 Bec Gorton
 Moved the static output files in the atPhysicsModule
 so they can be freed at the end of the model run.
 void routine_name(int blah, double blahblah)

 24-07-2009 Bec Gorton
 Changed the souresink function to use the new arrays for the mult, start and period
 values instead of using a single set of values for all point sources.
 This allows the user to scale each point source differently.
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atEcologyLib.h>
#include <atUtilLib.h>

/* Prototypes */
double fw_area_input(MSEBoxModel *bm, TimeSeries *ts, int id, double scale, double ***newwattr);
double Get_PSS_Scalar(MSEBoxModel *bm, int s, int variableIndex);

FILE *initInputsFile(MSEBoxModel *bm);
void writeInputs(FILE *fp, MSEBoxModel *bm, double *inp);
void readSolar(MSEBoxModel *bm, char *tunit);
void freeSolar(MSEBoxModel *bm);
void swrForcingBM(MSEBoxModel *bm, PhyPropertyData *inputData);

/*
 * brief\ Routine handing point source and sinks form the foring file
 *
 */
void sourceSink(MSEBoxModel *bm, double ***newwattr, FILE *llogfp)
/* double ***newwattr      new water column tracer values */
{
	int s = 0;
	int i = 0;
	int index;
	int variableIndex; /* The index of the point source variable in the change arrays. This allows for the
	 fact that we are not going to scale the time variable - so there is no space for it in the change arrays */
	double vol = 0;
	double nut_scale = 0.0, end_date, ask_t;
	//static FILE *inpfp = NULL;
	//static double *totinp;

    if (bm->ts_on_hydro_time) {
        ask_t = bm->hd.t;
    } else {
        ask_t = bm->t;
    }

    if (verbose)
		fprintf(stderr, "Entering sourceSink\n");

	/* Initialise Inputs totals file if necessary */
	if (!bm->atPhysicsModule->inpfp) {
		bm->atPhysicsModule->inpfp = initInputsFile(bm);
		bm->atPhysicsModule->totinp = alloc1d(bm->ntracer);
	}

	/* Set total inputs to zero */
    for (i = 0; i < bm->ntracer; i++) {
        bm->atPhysicsModule->totinp[i] = 0;
    }

    /* Loop over point source/sinks */
	for (s = 0; s < bm->npss; s++) {
		PointSourceSink *pss = &bm->pss[s];
		int b = pss->e1;
		int k = pss->e3;
		Box *bp = &bm->boxes[b];
		double vol = bp->volume[k];
        
        if (bp->type == BOUNDARY || bp->type == LAND)
			continue;

		if (pss->watertsid >= 0) {
			/* Water is associated with this source/sink. */
			double val = tsEvalR(&pss->ts, pss->watertsid, ask_t, pss->rewindid);

			double wvol = bm->dt * val;

			/* FIX - How to adjust volumes if not in surface layer */
            if (k != bp->nz - 1) {
                quit("ERROR: The box model does not yet handle point source/sinks\n"
                     "containing water correctly unless they are located in the\n"
                     "surface layer. %s is located in box %d layer k=%d\n", pss->name, b, k);
            }

			if (wvol > 0.0) {
				int n; //TODO: Change variable name...

				/* Water flowing in - loop over each variable
				 * in the point source/sink time series to add
				 * mass of tracer.
				 */
                variableIndex = -1;
				for (i = 0; i < pss->ts.nv; i++) {
					/* Get model tracer index */
					int n = pss->vid[i];
					/* Add mass if it is a valid index */
					if (n >= 0 && bm->tinfo[n].inwc && n != bm->waterid) {
						double mass = wvol * tsEvalR(&pss->ts, i, ask_t, pss->rewindid);
                        
                        variableIndex++;
                        nut_scale = 1.0;
                        
                        if (bm->nutrientChange) {
                            nut_scale = Get_PSS_Scalar(bm, s, variableIndex);
                        }
                        mass = mass * nut_scale;
                        
						bm->atPhysicsModule->totinp[n] += mass;
						newwattr[b][k][n] += mass / vol;
					}
				}
				/* Now loop over all model tracers to correct concentrations
				 * due to change in volume - but only if not an absolute value to start with
				 */
				for (n = 0; n < bm->ntracer; n++)
					if (bm->tinfo[n].inwc && (n != bm->waterid))
						newwattr[b][k][n] *= vol / (vol + wvol);
			}
			/* Adjust box volumes and dz. Note that if water is flowing
			 * out (a sink), then this is all that needs to be done, as
			 * tracer concentrations in the box do not change.
			 */
			bp->volume[k] += wvol;
			bp->dz[k] = bp->volume[k] / bp->area;

			/* Add water volume to total inputs */
            if (bm->waterid >= 0){
                bm->atPhysicsModule->totinp[bm->waterid] += wvol;
            }
		} else {
			/* No water associated with this source/sink */
			variableIndex = -1;

			/* Loop over each variable in the point source/sink time series */
			for (i = 0; i < pss->ts.nv; i++) {
				/* Get model tracer index */
				int n = pss->vid[i];
				/* Add source/sink if it is a valid index */
				if (n >= 0 && bm->tinfo[n].inwc && n != bm->waterid) {
					double val = tsEvalR(&pss->ts, i, ask_t, pss->rewindid);
					double mass = bm->dt * val;

					variableIndex++;
					nut_scale = 1.0;

					if (bm->nutrientChange) {
                        nut_scale = Get_PSS_Scalar(bm, s, variableIndex);
						/* Loop over each possible change */
						for (index = 0; index < bm->pss[s].numPssChanges; index++) {
							if (bm->pss[s].pssChange[index][variableIndex] && (bm->t >= (bm->pss[s].pssStart[index][variableIndex] * 86400.0))) {
								/* Must correct nutrient change period into seconds as that's what bm works in */
								end_date = (bm->pss[s].pssPeriod[index][variableIndex] + bm->pss[s].pssStart[index][variableIndex]) * 86400.0;

								if ((strcasecmp(pss->ts.varname[variableIndex], "Si") == 0) || (bm->pss[s].pssMult[index][variableIndex] == 1.0)) {
									nut_scale = nut_scale * 1.0;
								} else if (end_date < bm->t) {
									if (bm->pulsechange) {
										// Nothing to do as simply * 1.0
									} else {
										nut_scale = nut_scale * bm->pss[s].pssMult[index][variableIndex];
									}
								} else {
                                    if (bm->t == (bm->pss[s].pssStart[index][variableIndex])) {
                                        // Nothing to do as simply * 1.0
                                    } else {
                                        nut_scale = nut_scale * (bm->pss[s].pssMult[index][variableIndex] * ((bm->t - (bm->pss[s].pssStart[index][variableIndex] * 86400.0)) / (bm->pss[s].pssPeriod[index][variableIndex] * 86400.0)));
                                    }
								}
							}
						}
					}
					if (verbose > 1) {
						if (nut_scale != 1.0) {
							fprintf(llogfp, "sourceSink - time:%e box:%d, layer:%d sourcesink %s (%s) scaling factor = %e. Unscaled mass = %e, scaled mass = %e, scaledMass/vol = %e\n", bm->t, b, k, bm->pss[s].name, bm->pss[s].ts.varname[i], nut_scale, mass, (mass * nut_scale), (mass * nut_scale) / vol);
						}
					}

					/**
					if(b == bm->checkbox){
						fprintf(llogfp, "sourceSink - time:%e box:%d layer:%d sourcesink %s (%s) scaling factor = %e Unscaled mass = %e scaled mass = %e scaledMass/vol = %e val = %e\n",
							bm->t, b, k, bm->pss[s].name, bm->pss[s].ts.varname[i], nut_scale, mass, (mass * nut_scale), (mass * nut_scale)/vol, val);
					}
					**/

					mass = mass * nut_scale;

					bm->atPhysicsModule->totinp[n] += mass;

					//			printf("starting conc %e (%s) in box %d ",newwattr[b][k][n], bm->tinfo[n].name, b);

					newwattr[b][k][n] += mass / vol;

				}
 			}
		}
	} //for

	/* NOTE - the following inputs evaluate time series at the
	 * single point designated as inside each box. Some sort of
	 * integral over the box area might be better in the long run
	 */

    /* Precipitation  - input file is in mm day -1. - divide by 86400 to get rainfall per second.
	 * Divide by 1000 to get rainfall in m per second. */
	vol = fw_area_input(bm, bm->precip, bm->precip_id, 1.0 / (1000.0 * 86400.0), newwattr);
	if (bm->waterid >= 0)
		bm->atPhysicsModule->totinp[bm->waterid] += vol;

    /* Evaporation - note negative scale factor */
	vol = fw_area_input(bm, bm->evap, bm->evap_id, -1.0 / (1000.0 * 86400.0), newwattr);
	if (bm->waterid >= 0)
		bm->atPhysicsModule->totinp[bm->waterid] += vol;

    /* Area tracer inputs in to surface water column layer.
	 * It is assumed that no water is associated with these
	 * inputs (they are hence specified as mass per unit
	 * area per unit time).
	 */
	if (bm->tr_areainp) {
		int *tsid = i_alloc1d(bm->ntracer);
		int b;
		int n;

		/* Make list of tracers in input time series */
		for (n = 0; n < bm->ntracer; n++) {
			if (bm->tinfo[n].inwc && n != bm->waterid)
				tsid[n] = tsIndex(bm->tr_areainp, bm->tinfo[n].name);
			else
				tsid[n] = -1;
		}

		/* Loop over each box */
		for (b = 0; b < bm->nbox; b++) {
			int k = bm->boxes[b].nz - 1;
			double vol = bm->boxes[b].volume[k];
			double x = bm->boxes[b].inside.x;
			double y = bm->boxes[b].inside.y;
			double a = bm->boxes[b].area;

			/* Skip boundary boxes */
			if (bm->boxes[b].type == BOUNDARY || bm->boxes[b].type == LAND)
				continue;

			/* Loop over each tracer */
			for (n = 0; n < bm->ntracer; n++) {
				if (tsid[n] >= 0) {
					double mass = a * bm->dt * tsEvalXY(bm->tr_areainp, tsid[n], ask_t, x, y);
					bm->atPhysicsModule->totinp[n] += mass;
					newwattr[b][k][n] += mass / vol;
				}
			}
		}

		/* Free tracer id list */
		i_free1d(tsid);
	}
    
    /* Re-calculate layer coordinates */
    layer_coords(bm, llogfp);
    
    /* Write total inputs */
    if (bm->t >= bm->inputs_toutNext){
        writeInputs(bm->atPhysicsModule->inpfp, bm, bm->atPhysicsModule->totinp);
    }

}

/*
 * brief\ Routine handing solar irradiance file form the foring file
 *
 */
void solarIrradiance(MSEBoxModel *bm, double ***newwattr, FILE *llogfp) {    
	double ask_t;

    if (bm->ts_on_hydro_time)
        ask_t = bm->hd.t;
    else
        ask_t = bm->t;

	/* Solar radiation input */
    if (bm->swr) {
		/* Store value in each box */
		int b;
		if (!bm->swr_rewindid) {
			for (b = 0; b < bm->nbox; b++) {
				bm->boxes[b].swr = tsEvalXY(bm->swr, bm->swr_id, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y);
				if (!(_finite(bm->boxes[b].swr))) {
					quit("sourceSink - box: %d, Invalid value calculated for solar radiation 1 - ask_t: %e x: %e y: %e swr_id: %d\n", b, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y, bm->swr_id);
				}
			}
		} else {
			for (b = 0; b < bm->nbox; b++) {
				bm->boxes[b].swr = tsEvalR(bm->swr, bm->swr_id, ask_t, bm->swr_rewindid);
				if (!(_finite(bm->boxes[b].swr))) {
					quit("sourceSink - box: %d, Invalid value calculated for solar radiation 2 - ask_t: %e x: %e y: %e swr_id: %d\n", b, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y, bm->swr_id);
				}
			}
		}
	} else {
		if(bm->swrinput.nFiles > 0){
			swrForcingBM(bm, &(bm->swrinput));
		}
	}
	/*for (i = 0; i < bm->nbox; i++) {
		fprintf(bm->logFile, "bm->boxes[b = %d].swr  = %e\n", i, bm->boxes[i].swr );
	}*/

}

/** Area inputs of fresh water - precipitation and evaporation

 @param double ***newwattr       new water column tracer values

 */
double fw_area_input(MSEBoxModel *bm, TimeSeries *ts, int id, double scale, double ***newwattr) {
	int b;
	double total = 0, ask_t;
    
    if (bm->ts_on_hydro_time)
        ask_t = bm->hd.t;
    else
        ask_t = bm->t;


    /* Check that parameters are sensible */
	if (!bm || !ts)
		return (0.0);

	for (b = 0; b < bm->nbox; b++) {
		Box *bp = &bm->boxes[b];
		int k = bp->nz - 1;
		double x = bp->inside.x;
		double y = bp->inside.y;
		double vol = bp->volume[k];
		double wvol = bp->area * bm->dt * tsEvalXY(ts, id, ask_t, x, y) * scale;
		int n;

		/* Skip boundary boxes */
		if (bm->boxes[b].type == BOUNDARY || bm->boxes[b].type == LAND)
			continue;

		/* Check for complete drying of top layer */
		if (wvol <= -vol)
			quit("fw_area_input: Top layer would dry due to %s in box %d\n", ts->varname[id], b);

		for (n = 0; n < bm->ntracer; n++) {
			if (bm->tinfo[n].inwc && n != bm->waterid)
				newwattr[b][k][n] *= vol / (vol + wvol);
		}

		bp->volume[k] += wvol;
		bp->dz[k] = bp->volume[k] / bp->area;
		total += wvol;

	} //for

	return (total);
}

void freeSourceSink(MSEBoxModel *bm) {
	int s;

	if (bm->precip != NULL) {
		tsFree(bm->precip);
		free(bm->precip);
	}

    /*
    if (bm->airtemp != NULL) {
        tsFree(bm->airtemp);
        free(bm->airtemp);
    }
    */
    
	if (bm->evap != NULL) {
		tsFree(bm->evap);
		free(bm->evap);
	}
	//tsFree(bm->tr_areainp);

	freeSolar(bm);

	if (bm->env_force != NULL){
		tsFree(bm->env_force);
		free(bm->env_force);
	}

    if (bm->KWSR_force != NULL){
        tsFree(bm->KWSR_force);
        free(bm->KWSR_force);
    }
    
    if (bm->tslinearMort != NULL){
		tsFree(bm->tslinearMort);
		free(bm->tslinearMort);
	}


	if(bm->tspCO2 != NULL){
		tsFree(bm->tspCO2);
		free(bm->tspCO2);
	}
    
    if (bm->ships != NULL) {
        tsFree(bm->ships);
        free(bm->ships);
    }

    if (bm->dredge != NULL) {
        tsFree(bm->dredge);
        free(bm->dredge);
    }

	//	tsFree(bm->env_force);

	for (s = 0; s < bm->npss; s++) {
		PointSourceSink *ps = &bm->pss[s];
		i_free1d(ps->vid);
		tsFree(&ps->ts);
		if (bm->nutrientChange) {
			if(bm->pss[s].numPssChanges > 0){

				/* Free up the scaling arrays */
				if(bm->pss[s].pssMult)
					free2d(bm->pss[s].pssMult);
				i_free2d(bm->pss[s].pssStart);
				i_free2d(bm->pss[s].pssPeriod);
				i_free2d(bm->pss[s].pssChange);
			}
		}
	}
	free(bm->pss);

}


/************************************************************************************************************************************
 *
 * \brief Routine to get nutrient scaling on source sinks
 *
 **************************************************************************************************************************************/
double Get_PSS_Scalar(MSEBoxModel *bm, int s, int variableIndex) {
    double nut_scale = 1.0;
    int index;
    double end_date;
    
    /* Loop over each possible change */
    for (index = 0; index < bm->pss[s].numPssChanges; index++) {
        if (bm->pss[s].pssChange[index][variableIndex] && (bm->t >= (bm->pss[s].pssStart[index][variableIndex] * 86400.0))) {
            /* Must correct nutrient change period into seconds as that's what bm works in */
            end_date = (bm->pss[s].pssPeriod[index][variableIndex] + bm->pss[s].pssStart[index][variableIndex]) * 86400.0;
            
            if ((strcasecmp(bm->pss[s].ts.varname[variableIndex], "Si") == 0) || (bm->pss[s].pssMult[index][variableIndex] == 1.0)) {
                nut_scale = nut_scale * 1.0;
            } else if (end_date < bm->t) {
                if (bm->pulsechange) {
                    nut_scale = nut_scale * 1.0;
                } else {
                    nut_scale = nut_scale * bm->pss[s].pssMult[index][variableIndex];
                }
            } else {
                nut_scale = nut_scale * (bm->pss[s].pssMult[index][variableIndex] * ((bm->t - (bm->pss[s].pssStart[index][variableIndex] * 86400.0)) / (bm->pss[s].pssPeriod[index][variableIndex] * 86400.0)));
            }
        }
    }
    
    return nut_scale;
}

/*******************************************************************//**
 Initialisation routines for sources/sinks
 *********************************************************************/
void sourcesink_init(MSEBoxModel *bm) {
    char tunit[STRLEN];
    
	/* Set the gloabl variable */
	setIndexBM(bm);

	/* Read point source/sinks */
    if(bm->ts_on_hydro_time)
        strcpy(tunit, bm->hd.t_units);
    else
        strcpy(tunit, bm->t_units);
    
    readPointSourceSink(bm->inputFolder,
		bm->forceIfname, // input force file reference
		tunit, // Time units to be unsed to time series
		&bm->pss, // array of PointSourceSink structures
		&bm->npss, // Returned number of point sources/sinks
		NULL, // model data
		ext_xyztoijk, // Pointer to function that convers (x,y,z) to (i,j,k)
		ext_trIndex // Pointer to function which finds model index of tracer name.
    );

	/* Read precipitation time series, hydrodynamic time units */
	read_bm_ts(bm, "Precipitation", &bm->precip, tunit, "precipitation", "mm day-1", &bm->precip_id, warn);

	/* Read evaporation time series, hydrodynamic time units */
	read_bm_ts(bm, "Evaporation", &bm->evap, tunit, "evaporation", "mm day-1", &bm->evap_id, warn);

	/* Read area tracer inputs time series, model time units */
	read_bm_ts(bm, "Tracer_area_inputs", &bm->tr_areainp, tunit, NULL, NULL, NULL, warn);

    /* Read shipping information - if appropriate *
    if (bm->flagindustry_on) {
        Read_Shipping_TS(bm, "Shipping_Schedule", &bm->ships, bm->t_units, &bm->ships_rewind, warn);
        
        printf("Read dredge ts\n");
        read_bm_ts_rewind(bm, "Dredge_Schedule", &bm->dredge, bm->t_units, "dredging", "g day-1", &bm->dredge_id, &bm->dredge_rewind, warn);
        
        // Read precipitation time series, hydrodynamic time units
        read_bm_ts_rewind(bm, "Air_temperature", &bm->airtemp, tunit, "air_temperature", "celcius", &bm->airtemp_id, &bm->airtemp_rewind, warn);

    }
    */

	/* Read short wave radiation input */
	//read_bm_ts_rewind(bm, "Solar_radiation", &bm->swr, tunit, "swr", "W m-2", &bm->swr_id, &bm->swr_rewindid, warn);
    readSolar(bm, tunit);

	/* Read recruitment forcing timeseries */
	Ecology_Read_Recruit_TS(bm, "Recruitment_time_series", &bm->tsRecruit, bm->t_units, warn);

	/* Read recruitment environmental forcing timeseries */
	Ecology_Read_Enviro_Forcing(bm, "Recruitment_enviro_forcing", &bm->env_force, bm->t_units, warn);

    /* Read recruitment environmental forcing timeseries */
    Ecology_Read_Enviro_Forcing(bm, "KWSR_forcing", &bm->KWSR_force, bm->t_units, warn);
    
    /* Lake fish recruitment forcng timeseries */
    if(bm->ice_on) {
        read_bm_ts_rewind(bm, "thermal_index", &bm->thermal_index, bm->t_units, "tindex", "deg C", &bm->tindex_id, &bm->tindex_rewindid, warn);
        read_bm_ts_rewind(bm, "rate_index", &bm->rate_index, bm->t_units, "rindex", "deg C", &bm->rindex_id, &bm->rindex_rewindid, warn);
    }
    
    /* Read linear mortality forcing timeseries - basing it on model time units rather than hydrodynamic time units for ease of use */
	Ecology_Read_LinearMortality_TS(bm, "LinearMort", &bm->tslinearMort, bm->t_units, warn);

	/* Read size change forcing timeseries - basing it on model time units rather than hydrodynamic time units for ease of use */
	Ecology_Read_Size_Change_TS(bm, "SizeChange",&bm->tsSizeChange,bm->t_units,warn);
	Ecology_Read_Growth_Rate_TS(bm, "GrowthRateChange", &bm->tsGrowthRateChange, bm->t_units,warn);
	Ecology_Read_FSPB_TS(bm, "FSPBChange", &bm->tsFSPBChange, bm->t_units,warn);

	/* Read in atmospheric pCO2 timeseries */
	Ecology_Read_pCO2_TS(bm, "pCO2_forcing",&bm->tspCO2, bm->t_units,warn);

    /* Store initial short wave radiation values */
    if( bm->swr ) {
		int b;
        double ask_t;
        
        if (bm->ts_on_hydro_time)
            ask_t = bm->hd.t;
        else
            ask_t = bm->t;


		/* Check that we have a valid time */
		if (!(_finite(ask_t))) {
			quit("ERROR - sourceSink - Invalid value calculated for bm->hd.t or bm->t\n");
		}
		for (b = 0; b < bm->nbox; b++) {
			bm->boxes[b].swr = tsEvalXY(bm->swr, bm->swr_id, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y);
			if (!(_finite(bm->boxes[b].swr))) {
				quit("ERROR - sourceSink - box: %d, Invalid value calculated for solar radiation 3 - ask_t: %e x: %e y: %e swr_id: %d\n", b, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y, bm->swr_id);
			}
		}
	} else {
		if(bm->swrinput.nFiles > 0) {
			swrForcingBM(bm, &(bm->swrinput));
		}
	}
}

/** Routine to read a time series, check that
 * it has a particular variable with particular
 * units, and convert time units to model
 * units if necessary
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param varname variable of interest
 *	@param varunits Units of the variable
 *	@param id - No idea what this is for - used as a flag, if not null then variable and units are checked.
 *	@param err Pointer to the error function to be called if there is an error.
 */
void read_bm_ts(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, char *varname, char *varunit, int *id, void(*err)(char *format, ...)) {
	char buf[BMSLEN];
	FILE *fp;

	/* Open parameter file */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("read_bm_ts: Can't open %s%s\n", bm->inputFolder,bm->forceIfname);

	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		ts = NULL;
		/* Close parameter file */
		fclose(fp);
		return;
	}

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries))) == NULL)
		quit("read_bm_ts: Can't allocate memory for %s time series\n", key);

	/* Read the time series */
	tsRead(bm->inputFolder, buf, *ts);

	/* Check that requested variable is present */
	if (id && varname && (*id = tsIndex(*ts, varname)) < 0)
		quit("read_bm_ts: Can't find %s variable in %s\n", varname, (*ts)->name);

	/* Check that variable has requested units */
	if (id && varname && varunit && strcasecmp((*ts)->varunit[*id], varunit) != 0)
		quit("read_cell_ts: %s units must be %s\n", varname, varunit);

	/* Convert time units if necessary */
	if (strcmp(t_units, (*ts)->t_units) != 0)
		tsNewTimeUnits(*ts, t_units);

	/* Close parameter file */
	fclose(fp);
}

/** Routine to read a time series with associated rewind value
 *
 *
 *	@param bm Pointer to box model
 *	@param key Pointer to name of series to read
 *	@param ts Pointer to array of TimeSeries structures to populate - not yet allocated.
 *	@param t_units Pointer to time units used by the hydrological model
 *	@param varname variable of interest
 *	@param varunits Units of the variable
 *	@param id - No idea what this is for - used as a flag, if not null then variable and units are checked.
 *  @param rid - Pointer to the rewind id value
 *	@param err Pointer to the error function to be called if there is an error.
 *
 */
void read_bm_ts_rewind(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, char *varname, char *varunit, int *id, int *rid, void(*err)(char *format,
		...)) {
	char buf[BMSLEN];
	char keyid[BMSLEN];
	FILE *fp;

	/* Open parameter file */
	if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
		quit("read_bm_ts: Can't open %%s\n", bm->inputFolder, bm->forceIfname);

	/* Find time series name in parameter file */
	set_keyprm_errfn(err);
	if (!readkeyprm_s(fp, key, buf)) {
		/* Close parameter file */
		fclose(fp);
		return;
	}

	/* Allocate space for time series */
	if (*ts == NULL && (*ts = (TimeSeries *) malloc(sizeof(TimeSeries))) == NULL)
		quit("read_bm_ts: Can't allocate memory for %s time series\n", key);

	/* Read the time series */
	tsRead(bm->inputFolder, buf, *ts);

	/* Check that requested variable is present */
	if (id && varname && (*id = tsIndex(*ts, varname)) < 0)
		quit("read_bm_ts: Can't find %s variable in %s\n", varname, (*ts)->name);

	/* Check that variable has requested units */
	if (id && varname && varunit && strcasecmp((*ts)->varunit[*id], varunit) != 0)
		quit("read_cell_ts: %s units must be '%s', values are '%s'\n", varname, varunit, (*ts)->varunit[*id]);

	/* Convert time units if necessary */
	if (strcmp(t_units, (*ts)->t_units) != 0)
		tsNewTimeUnits(*ts, t_units);

	/* Read the rewind_id */
	snprintf(keyid, sizeof(keyid), "%s_rewind", key);
	readkeyprm_i(fp, keyid, rid);

	/* Close parameter file */
	fclose(fp);
}

/* Routine to initialise input totals file */
FILE * initInputsFile(MSEBoxModel *bm) {
	FILE *fp;
	char *fname = "inputs.ts";
	int n;

    /* Create file */
    if( (fp=Util_fopen(bm, fname, "w")) == NULL )
		quit("initInputsFile: Can't open %s\n",fname);

	/* File title */
	fprintf(fp, "# Total tracer inputs\n#\n");

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
		fprintf(fp, "## COLUMN%d.units %s per timestep\n", c, ((n == bm->waterid) ? "volume" : "mass"));
		fprintf(fp, "## COLUMN%d.missing_value -99999\n##\n", c);
	}

	/* Return file pointer */
	return (fp);
}

void writeInputs(FILE *fp, MSEBoxModel *bm, double *inp) {
	int n = 0;

	/* Write time */
	fprintf(fp, "%.10g", bm->t);

	/* Write values */
	for (n = 0; n < bm->ntracer; n++)
		fprintf(fp, " %.8g", inp[n]);

	/* New line */
	fprintf(fp, "\n");
}


/*****************************************************************************************************************//***
 Routine to deal with read-in of stress data
 
 Read the bottom stress data, hydrodynamic time units, from timeseries unless supplied
 initial conditons file

***//******************************************************************************************************************/
/* Read the bottom stress data, hydrodynamic time units, from timeseries unless supplied
 initial conditons file */

void ReadStress(MSEBoxModel *bm, FILE *llogfp) {
    FILE *fp;
    int b;
    double ask_t, step1;
    
    /* Open parameter file */
    if ((fp = Open_Input_File(bm->inputFolder,bm->forceIfname, "r")) == NULL)
        quit("read_force: Can't open %s%s\n", bm->inputFolder,bm->forceIfname);
    
    readkeyprm_i(fp, "use_stressfiles", &bm->use_stressfiles);
    
    if (!bm->supplied_stress && bm->use_stressfiles) {
        
        if (bm->resuspension) {
            if(bm->ts_on_hydro_time)
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->hd.t_units, "nxsbs", "1", &bm->stress_id, quit);
            else
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->t_units, "nxsbs", "1", &bm->stress_id, quit);
        } else {
            if(bm->ts_on_hydro_time)
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->hd.t_units, "nxsbs", "1", &bm->stress_id, warn);
            else
                read_bm_ts(bm, "BottomStress", &bm->stress, bm->t_units, "nxsbs", "1", &bm->stress_id, warn);
        }
    
        for (b = 0; b < bm->nbox; b++) {
            /* Store initial stress values if not already loaded */
    
            /* FIX - if not supplied in initial conditions file then
             need to have it so that if stress file is there than
             read in otherwise set to zero  i.e. need to base it on
             outcome of above call. */
        
            if (bm->ts_on_hydro_time)
                ask_t = bm->hd.t;
            else
                ask_t = bm->t;
        
            if (bm->resuspension)
                bm->boxes[b].stress = tsEvalXY(bm->stress, bm->stress_id, ask_t, bm->boxes[b].inside.x, bm->boxes[b].inside.y);
            else
                bm->boxes[b].stress = 0.0;
        }
    } else {
        /* Otherwise use a relationship with mud - from Mitchener and Torfs, 1996. Erosion of mud/sand mixtures. Coastal Engineering. V29. 1-25. */
        for (b = 0; b < bm->nbox; b++) {
            step1 = (1.0 - bm->boxes[b].soft) * 300.0;
            bm->boxes[b].stress = 0.0012 * pow(step1, 1.2);
        }
    }
    
    /* Close parameter file */
    fclose(fp);

    return;
}

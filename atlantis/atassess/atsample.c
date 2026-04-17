/**
 \file
 \brief Routines to for sampling and assessment model calls
 \ingroup atassess

 File:           atsample.c

 Created:        8/8/2004

 Author:         Beth Fulton

 Purpose:        Routines to for sampling and assessment model calls.
 All raw biomass (stock, catch, discard) are in mg N m-2
 Calls for sampling, attribute calculation and assessment.

 Arguments:      bm - Pointer to box model.

 Returns:        void

 Revisions:      8/8/2004 Created from southeast sample.c

 26/8/2004 Moved initialisation code to msesamplesetup.c

 10/9/2004 Added bm->annual_sample don't need to
 repeat code, just loop until hit annual_sample and then
 do assessment and account keeping routines and sections

 12/9/2004 Added bm->flagecosysassess so only assess
 fisheries independent data when appropriate (i.e.
 when calculating ecosystem indicators not when using
 standard single species assessments. Also added
 Classical_Assessment() call.

 20/9/2004 Added WriteAssessOut() call

 27/5/2008 Added sample_now so can have mid year sampling of
 fisheries surveys that were originally annual.

 3/01/2009 moved the updating of sampling status to before sampling
 to solve issues about getting correct number of samples per year


 *************************************************************************/

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <sjwlib.h>
#include "atsample.h"

/******************** Assessment models main call routined ***************************/
void Assess_Resources(MSEBoxModel *bm, FILE *llogfp) {
	int datain = 0, nt;

	nt = (int) floor(ROUNDGUARD + (bm->dayt / 365.0));

	if (verbose)
		fprintf(stderr, "Assess resources\n");

	if (!bm->do_assessing) {
		if (verbose)
			fprintf(stderr, "Not assessing resources as nothing to do (do_assessing = 0)\n");

		return;
	}

	/**** Check sampling status ****/
	if (bm->dayt == bm->tassess) {

		/* Update counter to check for annual sampling data */
		bm->annual_sample++;

		/* Do check to see whether should be doing survey sampling in this sampling round */
		bm->sample_now = 0;
		bm->annual_assess = 0;
		//bm->annual_assess = 1; //TODO: Get rid of this bec testing change.

		if (numsteps > 0) {
			if (!bm->flagcont) {
				if (bm->annual_sample == numsteps) {
					bm->sample_now = 1;
					bm->annual_assess = 1;
				}
			} else {
				bm->sample_now = 1;
				if (bm->annual_sample == numsteps)
					bm->annual_assess = 1;
			}
		}
		if (numsteps_orig < 1) {
			bm->sample_now = 1;
			bm->annual_assess = 1;
		}


		//		fprintf(llogfp, "Time: %e, annual_sample: %d, numsteps: %d annual_assess: %d, flagcont: %d, sample_now: %d, tassess: %e\n",
		//			bm->dayt, bm->annual_sample, numsteps, bm->annual_assess, bm->flagcont, bm->sample_now, bm->tassess);
	}

	/**** Ecosystem Indicators ****/
	/* Do sampling */
	if (bm->dayt == bm->tassess) {
		Init_Data_Array(bm);
		Init_Attribute_Array(bm, llogfp, sample_id);
		Populate_Arrays(bm, llogfp);
		Sample_Stuff(bm, llogfp);
		datain = 1;
	}

	if (bm->flagecosysassess) {

		/* Do diet sampling if appropriate */
		if ((bm->dayt == bm->teatassess) && datain) {
			Year_Of_Stomach(bm, llogfp);

			/* Calculate diet based indices */
			Diet_Indices(bm);
		}

		/* Calculate Indices and write output */
		if ((bm->dayt == bm->tassess) && bm->annual_assess) {
			/* Calculate Other Indices */
			Indices(bm, llogfp);

			/* Write output files */
			Write_Index_Out(bm, nt);
		}

		/* Do true diet sampling if appropriate */
		if ((bm->dayt == bm->teatassess) && datain) {
			Calculate_Real_Stomachs(bm, llogfp);

			/* Calculate real OI */
			Calculate_True_Diet_Indices(bm);

			/* Write diet output files */
			Write_True_Diet_Out(bm, nt);

			bm->teatassess += (bm->teatassessinc * bm->tassPatchy[nexteatij][tasseat_id]);
			nexteatij++;

		}

		/* Calculate Attributes and Write output */
		if (bm->dayt == bm->tassess) {
			/* True Values recorded */
			Calculate_Attribute_Values(bm, llogfp);
		}
	}

	/**** Other forms of assessment ****/
	//fprintf(llogfp, "dayt: %e, tassess: %e, annual_assess: %d, tClassicalAssessStart: %e\n", bm->dayt, bm->tassess, bm->annual_assess,
	//		bm->tClassicalAssessStart);

	if ((bm->dayt == bm->tassess) && bm->annual_assess && (bm->dayt > bm->tClassicalAssessStart)) {

		fprintf(llogfp, "Doing classical assessment");

		Classical_Assessment(bm, llogfp);
		Write_Assess_Out(bm, nt);
	}

	/**** Update sample counter for next round of collection ****/
	if (bm->dayt == bm->tassess) {

		//fprintf(llogfp, "TIme: %e, tassess: %e, next tassess: %e (assessinc: %e, tassPatchy[%d][%d]: %e)\n", bm->dayt, bm->tassess, bm->tassess
		//		+ max(1.0,(ceil(bm->tassessinc * bm->tassPatchy[nextij][tass_id]))), bm->tassessinc, nextij, tass_id, bm->tassPatchy[nextij][tass_id]);

		bm->tassess += max(1.0,(ceil(bm->tassessinc * bm->tassPatchy[nextij][tass_id])));
		nextij++;
	}

	return;
}

/**************************** Annual sampling schedule creation *****************************************/
void Assess_Annual_Schedule(MSEBoxModel *bm, FILE *llogfp) {
	int bb, intense_sampling;
	double time_since_last_snapshot;

	/* Is this a case of punctuated snapshots with low level in between (if not skip ahead) */
	if (bm->flagpatchy < 2) {

		/* Don't reload sample boxes until year 2 and beyond (just use intensive in year 1 */
		if (bm->dayt < 364)
			return;

		/* Figure out what kind of sampling year it is*/
		time_since_last_snapshot = bm->dayt - bm->last_snap;

		intense_sampling = 0;
		if ((time_since_last_snapshot / bm->maxfreq) > 1.0) {
			bm->last_snap = bm->dayt;
			intense_sampling = 1;
		}

		/* Set nsbox for this year */
		if (intense_sampling)
			bm->nsbox = bm->nsbox_i;
		else
			bm->nsbox = bm->nsbox_o;

		/* Load list of sample boxes */
		for (bb = 0; bb < bm->nsbox; bb++) {
			if (intense_sampling) {
				bm->nsboxes[bb] = bm->nsboxes_intense[bb];
			} else {
				bm->nsboxes[bb] = bm->nsboxes_other[bb];
			}
		}
	}

	fprintf(llogfp, "Resetting annual_sample\n");
	bm->annual_sample = 0;

	return;
}

/**
 * \brief Return the value of the sample in the given zone.
 *
 */
double Assess_Get_Physical_Sampled_Value(MSEBoxModel *bm, int sample, int b) {

	int zone;

	zone = bm->boxes[b].zone - 1;

	if(sample > num_sampled_phy_id){
		quit("Get_Physical_Sampled_Value attempting to sample physical property %d that isn't sampled\n", sample);
	}

	/* TODO: Not 100% sure this is correct - dividing by the sampleingsize */
	return samplephys[sample - 2][zone][sample_id] / (phys_samplingsize * bm->annual_sample);
}

/**
 * \brief Return the Standard Deviation of the sample in the given zone.
 *
 */
double Assess_Get_Physical_Sampled_SD(MSEBoxModel *bm, int sample, int b) {
	int zone;

	zone = bm->boxes[b].zone - 1;
	return physicalSigma[sample][zone];

}


double Assess_Get_Process_Sampled_Value(MSEBoxModel *bm, int sample, int b){

	int zone;

	if(sample > 3){
		quit("Assess_Get_Process_Sampled_Value attempting to sample property %d that isn't sampled\n", sample);
	}

	zone = bm->boxes[b].zone - 1;

	/* Not 100% sure this is correct - dividing by the sampleingsize */
	return sampleprocess[sample][zone][sample_id]/(bm->annual_sample);
}


double Assess_Get_Biomass_Sampled_Value(MSEBoxModel *bm, int sample, int b){

	int zone;

	if(sample > (bm->K_num_tot_sp)){
		quit("Assess_Get_Process_Sampled_Value attempting to sample species property %d that isn't sampled\n", sample);
	}

	zone = bm->boxes[b].zone - 1;

	return samplebiom[sample][zone][sample_id];
}

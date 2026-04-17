/**
 \file
 \brief Initialises assessment model arrays and reads-in assessment parameters
 \ingroup atassess

 File:       atsamplesetup.c

 Author:	    Beth Fulton

 date:	    26/8/2004

 comments:   Initialises assessment model arrays and reads-in assessment
 parameters

 revisions:  26/8/2004 Created from initialisation parts of sesample.c

 11/9/2004 Removed arrays used in Ecopath calculations
 (no longer required)

 18/9/2004 Added creation of the stock assessment arrays

 7/10/2004 Added arrays and strong vectors additional bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG). Also
 added the spInvName vector.

 22/10/2004 Replaced K_num_tot with K_num_tot_assess

 19/1/2005 Added Microphytobenthos


 02-02-2009 Bec Gorton
 Added code to free up the bm->rand and bm->tassPatchy arrays.
 Also added some code to check that the numsamples will be greater than one.
 If not the code will quit with an error message.
 This check is currently commented out for debugging purposes as its often
 useful to run short models.

 14-04-2009 Bec Gorton
 Changed the code that allocated the CPUETrend array. If pseudo_assess is true
 then space is allocated for each year of the model run regardless of the
 tassessstart value.

 03-01-2009 Beth Fulton
 Changed first tassess data to tassesststart+tassessinc (so don't get 3 samples in the first year instead of two)

 22-04-2009 Bec Gorton
 Added the atassesssModule code. This structure is used to contain the
 assessment arrays and state data.

 02-11-2009 Bec Gorton
 Changed the assessment xml file to use the management input file name as a guide.

 09-03-2010 Bec Gorton
 Resized all K_num_vert_sp arrays.

 07-04-2010 Bec Gorton
 Resized the invstockinfo array from bm->K_num_invfished to K_num_tot_sp
 Resized the globalfledge array from K_num_charis to K_num_tot_sp
 Resized the food array from bm->K_num_eweinvert + bm->K_num_detritus to k_num_tot_sp, 2;
 Removed the global array contribvert - its now just a local array in the function that its used.
 Removed the rhsmat and coefftmat arrays - its also now a local array.

 12-04-2010 Bec Gorton
 Resized the oi array.
 Resized all the network arrays from :

 (double **) alloc2d(bm->nfzones + 1, bm->K_num_tot_assess + 1);
 to:
 (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_assess + 1);

 Resized the following:

 bioleat = (double **) alloc2d(bm->nbox, bm->K_num_eat);
 biolprod = (double **) alloc2d(bm->nbox, bm->K_num_prod);
 to:

 bioleat = (double **) alloc2d(bm->nbox, bm->K_num_tot_sp);
 biolprod = (double **) alloc2d(bm->nbox, bm->K_num_tot_sp);

 12-05-2010 Bec Gorton
 Resize the sampleprod array to from K_num_prop to K_num_tot_sp.
 Resized the sampleeat array from K_num_eat to K_num_tot_sp.

 14-05-2010 Bec Gorton
 Trunk merge 1558.

 13-07-2010 Bec Gorton
 Removed the last references to K_num_macarofauna

 13-07-2010 Bec Gorton
 Removed spInvName.

 20-10-2010 Bec Gorton
 Removed all references to bm->K_num_charis.
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
#include "atsamplesetup.h"
#include <convertXML.h>

static void Setup_Biased_Sample_Values(MSEBoxModel *bm, FILE *llogfp);

/**
 * \brief Initialise the assessment model.
 */
void Assess_Init(MSEBoxModel *bm, FILE *llogfp) {

	int i, b, nk, nyr, max_nbs, nkk, numageclass, maxageclass, nsp, numyears, numsamples, sp, nf;
	double max_avail = 0;
	char convertedXMLFileName[STRLEN];

	if (verbose)
		fprintf(stderr, "Initialise assessment model\n");

	TINY = 1.0e-20;
	minpool = 1.0e-6;
        
	sprintf(convertedXMLFileName, "%s", bm->assessprmIfname);

	/* If the input file is a prm file convert it to the XML format */
	if(strstr(bm->assessprmIfname, ".xml") == NULL){
		/* Build the converted filename */
		*(strstr(convertedXMLFileName, ".prm")) = '\0';
		strcat(convertedXMLFileName, ".xml");

		/* Convert the input file to XML */
		Convert_Assess_To_XML(bm, bm->assessprmIfname, convertedXMLFileName);
	}

	readModelAssessmentParameters(bm, convertedXMLFileName);

	Setup_Biased_Sample_Values(bm, llogfp);

	/* Setup preference matrix array */
	dietpref = (double ****) alloc4d(bm->K_num_max_stages, bm->K_num_tot_sp, bm->K_num_max_stages, bm->K_num_tot_sp);

	/* Copy over consumer status to integer vector and free read-in vector
	 in Set_Preference_Matrix() */
	//    for (i=0; i<bm->K_num_tot_assess; i++)
	//        CON[i] = (int)(CONii[i]);

	/**** Memory allocation *******************************************************/
	/* NOTES on arrays - particularly sampling arrays and what each vetcor is storing

	 1. Groups follow id number given in setConstantID()

	 2. size bins assumes:
	 [0][group][zone][bin] = weights
	 [1][group][zone][bin] = numbers in stock
	 [2][group][zone][bin] = numbers in catch
	 [3][group][zone][bin] = numbers in discards
	 [4][group][zone][bin] = numbers for selectivity estimate


	 3. Stock arrays use:
	 [0][group][zones] = biomass
	 [1][group][zones] = total numbers in the stock
	 [2][group][zones] = numbers in the catch
	 [3][group][zones] = production
	 [4][group][zones] = consumption
	 [5][group][zones] = numbers in the discards (across fisheries)
	 [6][group][zones] = total catch (across fisheries)
	 [7][group][zones] = total effort (across fisheries)
	 [8][group][zones] = discards (across fisheries)
	 [9][group][zones] = size at maturity


	 4. Invert stock arrays use:
	 [0][group][zones] = total catch (across fisheries)
	 [1][group][zones] = total effort (across fisheries)
	 [2][group][zones] = discards (across fisheries)

	 Note: this replaced samplecatch[][], samplediscards[][]
	 and sampleeffort [][]

	 5. The individual arrays use:
	 [0][size][zones] = weight
	 [1][size][zones] = numbers in the stock
	 [2][size][zones] = numbers in the catch
	 [3][size][zones] = biomass
	 [4][size][zones] = discards
	 [5][size][zones] = q
	 [6][size][zones] = condition

	 Note for birds only use the first 3 size bins, as only classified as
	 fledgling, juvenile and adult not actually sized
	 */

	/* Constants */
	samplearea = (double *) alloc1d(bm->nsbox);
	totalarea = (double *) alloc1d(bm->nsbox);
	zonearea = (double *) alloc1d(bm->nfzones);
	zoneboxes = (int *) i_alloc1d(bm->nfzones);

	/* Data storage */
	biolbiom = (double ***) alloc3d(2, bm->nbox, bm->K_num_tot_sp);
	bioleat = (double **) alloc2d(bm->nbox, bm->K_num_tot_sp);
	biolprod = (double **) alloc2d(bm->nbox, bm->K_num_tot_sp);
	biolVERTinfo = (double ****) alloc4d(bm->nbox, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp, 7);

	phys = (double **) alloc2d(bm->nbox, num_sampled_phy_id);
	physprocess = (double **) alloc2d(bm->nbox, 3);

	/* Sampled data (mostly fisheries independent) */
	individVERTinfo = (double *****) alloc5d(2, bm->nfzones, bm->K_num_size, bm->K_num_tot_sp, K_num_individ_char);

	invstockinfo = (double ****) alloc4d(2, bm->nfzones, bm->K_num_tot_sp, 3);
	pop_fraction = (double ***) alloc3d(bm->nfzones, bm->K_num_tot_sp, 2);
	samplebiom = (double ***) alloc3d(2, bm->nfzones, bm->K_num_tot_sp);
	sampledetbiom = (double ****) alloc4d(2, bm->nfzones, 2, bm->K_num_detritus);
	sampleeat = (double ***) alloc3d(2, bm->nfzones, bm->K_num_tot_sp);
	samplephys = (double ***) alloc3d(2, bm->nfzones, num_sampled_phy_id - 2); // Don't need to sample depth and area.
	sampleprocess = (double ***) alloc3d(2, bm->nfzones, 3);
	sampleprod = (double ***) alloc3d(2, bm->nfzones, bm->K_num_tot_sp);
	stockinfo = (double ****) alloc4d(2, bm->nfzones, bm->K_num_tot_sp, 11);

	/* Fisheries data */
	agelengthkey = (double *****) alloc5d(2, bm->nfzones, bm->K_num_tot_sp, bm->K_max_agekey, bm->K_num_size);
	agebins = (double *****) alloc5d(2, bm->K_num_size, bm->K_max_agekey, bm->nfzones, bm->K_num_tot_sp);
	fishery = (double *****) alloc5d(2, bm->nbox, bm->K_num_tot_sp, bm->K_num_fisheries, 3);
	samplenums = (double **) alloc2d(3, bm->K_num_sampleage);
	sizebins = (double ******) alloc6d(2, 2, bm->K_num_size, bm->nfzones, bm->K_num_tot_sp, 5);

	/* Diet information */
	inshorediet = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	intruediet = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	offshorediet = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	offtruediet = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	totareadiet = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	tottruediet = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	TL = (double ***) alloc3d(2, bm->K_num_tot_sp, 6);
	trueTL = (double ***) alloc3d(2, bm->K_num_tot_sp, 6);

	/*
	 * Replace K_num_tot_assess with
	 * [k_num_tot_sp][2] to allow for adult juv values.
	 * Just not sure what to do about the sediment values
	 * If i include these i end up with a 6D array which is just silly
	 *
	 * I could just have a WC or SED value for the prey as detritus is not a predator.
	 *
	 * So it would be:
	 * K_num_tot_sp, 2, K_num_tot_sp, 2, 2
	 */
	//inshorediet = (double **) alloc2d(bm->K_num_tot_assess, bm->K_num_tot_assess);

	/* Temporary data processing arrays */
	maxageclass = 0;
	for (nsp = 0; nsp < bm->K_num_tot_sp; nsp++) {
		//if (FunctGroupArray[nsp].isVertebrate == TRUE ) {
		if (FunctGroupArray[nsp].isVertebrate == TRUE && FunctGroupArray[nsp].speciesParams[flag_id]) {
			if (!FunctGroupArray[nsp].ageClassSize)
				quit("%s ageclass size set to 0 in biology.prm correct this and restart (set to 1 if group not active)\n", FunctGroupArray[nsp].groupCode);

			if (FunctGroupArray[nsp].ageClassSize > maxageclass)
				maxageclass = FunctGroupArray[nsp].ageClassSize;
		}
	}
	numageclass = maxageclass * bm->K_num_max_cohort * bm->K_num_max_genetypes;

	altavailfood = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);
	availfood = (double ****) alloc4d(2, bm->K_num_tot_sp, 2, bm->K_num_tot_sp);

	//new_altavailfood = (double **) alloc2d(2 * bm->K_num_tot_sp, 2 * bm->K_num_tot_sp);
	//new_availfood = (double **) alloc2d(2 * bm->K_num_tot_sp, 2 * bm->K_num_tot_sp);

	//printf("Creating sample arrays\n");

	contribvert = Util_Alloc_Init_2D_Double(2, bm->K_num_tot_sp, 0.0);
	biom = (double ***) alloc3d(2, bm->nfzones, bm->K_num_tot_sp);
	globalnums = (double ***) alloc3d(2, 2, bm->K_num_tot_sp);
	nsq = (int ****) i_alloc4d(2, bm->K_num_size, bm->nfzones, bm->K_num_tot_sp);
	nums = (double **) alloc2d(2, bm->K_num_tot_sp);
	num_nyr = (double **) alloc2d(2, numageclass);
	oldbaby = (double ***) alloc3d(2, bm->nfzones, bm->K_num_tot_sp);
	totn = (double *) alloc1d(3);
	totnums = (double ***) alloc3d(bm->nfzones, bm->K_num_tot_sp, 4);
	totsamplebiom = (double **) alloc2d(bm->nfzones + 1, 3);
	checkedz = (int *) i_alloc1d(bm->nfzones);

	/* Network calculation arrays */
	biomnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);
	eatnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);
	exportnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);
	importnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);
	mortnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);
	prodnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);
	respnetwk = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp + 1);

	/* Network indice calculation arrays */
	cvsample = (double ***) alloc3d(bm->nfzones, bm->K_num_tot_sp, 3);
	divfn = (int **) i_alloc2d(bm->nfzones, bm->K_num_tot_sp);
	divsp = (int **) i_alloc2d(bm->nfzones, bm->K_num_tot_sp);
	cvphys = (double ***) alloc3d(bm->nfzones, num_sampled_phy_id + 1, 3);
	physicalSigma = (double **) alloc2d(bm->nfzones, num_sampled_phy_id);

	/* Allow for the sediment and wc value of detritus  - could possibly be smarter about this or split them.*/
	//food = (double *) alloc2d(2, bm->K_num_tot_sp);

	globalfledge = (double **) alloc2d(bm->K_num_tot_sp, 3);
	globalpd = (double **) alloc2d(6, 2);
	mineat = Util_Alloc_Init_2D_Double(2, bm->nfzones + 1, MAXDOUBLE);
	numsort = (int *) i_alloc1d(bm->K_num_tot_sp);
	oi = (double ***) alloc3d(2, bm->K_num_tot_sp, 3);
	PBRglobal = (double **) alloc2d(2, bm->K_num_tot_sp);
	permpivot = (int *) i_alloc1d(bm->K_num_tot_sp * 2);

	stomachs = (int **) i_alloc2d(2, bm->K_num_tot_sp);
	upper = (double *) alloc1d(2 * bm->K_num_tot_sp);
	lower = (double *) alloc1d(2 * bm->K_num_tot_sp);
	zasum = (double *) alloc1d(bm->nfzones + 1);

	MultiNom_binIndex = (int *)i_alloc1d(2* bm->K_num_tot_sp);
	MultiNom_cohortIndex= (int *)i_alloc1d(2* bm->K_num_tot_sp);

	/* Arrays for indices */
	avgtl = (double **) alloc2d(bm->nfzones + 2, 8);
	cvt = (double *) alloc1d(bm->nfzones + 1);
	endnums = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp);
	endnumsbig = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_tot_sp); // as K_num_microfauna includes K_num_bacteria
	disrate = (double *) alloc1d(bm->nfzones + 1);
	divindx = (int **) i_alloc2d(bm->nfzones + 1, 2);
	fledge = (double **) alloc2d(bm->nfzones + 1, bm->K_num_tot_sp * 2);
	habindx = (double **) alloc2d(bm->nfzones + 1, 2);
	monbet = (double **) alloc2d(bm->nfzones + 1, 2);
	netwkindx = (double **) alloc2d(bm->nfzones + 1, K_netwk_properties);
	nppb = (double *) alloc1d(bm->nfzones + 1);
	PBRcat = (double **) alloc2d(bm->nfzones + 1, bm->K_num_tot_sp);
	pd = (double **) alloc2d(bm->nfzones + 1, 3);
	pdcat = (double **) alloc2d(bm->nfzones + 1, 3);
	pelbin = (double ***) alloc3d(bm->nfzones + 1, 2, bm->K_num_pelbin);
	sedbin = (double **) alloc2d(bm->nfzones + 1, bm->K_num_sedbin);
	soi = (double *) alloc1d(bm->nfzones + 1);
	trophspect = (double ***) alloc3d(bm->nfzones + 2, 2, bm->K_num_trophbin);

	max_lngth = (double **) alloc2d(3, bm->nfzones + 1);
	bm->rep_box_of_zone = (int ***) i_alloc3d(2, bm->nfzones, bm->K_num_tot_sp);

	/* Stock assessment arrays */
    if (bm->tassessstart > (bm->tstop / 86400.0)){
    	if(bm->pseudo_assess == TRUE){
    		nyr = (int)floor(ROUNDGUARD + ((bm->tstop - bm->tstart)/(86400.0*365.0) + 1.0));
    	}else
    		nyr = 1;
    }else{
        nyr = (int)floor(ROUNDGUARD + (((bm->tstop / 86400.0) - bm->tassessstart)/365.0 + 1.0));
    }

    if(nyr < 0){
    	fprintf(stderr, "ERROR - initAssess. The assessment model has determined that the number of years %d which is a negative number.\nCheck your tassessstart values in your assessment input file\n", nyr);
    	quit("");
    }

	CPUEtrend = (double **) alloc2d(nyr, bm->K_num_tot_sp);

	nk = bm->K_num_max_cohort * bm->K_num_max_genetypes * nyr + 1;
	BootResu = (double *) alloc1d(nk);
	CData = (double **) alloc2d(bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);
	IData = (double **) alloc2d(bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);
	IDatahat = (double **) alloc2d(bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);

	if (bm->K_num_max_cohort * bm->K_num_max_genetypes > num_est_prm)
		nk = bm->K_num_max_cohort * bm->K_num_max_genetypes;
	else
		nk = num_est_prm;

	NEst = (double **) alloc2d(nk, nyr);
	Resu = (double **) alloc2d(nk, nyr);
	F = (double **) alloc2d(nk, nyr);
	P = (double **) alloc2d(nk, nk);
	X = (double *) alloc1d(nk);
	Y = (double *) alloc1d(nk);

	nkk = (int) floor(ROUNDGUARD + (floor(bm->tstop / 365.0)));
	if (nkk < 1)
		nkk = 1;
	XX = (double *) alloc1d(nkk);
	YY = (double *) alloc1d(nkk);

	max_nbs = 0;
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isFished == TRUE) {
			if ((int) (FunctGroupArray[i].speciesParams[assess_bootstrap_id]) > max_nbs)
				max_nbs = (int) (FunctGroupArray[i].speciesParams[assess_bootstrap_id]);
		}
	}

	nk = max_nbs + 2; // One spare in case of overlaps
	NResult = (double ***) alloc3d(nk, bm->K_num_max_cohort * bm->K_num_max_genetypes, nyr);
	ResultToSort = (double *) alloc1d(nk);
	ResultSorted = (double *) alloc1d(nk);
	nbsSort = (int *) i_alloc1d(nk);

	//fprintf(llogfp, "nk: %d\n", nk);

	/* Determine maximum vertebrate ageclass size */
	zoneVERTpopratio = (double ****) alloc4d(bm->nfzones, bm->maxspage, bm->K_num_max_cohort * bm->K_num_max_genetypes, bm->K_num_tot_sp);

	/* Setup preference matrix */
	Set_Preference_Matrix(bm, llogfp);

	/* Initialise arrays */
	for (i = 0; i < bm->nfzones; i++) {
		zonearea[i] = 0.0;
		for (b = 0; b < 8; b++)
			avgtl[b][i] = 0.0;
	}

	for (b = 0; b < bm->nsbox; b++) {
		samplearea[b] = 0.0;
		totalarea[b] = 0.0;
	}

	bm->tassess = bm->tassessstart + bm->tassessinc;
	bm->teatassess = bm->tassessstart + bm->teatassessinc;
	datain = 0;
	firstdata = 1;

	numsteps = (int) (floor(364.0 / bm->tassessinc + 0.5));
	numsteps_orig = numsteps;

	//fprintf(llogfp, "tassess: %e, numsteps: %d, tassessinc: %e, ratio: %e\n", bm->tassess, numsteps, bm->tassessinc, 364.0 / bm->tassessinc);

	if (numsteps < 0) {
		numsteps_orig = 0;
		numsteps = 1;
	}

	if (bm->flagcont && (numsteps > 0) && ((numsteps * bm->tassessinc) > 364)) {
		quit(
				"With the current configuration of tassessinc (%e) which gives numsteps = %d annual samping will fail as would happen after end of year, so won't happen as reset for the new year\n",
				bm->tassessinc, numsteps);
	}

	bm->sample_now = 1;
	bm->annual_assess = 0;
	if (numsteps_orig < 1) {
		bm->annual_assess = 1;
	}

	//printf("numsteps %d, floor: %e, bracket: %e, annual_assess: %d\n", numsteps, (double)floor(364.0 / bm->tassessinc + 0.5),
	//		(double)floor(364.0 / bm->tassessinc + 0.5) - 1.0, bm->annual_assess);

	/** Set up random arrays - strating by defining actual arrays **/
	numyears = (int) (ceil((bm->tstop - bm->tstart) / (365.0 * 86400.0)));
	numsamples = (int) (ceil((bm->tstop - bm->tstart) / (bm->minfreq * 86400.0)));

	bm->rand = (double ***) alloc3d(numyears, nrand_id, bm->K_num_tot_sp);

	/* Numsamples +1 for time 0 and extra +1 for time after tstop (never executed but so don't trip nextij++) */
	bm->tassPatchy = (double **) alloc2d(2, numsamples + 2);

	/* Open the output files in the Assessment Module */
	Open_Attribute_Files(bm);
	Open_Index_Files(bm);

	/* Handle random number generation */

	if (bm->flagrandom == 1) {
		if (bm->flaggen == 1) {
			/* Generate and record random numbers */
			for (b = 0; b < numsamples; b++) {
				bm->tassPatchy[b][tass_id] = bm->tassessinc / drandom(bm->minfreq, bm->maxfreq);
				bm->tassPatchy[b][tasseat_id] = bm->teatassessinc / drandom(bm->minfreq, bm->maxfreq);
			}
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isFished == TRUE) {
					for (b = 0; b < numyears; b++) {
						bm->rand[i][startN_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
						bm->rand[i][calcF_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
						bm->rand[i][calcM_id][b] = Assess_Add_Error(bm, flagprms, 1.0, k_avgprms, k_varprms);
					}
				}
			}
			Write_Random_Numbers(bm, numyears, numsamples);
		}
	} else {
		/* No randomisation so make all scalars 1.0 */
		for (b = 0; b < numsamples; b++) {
			bm->tassPatchy[b][tass_id] = 1.0;
			bm->tassPatchy[b][tasseat_id] = 1.0;
		}
		for (i = 0; i < bm->K_num_tot_sp; i++) {
			if (FunctGroupArray[i].isFished == TRUE) {
				for (b = 0; b < numyears; b++) {
					bm->rand[i][startN_id][b] = 1.0;
					bm->rand[i][calcF_id][b] = 1.0;
					bm->rand[i][calcM_id][b] = 1.0;
				}
			}
		}
	}

	/* No patch sampling so set scalar to 1.0 */
	if (bm->flagpatchy != 1) {
		for (b = 0; b < numsamples; b++) {
			bm->tassPatchy[b][tass_id] = 1.0;
			bm->tassPatchy[b][tasseat_id] = 1.0;
		}
	}
	nextij = 0;
	nexteatij = 0;

	/* Find the primary fishery to sample */
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			max_avail = 0;
			for (nf = 0; nf < bm->K_num_fisheries; nf++) {
				if (bm->SP_FISHERYprms[sp][nf][avail_id] > max_avail) {
					FunctGroupArray[sp].speciesParams[primary_fishery_id] = nf;
					max_avail = bm->SP_FISHERYprms[sp][nf][avail_id];
				}
			}
		}
	}

	return;

}

/*************************** Model ID Set-up Routines ****************************/
/**
 * \brief This routine names the classical assessment parameters that are estimated
 */
void Assess_Set_Name_ID(MSEBoxModel *bm) {
    int paramlength = 20;
    
	snprintf(bm->NAssessNAME[est_med_recruit_id], paramlength, "%s", "Med_Recruit");
	snprintf(bm->NAssessNAME[est_top_recruit_id], paramlength, "%s", "Top_Recruit");
	snprintf(bm->NAssessNAME[est_bot_recruit_id], paramlength, "%s", "Bott_Recruit");
	snprintf(bm->NAssessNAME[est_med_prm2_id], paramlength, "%s", "Med_Prm2");
	snprintf(bm->NAssessNAME[est_top_prm2_id], paramlength, "%s", "Top_Prm2");
	snprintf(bm->NAssessNAME[est_bot_prm2_id], paramlength, "%s", "Bott_Prm2");
	snprintf(bm->NAssessNAME[est_med_stock_id], paramlength, "%s", "Med_Stock");
	snprintf(bm->NAssessNAME[est_top_stock_id], paramlength, "%s", "Top_Stock");
	snprintf(bm->NAssessNAME[est_bot_stock_id], paramlength, "%s", "Bott_Stock");
	snprintf(bm->NAssessNAME[est_SS_id], paramlength, "%s", "Sum_Sq");
	snprintf(bm->NAssessNAME[est_FrefA_id], paramlength, "%s", "FrefA");
	snprintf(bm->NAssessNAME[est_FrefC_id], paramlength, "%s", "FrefC");
	snprintf(bm->NAssessNAME[est_Fcurr_id], paramlength, "%s", "Fcurr");
	snprintf(bm->NAssessNAME[est_M_id], paramlength, "%s", "M_est");
	snprintf(bm->NAssessNAME[est_Ccurr_id], paramlength, "%s", "AvgCurrentCatch");
	snprintf(bm->NAssessNAME[est_Cslope_id], paramlength, "%s", "CPUEtrend");
	snprintf(bm->NAssessNAME[est_numyr_id], paramlength, "%s", "CPUErun");
	snprintf(bm->NAssessNAME[est_CPUEnow_id], paramlength, "%s", "CPUEnow");
	snprintf(bm->NAssessNAME[est_CPUEstart_id], paramlength, "%s", "CPUEtrendStart");

	return;
}

/******************** Free assessment arrays ***************************/
void Assess_Free(MSEBoxModel *bm) {

    printf("Doing Assess_Free\n");
    
	if(bm->useRBCTiers)
		Tier_Assessment_Free(bm);

    i_free1d(bm->nsboxes);
	i_free1d(bm->nsboxes_other);
	i_free1d(bm->nsboxes_intense);
	free2d(detrit_import);
	free2d(DiscardFate);
    
    i_free1d(InOffshorei);
	free4d(dietpref);
	free1d(samplearea);
	free1d(totalarea);
	free1d(zonearea);
	i_free1d(zoneboxes);
	free3d(biolbiom);
	free2d(bioleat);
	free2d(biolprod);
	free4d(biolVERTinfo);
    
    free2d(phys);
	free2d(physprocess);
	free5d(individVERTinfo);
	free4d(invstockinfo);
	free3d(pop_fraction);
	free3d(samplebiom);
	free4d(sampledetbiom);
	free3d(sampleeat);
	free3d(samplephys);
	free3d(sampleprocess);
    
    free3d(sampleprod);
	free4d(stockinfo);
	free5d(agelengthkey);
	free5d(agebins);
	free5d(fishery);
	free2d(samplenums);
	free6d(sizebins);
	free4d(inshorediet);
	free4d(intruediet);
	free4d(offshorediet);
	free4d(offtruediet);
	free4d(totareadiet);
	free4d(tottruediet);
    
    free3d(TL);
	free3d(trueTL);
	free4d(altavailfood);
	free4d(availfood);

	//free2d(new_altavailfood);
	//free2d(new_availfood);

    free2d(contribvert);
	free3d(biom);
	free3d(globalnums);
	i_free4d(nsq);
	free2d(nums);
	free3d(oldbaby);
	free1d(totn);
	free3d(totnums);
	free2d(totsamplebiom);
	free3d(biomnetwk);
	free3d(eatnetwk);
	free3d(exportnetwk);
	free3d(importnetwk);
	free3d(mortnetwk);
	free3d(prodnetwk);
	free3d(respnetwk);
    
    free3d(cvsample);
	free2d(physicalSigma);
	free3d(cvphys);
	i_free2d(divfn);
	i_free2d(divsp);

	free2d(globalfledge);
    
    free2d(globalpd);
	free2d(mineat);
	i_free1d(numsort);
	free3d(oi);
	free2d(PBRglobal);
	i_free1d(permpivot);
	i_free2d(stomachs);
	free1d(upper);
	free1d(lower);
	i_free1d(MultiNom_binIndex);
	i_free1d(MultiNom_cohortIndex);
	free1d(zasum);
	free2d(avgtl);
	free1d(cvt);
	free3d(endnums);
    
    free3d(endnumsbig);
	free1d(disrate);
	i_free2d(divindx);
	free2d(fledge);
	free2d(habindx);
	free2d(monbet);
	free2d(netwkindx);
	free1d(nppb);
	free2d(PBRcat);
	free2d(pd);
	free2d(pdcat);
	free3d(pelbin);
	free2d(sedbin);
    
    free1d(soi);
	free3d(trophspect);
	free2d(max_lngth);
	free1d(BootResu);
	free2d(CData);
	free2d(IData);
	free2d(IDatahat);
	free2d(NEst);
	free2d(Resu);
	free2d(F);
	free2d(P);
	free1d(X);
	free1d(Y);
    
    free3d(NResult);
	free1d(ResultToSort);
	free1d(ResultSorted);
	i_free1d(nbsSort);
	free4d(zoneVERTpopratio);
	i_free1d(checkedz);
	free2d(CPUEtrend);
	free2d(num_nyr);
	free1d(XX);
	free1d(YY);

	free3d(bm->rand);
	free2d(bm->tassPatchy);
	i_free3d(bm->rep_box_of_zone);
    
    /* Free up the error structure */
	free(spErrorStructure);

	/* Close the output files */
	Close_Attribute_Files(bm);
	Close_Index_Files(bm);
    
	return;
}

/**
 * \brief Populate the ErrorStructure for the given invert_type with the given values.
 */
static void Set_Species_Biased_Value(ErrorStructure *spErrorStructure, GROUP_TYPES type, int flag, double WC_k_avg_bb, double WC_k_var_bb,
		double SED_k_avg_bb, double SED_k_var_bb, double k_avg_pp, double k_var_pp, double k_avg_eat, double k_var_eat) {

	spErrorStructure[type].spErrorStructureFlag = flag;
	spErrorStructure[type].k_avg_Biomass[WC] = WC_k_avg_bb;
	spErrorStructure[type].k_var_Biomass[WC] = WC_k_var_bb;
	spErrorStructure[type].k_avg_Biomass[SED] = SED_k_avg_bb;
	spErrorStructure[type].k_var_Biomass[SED] = SED_k_var_bb;
	spErrorStructure[type].k_avg_pp = k_avg_pp;
	spErrorStructure[type].k_var_pp = k_var_pp;
	spErrorStructure[type].k_avg_eat = k_avg_eat;
	spErrorStructure[type].k_var_eat = k_var_eat;
}

/**
 * \brief Set the biased sample values for each invertebrate type.
 */
static void Setup_Biased_Sample_Values(MSEBoxModel *bm, FILE *llogfp) {

	spErrorStructure = (ErrorStructure *) malloc(sizeof(ErrorStructure) * NUM_GROUP_TYPES);

	Set_Species_Biased_Value(spErrorStructure, LG_PHY, flaginvpbiom, k_avgphytl, k_varphytl, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, SM_PHY, flaginvpbiom, k_avgphyts, k_varphyts, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
	// TODO: no values read in for DF and its not handled in code. Need to check with Beth about this.
	Set_Species_Biased_Value(spErrorStructure, DINOFLAG, flagdetbiom, k_avgphyts, k_varphyts, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);

	Set_Species_Biased_Value(spErrorStructure, SM_ZOO, flaginvpbiom, k_avgzoops, k_varzoops, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
	Set_Species_Biased_Value(spErrorStructure, MED_ZOO, flaginvpbiom, k_avgzoopl, k_varzoopl, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
	Set_Species_Biased_Value(spErrorStructure, LG_ZOO, flaginvpbiom, k_avgzoopl, k_varzoopl, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);
	Set_Species_Biased_Value(spErrorStructure, CEP, flaginvpbiom, k_avgcep, k_varcep, 0, 0, k_avgcepp, k_varcepp, k_avgcepeat, k_varcepeat);

	Set_Species_Biased_Value(spErrorStructure, SED_EP_FF, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
	Set_Species_Biased_Value(spErrorStructure, SED_EP_OTHER, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
	Set_Species_Biased_Value(spErrorStructure, CORAL, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
    Set_Species_Biased_Value(spErrorStructure, SPONGE, flagepibiom, k_avgepif1, k_varepif1, 0, 0, k_avgepi1p, k_varepi1p, k_avgepi1eat, k_varepi1eat);
    
	Set_Species_Biased_Value(spErrorStructure, PWN, flagepibiom, k_avgepif2, k_varepif2, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);
	Set_Species_Biased_Value(spErrorStructure, MOB_EP_OTHER, flagepibiom, k_avgepif2, k_varepif2, 0, 0, k_avgepi2p, k_varepi2p, k_avgepi2eat, k_varepi2eat);

	Set_Species_Biased_Value(spErrorStructure, PL_BACT, flaginvpbiom, k_avgpbact, k_varpbact, 0, 0, k_avgpbactp, k_varpbactp, k_avgpbacteat, k_varpbacteat);
	Set_Species_Biased_Value(spErrorStructure, SED_BACT, flaginvpbiom, k_avgsbact, k_varsbact, 0, 0, k_avgsbactp, k_varsbactp, k_avgsbacteat, k_varsbacteat);
	Set_Species_Biased_Value(spErrorStructure, ICE_BACT, flaginvpbiom, k_avgpbact, k_varpbact, 0, 0, k_avgpbactp, k_varpbactp, k_avgpbacteat, k_varpbacteat);
	// Treat ice bacteria like pelagic bacteria for now

	Set_Species_Biased_Value(spErrorStructure, SM_INF, flaginfbiom, k_avginfs, k_varinfs, 0, 0, k_avginfsp, k_varinfsp, k_avginfseat, k_varinfseat);
	Set_Species_Biased_Value(spErrorStructure, LG_INF, flaginfbiom, k_avginfl, k_varinfl, 0, 0, k_avginflp, k_varinflp, k_avginfleat, k_varinfleat);

	Set_Species_Biased_Value(spErrorStructure, PHYTOBEN, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, MICROPHTYBENTHOS, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, SEAGRASS, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, TURF, flagepibiom, k_avgppben, k_varppben, 0, 0, k_avgbenpp, k_varbenpp, 0, 0);


	Set_Species_Biased_Value(spErrorStructure, ICE_DIATOMS, flaginvpbiom, k_avgphytl, k_varphytl, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, ICE_MIXOTROPHS, flagdetbiom, k_avgphyts, k_varphyts, 0, 0, k_avgpelpp, k_varpelpp, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, ICE_ZOOBIOTA, flaginvpbiom, k_avgzoops, k_varzoops, 0, 0, k_avgzoopp, k_varzoopp, k_avgzoopeat, k_varzoopeat);

	//Set_Species_Biased_Value(spErrorStructure, LAB_DET, flagdetbiom, k_avgldet, k_varldet, 0, 0, 0, 0, 0, 0);
	//Set_Species_Biased_Value(spErrorStructure, CARRION, flagdetbiom, k_avgldet, k_varldet, 0, 0, 0, 0, 0, 0);

	Set_Species_Biased_Value(spErrorStructure, REF_DET, flagdetbiom, k_avgrdet, k_varrdet, k_avgrdet, k_varrdet, 0, 0, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, LAB_DET, flagdetbiom, k_avgldet, k_varldet, k_avgldet, k_varldet, 0, 0, 0, 0);
	Set_Species_Biased_Value(spErrorStructure, CARRION, flagdetbiom, k_avgldet, k_varldet, k_avgldet, k_varldet, 0, 0, 0, 0);

}

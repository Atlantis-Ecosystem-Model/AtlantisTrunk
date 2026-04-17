/**
 \file
 \brief Routines to write output
 \ingroup atassess

 File:           atwriting.c

 Created:        8/18/2004

 Author:         Beth Fulton

 Purpose:        Routines to write output
 Modified from general bay version of sampling model

 Arguments:      bm    - Pointer to box model.

 Returns:        void

 Revisions:		8/8/2004 Created from southeast writing.c

 20/9/2004 Removed redundant file creations and
 added WriteAssessOut(), initAssessFile() and
 writeAssessResults()

 7/10/2004 Added write out for additional bioloigcal groups
 (PWN, BFF, BML, FBP, FDE, FDF, FVB, SHR, SSK, REP, SP, WDG)

 22/10/2004 Replaced K_num_tot with K_num_tot_assess

 22-04-2009 Bec Gorton
 Moved the assessment output file into the atAssessModule so
 that they can easily be closed at the end of the model run.

 19-10-2010 Bec Gorton
 Changed the random and assessment output file to be opened and closed
 with the index files.

 12-11-2010 Bec Gorton
 Moved the attribute IO code into this file so i can change the IO functions to only be defined
 within this file.

 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sjwlib.h>
#include "atsample.h"

/* Output files */
static FILE * Init_Abc_File(MSEBoxModel *bm);
static FILE * Init_Cvt_File(MSEBoxModel *bm);
static FILE * Init_Fledge_File(MSEBoxModel *bm);
static FILE * Init_Hab_File(MSEBoxModel *bm);
static FILE * Init_MaxL_File(MSEBoxModel *bm);
static FILE * Init_Mort_File(MSEBoxModel *bm);
static FILE * Init_Monbet_File(MSEBoxModel *bm);
static FILE * Init_Nppb_File(MSEBoxModel *bm);
static FILE * Init_Pbr_File(MSEBoxModel *bm);
static FILE * Init_Pd_File(MSEBoxModel *bm);
static FILE * Init_Tot_Biom_File(MSEBoxModel *bm);

static FILE * Init_Rand_File(MSEBoxModel *bm);

static void Write_Assessed_Group_Names(MSEBoxModel *bm, FILE *fid);

FILE *Init_Stock_File(MSEBoxModel *bm, char *fileName);
FILE *Init_EP_File(MSEBoxModel *bm, char *fname);
FILE *Init_StockSize_File(MSEBoxModel *bm, char *fname);
FILE *Init_StockCatch_File(MSEBoxModel *bm, char *fname);
FILE *Init_Individ_File(MSEBoxModel *bm, char *fname);
FILE * Init_Age_File(MSEBoxModel *bm, char *fname);

FILE *Init_Biom_File(MSEBoxModel *bm, char *fname);
FILE *Init_Catch_File(MSEBoxModel *bm, char *fname);
FILE *Init_Prod_File(MSEBoxModel *bm, char *fname);
FILE * Init_Eat_File(MSEBoxModel *bm, char *fname);

FILE * Init_InDiet_File(MSEBoxModel *bm, char *fname);
FILE * Init_OffDiet_File(MSEBoxModel *bm, char *fname);
FILE * Init_TL_File(MSEBoxModel *bm, char *fname);
FILE * Init_Discard_File(MSEBoxModel *bm, char *fname);
FILE * Init_Phys_File(MSEBoxModel *bm, char *fname);
FILE *Init_Binbiom_File(MSEBoxModel *bm, char *fname);
FILE * Init_Binabund_File(MSEBoxModel *bm, char *fname);
FILE * Init_Div_File(MSEBoxModel *bm, char *fname);
FILE * Init_Avgtl_File(MSEBoxModel *bm, char *fname);
FILE * Init_TrophSpectra_File(MSEBoxModel *bm, char *fname);
FILE * Init_Soi_File(MSEBoxModel *bm, char *fname);
FILE * Init_Oi_File(MSEBoxModel *bm, char *fname);
FILE * Init_PopFract_File(MSEBoxModel *bm, char *fname);
FILE * Init_Netwk_File(MSEBoxModel *bm, char *fname);

FILE * initAssessFile(MSEBoxModel *bm);

/* Write output routines */
void Write_Abcs(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Ages(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_Avgtls(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Binbioms(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Bin_Abunds(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Bioms(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_Catchs(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_Cvts(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Discards(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Divs(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Eats(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_Fledges(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Habs(FILE *fid, MSEBoxModel *bm, int ij);

void Write_InDiets(FILE *fid, MSEBoxModel *bm, int ij, double ****indiet);
void Write_Individs(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_MaxLs(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Monbets(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Morts(FILE *fid, MSEBoxModel *bm, int ij);
void Write_NetwkInfos(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Nppbs(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Pds(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Physs(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_Pbrs(FILE *fid, MSEBoxModel *bm, int ij);
void Write_PopFracts(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Prods(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_OffDiets(FILE *fid, MSEBoxModel *bm, int ij, double ****offdiet);
void Write_Ois(FILE *fid, MSEBoxModel *bm, int ij);
void Write_Sois(FILE *fid, MSEBoxModel *bm, int ij);
void Write_StockEP(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_StockF(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_StockNums(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_StockSize(FILE *fid, MSEBoxModel *bm, int ij, int id);
void Write_TLs(FILE *fid, MSEBoxModel *bm, int ij, double ***tl);
void Write_TrophSpec(FILE *fid, MSEBoxModel *bm, int ij);
void Write_TotBioms(FILE *fid, MSEBoxModel *bm, int ij);

void Write_Assess_Results(FILE *fid, MSEBoxModel *bm, int ij);
void Write_RandNum(FILE *fid, MSEBoxModel *bm, int numyr, int numsamples);

/**
 * @name The diet out files
 */
//@{


FILE *indietfp;
FILE *offdietfp;
FILE *oifp;

FILE *stocknumfp;
FILE *stockepfp;
FILE *stockszfp;
FILE *stockffp;
FILE *individfp;
FILE *agefp;
FILE *physfp;
FILE *biomfp;
FILE *catchfp;
FILE *prodfp;
FILE *eatfp;
FILE *discardfp;
FILE *pbrfp;
FILE *binbiomfp;
FILE *binabundfp;
FILE *abcfp;
FILE *cvtfp;
FILE *divfp;
FILE *habfp;
FILE *fledgefp;
FILE *monbetfp;
FILE *pdfp;
FILE *nppbfp;
FILE *avgtlfp;
FILE *trophspectfp;
FILE *popfractfp;
FILE *nifp;
FILE *TLfp;
FILE *soifp;
FILE *mortfp;
FILE *totbiomfp;
FILE *maxlfp;

/**
 * @name The Attribute Files. These are opened in Write_Attributes
 * and left open. They are closed at the end of the model run in the function
 * Close_Attribute_Files().
 */
//@{
static FILE *stnumsfp;
static FILE *stepfp;
static FILE *stszfp;
static FILE *stffp;
static FILE *ifp;
static FILE *bfp;
static FILE *cfp;
static FILE *pfp;
static FILE *efp;
static FILE *bbfp;
static FILE *bafp;
static FILE *dfp;
static FILE *atlfp;
static FILE *tlsfp;
static FILE *TL2fp;
static FILE *sofp;
static FILE *phfp;
static FILE *popfp;
static FILE *netfp;

static FILE *indfp;
static FILE *offdfp;
static FILE *oofp;

//static FILE * initRtFile(MSEBoxModel *bm);

/* Random number files */
FILE *randoutfp;
FILE *assessfp;

/* Utility Functions */

static void Write_Group_Name(MSEBoxModel *bm, FILE *fid, int prey, int preyStage) {

	if (FunctGroupArray[prey].isVertebrate == TRUE) {
		if (preyStage == juv_id)
			fprintf(fid, "%sj ", FunctGroupArray[prey].groupCode);
		else
			fprintf(fid, "%sa ", FunctGroupArray[prey].groupCode);

	} else if (FunctGroupArray[prey].isDetritus == TRUE) {
		if (preyStage == SED) {
			fprintf(fid, "%ssed ", FunctGroupArray[prey].groupCode);
		} else {
			fprintf(fid, "%s ", FunctGroupArray[prey].groupCode);
		}
	} else {
		fprintf(fid, "%s ", FunctGroupArray[prey].groupCode);
	}
}

static void Write_Verts(MSEBoxModel *bm, FILE *fid, char *str) {
	int sp;

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			fprintf(fid, "%s%s ", FunctGroupArray[sp].groupCode, str);
		}
	}
}

static void Write_Stock_Values(MSEBoxModel *bm, FILE *fid, int z, int id, int paramID) {
	int sp;

	if (paramID == sbiomass_id) {
		quit("Cannot function Write_Stock_Values use with sbiomass_id");
	}
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			fprintf(fid, "%.8g ", stockinfo[paramID][sp][z][id]);

		}
	}

}
void Write_Assessed_Group_Names(MSEBoxModel *bm, FILE *fid) {
	int pred;

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE) {

			if (FunctGroupArray[pred].isVertebrate == TRUE) {
				fprintf(fid, "%sj ", FunctGroupArray[pred].groupCode);
				fprintf(fid, "%sa ", FunctGroupArray[pred].groupCode);

			} else if (FunctGroupArray[pred].isDetritus == TRUE) {
				fprintf(fid, "%s ", FunctGroupArray[pred].groupCode);
				fprintf(fid, "%ssed ", FunctGroupArray[pred].groupCode);
			} else {
				fprintf(fid, "%s ", FunctGroupArray[pred].groupCode);
			}
		}
	}
}

/******************************************************************************************/
/*************************** Index IO functions ****************/

/**
 * \brief Open the output index files.
 *
 */
void Open_Index_Files(MSEBoxModel *bm) {
	/* Initialise files if necessary */
	stocknumfp = Init_Stock_File(bm, "stocknuminfo.txt");
	stockepfp = Init_EP_File(bm, "stockprodinfo.txt");
	stockszfp = Init_StockSize_File(bm, "stocksizeinfo.txt");

	stockffp = Init_StockCatch_File(bm, "stockcatchinfo.txt");
	individfp = Init_Individ_File(bm, "individinfo.txt");
	agefp = Init_Age_File(bm, "ageinfo.txt");
	physfp = Init_Phys_File(bm, "physinfo.txt");
	biomfp = Init_Biom_File(bm, "biominfo.txt");
	catchfp = Init_Catch_File(bm, "catchinfo.txt");
	prodfp = Init_Prod_File(bm, "prodinfo.txt");
	eatfp = Init_Eat_File(bm, "eatinfo.txt");
	TLfp = Init_TL_File(bm, "TLinfo.txt");
	soifp = Init_Soi_File(bm, "soiindex.txt");
	discardfp = Init_Discard_File(bm, "discardindex.txt");
	pbrfp = Init_Pbr_File(bm);
	binbiomfp = Init_Binbiom_File(bm, "biombinindex.txt");
	binabundfp = Init_Binabund_File(bm, "abundbinindex.txt");
	abcfp = Init_Abc_File(bm);
	cvtfp = Init_Cvt_File(bm);
	divfp = Init_Div_File(bm, "divindex.txt");
	habfp = Init_Hab_File(bm);
	fledgefp = Init_Fledge_File(bm);
	monbetfp = Init_Monbet_File(bm);
	pdfp = Init_Pd_File(bm);
	nppbfp = Init_Nppb_File(bm);
	avgtlfp = Init_Avgtl_File(bm, "avgtlindex.txt");
	trophspectfp = Init_TrophSpectra_File(bm, "trophspecindex.txt");
	popfractfp = Init_PopFract_File(bm, "popfractsindex.txt");
	nifp = Init_Netwk_File(bm, "netwkindex.txt");
	mortfp = Init_Mort_File(bm);
	totbiomfp = Init_Tot_Biom_File(bm);
	maxlfp = Init_MaxL_File(bm);

	/* Initialise files if necessary */
	randoutfp = Init_Rand_File(bm);
	assessfp = initAssessFile(bm);
}

/**
 * \brief Close all the index output files
 */
void Close_Index_Files(MSEBoxModel *bm) {

	Util_Close_Output_File(indietfp);
	Util_Close_Output_File(offdietfp);
	Util_Close_Output_File(oifp);

	/* The sample output files */
	Util_Close_Output_File(stocknumfp);
	Util_Close_Output_File(stockepfp);
	Util_Close_Output_File(stockszfp);
	Util_Close_Output_File(stockffp);
	Util_Close_Output_File(individfp);
	Util_Close_Output_File(agefp);
	Util_Close_Output_File(physfp);
	Util_Close_Output_File(biomfp);
	Util_Close_Output_File(catchfp);
	Util_Close_Output_File(prodfp);
	Util_Close_Output_File(eatfp);
	Util_Close_Output_File(discardfp);
	Util_Close_Output_File(pbrfp);
	Util_Close_Output_File(binbiomfp);
	Util_Close_Output_File(binabundfp);
	Util_Close_Output_File(abcfp);
	Util_Close_Output_File(cvtfp);
	Util_Close_Output_File(divfp);
	Util_Close_Output_File(habfp);
	Util_Close_Output_File(fledgefp);
	Util_Close_Output_File(monbetfp);
	Util_Close_Output_File(pdfp);
	Util_Close_Output_File(nppbfp);
	Util_Close_Output_File(avgtlfp);
	Util_Close_Output_File(trophspectfp);
	Util_Close_Output_File(popfractfp);
	Util_Close_Output_File(nifp);
	Util_Close_Output_File(TLfp);
	Util_Close_Output_File(soifp);
	Util_Close_Output_File(mortfp);
	Util_Close_Output_File(totbiomfp);
	Util_Close_Output_File(maxlfp);
	/* Close the random number files */
	Util_Close_Output_File(randoutfp);
	Util_Close_Output_File(assessfp);

}
/**
 *	\brief This writes out all the sample output
 *
 */
void Write_Index_Out(MSEBoxModel *bm, int ij) {
	if (verbose)
		fprintf(stderr, "Writing output\n");

	/* Write files */
	Write_StockNums(stocknumfp, bm, ij, sample_id);
	Write_StockEP(stockepfp, bm, ij, sample_id);
	Write_StockSize(stockszfp, bm, ij, sample_id);
	Write_StockF(stockffp, bm, ij, sample_id);
	Write_Individs(individfp, bm, ij, sample_id);
	Write_Physs(physfp, bm, ij, sample_id);
	Write_Bioms(biomfp, bm, ij, sample_id);
	Write_Catchs(catchfp, bm, ij, sample_id);
	Write_Prods(prodfp, bm, ij, sample_id);
	Write_Eats(eatfp, bm, ij, sample_id);

	Write_Discards(discardfp, bm, ij);
	Write_Pbrs(pbrfp, bm, ij);
	Write_Binbioms(binbiomfp, bm, ij);
	Write_Bin_Abunds(binabundfp, bm, ij);
	Write_Abcs(abcfp, bm, ij);
	Write_Cvts(cvtfp, bm, ij);
	Write_Divs(divfp, bm, ij);
	Write_Habs(habfp, bm, ij);
	Write_Fledges(fledgefp, bm, ij);
	Write_Monbets(monbetfp, bm, ij);
	Write_Pds(pdfp, bm, ij);
	Write_Nppbs(nppbfp, bm, ij);
	Write_Avgtls(avgtlfp, bm, ij);
	Write_TrophSpec(trophspectfp, bm, ij);
	Write_PopFracts(popfractfp, bm, ij);
	Write_NetwkInfos(nifp, bm, ij);
	Write_TLs(TLfp, bm, ij, TL);
	Write_Sois(soifp, bm, ij);
	Write_Morts(mortfp, bm, ij);
	Write_MaxLs(maxlfp, bm, ij);
	Write_TotBioms(totbiomfp, bm, ij);

	if (bm->ageout)
		Write_Ages(agefp, bm, ij, sample_id);

	return;
}

/******************************************************************************************/
/*************************** Attribute IO functions ****************/

/* This writes out all the attribute output
 */
void Write_Attributes(MSEBoxModel *bm, int ij) {
	if (verbose)
		fprintf(stderr, "Writing attribute output\n");

	/* Write files */
	Write_StockNums(stnumsfp, bm, ij, attrib_id);
	Write_StockEP(stepfp, bm, ij, attrib_id);
	Write_StockSize(stszfp, bm, ij, attrib_id);
	Write_StockF(stffp, bm, ij, attrib_id);
	Write_Individs(ifp, bm, ij, attrib_id);
	Write_Physs(phfp, bm, ij, attrib_id);
	Write_Bioms(bfp, bm, ij, attrib_id);
	Write_Catchs(cfp, bm, ij, attrib_id);
	Write_Prods(pfp, bm, ij, attrib_id);
	Write_Eats(efp, bm, ij, attrib_id);

	Write_Binbioms(bbfp, bm, ij);
	Write_Bin_Abunds(bafp, bm, ij);
	Write_Divs(dfp, bm, ij);
	Write_Avgtls(atlfp, bm, ij);
	Write_TrophSpec(tlsfp, bm, ij);
	Write_TLs(TL2fp, bm, ij, trueTL);
	Write_Sois(sofp, bm, ij);
	Write_PopFracts(popfp, bm, ij);
	Write_NetwkInfos(netfp, bm, ij);

	return;
}

/**
 * \brief Open the attribute output files
 *
 */
void Open_Attribute_Files(MSEBoxModel *bm) {

	/* Initialise files if necessary */
	stnumsfp = Init_Stock_File(bm, "stocknumattribs.txt");
	stepfp = Init_EP_File(bm, "stockprodattribs.txt");
	stszfp = Init_StockSize_File(bm, "stocksizeattribs.txt");
	stffp = Init_StockCatch_File(bm, "stockcatchattribs.txt");
	ifp = Init_Individ_File(bm, "individattribs.txt");

	//if( !afp )
	//	afp = Init_Age_File(bm, "ageattribs.txt");

	phfp = Init_Phys_File(bm, "physattrib.txt");
	bfp = Init_Biom_File(bm, "biomattribs.txt");
	cfp = Init_Catch_File(bm, "catchattribs.txt");
	pfp = Init_Prod_File(bm, "prodattribs.txt");
	efp = Init_Eat_File(bm, "eatattribs.txt");
	bbfp = Init_Binbiom_File(bm, "biombinattribs.txt");
	bafp = Init_Binabund_File(bm, "abundbinattribs.txt");
	dfp = Init_Div_File(bm, "divattribs.txt");
	atlfp = Init_Avgtl_File(bm, "avgtlattribs.txt");
	tlsfp = Init_TrophSpectra_File(bm, "trophspecattrib.txt");
	//if( !rtfp )				Currently not calculating residence time
	//	rtfp = initRtFile(bm);

	TL2fp = Init_TL_File(bm, "TLattribs.txt");
	sofp = Init_Soi_File(bm, "soiattribs.txt");
	popfp = Init_PopFract_File(bm, "popfractattrib.txt");
	netfp = Init_Netwk_File(bm, "netwkattrib.txt");
}
/**
 * \brief Close the attribute output files.
 */
void Close_Attribute_Files(MSEBoxModel *bm) {

	Util_Close_Output_File(stnumsfp);
	Util_Close_Output_File(stepfp);
	Util_Close_Output_File(stszfp);
	Util_Close_Output_File(stffp);
	Util_Close_Output_File(ifp);

	Util_Close_Output_File(phfp);
	Util_Close_Output_File(bfp);
	Util_Close_Output_File(cfp);
	Util_Close_Output_File(pfp);
	Util_Close_Output_File(efp);

	Util_Close_Output_File(atlfp);
	Util_Close_Output_File(tlsfp);
	Util_Close_Output_File(TL2fp);
	Util_Close_Output_File(sofp);

	Util_Close_Output_File(popfp);
	Util_Close_Output_File(netfp);

	Util_Close_Output_File(bbfp);
	Util_Close_Output_File(bafp);
	Util_Close_Output_File(dfp);

	/* close the diet out files */
	Util_Close_Output_File(offdfp);
	Util_Close_Output_File(oofp);
	Util_Close_Output_File(indfp);
}
/**
 * \brief This writes out all the attribute output
 */
void Write_True_Diet_Out(MSEBoxModel *bm, int ij) {
	if (verbose)
		fprintf(stderr, "Writing attribute diet output\n");

	/* Initialise files if necessary */
	if (!indfp)
		indfp = Init_InDiet_File(bm, "indietattribs.txt");
	if (!offdfp)
		offdfp = Init_OffDiet_File(bm, "offdietattribs.txt");
	if (!oofp)
		oofp = Init_Oi_File(bm, "oiattribs.txt");

	/* Write files */
	Write_InDiets(indfp, bm, ij, intruediet);
	Write_OffDiets(offdfp, bm, ij, offtruediet);
	Write_Ois(oofp, bm, ij);

	return;
}
/******************************************************************************************/
/*************************** The actual IO functions that are called above ****************/
FILE *Init_Stock_File(MSEBoxModel *bm, char *fileName) {
	FILE *fid;

	/* Create file */
	if ((fid = Util_fopen(bm,fileName, "w")) == NULL)
		quit("Init_Stock_File: Can't open %s\n", fileName);

	/* Column definitions */
	fprintf(fid, "#Time Zone ");

	Write_Verts(bm, fid, "_Biom");
	Write_Verts(bm, fid, "_NumStock");
	Write_Verts(bm, fid, "_NumCatch");
	Write_Verts(bm, fid, "_Discards");

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the stock information file
 *
 */
void Write_StockNums(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int z, sp;
	double area_correct = 0.0;

	if (verbose > 1)
        printf( "Write stock numbers\n");

	for (z = 0; z < bm->nfzones; z++) {
		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		if (bm->flagareacorrect)
			area_correct = (zonearea[z] / (1000.0 * 1000.0)) + small_num; /* So can get output per km2 from m2 total areas*/
		else
			area_correct = 1.0;

		for (sp = 0; sp < bm->K_num_tot_sp; sp++)
			if (FunctGroupArray[sp].isVertebrate == TRUE)
				fprintf(fid, "%.8g ", stockinfo[sbiomass_id][sp][z][id] * mg_2_tonne / area_correct);

		Write_Stock_Values(bm, fid, z, id, sstocknums_id );
		Write_Stock_Values(bm, fid, z, id, scatchnums_id );
		Write_Stock_Values(bm, fid, z, id, sdiscardnums_id );

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise stock EP information file
 *
 */
FILE * Init_EP_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_EP_File: Can't open %s\n",fname);

	/* Column definitions */
    fprintf(fid, "#Time Zone ");

	Write_Verts(bm, fid, "_Prod");
	Write_Verts(bm, fid, "_Eat");

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write out the data to the EP stock information file
 *
 */
void Write_StockEP(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int z;

	if (verbose > 1)
        printf( "Write stock production\n");

	for (z = 0; z < bm->nfzones; z++) {
		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		Write_Stock_Values(bm, fid, z, id, sprod_id );
		Write_Stock_Values(bm, fid, z, id, seat_id );

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the stock Sz information file
 *
 */
FILE *Init_StockSize_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_StockSize_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");

	Write_Verts(bm, fid, "_Sizemat");
	Write_Verts(bm, fid, "_Condn");

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write out data to the stock size information file
 *
 */
void Write_StockSize(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int z;

	if (verbose > 1)
        printf( "Write stock condition info\n");

	for (z = 0; z < bm->nfzones; z++) {
		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		Write_Stock_Values(bm, fid, z, id, ssizemat_id );
		Write_Stock_Values(bm, fid, z, id, scondn_id );

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the stock F information file
 *
 */
FILE *Init_StockCatch_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_StockCatch_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");

	Write_Verts(bm, fid, "_Catch");
	Write_Verts(bm, fid, "_Discards");
	Write_Verts(bm, fid, "_Effort");
	Write_Verts(bm, fid, "_CPUE");

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write out data to the stock F information file
 *
 */
void Write_StockF(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int z, sp;
	if (verbose > 1)
        printf( "Stock catch info\n");

	for (z = 0; z < bm->nfzones; z++) {
		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		Write_Stock_Values(bm, fid, z, id, stotcatch_id );
		Write_Stock_Values(bm, fid, z, id, sdiscards_id );
		Write_Stock_Values(bm, fid, z, id, stoteffort_id );

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE) {
				fprintf(fid, "%.8g ", stockinfo[stotcatch_id][sp][z][id] / stockinfo[stoteffort_id][sp][z][id]);
			}
		}

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise individual stock information file
 *
 */
FILE *Init_Individ_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Individ_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone Species Bin Weight NumStock NumCatch Biom Discards q condn #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the individual stock information file
 *
 */
void Write_Individs(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z, k, sp;

	if (verbose > 1)
        printf( "Write individual info\n");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			for (k = 0; k < bm->K_num_size; k++) {
				for (z = 0; z < bm->nfzones; z++) {
					/* Write Time */
					fprintf(fid, "%d ", ij);

					/* Write Zone */
					fprintf(fid, "%d ", z);

					/* Write Species */
					fprintf(fid, "%d ", sp);

					/* Write Bin */
					fprintf(fid, "%d ", k);

					/* Write Info */
					for (j = 0; j < 7; j++) {
						fprintf(fid, "%.8g ", individVERTinfo[j][sp][k][z][id]);
					}

					/* New line */
					fprintf(fid, "\n");
				}
			}

		}
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Age Length stock information file
 *
 */
FILE * Init_Age_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Age_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone Species Age Bin0 Bin1 Bin2 Bin3 Bin4 Bin5 Bin6 Bin7 Bin8 Bin9 Bin10 #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Age Length stock information file
 *
 */
void Write_Ages(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z, k, sp;

	if (verbose > 1)
        printf( "Write age info\n");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE) {
			for (k = 0; k < bm->K_max_agekey; k++) {
				for (z = 0; z < bm->nfzones; z++) {
					/* Write Time */
					fprintf(fid, "%d ", ij);

					/* Write Zone */
					fprintf(fid, "%d ", z);

					/* Write Species */
					fprintf(fid, "%d ", sp);

					/* Write Age */
					fprintf(fid, "%d ", k);

					/* Write Info */
					for (j = 0; j < bm->K_num_size; j++)
						fprintf(fid, "%.8g ", agelengthkey[j][k][sp][z][id]);

					/* New line */
					fprintf(fid, "\n");
				}
			}
		}
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the physical and nutrient information file
 *
 */
FILE * Init_Phys_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Phys_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone Salt LightWc LightSed O2 DON DIN Chla nitrif denitrif #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the physical and nutrient information file
 *
 */
void Write_Physs(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z;

	if (verbose > 1)
        printf( "Write physics\n");

	for (z = 0; z < bm->nfzones; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		for (j = 0; j < 6; j++)
			fprintf(fid, "%.8g ", samplephys[j][z][id]);
		for (j = 2; j >= 0; j--)
			fprintf(fid, "%.8g ", sampleprocess[j][z][id]);

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Invertebrate Biomass information file
 *
 */
FILE * Init_Biom_File(MSEBoxModel *bm, char *fname) {
	int sp;
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Biom_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE) {
			if (FunctGroupArray[sp].isDetritus == TRUE) {
				fprintf(fid, "%s ", FunctGroupArray[sp].groupCode);
				fprintf(fid, "%ssed ", FunctGroupArray[sp].groupCode);
			} else {
				fprintf(fid, "%s ", FunctGroupArray[sp].groupCode);
			}
		}
	}

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Invertebrate Biomass information file
 *
 */
void Write_Bioms(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z;
	int detIndex;
	double area_correct = 0.0;

	if (verbose > 1)
        printf( "Write biomasses\n");

	for (z = 0; z < bm->nfzones; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		if (bm->flagareacorrect)
			area_correct = (zonearea[z] / (1000.0 * 1000.0)) + small_num; /* So can get output per km2 from m2 total areas*/
		else
			area_correct = 1.0;

		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isVertebrate == FALSE) {
				if (FunctGroupArray[j].isDetritus == TRUE) {
					detIndex = FunctGroupArray[j].detIndex;
					fprintf(fid, "%.8g ", sampledetbiom[detIndex][WC][z][id] * mg_2_tonne / area_correct);
					fprintf(fid, "%.8g ", sampledetbiom[detIndex][SED][z][id] * mg_2_tonne / area_correct);
				} else {
					fprintf(fid, "%.8g ", samplebiom[j][z][id] * mg_2_tonne / area_correct);
				}
			}
		}

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Invertebrate Catch information file
 *
 */
FILE *Init_Catch_File(MSEBoxModel *bm, char *fname) {
	int sp;
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Catch_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE))
			fprintf(fid, "Catch%s ", FunctGroupArray[sp].groupCode);
	}
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE))
			fprintf(fid, "Eff%s ", FunctGroupArray[sp].groupCode);
	}
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE))
			fprintf(fid, "Dis%s ", FunctGroupArray[sp].groupCode);
	}
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE))
			fprintf(fid, "CPUE%s ", FunctGroupArray[sp].groupCode);
	}
	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Invertebrate Biomass information file
 *
 */
void Write_Catchs(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z, sp;

	if (verbose > 1)
        printf( "Write invertebrate catches\n");

	for (z = 0; z < bm->nfzones; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		for (j = 0; j < 3; j++) {
			for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
				if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {
					fprintf(fid, "%.8g ", invstockinfo[j][sp][z][id]);
				}
			}
		}

		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == FALSE && (FunctGroupArray[sp].isFished == TRUE || FunctGroupArray[sp].isImpacted == TRUE)) {
				fprintf(fid, "%.8g ", invstockinfo[0][sp][z][id] / (invstockinfo[1][sp][z][id] + small_num));
			}
		}

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Invertebrate Production information file
 *
 */
FILE *Init_Prod_File(MSEBoxModel *bm, char *fname) {
	int sp;
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Prod_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isDetritus == FALSE) {
			fprintf(fid, "%sp ", FunctGroupArray[sp].groupCode);
		}
	}

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Invertebrate Production  information file
 *
 */
void Write_Prods(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z;

	if (verbose > 1)
        printf( "Write production\n");

	for (z = 0; z < bm->nfzones; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isVertebrate == FALSE && FunctGroupArray[j].isDetritus == FALSE) {
				fprintf(fid, "%.8g ", sampleprod[j][z][id]);
			}
		}

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Invertebrate Consumption information file
 *
 */
FILE * Init_Eat_File(MSEBoxModel *bm, char *fname) {
	int sp;
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Eat_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].isGrazer == TRUE && FunctGroupArray[sp].groupType != DINOFLAG) {
			fprintf(fid, "%s ", FunctGroupArray[sp].groupCode);
		}
	}
	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Invertebrate Consumption information file
 *
 */
void Write_Eats(FILE *fid, MSEBoxModel *bm, int ij, int id) {
	int j, z;

	if (verbose > 1)
        printf( "Write consumption\n");

	for (z = 0; z < bm->nfzones; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */

		// TODO : We should be including DF in this - check with Beth.
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isVertebrate == FALSE && FunctGroupArray[j].isGrazer == TRUE && FunctGroupArray[j].groupType != DINOFLAG) {
				fprintf(fid, "%.8g ", sampleeat[j][z][id]);
			}
		}

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

static void Write_Diet_Info(MSEBoxModel *bm, FILE *fid, int time, double ****dietInfo) {

	int pred, predStage, prey, preyStage;
	int maxPredStage, maxPreyStage;

	for (prey = 0; prey < bm->K_num_tot_sp; prey++) {
		if (FunctGroupArray[prey].isAssessed == TRUE) {

			if (FunctGroupArray[prey].isVertebrate == TRUE || FunctGroupArray[prey].isDetritus == TRUE)
				maxPreyStage = adult_id;
			else
				maxPreyStage = juv_id;

			for (preyStage = 0; preyStage <= maxPreyStage; preyStage++) {

				/* Write Time */
				fprintf(fid, "%d ", time);

				/* Write Prey Name*/
				Write_Group_Name(bm, fid, prey, preyStage);

				for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
					if (FunctGroupArray[pred].isAssessed == TRUE) {
						if (FunctGroupArray[pred].isVertebrate == TRUE || FunctGroupArray[pred].isDetritus == TRUE)
							maxPredStage = adult_id;
						else
							maxPredStage = juv_id;

						for (predStage = 0; predStage <= maxPredStage; predStage++) {
							fprintf(fid, "%.8g ", dietInfo[pred][predStage][prey][preyStage]);

						}
					}
				}
				/* New line */
				fprintf(fid, "\n");
			}
		}
	}
}

/**
 *
 *	\brief Routine to initialise the Inshore Data Matrix information file
 *
 */
FILE * Init_InDiet_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

	/* Create file */
	if ((fid = Util_fopen(bm, fname, "w")) == NULL)
		quit("Init_InDiet_File: Can't open %s\n", fname);

	/* Column definitions */
	fprintf(fid, "#Time Location ");

	Write_Assessed_Group_Names(bm, fid);

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Inshore Data Matrix information file
 *
 */
void Write_InDiets(FILE *fid, MSEBoxModel *bm, int ij, double ****indiet) {

	if (verbose > 1)
        printf( "Write inshore diet information\n");

	Write_Diet_Info(bm, fid, ij, indiet);
	return;
}

/**
 *
 *	\brief Routine to initialise the Offshore Data Matrix information file
 *
 */
FILE * Init_OffDiet_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_OffDiet_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Location ");

	Write_Assessed_Group_Names(bm, fid);

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Offshore Data Matrix information file
 *
 */
void Write_OffDiets(FILE *fid, MSEBoxModel *bm, int ij, double ****offdiet) {

	if (verbose > 1)
        printf( "Writing offshore diet information\n");

	Write_Diet_Info(bm, fid, ij, offdiet);

	return;
}

/**
 *
 *	\brief Routine to initialise the Trophic level information file
 *
 */
FILE * Init_TL_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_TL_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Location ");

	Write_Assessed_Group_Names(bm, fid);

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Trophic level information file
 *
 */
void Write_TLs(FILE *fid, MSEBoxModel *bm, int ij, double ***tl) {
	int sp, stage;
	int maxStage;
	char *location[3] =
		{ "In", "Off", "Tot" };
	int locationIndex;

	if (verbose > 1)
        printf( "Write trophic level information\n");

	for (locationIndex = 0; locationIndex <= 2; locationIndex++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Location */
		fprintf(fid, "%s ", location[locationIndex]);

		/* Write Info */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isAssessed == TRUE) {
				if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isDetritus == TRUE) {
					maxStage = adult_id;
				} else {
					maxStage = juv_id;
				}
				for (stage = 0; stage <= maxStage; stage++) {
					fprintf(fid, "%.8g ", tl[locationIndex][sp][stage]);
				}
			}
		}

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Discard information file
 *
 */
FILE * Init_Discard_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Discard_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone DiscardRate#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Discard information file
 *
 */
void Write_Discards(FILE *fid, MSEBoxModel *bm, int ij) {
	int z;

	if (verbose > 1)
        printf( "Write discards\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Index */
		fprintf(fid, "%.8g\n", disrate[z]);

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the PBR indices information file
 *
 */
FILE * Init_Pbr_File(MSEBoxModel *bm) {
	int sp;
	FILE *fid;
	char *fname = "pbrindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Pbr_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].groupType == CEP) {
			fprintf(fid, "PBR_%s ", FunctGroupArray[sp].groupCode);
		}
	}

	/* New line */
	fprintf(fid, "\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Discard information file
 *
 */
void Write_Pbrs(FILE *fid, MSEBoxModel *bm, int ij) {
	int sp, z;

	if (verbose > 1)
        printf( "Write PBR\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].groupType == CEP) {
				fprintf(fid, "%.8g ", PBRcat[sp][z]);
			}
		}

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Biomass size spectra indices file
 *
 */
FILE *Init_Binbiom_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Binbiom_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid,
			"#Time Zone p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17 p18 p19 p20 p21 p22 p23 p24 p25 p26 p27 p28 p29 p30 p31 p32 p33 p34 s0 s1 s2 s3 #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Biomass size spectra indices file
 *
 */
void Write_Binbioms(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write biomass bins\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Biomass Bin Info */
		for (j = 0; j < bm->K_num_pelbin; j++)
			fprintf(fid, "%.8g ", pelbin[j][bioindex_id][z]);
		for (j = 0; j < bm->K_num_sedbin; j++)
			fprintf(fid, "%.8g ", sedbin[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Abundance size spectra indices file
 *
 */
FILE * Init_Binabund_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Binabund_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid,
			"#Time Zone p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17 p18 p19 p20 p21 p22 p23 p24 p25 p26 p27 p28 p29 p30 p31 p32 p33 p34 #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Abundance size spectra indices file
 *
 */
void Write_Bin_Abunds(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write abundance bins\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Abundance Bin Info */
		for (j = 0; j < bm->K_num_pelbin; j++)
			fprintf(fid, "%.8g ", pelbin[j][abundindex_id][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the ABC curve indices file
 *
 */
FILE * Init_Abc_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "abcindex.txt";
	int j = 0;
	int index;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Abc_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone");
	for (j = 0; j < bm->K_num_abcbin; j++)
		fprintf(fid, " B%d", j + 1);
	for (j = 0; j < bm->K_num_abcbin; j++)
		fprintf(fid, " A%d", j + 1);

	index = 0;
	for (j = 0; j < bm->K_num_tot_sp; j++) {
		if (FunctGroupArray[j].isMacroFauna == TRUE) {
			fprintf(fid, " BB%d", index + 1);
			index++;
		}
	}
	fprintf(fid, " BB%d", index + 1);

	index = 0;
	for (j = 0; j < bm->K_num_tot_sp; j++) {
		if (FunctGroupArray[j].isMacroFauna == TRUE) {
			fprintf(fid, " BA%d", index + 1);
			index++;
		}
	}
	fprintf(fid, " BA%d", index + 1);

	//	for (j = 0; j < bm->K_num_abcbin - bm->K_num_microfauna; j++)
	//		fprintf(fid, " BA%d", j + 1);

	/* New line */
	fprintf(fid, "\n");
	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the ABC curve indices file
 *
 */
void Write_Abcs(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;
	int index;

	if (verbose > 1)
        printf( "Write abc bins\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Biomass Info */
        for (j = 0; j < bm->K_num_abcbin; j++) {
			fprintf(fid, "%.8g ", endnums[j][bioindex_id][z]);
		}

		/* Write Abundance Info */
		for (j = 0; j < bm->K_num_abcbin; j++) {
			fprintf(fid, "%.8g ", endnums[j][abundindex_id][z]);
		}

		/* Write Biomass Info */
		index = 0;
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isMacroFauna == TRUE) {
				fprintf(fid, "%.8g ", endnumsbig[index][bioindex_id][z]);
				index++;
			}
		}
		fprintf(fid, "%.8g ", (double)0.0);
		
		/* Write Abundance Info */
		index = 0;
		for (j = 0; j < bm->K_num_tot_sp; j++) {
			if (FunctGroupArray[j].isMacroFauna == TRUE) {
				fprintf(fid, "%.8g ", endnumsbig[index][abundindex_id][z]);
				index++;
			}
		}
		fprintf(fid, "%.8g ", (double)0.0);
		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Coefficient of Variation indices file
 *
 */
FILE * Init_Cvt_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "cvtindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Cvt_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone CVt #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Coefficient of Variation indices file
 *
 */
void Write_Cvts(FILE *fid, MSEBoxModel *bm, int ij) {
	int z;

	if (verbose > 1)
        printf( "Write CVt information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Coefficient of Variation Info */
		fprintf(fid, "%.8g ", cvt[z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise Diversity indices file
 *
 */
FILE * Init_Div_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Div_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone GroupDiv FnDiv #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Diversity indices file
 *
 */
void Write_Divs(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write diversity information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Diversity Info */
		for (j = 0; j < 2; j++)
			fprintf(fid, "%d ", divindx[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Habitat complexity indices file
 *
 */
FILE * Init_Hab_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "habindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Hab_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone inf/epi totepi #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Habitat complexity indices file
 *
 */
void Write_Habs(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write habitat information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Habitat Complexity Info */
		for (j = 0; j < 2; j++)
			fprintf(fid, "%.8g ", habindx[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Reproductive success indices file
 *
 */
FILE * Init_Fledge_File(MSEBoxModel *bm) {
	int i;
	FILE *fid;
	char *fname = "fledgeindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Fledge_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			if ((FunctGroupArray[i].groupType == BIRD) || (FunctGroupArray[i].groupType == MAMMAL)) {
				fprintf(fid, "%smat ", FunctGroupArray[i].groupCode);
			}
		}
	}
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			if ((FunctGroupArray[i].groupType == BIRD) || (FunctGroupArray[i].groupType == MAMMAL)) {
				fprintf(fid, "%swean ", FunctGroupArray[i].groupCode);
			}
		}
	}
	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Reproductive success indices file
 *
 */
void Write_Fledges(FILE *fid, MSEBoxModel *bm, int ij) {
	int sp, z;

	if (verbose > 1)
        printf( "Write fledge information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Fledge Info */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
				fprintf(fid, "%.8g ", fledge[sp][z]);
			}
		}
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if(FunctGroupArray[sp].groupType == BIRD || FunctGroupArray[sp].groupType == MAMMAL){
				fprintf(fid, "%.8g ", fledge[sp + bm->K_num_tot_sp][z]);
			}
		}

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Monbet indices file
 *
 */
FILE * Init_Monbet_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "monbetindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Monbet_File: Can't open %s\n",fname);


    /* Column definitions */
	fprintf(fid, "#Time Zone Chl DIN #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Monbet indices file
 *
 */
void Write_Monbets(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write monbet information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Monbet Info */
		for (j = 0; j < 2; j++)
			fprintf(fid, "%.8g ", monbet[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the PD ratio indices file
 *
 */
FILE * Init_Pd_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "pdindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Pd_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone pd fpfv fpfvfd pdcat fpfvcat fpfvfdcat #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the PD ratio indices file
 *
 */
void Write_Pds(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write p:d ratio information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Pd Info */
		for (j = 0; j < 3; j++)
			fprintf(fid, "%.8g ", pd[j][z]);
		for (j = 0; j < 3; j++)
			fprintf(fid, "%.8g ", pdcat[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the NPP/B indices file
 *
 */
FILE * Init_Nppb_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "nppbindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Nppb_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone NppB #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the NPP/B indices file
 *
 */
void Write_Nppbs(FILE *fid, MSEBoxModel *bm, int ij) {
	int z;

	if (verbose > 1)
        printf( "Write NPP/B information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write NPP/B Info */
		fprintf(fid, "%.8g ", nppb[z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Average Trophic Level indices file
 *
 */
FILE * Init_Avgtl_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Avgtl_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone avgtlcat avgtl avgtlcatstat avgtlstat avgtlcatvert avgtlcatvertstat avgtlvert avgtlvertstat #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Average Trophic Level indices file
 *
 */
void Write_Avgtls(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z;

	if (verbose > 1)
        printf( "Write average trophic level information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Average Trophic Level Info */
		for (j = 0; j < 8; j++)
			fprintf(fid, "%.8g ", avgtl[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Trophic spectra indices indices file
 *
 */
FILE * Init_TrophSpectra_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_TrophSpectra_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone StaticTL bin1 bin15 bin2 bin25 bin3 bin35 bin4 bin45 bin5 bin55 bin6 bin65 bin7 bin75 bin8 bin85 bin9 bin95 bin10 #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Trophic spectra indices indices file
 *
 */
void Write_TrophSpec(FILE *fid, MSEBoxModel *bm, int ij) {
	int j, z, k;

	if (verbose > 1)
        printf( "Write average trophic level information\n");

	for (z = 0; z < bm->nfzones + 2; z++) {
		for (k = 0; k < 2; k++) {

			/* Write Time */
			fprintf(fid, "%d ", ij);

			/* Write Zone */
			fprintf(fid, "%d ", z);

			/* Write Static or Dynamic trophic levels used */
			fprintf(fid, "%d ", k);

			/* Write Trophic Spectra Info */
			for (j = 0; j < bm->K_num_trophbin; j++)
				fprintf(fid, "%.8g ", trophspect[j][k][z]);

			/* New line */
			fprintf(fid, "\n");
		}
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Population fractions as indices file
 *
 */
FILE * Init_PopFract_File(MSEBoxModel *bm, char *fname) {
	int i;
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_PopFract_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone ");
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			fprintf(fid, "%s_stock ", FunctGroupArray[i].groupCode);
		}
	}
	for (i = 0; i < bm->K_num_tot_sp; i++) {
		if (FunctGroupArray[i].isVertebrate == TRUE) {
			fprintf(fid, "%s_catch ", FunctGroupArray[i].groupCode);
		}
	}

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Population fractions as indices file
 *
 */
void Write_PopFracts(FILE *fid, MSEBoxModel *bm, int ij) {
	int z, i, j;

	if (verbose > 1)
        printf( "Write population fractions\n");

	for (z = 0; z < bm->nfzones; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Population fraction Info */
		for (j = 0; j < 2; j++)
			for (i = 0; i < bm->K_num_tot_sp; i++) {
				if (FunctGroupArray[i].isVertebrate == TRUE) {
					fprintf(fid, "%.8g ", pop_fraction[j][i][z]);
				}
			}

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Network indices file
 *
 */
FILE * Init_Netwk_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Netwk_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone totcons totprod totbiom totex totresp totflowdet totpp totim #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Network indices file
 *
 */
void Write_NetwkInfos(FILE *fid, MSEBoxModel *bm, int ij) {
	int z, i;

	if (verbose > 1)
        printf( "Write totalnetwork information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Network Info */
		for (i = 0; i < K_netwk_properties; i++)
			fprintf(fid, "%.8g ", netwkindx[i][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the System Omnivory indices file
 *
 */
FILE * Init_Soi_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Soi_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone soi #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the System Omnivory indices file
 *
 */
void Write_Sois(FILE *fid, MSEBoxModel *bm, int ij) {
	int z;

	if (verbose > 1)
        printf( "Write SOI\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write SOI Info */
		fprintf(fid, "%.8g ", soi[z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Omnivory index information file
 *
 */
FILE * Init_Oi_File(MSEBoxModel *bm, char *fname) {
	FILE *fid;
	int pred;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Oi_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Location ");

	//	Write_Non_Detritus_Assessed_Group_Names(bm, fid);
	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE && FunctGroupArray[pred].isDetritus == FALSE) {

			if (FunctGroupArray[pred].isVertebrate == TRUE) {
				fprintf(fid, "%sj ", FunctGroupArray[pred].groupCode);
				fprintf(fid, "%sa ", FunctGroupArray[pred].groupCode);

			} else {
				fprintf(fid, "%s ", FunctGroupArray[pred].groupCode);
			}
		}
	}

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}
/**
 *
 *	\brief Routine to write data to the Omnivory index information file
 *
 */
void Write_Ois(FILE *fid, MSEBoxModel *bm, int ij) {
	int sp, locationIndex;
	char *location[3] =
		{ "In", "Off", "Tot" };

	if (verbose > 1)
        printf( "Write OI\n");

	for (locationIndex = 0; locationIndex < 3; locationIndex++) {
		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Location */
		fprintf(fid, "%s ", location[locationIndex]);

		/* Write Inshore Info */
		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
			if (FunctGroupArray[sp].isAssessed == TRUE && FunctGroupArray[sp].isDetritus == FALSE) {
				if (FunctGroupArray[sp].isVertebrate == TRUE) {
					fprintf(fid, "%.8g ", oi[locationIndex][sp][juv_id]);
					fprintf(fid, "%.8g ", oi[locationIndex][sp][adult_id]);
				} else {
					fprintf(fid, "%.8g ", oi[locationIndex][sp][0]);
				}
			}
		}
		/* New line */
		fprintf(fid, "\n");
	}
	return;
}

/**
 *
 *	\brief Routine to initialise the Mortality information file
 *
 */
FILE * Init_Mort_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "mortinfo.txt";
	int pred;

	/* Create file */
	 if( (fid=Util_fopen(bm, fname, "w")) == NULL )
		quit("initmortFile: Can't open %s\n", fname);

	 /* Column definitions */
	fprintf(fid, "#Time Location ");

	//Write_Non_Detritus_Assessed_Group_Names(bm, fid);

	for (pred = 0; pred < bm->K_num_tot_sp; pred++) {
		if (FunctGroupArray[pred].isAssessed == TRUE) {

			if (FunctGroupArray[pred].isVertebrate == TRUE) {
				fprintf(fid, "%sj ", FunctGroupArray[pred].groupCode);
				fprintf(fid, "%sa ", FunctGroupArray[pred].groupCode);

			} else if (FunctGroupArray[pred].isDetritus == TRUE) {
				fprintf(fid, "%s ", FunctGroupArray[pred].groupCode);
				fprintf(fid, "%ssed ", FunctGroupArray[pred].groupCode);

			} else {

				fprintf(fid, "%s ", FunctGroupArray[pred].groupCode);
			}
		}
	}

	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Mortality information file
 *
 */
void Write_Morts(FILE *fid, MSEBoxModel *bm, int ij) {
	int z, sp;

	if (verbose > 1)
        printf( "Write mortality information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		for (sp = 1; sp < bm->K_num_tot_sp + 1; sp++) {
			if (FunctGroupArray[sp - 1].isAssessed == TRUE) {
				if (FunctGroupArray[sp - 1].isVertebrate == TRUE || FunctGroupArray[sp - 1].isDetritus == TRUE) {
					fprintf(fid, "%.8g ", mortnetwk[sp][0][z]);
					fprintf(fid, "%.8g ", mortnetwk[sp][1][z]);
				} else {
					fprintf(fid, "%.8g ", mortnetwk[sp][0][z]);
				}
			}
		}

		/* New line */
		fprintf(fid, "\n");
	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Total biomass indices information file
 *
 */
FILE * Init_Tot_Biom_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "totbiomindex.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Tot_Biom_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone inv_totbiom vert_totbiom tot_totbiom_nodetrit #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write data to the Total biomass indices information file
 *
 */
void Write_TotBioms(FILE *fid, MSEBoxModel *bm, int ij) {
	int z, j;

	if (verbose > 1)
        printf( "Write total biomass information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Total Biomass Info */
		for (j = 0; j < 3; j++)
			fprintf(fid, "%.8g ", totsamplebiom[j][z]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to initialise the Maximum length caught information information file
 *
 */
FILE * Init_MaxL_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "maxlinfo.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_MaxL_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Time Zone fixed var avg #\n");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Routine to write out data to the Maximum length caught information information file
 *
 */
void Write_MaxLs(FILE *fid, MSEBoxModel *bm, int ij) {
	int z, i;

	if (verbose > 1)
        printf( "Write maximum length information\n");

	for (z = 0; z < bm->nfzones + 1; z++) {

		/* Write Time */
		fprintf(fid, "%d ", ij);

		/* Write Zone */
		fprintf(fid, "%d ", z);

		/* Write Info */
		for (i = 0; i < 3; i++)
			fprintf(fid, "%.8g ", max_lngth[z][i]);

		/* New line */
		fprintf(fid, "\n");

	}

	return;
}

/**
 *
 *	\brief Routine to write out data to the diet out information file
 *
 *	This writes out all the sample output - don't write sampled diet out
 *
 */
void WriteDietOut(MSEBoxModel *bm, int ij) {

	if (verbose)
		fprintf(stderr, "Writing diet output\n");

	/* Initialise files if necessary */
	if (!indietfp && bm->samplediet)
		indietfp = Init_InDiet_File(bm, "indietinfo.txt");
	if (!offdietfp && bm->samplediet)
		offdietfp = Init_OffDiet_File(bm, "offdietinfo.txt");
	if (!oifp)
		oifp = Init_Oi_File(bm, "oiinfo.txt");

	/* Write files */
	if (bm->samplediet) {
		Write_InDiets(indietfp, bm, ij, inshorediet);
		Write_OffDiets(offdietfp, bm, ij, offshorediet);
	}
	Write_Ois(oifp, bm, ij);

	return;
}

/**
 *	Write out results of this assessment
 *	This writes out all the sample output
 *
 **/
void Write_Assess_Out(MSEBoxModel *bm, int ij) {
	// static FILE *assessfp = NULL;

	if (verbose)
		fprintf(stderr, "Writing assessment output\n");

	/* Write files */
	Write_Assess_Results(assessfp, bm, ij);

	return;
}

/**
 *	Initialise assess output file
 *
 **/
FILE * initAssessFile(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "assessinfo.txt";
	int i;

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("initAssessFile: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Prm2 stands for different things in different models\n");
	fprintf(fid, "#Schaefer production model: Prm2 = K\n");
	fprintf(fid, "#VPA: Prm2 = plus group estimate\n");
	fprintf(fid, "#Time Species");
	for (i = 0; i < K_num_assess_prms; i++)
		fprintf(fid, " %s", bm->NAssessNAME[i]);
	fprintf(fid, "#\n");

	/* Return file pointer */
	return (fid);
}

/**
 *	Write out data to the assess output file
 *
 **/
void Write_Assess_Results(FILE *fid, MSEBoxModel *bm, int ij) {
	int sp, k, assess_flag_sp;

	if (verbose > 1)
        printf( "Write stock numbers\n");

	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {

			/* Only write assessment results if actually assessed */
			assess_flag_sp = (int) (FunctGroupArray[sp].speciesParams[assess_flag_id]);
			if (!assess_flag_sp)
				continue;

			/* Write Time */
			fprintf(fid, "%d ", ij);

			/* Write Species */
			fprintf(fid, "%s ", FunctGroupArray[sp].groupCode);

			/* Write Info */
			for (k = est_med_recruit_id; k < K_num_assess_prms; k++)
				fprintf(fid, "%.8g ", bm->NAssess[sp][k]);

			/* New line */
			fprintf(fid, "\n");
		}
	}

	return;
}

/**
 *
 *	\brief Write out random number
 *	This writes out all of the random numbers generated for use in the sampling and assesment model
 *
 */
void Write_Random_Numbers(MSEBoxModel *bm, int numyr, int numsamples) {
	// static FILE *randoutfp = NULL;

	if (verbose)
		fprintf(stderr, "Writing random number log file\n");

	/* Write files */
	Write_RandNum(randoutfp, bm, numyr, numsamples);

	return;
}

/**
 *
 *	\brief Initialise the random number information file
 *
 */
FILE * Init_Rand_File(MSEBoxModel *bm) {
	FILE *fid;
	char *fname = "rand_num_log.txt";

    /* Create file */
    if( (fid=Util_fopen(bm, fname, "w")) == NULL )
    	quit("Init_Rand_File: Can't open %s\n",fname);

    /* Column definitions */
	fprintf(fid, "#Random numbers used in run of Atlantis");

	/* Return file pointer */
	return (fid);
}

/**
 *
 *	\brief Write out the random numbers to the initialise output file.
 *
 */
void Write_RandNum(FILE *fid, MSEBoxModel *bm, int numyr, int numsamples) {
	int b, sp;

	if (verbose > 1)
        printf( "Write rand number log file\n");

	fprintf(fid, "# Frequency of sampling scalars (multiply tassessinc by these to get frequency of sampling)\n");
	for (b = 0; b < numsamples; b++) {
		fprintf(fid, "%e ", bm->tassPatchy[b][tass_id]);
	}
	fprintf(fid, "\n");
	fprintf(fid, "# Frequency of sampling scalars (multiply teatassessinc by these to get frequency of diet sampling)\n");
	for (b = 0; b < numsamples; b++) {
		fprintf(fid, "%e ", bm->tassPatchy[b][tasseat_id]);
	}
	fprintf(fid, "\n");
	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
		if (FunctGroupArray[sp].isFished == TRUE) {
			fprintf(fid, "# Assessment errors for %s\n", FunctGroupArray[sp].groupCode);
			fprintf(fid, "# %s Numbers errors\n", FunctGroupArray[sp].groupCode);
			for (b = 0; b < numyr; b++) {
				fprintf(fid, "%e ", bm->rand[sp][startN_id][b]);
			}
			fprintf(fid, "\n");
			fprintf(fid, "# %s F errors\n", FunctGroupArray[sp].groupCode);
			for (b = 0; b < numyr; b++) {
				fprintf(fid, "%e ", bm->rand[sp][calcF_id][b]);
			}
			fprintf(fid, "\n");
			fprintf(fid, "# %s M errors\n", FunctGroupArray[sp].groupCode);
			for (b = 0; b < numyr; b++) {
				fprintf(fid, "%e ", bm->rand[sp][calcM_id][b]);
			}
			fprintf(fid, "\n");
		}
	}
	return;
}


/**
 * \defgroup atUtil atUtil
 *
 * The utility module of Atlantis
 *
 */
/**
 *
 */
/**
 *  Revisions:
 *
 *  06-06-2008 Bec Gorton
 *
 *  Added isProducer, isGrazer, isPrey and isCover to the functional group structure.
 *
 *  11-06-2008 Bec Gorton
 *  Added the functional group full name.
 *
 *  19/08/2008 Bec Gorton
 *  Added preyID to the FunctGroup array. This will eventually be removed.
 *
 *	09/09/2008 Bec Gorton/Beth Fulton
 *	Added the PhysioChemStruct to store information about the Physio/Chemical tracers/
 *	Changed the tracerIndex value to be a pointer to the orginal tracer index. Eventually
 *	we will hopefully be able to get rid of the original tracer index values like NH_i
 *  and just store the tracer in the PhysioChemStruct structure instead of a reference. But this
 *	will work for now.
 *
 *	15/01/2009 Bec Gorton
 *	Moved the spawning and aging parameters into the functional group structure. It might be good
 *	to move these into a better place later.
 *
 * 	05/03/2009 Bec Gorton
 * 	Added the remin, brokenDown and solDON variables to the InvertProps structure. These are used by the
 * 	DR, DC and DL groups.
 *  Added the FunctGroupArray[guild].isDetritus flag to simplify code where we want to exclude detritus groups.
 *
 *  22-04-2009 Bec Gorton
 *  Added the following new invert_types:
 *	CARRION,
 * 	MICROPHTYBENTHOS,
 *  DINOFLAG
 *  Changed the isvertical to isMobile. Split the isharvested into isimpacted and isfished.
 *  Added the isSiliconDependant, silicon tracer second tracer and second tracer index to the FunctGroupArray structure.
 *  Added the debugIndex to the PhysioChem structure so that it can be used to reference the tracer value in the debug values array.
 *  This mean i can almost get rid of the nutrient tracer guild values defined in AtlantisBoxModel.h
 *
 *
 *	10-06-2009 Bec Gorton
 *	Added the SEAGRASS invertebrate group.
 *
 *	16-06-2009 Bec Gorton
 *	Added a flag to the FunctGroupArray that is used to indicate if the group eats detritis. This is used in the atecology code
 *	in the reminNet calculations.
 *
 *	30-06-2009 Bec Gorton
 *	Moved the SpeciesParamStruct structure into this header so it can be used to read in the fishery parameters.
 *
 *	28-10-2009 Bec Gorton
 *	Added the isTAC functional group variable.
 *
 *	30-10-2009 Bec Gorton
 *	Added the Util_Close_Output_File function.
 *
 *	05-11-2009 Bec Gorton
 *	Merged with trunk - Added support for the species specific Tcorr value to be stored in the FunctGroupArray structure for
 *	use in reproduction calculation.
 *
 *	29-01-2010 Bec Gorton
 *	Updated the header files that are included. Moved the hash structured used by the old parsing code into a new header called
 *	atHashUtil.h so these are not included in all libraries when not required.
 *	Added the tracer variables to the FunctionalGroupStruct to allow generalisation of the code that calculates the harvest tracers.
 *
 *	07-04-2010 Bec Gorton
 *	Add a couple of new flags to the FunctGroup structure:
 *		isInfauna - true if invert_type is SM_INF or LG_INF
 *		isEpifauna - true if invert_type is one of the filter feeder groups, phytoben, seagrass or MB.
 *		isPrimaryProducer - true if the group is a primary producer.
 *
 *	22/02/2011 Bec Gorton
 *		Updated the vert code to use the preyEaten array in the processProps instead of having a seperate array for the vertebrates.
 *		Removed the isConsumer flag from the functional group structure - can just use the isPredator flag set in the FG input file.
 */

#include "atFunctGroup.h"
#include "atFisheryStruct.h"

/* Prototypes for routines used in this file */
int 	Check_Value(char *paramString, double value, int checkType, char *typeName);

/**************************************************************************
Functional group ********************************************************/


/* The structure used to store the box layer data.
 */

typedef struct {
	double ***DebugInfo; /**< Array to store debug variable term values */
	double ***DebugFluxInfo; /**< Array to store debug flux values */

	long double **NutsProd; /* Array of nutrients produced in current time-step */
	long double **NutsLost; /* Array of nutrients lost in current time-step */
	long double ***NutsProdGlobal; /* Array of nutrients produced only in it_count == 1

	 that need to carry through adaptive time-steps */
	long double ***NutsLostGlobal; /**< Array of nutrients lost only in it_count == 1
	 that need to carry through adaptive time-steps */
	long double **DetritusProd; /**< Array of detritus produced in latest time-step */
	long double **DetritusLost; /**< Array of detritus lost in latest time-step */
	long double ***DetritusProdGlobal; /**< Array of detritus produced only in it_count == 1
	 that need to carry through adaptive time-steps */
	long double ***DetritusLostGlobal; /**< Array of detritus lost only in it_count == 1
	 that need to carry through adaptive time-steps */

	double *localWCTracers;
	double *localSEDTracers;
	double *localEPITracers;
	double *localICETracers;
	double *localLANDTracers;

	double *localWCFlux;
	double *localSEDFlux;
	double *localEPIFlux;
	double *localLANDFlux;

	double *localDiagTracers;
	double *localDiagFlux;
	double *localFishTracers;
	double *localFishFlux;

	double *localICEFlux;

    double PB_DL;
    double PB_DR;

	double BB_DL;
	double BB_DR;

	double ICEB_DL;
	double ICEB_DR;

    double sDLscale, sDRscale, sDCscale;
    double DLscale, DRscale, DCscale;

	double DIN;
	double DINs;

	double Remin;
	double Nitrification;
	double DONremin;
	double Denitrification;


} BoxLayerValues;



/**
 *	The structure that is used to store information about each physioChem tracer
 *	in the model.
 */
typedef struct {
	char name[300];					/**< The verbose name */
#ifdef LINK_ENABLED
	int isExported;					/**< If TRUE then the tracer can be exported to another model */
    DATA_TYPES exportType;			/**< The tracer export type. Only valid if isExported is TRUE */
#endif

    int *tracerIndex;				/**< The tracer index.*/
	int diagTol;					/**< flag of tolerance checking variable, 1=yes, 0=no,
										2=do only once a timestep, 3=do once a timestep
										and don't reset to minpool */
	int diagFlux;					/**< flag of diagonostic variable, 1=yes, 0=no */
	int diagBiol;					/**< flag indicating whether flag_id sensitive, 1=yes, 0=no */
	int debugIndex;					/**< The index of this tracer in the debugFluxInfo array
									 * This is set in Add_Physio_Chem_Tracer.
									 */


    HABITAT_TYPES habitat_type;

    /** The rate of change function - this is used to calculate the flux of the tracer */
    void (*ROCFunction)(MSEBoxModel *bm, FILE *llogfp, HABITAT_TYPES habitatType, int index, BoxLayerValues *boxLayerInfo);

}PhysioChemStruct;
extern PhysioChemStruct *PhysioChemArray;


extern char *cohortStrings[];
extern char *sexStrings[];

void Free_Functional_Group_Memory(MSEBoxModel *bm);
void Free_Fishery_Def_Memory(MSEBoxModel *bm);


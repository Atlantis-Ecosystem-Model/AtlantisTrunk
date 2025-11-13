/*
 * atFunctGroup.h
 *
 *
 *	Changes:
 *
 *	05-07-2012 Bec Gorton
 *	Added the detIndex to the FunctGroupArray structure.
 *
 *
 *  Created on: 03/08/2011
 *      Author: bec
 *
 *  Revisions: 28/7/2017 Added dynamic X_RS from Asta
 */

#ifndef ATFUNCTGROUP_H_
#define ATFUNCTGROUP_H_


/*
 * The different types of groups.
 */
typedef enum
{
	AGE_STRUCTURED,
	AGE_STRUCTURED_BIOMASS,
	BIOMASS
}GROUP_AGE_TYPES;


/**
 * The different group types that are supported.
 *
 */
typedef enum {
	BIRD,
	MAMMAL,
	SHARK,
	FISH,
    FISH_INVERT,
	LG_PHY,
	SM_PHY,
	SM_ZOO,
	MED_ZOO,
	LG_ZOO,
	//JELLY,
	CEP,
	PWN,
	PL_BACT,
	SED_BACT,
	ICE_BACT,
	SM_INF,
	LG_INF,
	SED_EP_FF,
	SED_EP_OTHER,
	MOB_EP_OTHER,
	CORAL,
    SPONGE,
	PHYTOBEN,
	LAB_DET,
	REF_DET,
	CARRION,
	MICROPHTYBENTHOS,
	DINOFLAG,
	ICE_DIATOMS,    // (ice dwelling diatoms)
	ICE_MIXOTROPHS, // (ice dwelling dinoflagellates)
	ICE_ZOOBIOTA, 	// (ice dwelling small zooplankton)
	SEAGRASS,
	TURF,
	NUM_GROUP_TYPES
}GROUP_TYPES;


/**
 *	The structure that is used to store information about each functional group
 *	in the model.
 */
typedef struct {
    char groupCode[20];					/**< The code used to identify this functional group in the input files */
	char name[200];						/**< The verbose name */
    char fullName[200];                 /**< The functional groups full name */
    int diagTol;                      	/**< The tolerance of the diagnostic tracer */
    int index;							/**< The index of the functional group in arrays - might be able to get rid of this */
    int numCohorts;						/**< The number of age classes */
    int numGeneTypes;					/**< The number of gene types per age class */
    int numCohortsXnumGenes;
    int numStages;						/**< The number of gross life history stages */
    int numSpawns;						/**< The number of spawnings per year */
    int ageClassSize;					/**< The number of annual age classes per age cohort */
    int numStocks;                      /**< The number of biological stocks represented in the model in stock_struct arrays */
    int MaxNumMig;                      /**< Maximum number of migrations in the queue */

    int HowFar;                         /**< Species level HowFar in case have more movement entries than quarters in the year */
    
    int externalReproducer;             /**< Whether the species does external reproduction or not */
    int recruitType;                    /**< Recruit distribution option */

    int isVertebrate;					/**< TRUE if the group are vertebrates, FALSE if they are invertebrates */
    int isMobile;						/**< TRUE if the group can move out of current layer (either horizontally or vertically) using movement and migration code */
    
    int isNoiseEffected;                /**< TRUE if the group is effected by noise pollution */
    int isLightEffected;                /**< TRUE if the group is effected by light pollution */
    
    int sp_geo_move;					/**< True if the group can move horizontially */
    int num_migrate;                    /**< Max number of migrations per year for any stage */
    int multiyr_mig;                    /**< Whether doing multyr migration (1 if annual spawning 2 if multiple spawns per year) or not (0) */
    int *needed_for_age_away;            /**< Flag for use during start up to mark if need an age class around in KMIG because age while away */

    int isImpacted;						/**< True if the functional group is impacted by fishing.*/
    int isFished;						/**< True if the functional group is fished.*/
    int isTAC;							/**< True if the functional groups is under a TAC */
    int isProducer;                     /**< True if this functional group produces */
    int isCover;                        /**< True if this functional group is a cover species */
    int isGrazer;                       /**< True if this functional group is a grazer */
	int isPredator;
	int isAssessed;
    int isCloseKin; // Activated if isAssessed is > 1
	int isBacteria;
	int isZooPlankton;
	int isInfauna;
	int isEpiFauna;
    int isBioTurbator;
	int isPrimaryProducer;				/**< True if the group is a primary producer. (SM_PHY, LG_PHY, PHYTOBEN, MICROPHTYBENTHOS, DINOFLAG, SEAGRASS) */
	int isMicroFauna;					/**< True if the group is a micro fauna. This is used in the assessment code when calculating indicies. (SM_PHY, LG_PHY, DINOFLAG, LG_ZOO, MED_ZOO, SM_ZOO, SM_INF) */
	int isMacroFauna;					/**< True if the group is a macro fauna. Also used ine assessment code. (vertebrate, LG_INF, CEP, PWN, SED_EP_FF, SED_EP_OTHER, MOB_EP_OTHER) */
    int isBioEroder;                    /**< True if bio-eroding group */

    int isOncePerDt;					/**< True if this functional group is only executed once per time step, This is determined by the groupType. Larger groups are just done once per timestep, small groups more than once. */
	int isWCActiveOnly;					/**< True if a phytoplankton group - so mortality in watercolumn not stored in diagnostics (as typically annoyed) */
	int isDetritus;						/**< true if the group is either DR. DL or DC */
	int detIndex;						/**< The index of the detritux groups */
	int isDetritusEater;				/**< true if the group eats DR or DL */
    int isSiliconDependant;             /**< If True then silicon is traced for this group */
    int isOverWinter;					/**< True if the group overwinters or encysts */
    int isCultured;						/**< True if the group is an aquaculture species */
    int isHabDepend;                    /**< True if habitat dependent */
    int isLandActive;					/**< True if the group is allowed on land */
    int isSupplemented;                 /**< True if the group feeds on material outside the model domain */
    int isExternal;                     /**< True if the group uses external population model */
    int isComplexMigrator;              /**< Indicates whether has multiple complex migrations overlapping */
    
    int RAssessFileNum;                  /**< Counter for the RAssess filenumbering system - so don't need to create more files than needed */

    int updatedDiet;                    /**< Updated diet being out */

    double secondNutrient;
    int    secondNutrientTracerIndex;
    GROUP_AGE_TYPES groupAgeType;
    HABITAT_TYPES habitatType;	               /**< The functional group habitat type */
    GROUP_TYPES groupType;		        /**< If this is a group if inverts then what type of invert is it.
											This might be changed to allow for vert types if necessary */
    double Tcorr;	/** Temperature correction value for this functional group */
    double TcorrEff; /**< Temperature efficiency correction */
    double Scorr;	/** Salinity correction value for this functional group */
    double pHcorr;	/** pH correction value for this functional group */
    double Ccorr; 	/** General contaminantion correction value for this functional group */
    double PolluteCorr;     /** Noise and light contamination correction value for this functional group */
    double *C_growth_corr; 	/** Growth contaminantion correction value for this functional group */
    double *C_move_corr;     /** Growth contaminantion correction value for this functional group */
    double *C_reprod_corr;     /** Reproduction contaminantion correction value for this functional group */

	int *habitatCoeffs; /* If the group resides in the habitat then this value will be 1.0, else it will be 0.0 */

    int* allgone;  /* Flag to indicate whether no remaining numbers for this cohort for the group - for use in aging steps */

	/************************************************************************************************/
    /* Invertebrate values */
    int *totNTracers;		// Only used for inverts.
    int *prodnTracers;		// Only used for inverts.
    int *GrazingTracers;	// Only used for inverts.
    int CoverTracer;
    int coverID; 			/* The index of the group in the coveramt array */

    double *C_T15;
    double *C_T15_per_day;
    double *mum_T15;
    double *mum_T15_per_day;
    double *age_mat;   	// Activate this to allow for multiple stages not just juv, adult
    int *cohort_stage;

    /* Additional properties */
    double uptakeDL;
    double uptakeDR;
    double prodnDON;		/* used by bacteria */
    double *releaseNH;
    double *uptakeNH;
    double *prodnDR;
    double *prodnDL;
    double *growth;			/** Non-vertebrate growth value */
    double *mortality;		/* More invert type deaths. */
    double *grazing;			/** Set in Invert_Activities - the sum of the GrazeLive value and the DL and DR grazed */
    double *lysis;
    double *sn;				/* Size of fish eating invertebrates (needed here as required in ecology and assessment libraries */
    double *transDR;
	double maxPhagotrophy;	/* Used for DF */
	double chl;				/* Used by primary producers - then used to update the Chl_a tracer */
    double nitrif;			/* used by pelagic bacteria */

    double brokenDown;		/* value required for detritus and sed bacteria and DC*/
    double remin;			/* Used by detritus */
    double solDON;			/* Used by detritus */

    /* Phosphorus and Carbon values */
    double uptakeP;
    double uptakeC;

    /**@name
     *          Used for primary producers.
     */
    /*@{*/
	int Light_Adaptn_TracerID;
	double SP_IRR;

	/************************************************************************************************/
	/* Age structured biomass group values */
	double **INVpopratio;		/*  Keep track of the proportion of biomass in each biomass pool (adult and juv) that
									is a result of this cohort spawning and aging.

									Need to remember that each cohort can spawn and age independently at different dates
									so need to keep track of which proportion in each pool is from each cohort spawning/aging event
									so we spawn/age the correct proportion.
									*/

	double **tempINVpopratio; /* This might be better as a structure instead of an array */

	/************************************************************************************************/
    /* Values for fished groups */
    int *CaughtByFisheryTracers;
    int *DiscardedByFisheryTracers;
    int totCatchTracer;
    int totRecCatchTracer;
    int totDiscardsTracer;

	/************************************************************************************************/
    /** Vertebrate values */
    int *structNTracers;				/**< The array of structural nitrogen tracer indices.*/
    int *resNTracers;					/**< The array of residual nitrogen tracer indices.*/
    int *NumsTracers;
    int *GrowthTracers;
    int *EatTracers;
    int *CatchTracers;
    int *DiscardTracers;
    int SpawnSizeTracer;

    /**
     * The contaminant tracers.
     *
     * One per contaminant per cohort.
     */
    int **contaminantTracers;   /* Base contaminant levels in a species */
    int **contamPropTracers;    /* So can track proportion of the individuals effected */
    double ****agingContam;       /* For transferring contaminants */
    double *contaminantSpMort;	/* Mortality of each group/cohort due to all contaminants - calculated for each timestep */
    double **calcCLinearMort;	/* Mortality of each group/cohort due to this contaminant - calculated for each timestep */
    double *reprodContam; /* Maternal transfer occuring during recruitment per species */
    double *reprodContamCount; /* Count of parents contributing contaminants */
    double *LocalPopCount; /* Count of local juveniels for suckling transfer of contaminants */

    /* Additional tracers - only allocated if bm->track_atomic_ratio is TRUE.*/
    int **addRatioTracers;		/** The index of the additinal ratio tracers. One tracer per cohort per additional element we are tracking */
    double **addRatioFluxes;	/** The tracer flux. Allocated one tracer per cohort per additional element we are tracking */
    long double ***ratioLost;			/** The amount of each element lost due to predation and other losses su. One value per cohort, element and habitat.*/
    long double **ratioGainedPred;			/** The amount of each element gained due to predation. One value per cohort, element and habitat.*/

    long double ****ratioLostGlobal;
   // long double ****ratioGainedPredGlobal;


    long double ***ratioLostPred;		/* Amount lost due to being eaten - stored separately to help debugging */
    long double ****ratioLostPredGlobal; /* Amount lost due to being eaten - stored separately to help debugging */

    long double ***ratioGained;		/* Amount gained not due to predation */
    long double ****ratioGainedGlobal; /* Amount gained not due to predation */


    double **addRatioLost;			/** The amount of each element lost. Only used for primary producers. */

    /** Used in the assessment module - moved here to save memory so we don't allocate huge arrays for groups where
     * there is no fishing.
     */
    double ***SizeNumDiscard; /**< Sampled discards - number in discards */
    double ***SizeDiscard; /**< Sampled discards - size in discards */
    double ***SizeNumCaught; /**< Sampled catch - number in catch */
    double ***SizeCaught; /**< Sampled catch - size in catch */
    double **RAssessSpringSurvey; /**< Spring survey abundance - for RAssess */
    double **RAssessAutumnSurvey; /**< Autumn survey abundance - for RAssess */
    
    /* Used in tracking informaiton for calcualting system cap in fisheries */
    double *min_wgt;
    double *max_wgt;
    double **rolling_wgt;
    double *min_B;
    double *max_B;
    double **rolling_B;

    /** Reproduction related */
    double *FSPB; /* Spawning ogives for vertebrates. This is the original value read in. This is then scaled and then the scaled array is the one that is used. */
    double *scaled_FSPB; /* Scaled spawning ogives for vertebrates */
    double **grow;			/** Growth in RN and SN of the vertebrates - set in Do_Vertebrate_Living */

    /* For the vertebrate eaters, set bounds on juvenile and adult size (upper and lower
	bounds on structural weight) so can determine which grey groups can fit through
	their gapes. */
	int spMinMax[4];

    double ***AGEnewden;
    double ***agingVERT;
    double ****boxPopRatio;

    double iceBact_Scale;


#ifdef LINK_ENABLED
	/************************************************************************************************/
    /* Parameters associated with linking to other models */
    int isExported;
    int isImported;
    DATA_TYPES exportType;
#endif

    /**
     *  The species parameters
     *  There are two separate arrays here. The majority of parameters are NOT cohort independent. These are stored in speciesParams.
     *  The params that are cohort dependant are stored in cohortSpeciesParams.  If the group is a age structured biomass group then there are values for adult and juv.
     *  Otherwise (age structured and biomass groups) only one value is stored per group and it is stored in the cohort = 0 position.
     *
     * */
    double *speciesParams;
    double **cohortSpeciesParams;
    double **spawnSpeciesParams;
    
    int *co_sp;
    double **co_sp_catch;

    long double **preyEaten;			/** Prey eaten in each habitat type */
    long double ***preyEatenGlobal;
    
    double **max_scalar;  /* USed in logistic shaped feeding window curve */

    /**
     * The array that is used to store information about which groups eat what and where.
     * The dimensions of this array are [cohort][prey][habitat]
     * Unlike the bm->spSPVERTeat array no information about the prey age is stored.
     *
     *
     */
    double ***pSPEat;

    /* Array of diet available for each cohort of seagrass for each predator
     * - this is only allocated and used if the flag_macro_model value is set to true.*/
    double *pSP_SG_eat;

    /* Clearance and max growth rates. */
    double *SP_C;
    double *SP_C_per_day;
    double *mum;			 		 /** Max growth rate */
    double *mum_per_day;			 /** Primary producer growth */
    double *scaled_mum;
    double *scaled_C;
    double *X_RS;                    /** Ratio of RN to SN per cohort */

    double *dead; 			/* Death due to fishing. Need to list units*/
    double *deadGlobal;

    /* Vertical distribution */
    double ***distrib_VERTICAL;
    double **distrib_OVERWINTER;

    double *CLEAR;			/** Clearance rate */
    long double *GrazeLive;		/** Biomass of live prey */


    /* Variables associated with seasonal movement. Modellers can specify the number of 'seasons' to have per species in the groups definition file.
     * The older versions of the code had a set 4 seasons for all groups. This allows much greater flexibility.
     */
    int numMoveEntries;
    int moveEntryIndex;
    int next_moveEntryIndex;
    int *NeedMoveUpdate;
    double ***distrib;

}FunctionalGroupStruct;
extern FunctionalGroupStruct *FunctGroupArray;

/* Data structure holding demographic information for functional groups */
typedef struct{
	/* Queue for storing when events will occur */
	int **Age_Now;
	int **Spawn_Now;

	/* Flags to trigger immediate action */
	int *readytospawn;
	int recruiting_now;
	int next_spawn;
	int next_age;
    int next_spawn_any_age;
    int next_age_any_age;
    int next_recruit;
    int next_larvae;
    int SpawnRecruitOverlap;
    int CounterNotDone;
	int *wraps;

	/* Timing */
	int **EndDay;
	int **StartDay;
	double *RecruitPeriod;

	/* Information on queues */
    int num_in_yr;
	int num_in_larval_queue;
	int num_in_spawn_queue;
	int num_in_aging_queue;

	/* Stored reproduction values */
	double *BulkRecruits;
	double *TotSpawn;
	double *IndSpawn;
	double *recruitSPden;
	double ***Larvae;
	double ****num_recruits;
    double ****num_recruits_updating;
    
    int *migIDmatch;
    
    /* Contaminant related */
    double ****Contam;
    double **AverageContam;
    double *****RecruitContam;
    double **SettlerContam;

}DemographicStruct;
extern DemographicStruct *EMBRYO;

/* Data structure holding migration information for functional groups */
typedef struct{
	/* Queue for storing when events will occur - need adult and juvenile entries */
    int *ActualMigrator;
    
	int *Leave_Now;
	int *Return_Now;

	/* Timing parameters */
	int *Return_Period;
    int *Leave_Period;
    int *MinYearsAway;
    int *MaxYearsAway;
    int *IsAnnualMigration;
    int *start_return_chrt;
    int *yrs_to_age_pre_model;
    int *end_pt;
    int **cohort_migrating;
    int *IsPartialMigration;
    int *PartialMinAge;
    int *PartialMaxAge;

	/* Information on queues */
	int num_in_yr;
	int num_in_queue;
    int num_in_queue_done;

	/* Properties while migrating - with entries in DEN/SN/RN = cohort * ngene */
	int *returnstock;
    int *start_cohort;
	int **all_go;
	double *survival;
	double *growth;
	double **prop_mig;
	double *totprop_mig;
	double **recruit;
    double *num_stagger;

	double **InitDEN;
	double **InitSN;
	double **InitRN;
	double **DEN;
	double **SN;
	double **RN;
	double **aging;
	double **Box;
    int **ReprodAllowed;
    
    double ***RecruitContam;
    double ***contam;
    double **contam_return;
    
    double **MigYOY;
    double **MigYOY_SN;
    double **MigYOY_RN;

    /* Needed for aging */
    int **Stagger;
    double ***pop_ratio;
    double **current_pop_ratio;
    double *num_aging_event;
    
    // Parameters
    double ***Box_Prm;
    int **StartDay_Prm;
    int **EndDay_Prm;
    int **Leave_Period_Prm;
    int **Return_Period_Prm;
    int **MinYearsAway_Prm;
    int **MaxYearsAway_Prm;
    int **IsAnnualMigration_Prm;
    int **IsPartialMigration_Prm;
    int **PartialMigration_MinPrm;
    int **PartialMigration_MaxPrm;
    int **ReprodAllowedPrm;
    double **survival_Prm;
    double **growth_Prm;
    int **returnstock_Prm;
    int ***Stagger_Prm;
    int **all_go_Prm;
    
    int *RecruitQueueMatch;

}MigrationStruct;
extern MigrationStruct *MIGRATION;

/* Data structure holding the evolution relevant information */
typedef struct {

	/* Properties */
	double *stdev_gene_map;
	double *tot_num;
	double *num;
    double *sn;
    double *rn;
    long double *scaled_change;
	double *phenotype_aging_up;
	double **phenotype_transition;
	long double ***trait;
	long double ****trait_shift;

	/* Flag of whether trait evolving or not */
	int *trait_active;

} EvolutionStruct;
extern EvolutionStruct *DNA;

/* Data structure for coral bleaching */
typedef struct {
	double *DHWsum;		/* Sum of degree heat weeks - to compare against thermal tolerance to see if bleaching occurs */
	double **DHWqueue;  /* list of daily temperature vs tolerance record for last XX period (typically 3 months) so can summ to get DHWsum */
	double *TempShift;  /* Shift in thermal tolerance due to taking on more robust zooxanthellae clade */
	double *GrowShift;  /* Shift in growth tolerance due to taking on more robust zooxanthellae clade */

	double **PropUnBleached;  /* Proportion of cohort of the species unbleached in the box */

	double **RugosityEaten; /* Erosion of rugosity due to consumption of coral by macrofauna */

} CoralStruct;
extern CoralStruct *CORALREEF;

/* Prototype for geting age of maturity */
//int Get_Cohort_Stage(MSEBoxModel *bm, int guildcase, int cohort);

#endif /* ATFUNCTGROUP_H_ */

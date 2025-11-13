void Allocate_Contaiminants(MSEBoxModel *bm);
void Free_Contaminants(MSEBoxModel *bm);
void Init_Contaminants(MSEBoxModel *bm);

int Species_Contaminant_Uptake(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int cIndex);
int Calculate_Species_Contaminant_Decay(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int cIndex, double cGroupLevel);
int Init_Contaminant_Transfer_Values(MSEBoxModel *bm);
int Reconcile_Global_Contaminant_Values(MSEBoxModel *bm,HABITAT_TYPES habitatType);
int Group_Transfer_Contaminant(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES globalHabitat, HABITAT_TYPES habitat, int toGuild, int toCohort, int fromGuild, int fromCohort, double amountEaten, double ***spSPinfo, double initialBiomass, double dtsz, int need_prop, int caseGTC);

int Degrade_Contaminants(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int cIndex, double cLevel);
//int Calculate_Contaminants_Flux(MSEBoxModel *bm,  double *tracerArray, double *fluxArray, HABITAT_TYPES habitat);
int Calculate_Contaminants_Flux(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitatType);
int Calculate_Species_Contaminant_Effects(MSEBoxModel *bm, int box, int clayer, double dtsz, HABITAT_TYPES habitatType);

void Calculate_Contaminant_Q10_Corrections(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat);
void Change_Contaminant_Levels(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES habitat, double dtsz, int offset);
void Contaminant_Record_Death(MSEBoxModel *bm, int sp, int cohort, double amount);
void Contaminant_Write_Contact_Record(MSEBoxModel *bm);
void Contaminant_Close_Contact_Record(MSEBoxModel *bm);
void Contaminant_Update_ContactMort_Record(MSEBoxModel *bm, int sp, int cohort);
void ContaminantMigrationIn(MSEBoxModel *bm, int sp, int cohort, int mid, double tot_num_mig, double num_returning);
void ContaminantMigrationOut(MSEBoxModel *bm, int sp, int cohort, int mid, double oldden, double num_leaving);
void ContaminantDirectRectuitMigrants(MSEBoxModel *bm, int sp, int cohort, int mid);

void Apply_Contam_Return(MSEBoxModel *bm, int sp, int cohort, int k, int ij, double new_num, double tot_den);
void Age_Contaminants_Store(MSEBoxModel *bm, int sp, int cohort, int nextcid, double dennow, double this_p_ageup);
void Age_Contaminants_Update(MSEBoxModel *bm, int sp, int cohort, double denup, double dennow, double nextden, int ij, int k);
void Age_MigrantContaminants_Update(MSEBoxModel *bm, int sp, int nextcid, int cohort, int mid, double oldden, double num_aging) ;
void Get_Parental_Contaminants(MSEBoxModel *bm, int sp, int cohort, int flagmother, double this_den);
void Store_Recruit_Contaminants(MSEBoxModel *bm, int sp, int stock_id, int ngene, int qid);
void Get_Recruit_Contaminants(MSEBoxModel *bm, int sp, int stock_id, int ngene, int qid);
void Set_Recruit_Final_Contaminants(MSEBoxModel *bm, int wclayer, int sp, int ngene, int mid, int qid, int recruit_outside);
void Get_SettlerMigrant_Contaminants(MSEBoxModel *bm, int sp, int ngene, int mid, double oldden, double newden) ;
void Get_Settler_Contaminants(MSEBoxModel *bm, int wclayer, int sp, int ngene, int qid, double recruitSPden);
void Apply_Settler_Contaminants(MSEBoxModel *bm, int sp, int cohort, double starting_num, double new_num, int qid);
void Get_Suckling_Contaminants(MSEBoxModel *bm, int sp, int ad_cohort, int juv_cohort);

int Loose_Contaminant(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES globalHabitat, HABITAT_TYPES habitat, int species, int cohort, double amountLost, double ***spSPinfo, double initialBiomass);
int Gain_Contaminants(MSEBoxModel *bm, BoxLayerValues *boxLayerInfo, HABITAT_TYPES globalHabitat, HABITAT_TYPES habitat, int species, int cohort, double amountLost, double ***spSPinfo, double initialBiomass);

double Calculate_Contaminant_Repro_Scalar(MSEBoxModel *bm, int species);
double Avoid_Contaminants(MSEBoxModel *bm, int groupIndex, int cohort, int box, int layer);


void Get_ContamMoveEffects(MSEBoxModel *bm, int species, int cohort, int box, int layer);
void Move_Vert_Contaminated(MSEBoxModel *bm, int sp, int cohort, double ****currentden);

typedef enum{
	linear_contaminant_uptake_id = 1,
	sigmoidal_uptake_id,
	piecewise_linear_id,
	flatlinear_contaminant_uptake_id,
    invitro_sigmoid_id

} CONTAMINANT_UPTAKE_OPTION;

typedef enum {
	ongoingC_id,
	finalC_id
} CONTAMINANT_MORT_OPTION;


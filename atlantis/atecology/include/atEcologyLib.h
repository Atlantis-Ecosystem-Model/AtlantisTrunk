/**
 * \defgroup atEcology atEcology
 * \brief The ecology library of the Atlantis ecosystem model.
 *
 *Revisions:
 *
 *
 * 17-04-2013 Bec Gorton
 * ATL-346 - Added support of tracking P:N:C ratios in groups.
 *
 *
 */

#ifndef ATECOLOGYLIB_H_
#define ATECOLOGYLIB_H_


/**********************************************************************
Performance Measure Ids
*/
#define PDratio_id 0
#define FPFVratio_id 1
#define DivCount_id 2
#define InfEpi_id 3
#define BSSslope_id 4
#define coverindx_id 5

#define K_num_ecol_indx 6

/* Setup functions */
void Ecology_Init(MSEBoxModel *bm, FILE *llogfp);
void Ecology_Free(MSEBoxModel *bm);
void Ecology_Allocate_Diag_Arrays(MSEBoxModel *bm);
void Ecology_Add_FStat_Tracer(MSEBoxModel *bm, int index, char *name, char *longName, char *units, int sumType, int dtype, int *tracerIndex, int flux, int tol,
		int bio);

/* IO Functions */

void Ecology_Output_Mort_Estimates(MSEBoxModel *bm, FILE *llogfp);
void Ecology_Output_Mort_Per_Pred_Estimates(MSEBoxModel *bm, FILE *llogfp);

void Ecology_Box_Biology(MSEBoxModel *bm, Box *pBox, double dt, FILE *llogfp);

/* Per-thread workspace for running boxes in parallel (OpenMP builds only) */
int Box_Biology_Can_Run_In_Parallel(MSEBoxModel *bm);
void Box_Biology_Parallel_Begin(MSEBoxModel *bm);
void Box_Biology_Thread_Start(MSEBoxModel *bm);
MSEBoxModel *Box_Biology_Thread_Model(MSEBoxModel *bm);
void Box_Biology_Thread_End(MSEBoxModel *bm);
void Box_Biology_Parallel_End(MSEBoxModel *bm);

void Ecology_Annual(MSEBoxModel *bm, FILE *llogfp);
void Ecology_Calculate_Total_Abundance(MSEBoxModel *bm, double dt, int call_type, FILE *llogfp);

void Ecology_Update_Vertebrate_Cohorts(MSEBoxModel *bm, FILE *llogfp);
void Ecology_Update_Invert_Cohorts(MSEBoxModel *bm, FILE *llogfp);

/* Migration prototypes */
void Ecology_Total_Verts_And_Migration(MSEBoxModel *bm, double dt, FILE *llogfp);
void Ecology_Invading_Species(MSEBoxModel *bm, double dt, FILE *llogfp);
void Ecology_Invert_Migration(MSEBoxModel *bm, double dt, FILE *llogfp);

int Invade_Spread(MSEBoxModel *bm, int sp, FILE *llogfp, int b, int nb, int k, double *dist );

/* Warning and testing prototypes */
void Ecology_Test_Fish_Total(MSEBoxModel *bm, double ***valtr, double **landtr, int calltype, char *spotcall, FILE *llogfp);
void Ecology_Starve_Notice(MSEBoxModel *bm, FILE *llogfp);
void Ecology_Check_VertAbund(MSEBoxModel *bm, double ***valtr, FILE *llogfp, int instance);

double Ecology_Spawn(int species, double KSPA_SP, double FSP_SP, double SN, double RN, double X_RS, double FSPB_SP, FILE *llogfp);

void Ecology_Set_Index_Name_ID(MSEBoxModel *bm);

void    Ecology_Init_Populations(MSEBoxModel *bm, FILE *llogfp);

/* Warning and testing prototypes */
void	Ecology_Boundary_Check(MSEBoxModel *bm, FILE *llogfp);

/* Reading in forcing files */
void	Ecology_Read_Recruit_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void (*err)(char *format, ...));
void	Ecology_Read_Enviro_Forcing(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void (*err)(char *format, ...));
void 	Ecology_Read_LinearMortality_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) ;
void 	Ecology_Read_Size_Change_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...)) ;
void 	Ecology_Read_Growth_Rate_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...));
void 	Ecology_Read_FSPB_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...));
void    Ecology_Read_pCO2_TS(MSEBoxModel *bm, char *key, TimeSeries **ts, char *t_units, void(*err)(char *format, ...));

/* PhysioChem tracers */
void Ecology_Assign_Physio_Chem(MSEBoxModel *bm, FILE *llogfp);
void Ecology_PhysioChem_Free(MSEBoxModel *bm);

void Scale_Habitat(MSEBoxModel *bm, char *name, int box, double start, double period, double mult, FILE *llogfp);
void Scale_Group_Linear_Mortality(MSEBoxModel *bm, int speciesIndex, int cohort, int box, double start, double period, double mult, FILE *llogfp);
void Ecology_Update_Scaled_Values(MSEBoxModel *bm);

/* Time related prototypes */
void    Ecology_Time_Check(MSEBoxModel *bm, double t, double dt, FILE *llogfp);

/* Functions related to P:N:C tracking */
void Ecology_Initialise_Atomic_Info(MSEBoxModel *bm);
void Ecology_Free_Atomic_Info(MSEBoxModel *bm);

double Ecology_Get_Linear_Mortality(MSEBoxModel *bm, int sp, int cohort, int isAdult);

void Ecology_Land_Biology_Process(MSEBoxModel *bm, Box *pBox);

void Ecology_Apply_Environ_Scalars(MSEBoxModel *bm, double ***newwc, double ***newsedtr);
double Ecology_Get_Mortality_Scalar(MSEBoxModel *bm, int sp, int cohort);

/**
 * Indices of specific functional groups - these just make things faster - it means we don't need to go searching for
 * particular groups.
 */
extern int pelagicBactIndex;
extern int SedBactIndex;
extern int LabDetIndex;
extern int RefDetIndex;
extern int CarrionIndex;
extern int AquacultFeedIndex;
extern int IceBactIndex;

#endif /*ATECOLOGYLIB_H_*/

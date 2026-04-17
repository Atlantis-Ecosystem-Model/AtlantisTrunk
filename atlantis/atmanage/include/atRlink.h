#ifndef ATRLINK_H_
#define ATRLINK_H_

/* atRlink.h */

//Redus related
void intRedus(int dummy);
void create_r_redus_object(int numSP, const char** names, int endYear);
void redus_do_assessment(int minyearC, int maxyearC, char* groupCodeC);
void redus_update_biomass(int year, char* groupCode, double biomass);

double redus_do_hcr(int curyearC, char* groupCodeC);

int initRedus(int ans);
int redus_getRintObject(char *name);
int exec_r(const char* str);

const char** redus_getRvecObject(char *name, int *length);

void writeMetierFile(MSEBoxModel *bm, int year, FILE *fid);
void source(const char *name);

#endif /* ATRLINK_H_ */

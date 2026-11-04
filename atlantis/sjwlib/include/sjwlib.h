/*********************************************************************

    File:           sjwlib.h

    Created:        Mon May 24 14:49:22 EST 1993

    Author:         Stephen Walker
                    CSIRO Division of Oceanography

    Purpose:        Include file for general purpose library

    Revisions:      18/06/97 JRW
		    Added dumfile.h

    $Id: sjwlib.h 3377 2012-08-29 23:35:53Z gor171 $

*********************************************************************/

#ifdef _WIN32
	#define FOLDER_SEP "\\"
	#define WRONG_FOLDER_SEP "/"
	#define FOLDER_SEP_CHAR '\\'
	#define FOLDER_SEP_WRONG_CHAR '/'
#else
	#define FOLDER_SEP "/"
	#define WRONG_FOLDER_SEP "\\"
	#define FOLDER_SEP_CHAR '/'
	#define FOLDER_SEP_WRONG_CHAR '\\'
#endif

#ifndef	_SJWLIB_H
	#define _SJWLIB_H

	#define double double

	#define doubleINPUT  double
	/* For WIN32 */
	#ifdef _WIN32
		#ifndef M_PI
			#define M_PI (3.14159265358979323846)
		#endif
		#include <float.h>
		#include "win32.h"
			#ifndef isnan
				#define isnan _isnan
			#endif
	#endif



#ifndef TRUE
	#define TRUE		1
#endif
#ifndef FALSE
	#define FALSE		0
#endif

/* Useful defines */
#if !defined(PI)
#define PI (3.14159265358979323846)
#endif

#if !defined(M_PI)
#define M_PI (3.14159265358979323846)
#endif


//#ifndef max
#ifndef _WIN32
//#if !defined(max)
#define max(x,y) ((x)>(y) ? (x) : (y) )
#define min(x,y) ((x)<(y) ? (x) : (y) )
#define _hypot( x, y ) (hypot( x,y ) )
#endif

//#if !defined(min)
//#define min(x,y) ((x)<(y) ? (x) : (y) )
//#//endif

#if !defined(round)
#define	round(x)	((x-floor(x))>0.5 ? ceil(x) : floor(x))
#endif

/* Maximum length of an input line */
#define MAXLINELEN (4000)

/* Maximum length of a file name */
#define MAXFNAMELEN (200)

#include "sjwalloc.h"
#include "txt_param.h"
#include "polyline.h"
#include "colourtable.h"
#include "mapproj.h"
#include "datafile.h"
#include "pointsourcesink.h"
#include "ptrack.h"
#include "xytoij.h"
#include "gridlib.h"
#include "nrcdflib.h"

/* Prototypes */

void	quit(char *format, ...);
extern int suppress_runtime_warnings;  // Set from the -Q command flag. Warnings are not part of the model state. //
void	warn(char *format, ...);
void 	quiet(char *format, ...);
extern void (*keyprm_errfn)(char *format, ...);// = quit;
void	dspline(double *x, double *y, long n, int derivspec,
		double start_deriv, double end_deriv, double *ydd);
void	dsplint(double *xa, double *ya, double *ydd, long n, double x,
		double *y);
void	contour(double **val, double **x, double **y, long nx, long ny,
	char *cval, double badval);
void	cfft(double data[], int ndata, int dirn);
void	flushline(FILE *fp);
int 	nextline(char *line, long n, FILE *fp);
int 	iscommentline(char *line);
int     parseline(char *line, char **str, int max);
void    set_keyprm_errfn(void (*fn)(char *format, ...));
void    set_keyprm_case(int c);
int     skipToKeyStart(FILE *fp, char *key);
int     skipToKeyEnd(FILE *fp, char *key);
int     readkeyprm_i(FILE *fp, char *key, int *p);
int     readkeyprm_d(FILE *fp, char *key, double *p);
int	    readkeyprm_darray(FILE *fp, char *key, double **p, int *size);
int 	readkeyprm_iarray(FILE *fp, char *key, int **p, int *size);
int
readkeyprm_sarray(FILE *fp, char *key, char ***p, int *size);
int     readkeyprm_s(FILE *fp, char *key, char *p);
int     keyfscanf(FILE *fp, char *key, char *format, ...);
int	    nc_dimid(int cdfid, const char* name);
int	    nc_varid(int cdfid, const char* name);
int     ncvarfind(int fid, int nvdims, int *vdims, char *attr, char *attval, int *list);
int     ncvarsize(int fid, int vid);
void    ncvarread(int fid, char *name, int size, long *start, long *count, void *buf);
void 	checkNetCDFData2D( char *functionName, char *varName, double **data, int size1, int size2);
void 	checkNetCDFData1D( char *functionName, char *varName, double *data, int size1);
void 	checkNetCDFData1DShort(char *functionName, char *varName, short *data, int size1);
double  decay_forward(double c,double k,double dt);
double  decay_centered(double c,double k,double dt);
double  decay_backward(double c,double k,double dt);
double  decay_exact(double c,double k,double dt);
void    diffusion1d(int n, double *c, double *xc, double *k, double *xk, double dt, double a);

void    free_diffusion1d(int dummy);

double  w_tophat(double x, double scale);
double  w_linear(double x, double scale);
double  w_parabolic(double x, double scale);
double  w_gaussian(double x, double scale);
double  drandom(double min, double max);
float   ran3(int *init);
double  tojul(int y, int mo, int d, int h, int mi, int s);
void    todat(double j, int *y, int *mo, int *d, int *h, int *mi, int *s);
int     strtosecs(char *str, double *sec);
int     time_sec_param(FILE *fp, char *key, double *v);
char   *TimeToDate(double t, char *u);
double  DateToTime(char *d, char *u);
double  DateToJulian(char *d);
void    ChangeTimeUnits(char *oepoch, char *nepoch, double *times, int n);
double water_vapourp(double t);
double ppbfetch(char *folderPath, int useuv, double dir, double u, double v, double x, double y);
double inv_robbins(double x1, double y1, double z1, double x2, double y2, double z2,
              double a, double ecs);
double inv_sodanos(double x1, double y1, double x2, double y2,
              double a, double ecs);
double inv_sodanos_angles(double x1, double y1, double x2, double y2,
                 double a, double e2, double *raz);
void sodanos(double x1, double y1, double az1, double s,
                 double a, double e2, double *x2, double *y2);
double gc_distance(double x1, double y1, double x2, double y2);
void fwd_spherical_rot(double elon, double elat, double plon, double plat,
                       double *alon, double *alat);
void inv_spherical_rot(double alon, double alat, double plon, double plat,
                       double *elon, double *elat);

void trim(char *s);


FILE *Open_Input_File(const char *inputFolder,const  char *name, const char *mode);


/* External variables */

extern int keyprm_verbose;

#endif /* _SJWLIB_H */

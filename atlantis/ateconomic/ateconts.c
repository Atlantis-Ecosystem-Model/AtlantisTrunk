/**
\file
\brief Routines dealing with economic time series for use by numerical models
\ingroup ateconomic

    File:           ateconts.c

    Created:        Tue May 2 8:52:10 EST 2006

    Author:         Beth Fulton
                    CSIRO Division of Marine and Atmospheric Research

    Purpose:        Routines dealing with economic time series for use by numerical models

    Arguments:      See routines below

    Returns:        See below

    Revisions:      2/5/2006 Created using atfisheriests.c

    02-02-2009 Bec Gorton
    Removed the unused Free_Econ_Time_Series function.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sjwlib.h>
#include <atEconomic.h>
#include "atManageLib.h"

/**
 *	\brief Reads a list of time series of economic statistics (e.g. GDP) from an ascii file.
 *	The time series are specified as shown below.
 *	econtsnn means ts followed by the integer box number time series is in nn
 *  (with the minimum number of digits needed).
 *  Here, S is a string (which must not contain whitespace),
 *  N is an integer and X, Y, Z and A are floating point
 *  numbers.
 *
 *  This routine depends on the existence of a global variable:
 *  verbose -   Sets level of messages printed
 *
\begin{verbatim
# Parameters for each fisheries time series
# Point source/sink name
econtsnn.name    S

# Location ( x y )
econtsnn.location X Y

# Data - the next line is a time series definition
# as used by my timeseries routines in sjwlib. The example
# below assumes that the data is in an ascii or netCDF file.
econtsnn.data  filename
\end{verbatim}
  *
  * Input variables and parameters:
  * name:		Name of ascii file containing list of fisheries time series
  * tsname:		Name of times series (catch or effort) being read
  * t_units:	Time units to be used for time series
  * econts:		Returned pointer to fisheries time series list
  */
void Read_Econ_Time_Series(char *folderPath, char *name, char *tsname, char *t_units, FisheryTimeSeries **ts)
{
    FILE *fp;
    int nnts = 0;
    int nEconts;
    FisheryTimeSeries *fts = NULL;
    int i = 0;
    char key[MAXLINELEN];
	char buf[MAXLINELEN];

    /* Open the file */
    if( (fp=Open_Input_File(folderPath, name,"r")) == NULL )
        quit("Read_Econ_Time_Series: Can't open %s%s\n",folderPath, name);

    /* Print a heading on stderr if verbose enough */
    if( verbose > 1)
		fprintf(stderr, "Reading Economic Time Series %s\n", name);

    readkeyprm_i(fp,"nEconts",&nEconts);
    if(!nEconts)
        quit("Economic model currently expects a GDP file, if you don't have one please create a dummy file (filled with zeroes for example)\n");

    nnts = nEconts;

	/* Allocate memory for list of time series */
    if( (fts=(FisheryTimeSeries *)malloc((size_t)nnts*sizeof(FisheryTimeSeries))) == NULL )
        quit("Read_Econ_Time_Series: Can't allocate memory for economic %s time series list\n", tsname);

    /** Read each economic time series input **/
	/* Location */
    snprintf(key, sizeof(key), "%sts%d.location",tsname,i);
	readkeyprm_s(fp,key,buf);
	if( sscanf(buf,"%lf %lf",&fts[i].x,&fts[i].y) != 2 )
		quit("Incorrect information specified for %s, need x y format", key);

	/* Box entry for FisheryTimeSeries template not actually needed for economic time
	   series as yet so set to default value of 0
	*/
	fts[i].b = 0;

	/* Data */
    snprintf(key, sizeof(key),"%sts%d.data",tsname,i);

	readkeyprm_s(fp,key,buf);
    tsRead(folderPath, buf,&fts[i].ts);
	if(verbose > 1)
		tsPrintInfo(&fts[i].ts,stderr);

	/* Check data time units */
	if( strcmp(fts[i].ts.t_units,t_units) != 0 ) {
		if( verbose > 1) {
		  fprintf(stderr,"Converting %s time series time units\n",tsname);
		  fprintf(stderr,"    Was %s\n",fts[i].ts.t_units);
		  fprintf(stderr,"    Now %s\n",t_units);
		}
		tsNewTimeUnits(&fts[i].ts,t_units);
	}

    /* Close the file */
    fclose(fp);

    /* Store pointer to list of point source/sinks */
    *ts = fts;

	return;
}

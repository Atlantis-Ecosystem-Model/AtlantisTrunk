/*

    File:           pointsourcesink.c

    Created:        Sat Dec 30 12:02:20 EST 1995

    Author:         Stephen Walker
                    CSIRO Division of Oceanography

    Purpose:        Routines dealing with point sources/sinks
		    for use by numerical models

    Arguments:      See routines below

    Returns:        See below

    Revisions:      19/12/1997 SJW
		    Added code to allow point sources/sinks to have
		    a location which varies in time, and which can be
		    distributed vertically.

		    19-04-2010 Bec Gorton
		    Added an additional variable to the readPointSourceSink function to return the
		    maximum number of variables in the point source files read in.

    $Id: pointsourcesink.c 3417 2012-09-24 04:34:46Z gor171 $

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <math.h>
#include <sjwlib.h>


void parse_location(char *folderPath, PointSourceSink *p, char *line);


extern int verbose;


/** Reads a list of point sources and sinks from an ascii file.
  * The sources sinks are specified as shown below.
  * pssnn means pss followed by the integer point source/sink number nn
  * (with the minimum number of digits needed).
  * Here, S is a string (which must not contain whitespace),
  * N is an integer and X, Y, Z and A are floating point
  * numbers.
  *
  * This routine depends on the existence of a global variable:
  * verbose -   Sets level of messages printed
  *
\begin{verbatim}
# Number of point source/sinks
npointss     N

# Parameters for each point source/sink
# Point source/sink name
pssnn.name    S

# Location ( x y z)
pssnn.location X Y Z

# Data - the next line is a time series definition
# as used by my timeseries routines in sjwlib. The example
# below assumes that the data is in an ascii or netCDF file.
pssnn.data  filename
\end{verbatim}
  *
  * @param name Name of ascii file containing point source/sink list
  * @param t_units Time units to be used for time series
  * @param pss Returned pointer to point source/sink list
  * @param np Returned number of point sources/sinks
  * @param xyzijk Pointer to function which converts (x,y,z) to (i,j,k)
  * @param trI Pointer to function which finds model index of tracer name
  */
void
readPointSourceSink(
char *folderPath,
char *name,		/* File name */
char *t_units,
PointSourceSink **pss,
int *np,
void *model_data,		/* Data for xyzijk and trI */
int (*xyzijk) (void *, double, double, double, int *, int *, int *),
int (*trI) (void *, char *)
)
{
    FILE *fp;
    int npss = 0;
    PointSourceSink *p = NULL;
    int i;
    int n;

    /* Open the file */
    if( (fp=Open_Input_File(folderPath, name,"r")) == NULL )
        quit("readPointSourceSink: Can't open %s%s\n", folderPath, name);

    if( verbose )
	fprintf(stderr,"readPointSourceSink: reading from %s\n",name);

    /* Print a heading on stderr if verbose enough */
    if( verbose > 1)
		fprintf(stderr, "# Sources/sinks\n");

    /* Get the number of point inputs */
    readkeyprm_i(fp,"npointss",&npss);
    
    /* Allocate memory for list of point inputs */
    if( (p=(PointSourceSink *)malloc((size_t)npss*sizeof(PointSourceSink))) == NULL )
        quit("readPointSourceSink: Can't allocate memory for point source/sink list\n");

    /* Read each point input */
    for(i=0; i<npss; i++) {
        char key[MAXLINELEN];
		char buf[MAXLINELEN];

		/* Store index routine pointer and data needed by it */
		p[i].xyzijk = xyzijk;
		p[i].model_data = model_data;

        /* Name */
        snprintf(key, sizeof(key),"pss%d.name",i);
        readkeyprm_s(fp,key,p[i].name);

        /* Location */
        snprintf(key, sizeof(key),"pss%d.location",i);
		readkeyprm_s(fp,key,buf);
		parse_location(folderPath, &p[i],buf);

        /* Data */
        snprintf(key, sizeof(key),"pss%d.data",i);
		readkeyprm_s(fp,key,buf);
		/* Read in the data */
        tsRead(folderPath, buf,&p[i].ts);
		if(verbose > 1)
			tsPrintInfo(&p[i].ts,stderr);

		/* Rewind flag */
		snprintf(key, sizeof(key), "pss%d.rewind", i);
		readkeyprm_i(fp,key,&p[i].rewindid);

		/* Check data time units */
		if( strcmp(p[i].ts.t_units,t_units) != 0 ) {
		   if( verbose > 1) {
			  fprintf(stderr,"Converting %s time units\n",p[i].name);
			  fprintf(stderr,"    Was %s\n",p[i].ts.t_units);
			  fprintf(stderr,"    Now %s\n",t_units);
		   }
		   tsNewTimeUnits(&p[i].ts,t_units);
		}

		/* Find variable indices for each of the time series variables */
		p[i].vid = i_alloc1d(p[i].ts.nv);
		p[i].watertsid = -1;
		for(n=0; n<p[i].ts.nv; n++) {
			if( strcasecmp(p[i].ts.varname[n],"water") == 0 ||
					strcasecmp(p[i].ts.varname[n],"flow") == 0 ) {
				p[i].watertsid = n;
			}
			p[i].vid[n] = (*trI)(model_data, p[i].ts.varname[n]);
			if( p[i].vid[n] < 0 && n != p[i].ts.ti && n != p[i].watertsid )
				warn("readPointSourceSink: file %s, variable '%s' in %s isn't a model tracer!\n",name, p[i].ts.varname[n],p[i].name);
		}

    }

    /* Close the file */
    fclose(fp);

    /* Store pointer to list of point source/sinks */
    *pss = p;
    *np = npss;
}

void
locatePointSourceSink(PointSourceSink *p, double t)
{
    /* Check if the source/sink has a time varying location */
    if( p->loc == NULL )
        return;

    /* Get x, y and z values */
    p->x = tsEval(p->loc, p->x_id, t);
    p->y = tsEval(p->loc, p->y_id, t);
    p->zlow = tsEval(p->loc, p->zl_id, t);
    p->zhigh = tsEval(p->loc, p->zh_id, t);
    p->z = (p->zlow+p->zhigh)/2;

    /* Convert to model indices */
    if( (*(p->xyzijk))(p->model_data, p->x,p->y,p->z,&p->e1,&p->e2,&p->e3) < 0 )
    	quit("locatePointSourceSink: %s location (%.10g,%.10g,%.10g) can't be converted to indices\n",p->name,p->x,p->y,p->z);
}

void
parse_location(char *folderPath, PointSourceSink *p, char *line)
{
    int e1, e2, e3;

    /* Clear location time series pointer */
    p->loc = NULL;

    /* Time independent, range of z values */
    if( sscanf(line,"%lf %lf %lf %lf",&p->x,&p->y,&p->zlow,&p->zhigh) == 4 ) {
        if( p->zlow >= p->zhigh )
	    quit("readPointSourceSink: %s has bad z range (must be low then high)\n",p->name);
	p->z = (p->zlow+p->zhigh)/2;
	if( (*(p->xyzijk))(p->model_data, p->x,p->y,p->z,&e1,&e2,&e3) < 0 )
	    quit("readPointSourceSink: %s location (%.10g,%.10g,%.10g) can't be converted to indices\n",p->name,p->x,p->y,p->z);
	p->e1 = e1;
	p->e2 = e2;
	p->e3 = e3;

    }
    /* Time independent, single  z value */
    else if( sscanf(line,"%lf %lf %lf",&p->x,&p->y,&p->z) == 3 ) {
	if( (*(p->xyzijk))(p->model_data, p->x,p->y,p->z,&e1,&e2,&e3) < 0 )
	    quit("readPointSourceSink: %s location (%.10g,%.10g,%.10g) can't be converted to indices\n",p->name,p->x,p->y,p->z);
	p->e1 = e1;
	p->e2 = e2;
	p->e3 = e3;
	p->zlow = p->z;
	p->zhigh = p->z;

    }
    /* Time dependent, all coords from a time series */
    else if (1) /*( access(line, R_OK) == 0 )*/ {
	if( (p->loc = malloc(sizeof(TimeSeries))) == NULL )
	     quit("readPointSourceSink: No memory for location time series\n");
    tsRead(folderPath, line,p->loc);
	/* Get coordinate indices */
	if( (p->x_id = tsIndex(p->loc, "x")) < 0 && (p->x_id = tsIndex(p->loc, "X")) < 0 )
	    quit("readPointSourceSink: Location timeseries must have a variable named x (or X)\n");
	if( (p->y_id = tsIndex(p->loc, "y")) < 0 && (p->y_id = tsIndex(p->loc, "Y")) < 0 )
	    quit("readPointSourceSink: Location timeseries must have a variable named y (or Y)\n");
	if( (p->zl_id = tsIndex(p->loc, "z_low")) < 0 || (p->zh_id = tsIndex(p->loc, "z_high")) < 0 )
	    quit("readPointSourceSink: Location timeseries must have variables named z_low and z_high\n");

	/* Set location at some arbitrary initial time.
	 * Probably don't really need to do this here, as
	 * any routines using the structure should call
	 * the location routine themselves.
	 */
	locatePointSourceSink(p, 0.0);
    }
    else {
	quit("readPointSourceSink: Can't understand location definition for %s\n",p->name);
    }
}

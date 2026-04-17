/*

 File:           timeseries.c

 Created:        Wed Dec 14 12:51:51 EST 1994

 Author:         Stephen Walker
 CSIRO Division of Oceanography

 Purpose:        Routines which deal with time series data.

 There are several ways of defining a time series.


 1. Ascii file of times and values
 If the file is an ascii file, it must have the
 following format:

 # Comments
 ## COLUMNS n
 ##
 ## COLUMN1.name XXXX
 ## COLUMN1.long_name XXXX
 ## COLUMN1.units XXXX
 ## COLUMN1.missing_value XXXX
 ##
 ## COLUMN2.name XXXX
 ## COLUMN2.long_name XXXX
 ## COLUMN2.units XXXX
 ## COLUMN2.missing_value XXXX
 ##
 .
 .
 .
 ##
 v   v   v   v   ...
 v   v   v   v   ...
 v   v   v   v   ...
 .
 .
 .

 2. netCDF file with time and 0, 1 or 2 spatial
 dimensions.

 In either case, the timeseries will contain all
 the variables in the specified file.


 Arguments:      See each routine below.

 Returns:        See each routine below.

 Revisions:      19/12/1995 SJW
 Altered ascii format to be compatible with PPB
 column file format.
 Implemented spatial netCDF files.

 11/06/97 JRW
 Implement XY to IJ for Polar grids.

 06/08/97 JRW
 The timeseries routines are now convenience
 routines for the datafile routines.

 21/05/98 JRW/SJW
 Corrected an error with the reallocation
 of memory for the time units.

 21/05/2008 Bec Gorton
 Changed tsEvalXYR to store the value ATT_TEXT(gt) to a local
 char * variable before calling strcmpcase to remove a complier warning.

 $Id: timeseries.c 3417 2012-09-24 04:34:46Z gor171 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <netcdf.h>
#include "sjwlib.h"

void df_ascii_write(FILE *fp, Datafile *df);

/** Read and set up a time series.
 *
 * @param name File containing the time series data.
 * @param ts pointer to time series structure (assumed
 * 		 not previously initialised)
 *
 * This routine calls quit() if anything goes wrong
 */
void tsRead(char *folderName, char *name, TimeSeries *ts) {
	int i;
	int index;
	Datafile *df;

	if ((df = (Datafile *) malloc(sizeof(Datafile))) == NULL)
		quit("tsRead: Failed to allocate memory for Datafile.\n");
	memset(df, 0, sizeof(Datafile));

	dfRead(folderName, name, df);
	if (df->records == NULL) {
		index = dfIndex(df, "t");
		if (index >= 0)
			dfSetRecord(df, index);
		else if ((index = dfIndex(df, "time")) >= 0)
			dfSetRecord(df, index);
	}

	/* Copy the convenience variables over */
	if (df->type == DFT_NETCDF) {
		switch (df->nd) {
		case 0:
			ts->type = TS_NC0;
			break;

		case 1:
			ts->type = TS_NC1;
			break;

		case 2:
			ts->type = TS_NC2;
			break;

		case 3:
			ts->type = TS_NC3;
			break;

		default:
			break;
		}
	} else
		ts->type = TS_ASCII;

	ts->name = df->name;
	ts->nt = df->nrecords;
	ts->ti = df->ri;
	ts->t_units = df->rec_units;
	ts->t = df->records;
	ts->nv = df->nv;

	/* Copy variables */
	if ((ts->varname = (char **) malloc(sizeof(char *) * (size_t)df->nv)) == NULL)
		quit("tsRead: Failed to allocate memory for variable names.\n");
	memset(ts->varname, 0, sizeof(char *) * (size_t)df->nv);
	for (i = 0; i < df->nv; ++i) {
		ts->varname[i] = (char *) malloc(sizeof(char) * (strlen(df->variables[i].name) + 1));
		if (ts->varname[i] == NULL)
			quit("tsRead: Failed to allocate memory.\n");
		strcpy(ts->varname[i], df->variables[i].name);
	}

	if ((ts->varunit = (char **) malloc(sizeof(char *) * (size_t)df->nv)) == NULL)
		quit("tsRead: Failed to allocate memory for variable units.\n");
	memset(ts->varunit, 0, sizeof(char *) * (size_t)df->nv);
	for (i = 0; i < df->nv; ++i) {
		if (df->variables[i].units != NULL) {
			ts->varunit[i] = (char *) malloc(sizeof(char) * (strlen(df->variables[i].units) + 1));
			if (ts->varunit[i] == NULL)
				quit("tsRead: Failed to allocate memory.\n");
			strcpy(ts->varunit[i], df->variables[i].units);
		}
	}

	ts->df = df;
}

/** Set the form in which all geographic data should be
 * returned.
 * @param ts pointer to time series structure
 * @param type character string containing the geotype.
 * NULL - no prefered output form, no transformation occurs,
 * data same as file format.
 * "geographic" - All 'geographic' related data should be
 * returned as latitudes and longitudes.
 * "proj=xxx .." - Standard MapProjection format.
 */
void tsSetGeoType(TimeSeries *ts, char *type) {
	dfSetGeoType(ts->df, type);
}

/** Set the default geographic data type.
 * @param type character string containing the geotype.
 * NULL - no prefered output form, no transformation occurs,
 * data same as file format.
 * "geographic" - All 'geographic' related data should be
 * returned as latitudes and longitudes.
 * "proj=xxx .." - Standard MapProjection format.
 */
void tsSetDefaultGeoType(char *type) {
	dfSetDefaultGeoType(type);
}

/** Find time indices which bracket a requested time
 *
 * @param ts pointer to time series structure
 * @param t specified sample time
 * @param before index value just before time specified
 * @param after index value just after time specified
 * @param frac fraction of time interval
 */
void tsFindTime(TimeSeries *ts, double t, int *before, int *after, double *frac) {
	dfFindRecord(ts->df, t, before, after, frac);
}

/** Evaluate a time series at a specified time.
 *
 * @param ts pointer to time series structure
 * @param id index of variable to evaluate
 * @param t time value
 * @return value at time t.
 *
 * Calls quit() if something goes wrong.
 */
double tsEval(TimeSeries *ts, int id, double t) {
	double ans;

	/* As no rewinding specified, assume none wanted */
	ans = tsEvalR(ts, id, t, 0);

	return ans;
}

double tsEvalR(TimeSeries *ts, int id, double t, int rwindid) {
	Variable *v;
	double newt = t;

	if (ts == NULL)
		quit("tsEval: NULL TimeSeries pointer\n");
    
	v = dfGetVariable(ts->df, id);
	if (v == NULL)
		quit("tsEval: No variable corresponding to id = %d in file %s\n", id, ts->name);

	if (rwindid && (dfFindTime(ts->df, t, rwindid, &newt) == 1)) {
		return dfEval(ts->df, v, newt);
	} else
		return dfEval(ts->df, v, t);
}

double tsEvalEx(TimeSeries *ts, int id, double t)
/* When do not want interpolation */
{
	Variable *v;

	if (ts == NULL)
		quit("tsEvalEx: NULL TimeSeries pointer\n");

	v = dfGetVariable(ts->df, id);
	if (v == NULL)
		quit("tsEvalEx: No variable corresponding to id = %d\n", id);

	return dfEvalEx(ts->df, v, t);
}

/** Evaluate a time series at a specified time and place.
 *
 * @param ts pointer to time series structure
 * @param id index of variable to evaluate
 * @param t time value
 * @param x x value
 * @param y y value
 * @return value at (t,x,y).
 *
 * Calls quit() if something goes wrong.
 */
double tsEvalXY(TimeSeries *ts, int id, double t, double x, double y) {

	double ans;

	/* As no rewinding specified, assume none wanted */
	ans = tsEvalXYR(ts, id, t, x, y, 0);
	return ans;
}

double tsEvalXYR(TimeSeries *ts, int id, double t, double x, double y, int rwindid) {
	Datafile *df = ts->df;
	Variable *v = dfGetVariable(df, id);
	double coords[MAXNUMCOORDS];
	int i;
	int nc = 0;
	int *coordtypes = NULL;
	double newt = t;

	if (v == NULL)
		quit("tsEvalXY: Invalid variable id specified (%d).\n", id);

	/* If 0 dimension, then the best we can do is give the
	 * interpolated time value */
	if (v->nd == 0) {
		if (rwindid && (dfFindTime(df, t, rwindid, &newt) == 1)) {
			return dfEval(df, v, newt);
		} else {
			return dfEval(df, v, t);
		}
	}

	/* Define the coordinate system if one is not specified. */
	if (v->csystem == NULL) {
		/* Guess by looking at the attributes */
		if (!dfInferAndSetCoordSystem(df, v)) {
			/* Failed to inter a coordinate system. To be backwardly
			 * compatible, we should look for the variables 'x' and 'y'. */
			int x_id = dfIndex(df, "x");
			int y_id = dfIndex(df, "y");
			if ((x_id != -1) && (y_id != -1)) {
				CoordinateMapping map;
				int coordids[MAXNUMCOORDS];
				int coordtypes[MAXNUMCOORDS];
				Variable *vx = dfGetVariable(df, x_id);
				Variable *vy = dfGetVariable(df, y_id);
				char *str;

				/* If the x and y coordinate have a single dimension
				 * only, then we assume the data represents point
				 * data. */
				if ((vx->nd < 2) && (vy->nd < 2)) {
				}

				/* If both variables have 2 dimensions we can shall assume
				 * a grid coordinate system. We shall also try to
				 * provide analytic attributes if these already exist
				 * in the file as global attributes. */
				else if ((vx->nd == 2) && (vy->nd == 2)) {
					int dim1size = (int)df->dimensions[vx->dimids[0]].size;
					int dim2size = (int)df->dimensions[vx->dimids[1]].size;

					Attribute *gt = dfGetGlobalAttribute(df, "gridtype");
					if (gt != NULL) {
						char analytic[MAXLINELEN];
						str = ATT_TEXT(gt);

						analytic[0] = 0;
						/* We have a analytic expression. Determine the type
						 * and add this as as an attribute to the x and y
						 * variables in the datafile structure.
						 * Always assume cell edge. */
						if (strcasecmp(str, "rectangular") == 0) {
							Attribute *x0 = dfGetGlobalAttribute(df, "xorigin");
							Attribute *y0 = dfGetGlobalAttribute(df, "yorigin");
							Attribute *dx = dfGetGlobalAttribute(df, "dx");
							Attribute *dy = dfGetGlobalAttribute(df, "dy");
							Attribute *rot = dfGetGlobalAttribute(df, "rotation");
							snprintf(analytic, sizeof(analytic), "rectangular 0.0 0.0 %d %d %g %g %g %g %g", dim1size, dim2size, ATT_DOUBLE(x0,0), ATT_DOUBLE(y0,0),
									ATT_DOUBLE(dx,0), ATT_DOUBLE(dy,0), ATT_DOUBLE(rot,0));
						}

						else if (strcasecmp(str, "polar") == 0) {
							Attribute *x0 = dfGetGlobalAttribute(df, "xorigin");
							Attribute *y0 = dfGetGlobalAttribute(df, "yorigin");
							Attribute *arc = dfGetGlobalAttribute(df, "arc");
							Attribute *rmin = dfGetGlobalAttribute(df, "rmin");
							Attribute *rot = dfGetGlobalAttribute(df, "rotation");
							snprintf(analytic, sizeof(analytic), "polar 0.0 0.0 %d %d %g %g %g %g %g", dim1size, dim2size, ATT_DOUBLE(x0,0), ATT_DOUBLE(y0,0),
									ATT_DOUBLE(arc,0), ATT_DOUBLE(rmin,0), ATT_DOUBLE(rot,0));
						}

						if (analytic[0] != 0) {
							dfAddTextAttribute(df, vx, "analytic", analytic);
							dfAddTextAttribute(df, vy, "analytic", analytic);
						}

					}
				}

				else
					quit("tsEvalXY: Unable to locate any appropriate coordinate variables for variable '%s'.\n", v->name);

				/* Build the coordinate map and try to set the coordinate.
				 */
				map.nc = 2;
				map.coordids = coordids;
				map.coordtypes = coordtypes;
				map.coordids[0] = x_id;
				map.coordtypes[0] = VT_X;
				map.coordids[1] = y_id;
				map.coordtypes[1] = VT_Y;
				if (dfSetCoordSystem(df, v, 1, &map) == 0)
					quit("tsEvalXY: Unable to locate any appropriate coordinate variables for variable '%s'.\n", v->name);
			} else
				quit("tsEvalXY: Unable to locate any appropriate coordinate variables for variable '%s'.\n", v->name);
		}
	}

	nc = dfGetNumCoordinates(df, v);
	coordtypes = dfGetCoordTypes(df, v);
	if (nc != 2)
		quit("tsEvalXY: Expected only 2 coordinates (X,Y) but had %d.\n", nc);

	for (i = 0; i < nc; ++i) {
		if (coordtypes[i] & (VT_X | VT_LONGITUDE))
			coords[i] = x;
		else if (coordtypes[i] & (VT_Y | VT_LATITUDE))
			coords[i] = y;
		else
			quit("tsEvalXY: The coordinate maps do not contain XY coordinate types.\n");
	}

	if (rwindid && (dfFindTime(ts->df, t, rwindid, &newt) == 1)) {
		return dfEvalCoords(df, v, newt, coords);
	} else
		return dfEvalCoords(df, v, t, coords);

}

/** Evaluate a time series at a specified time and 3d place.
 *
 * @param ts pointer to time series structure
 * @param id index of variable to evaluate
 * @param t time value
 * @param x x value
 * @param y y value
 * @param z z value
 * @return value at (t,x,y,z).
 *
 * Calls quit() if something goes wrong.
 */
double tsEvalXYZ(TimeSeries *ts, int id, double t, double x, double y, double z) {
	double ans;

	/* As no rewinding specified, assume none wanted */
	ans = tsEvalXYZR(ts, id, t, x, y, z, 0);

	return ans;

}

double tsEvalXYZR(TimeSeries *ts, int id, double t, double x, double y, double z, int rwindid) {
	Datafile *df = ts->df;
	Variable *v = dfGetVariable(df, id);
	double coords[MAXNUMCOORDS];
	int i;
	int nc = 0;
	int *coordtypes = NULL;
	double newt = t;

	if (v == NULL)
		quit("tsEvalXYZ: Invalid variable id specified (%d).\n", id);

	/* If 0 dimension, then the best we can do is give the
	 * interpolated time value */
	if (v->nd == 0) {
		if (rwindid && (dfFindTime(df, t, rwindid, &newt) == 1)) {
			return dfEval(df, v, newt);
		} else
			return dfEval(df, v, t);
	} else if (v->nc == 2)
		return tsEvalXY(ts, id, t, x, y);

	/* Define the coordinate system if one is not specified. */
	if (v->csystem == NULL) {

		/* Guess by looking at the attributes */
		if (!dfInferAndSetCoordSystem(df, v))
			quit("tsEvalXYZ: Unable to infer the coordinate system.");
	}

	nc = dfGetNumCoordinates(df, v);
	coordtypes = dfGetCoordTypes(df, v);
	if (nc == 2)
		return tsEvalXY(ts, id, t, x, y);
	else if (nc != 3)
		quit("tsEvalXYZ: Expected only 3 coordinates (X,Y,Z) but had %d.\n", nc);

	for (i = 0; i < nc; ++i) {
		if (coordtypes[i] & (VT_X | VT_LONGITUDE))
			coords[i] = x;
		else if (coordtypes[i] & (VT_Y | VT_LATITUDE))
			coords[i] = y;
		else if (coordtypes[i] & (VT_Z))
			coords[i] = z;
		else
			quit("tsEvalXYZ: The coordinate maps do not contain XYZ coordinate types.\n");
	}

	if (rwindid && (dfFindTime(ts->df, t, rwindid, &newt) == 1)) {
		return dfEvalCoords(df, v, newt, coords);
	} else
		return dfEvalCoords(df, v, t, coords);
}

/** Check whether the specified variable located in a series
 * of timeseries file. Also check whether the variable spans
 * the range.
 *
 * The time check is still fairly simple minded. It does not
 * look for holes in the data.
 *
 * @param ntsfiles Number of timeseries files in array.
 * @param tsfiles Array of timeseries files.
 * @param var Name of variable to check.
 * @param tstart Lower end of range to check.
 * @param tstop Upper end of range to check.
 * @return non-zero value if successful.
 */
int tsMultiCheck(int ntsfiles, TimeSeries *tsfiles, char *var, double tstart, double tstop) {
	double f_tstart = 1e300;
	double f_tstop = -1e300;
	int found_var = 0;

	if (ntsfiles > 0) {
		int i;
		for (i = 0; i < ntsfiles; ++i) {
			TimeSeries *ts = &tsfiles[i];
			int varid = tsIndex(ts, var);

			if (varid >= 0) {
				found_var = 1;
				f_tstart = (ts->t[0] < f_tstart) ? ts->t[0] : f_tstart;
				f_tstop = (ts->t[ts->nt - 1] > f_tstop) ? ts->t[ts->nt - 1] : f_tstop;
			}
		}

		if (!found_var) {
			quit("tsMultiCheck: Unable to located the variable '%s' in the timeseries files.\n", var);
			return 0;
		} else {
			if (!((tstart >= f_tstart) && (tstop <= f_tstop)))
				warn("tsMultiCheck: The variable '%s' does not span the\ntimeseries files for the time range %.10g to %.10g (%.10g to %.10g).\n", var,
						tstart, tstop, f_tstart, f_tstop);
		}
	} else {
		quit("tsMultiCheck: No timeseries files specified.\n");
		return 0;
	}

	return 1;

}

/** Evaluate in the first appropriate file the specified variable
 * at the given point and time.
 *
 * @param ntsfiles Number of timeseries files in array.
 * @param tsfiles Array of timeseries files.
 * @param var Name of variable to check.
 * @param t time value
 * @param x x value
 * @param y y value
 * @param z z value
 * @return non-zero value if successful.
 */
double tsMultiEvalXYZ(int ntsfiles, TimeSeries *tsfiles, char *var, double t, double x, double y, double z) {
	if (ntsfiles > 0) {
		int i;
		int lastvalidindex = -1;
		for (i = 0; i < ntsfiles; ++i) {
			TimeSeries *ts = &tsfiles[i];
			int varid = tsIndex(ts, var);

			if (varid >= 0)
				lastvalidindex = i;

			/* If the variable exists in the file and the current time
			 * is within the bounds of the timeseries files domain,
			 * then evaluate the point. */
			if (((varid >= 0) && (t >= ts->t[0]) && (t <= ts->t[ts->nt - 1]))) {
				return tsEvalXYZ(ts, varid, t, x, y, z);
			}

			/* If this is the last timeseries file, then evaluate the
			 * point. */
			else if (i == (ntsfiles - 1)) {
				if (lastvalidindex >= 0) {
					ts = &tsfiles[lastvalidindex];
					varid = tsIndex(ts, var);
					return tsEvalXYZ(ts, varid, t, x, y, z);
				} else
					quit("tsMultiEvalXYZ: Unable to evaluate the variable '%s'.\n", var);
			}
		}
	} else
		quit("tsMultiEvalXYZ: No timeseries files specified.\n");

	return 0.0;
}

/** Find the index of a variable in a time series
 *
 * @param ts pointer to time series structure
 * @param name name of variable to find
 * @return index value (>= 0) if sucessful, -1 otherwise.
 */
int tsIndex(TimeSeries *ts, char *name) {
	return dfIndex(ts->df, name);
}

/** Change time units in a time series
 *
 * @param ts pointer to time series structure
 * @param newunits new units string
 *
 * This routine calls quit() if anything goes wrong
 */
void tsNewTimeUnits(TimeSeries *ts, char *newunits) {
	Datafile *df = ts->df;

	ChangeTimeUnits(ts->t_units, newunits, df->records, (int)df->nrecords);

	/* Store new units in time series */
	if ((ts->t_units = (char *) realloc((void *) ts->t_units, strlen(newunits) + 1)) == NULL)
		quit("tsNewTimeUnits: Can't allocate memory for new units\n");
	strcpy(ts->t_units, newunits);

	/* Propogate the change into the DataFile */
	df->rec_units = ts->t_units;
	df->variables[df->ri].units = ts->t_units;
}

/** Free memory associated with a time series.
 *
 * @param ts pointer to time series structure
 *
 * This routine calls quit() if anything goes wrong.
 */
void tsFree(TimeSeries *ts) {

	if (ts->varname != NULL) {
		int i;
		for (i = 0; i < ts->nv; ++i) {

			if (ts->varname[i] != NULL)
				free(ts->varname[i]);
			if (ts->varunit[i] != NULL)
				free(ts->varunit[i]);

		}
		free(ts->varname);
		free(ts->varunit);
	}

	dfFree(ts->df);
}

/** Check that time values in a time series are
 * monotonic increasing.
 *
 * @param ts pointer to time series structure
 *
 * This routine calls quit() if anything goes wrong.
 */
void tsCheckTimes(TimeSeries *ts) {
	dfCheckRecords(ts->df);
}

/** Print summary information about a time series.
 *
 * @param ts pointer to time series structure
 * @param fp output FILE pointer
 *
 * This routine calls quit() if anything goes wrong.
 */
void tsPrintInfo(TimeSeries *ts, FILE *fp) {
	dfPrintInfo(ts->df, fp, 0);
}

/** Write the timeseries data as an ASCII file to the specified
 * file stream.
 *
 * @param fp output FILE pointer
 * @param ts pointer to file series structure
 */
void ts_ascii_write(FILE *fp, TimeSeries* ts) {
	df_ascii_write(fp, ts->df);
}

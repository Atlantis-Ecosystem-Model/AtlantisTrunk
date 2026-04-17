/*

 File:           dfeval.c

 Purpose:        Routines which deal interpolating datafile
 data.

 Created:        Wed Jun 18 13:41:27 EST 1997

 Author:         Jason R. Waring
 CSIRO Division of Marine Research

 Modifications:
 This file is based on Stephen Walker's
 original timeseries.c (see the CVS archive prior to
 18/06/97.

 $Id: dfeval.c 3369 2012-08-29 06:16:46Z gor171 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "sjwlib.h"

/* local prototypes */
double df_interp_1d_inv_weight(Datafile *df, Variable *v, int record, double coords[]);
double df_interp_linear(Datafile *df, Variable *v, int record, double coords[]);

/** Evaluate a zero dimension datafile variable for
 * a particular record value.
 *
 * @param df pointer to data file structure
 * @param v pointer to variable structure.
 * @param r record value
 * @return Interpolated value.
 *
 * @see quit() If anything goes wrong.
 */
double dfEval(Datafile *df, Variable *v, double r) {
	double frac;
	int before;
	int after;
	double val;

	/* Sanity checks */
	if (df == NULL)
		quit("dfEval: NULL Datafile pointer\n");
	if (v == NULL)
		quit("dfEval: NULL Variable pointer\n");
	if (v->nd > 0)
		quit("dfEval: Interpolation is ambiguous because data \
                 is multi-dimensional, use dfEvalCoords.\n");

	if ((df->records == NULL) || (!v->dim_as_record))
		return v->data[0];

	/* Find nearest records in table */
	if (dfFindRecord(df, r, &before, &after, &frac) == 0){
		/* Requested record not in range of those in file */
		/* NULL STATEMENT - Should take some action here */;
	}
	dfReadRecords(df, v, before, after - before + 1);

	val = v->missing;

	/* Value exactly at before record */
	if (frac == 0.0 && (fabs(v->data[before] - v->missing) > 1e-10)) {
		val = v->data[before];
		/* Value exactly at after record */
	} else if (frac == 1.0 && (fabs(v->data[after] - v->missing) > 1e-10)) {
		val = v->data[after];
		/* Value inbetween - need to interpolate */
	} else if ((fabs(v->data[after] - v->missing) > 1e-10) && (fabs(v->data[before] - v->missing) > 1e-10)) {
		val = v->data[before] * (1.0 - frac) + v->data[after] * frac;
	}

	return (val);
}

double dfEvalEx(Datafile *df, Variable *v, double r) {
	double frac;
	int before;
	int after;
	double val;

	/* Sanity checks */
	if (df == NULL)
		quit("dfEvalEx: NULL Datafile pointer\n");
	if (v == NULL)
		quit("dfEvalEx: NULL Variable pointer\n");
	if (v->nd > 0)
		quit("dfEvalEx: Ambiguous data - it is multi-dimensional, use dfEvalCoords.\n");

	if ((df->records == NULL) || (!v->dim_as_record))
		return v->data[0];

	/* Find nearest records in table */
	if (dfFindRecord(df, r, &before, &after, &frac) == 0){
		/* Requested record not in range of those in file */
		/* NULL STATEMENT - Should take some action here */;
	}
	dfReadRecords(df, v, before, after - before + 1);

	val = v->missing;

	/* Value exactly at before record */
	if (frac == 0.0 && (fabs(v->data[before] - v->missing) > 1e-10))
		val = v->data[before];
	/* Value exactly at after record */
	else if (frac == 1.0 && (fabs(v->data[after] - v->missing) > 1e-10))
		val = v->data[after];
	/* Value inbetween - need to use previous record as don't want interpolation
	 (for cases when have data per day - like fisheries records) so want that
	 value to last all day not just the first few seconds of the day */
	else if ((fabs(v->data[before] - v->missing) > 1e-10))
		val = v->data[before];

	return (val);
}

/** Interpolate the variable at known coordinates and record.
 *
 * @param df pointer to datafile structure.
 * @param v pointer to variable structure.
 * @param r record value.
 * @param coords coordinates values.
 * @return Interpolated value.
 *
 * @see dfGetNumCoordinates
 * @see dfGetCoordIds
 * @see quit() If anything goes wrong.
 */
double dfEvalCoords(Datafile *df, Variable *v, double r, double coords[]) {
	double val = 0.0;

	/* Sanity checks */
	if (df == NULL)
		quit("dfEvalCoords: NULL Datafile pointer\n");
	if (v == NULL)
		quit("dfEvalCoords: NULL Variable pointer\n");

	if (v->nd == 0) {
		val = dfEval(df, v, r);
	} else {
		int i;
		int r0, r1;
		double rfrac;
		double recvals[2];
		CoordinateSystem *cs = v->csystem;
		int nc = 0;
		int nd = 0;

		recvals[0] = 0.0;
		recvals[1] = 0.0;

		if (cs == NULL)
			quit("dfEvalCoords: No coordinate system specified for variable '%s'\n", v->name);

		nc = dfGetNumCoordinates(df, v);
		nd = dfGetNumDimensions(df, v);
		/* Find nearest records in table */
		if ((v->dim_as_record) && (df->records != NULL))
			dfFindRecord(df, r, &r0, &r1, &rfrac);
		else {
			r0 = r1 = 0;
			rfrac = 0.0;
		}

		/* Read in the data for each record and interpolate the coordinates
		 * for each record. Store the interpolated value for each record
		 * in the recvals array.
		 */
		dfReadRecords(df, v, r0, r1 - r0 + 1);
		for (i = r0; i <= r1; ++i) {
			int j = i - r0;
			recvals[j] = 0.0;

			/* Are there the same number of coordinates as dimensions.
			 * We shall assume a regular grid. */
			if (nc == nd) {
				recvals[j] = df_interp_linear(df, v, i, coords);
			}

			/* Are there more coordinate than dimensions, and is there
			 * only one dimension. If so, then we can assume than we
			 * shall interpolate using an inverse weighting scheme. */
			else if (nc > nd) {
				if (nd == 1){
					recvals[j] = df_interp_1d_inv_weight(df, v, i, coords);
				}else{
					quit("dfEvalCoords: Unable to interpolate data with %d coordinate and %d dimensions.\n", nc, nd);
				}
			} else
				quit("dfEvalCoords: Less coordinates than dimensions.\n");
		}

		/* Interpolate the record */
		val = recvals[0] * (1.0 - rfrac) + recvals[1] * rfrac;
	}

	return val;
}

/* PRIVATE and PROTECTED functions */
/*
 Routine to interpolate 1d spatial data.

 NOTE: This is quite simple minded, using an inverse squared distance
 weighting scheme. Because this routine is likely to be called twice
 for each time series evaluation, the weights do not really need
 to be re-calculated the second time. As well, you could use a hash
 table mechanism to remember the weights for a large number of points.
 This would be a good idea for use with numerical models, which tend
 to evaluate time series repeatedly at a fairly small set of points.
 For now, I haven't time to do this.
 */
double df_interp_1d_inv_weight(Datafile *df, Variable *v, int record, double coords[]) {
	int i, j;
	int dsize = (int)df->dimensions[v->dimids[0]].size;
	int nc = dfGetNumCoordinates(df, v);
	int *coordids = dfGetCoordIds(df, v);
	double div = 0.0;
	double sum = 0.0;
	double *d = VAR_1D(v)[record - v->start_record];
	Variable *vars = df->variables;

	/* Calculate weighted sum of all data points */
	for (i = 0; i < dsize; ++i) {
		double dist = 0.0;

		for (j = 0; j < nc; ++j) {
			double dx = VAR_1D(&vars[coordids[j]])[0][i] - coords[j];
			dist += dx * dx;
		}

		dist = sqrt(dist);
		if (dist < 1e-10)
			return d[i];
		div += 1 / dist;
		sum += d[i] / dist;
	}

	return (sum / div);
}

/*
 Routine to interpolate grid spatial data using a simple bilinear-like
 interpolation scheme.
 */
double df_interp_linear(Datafile *df, Variable *v, int record, double coords[]) {
	int i, j;
	double val = 0.0;
	double indices[MAXNUMDIMS];
	int nd = dfGetNumDimensions(df, v);
	int *dimids = dfGetDimIds(df, v);

	if (dfCoordsToIndices(df, v, coords, indices) == nd) {
		double findices[MAXNUMDIMS];
		int iindices[MAXNUMDIMS];
		int corner[MAXNUMDIMS];
		int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS];
		int dsize[MAXNUMDIMS];
		int ncorners = 1 << nd;

#if 0
		fprintf(stderr, "Coords =");
		for (i=0; i<v->csystnc; ++i)
		fprintf(stderr, " %g", coords[i]);
		fprintf(stderr, "\n");

		fprintf(stderr, "Indices =");
		for (i=0; i<nd; ++i)
		fprintf(stderr, " %g", indices[i]);
		fprintf(stderr, "\n");
#endif

		/* Compute the fraction indices and integer indices.
		 * Trim at boundaries if necessary. */
		for (i = 0; i < nd; ++i) {
			dsize[i] = (int)df->dimensions[dimids[i]].size - 1;
			findices[i] = indices[i];
			if (findices[i] < 0)
				findices[i] = 0.0;
			if (findices[i] > dsize[i])
				findices[i] = (double) dsize[i];
			iindices[i] = (int) floor(findices[i]);
			findices[i] -= iindices[i];
		}

		/* Create the corner indice offsets (0 or 1) */
		for (i = 0; i < ncorners; ++i)
			for (j = 0; j < nd; ++j)
				coffsets[i][j] = (i >> j) & 0x01;

		/* Now step though all of the corners and perform
		 * the linear interpolation */
		for (j = 0; j < ncorners; ++j) {
			double term = 1.0;
			for (i = 0; i < nd; ++i) {
				corner[i] = iindices[i] + coffsets[j][i];
				if (corner[i] > dsize[i])
					corner[i] = dsize[i];
				if (coffsets[j][i])
					term *= findices[i];
				else
					term *= (1 - findices[i]);
			}

#if 0
			fprintf(stderr, "Corner(%d) =", j);
			for (i=0; i<nd; ++i)
			fprintf(stderr, " %d", corner[i]);
			fprintf(stderr, "\n");
#endif
			val += term * dfGetDataValue(df, v, record, corner);
		}
	}

	return (val);
}

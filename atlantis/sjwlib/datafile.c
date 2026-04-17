/*
 *
 * Routines which deal with reading, writing and
 * interpolating data. The data may have associated
 * coordinate variable, and these may be multi-dimensional.
 *
 * This file is heavily based on Stephen Walker's
 * original timeseries.c (see the CVS archive prior to
 * 18/06/97.
 *
 * The routines will currently only allow interpolation
 * on a limited set of KNOWN coordinate systems. It is
 * difficult to see how this could be done for a VERY
 * general case.
 *
 * Created: Wed Jun 18 13:41:27 EST 1997.
 *
 * Modification:
 *
 * $Id: datafile.c 3417 2012-09-24 04:34:46Z gor171 $
 *
 * Author Jason R. Waring
 *
 * CSIRO Division of Marine Research
 *
 * See dfcoords.c and dfeval.c
 *
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
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

/* Prototypes for local routines */
void df_netcdf_read(int fid, Datafile *df);
void df_netcdf_free(Datafile *df);
void df_netcdf_read_attrib(Datafile *df, int varid, int attnum, Attribute *a);
void df_ascii_read(FILE *fp, Datafile *df);
void df_ascii_write(FILE *fp, Datafile *df);
void df_ascii_free(Datafile *df);

void df_alloc_data_records(Datafile *df, Variable *v, int nrecs);
void df_free_data_records(Datafile *df, Variable *v);
double* df_alloc_data(Datafile *df, Variable *v);
/*void df_free_data(Datafile *df, Variable *v, double *data); */
void df_read_data(Datafile *df, Variable *v, int rec);
void df_left_shift_data(Datafile *df, Variable *v, int nrecs);
void df_right_shift_data(Datafile *df, Variable *v, int nrecs);

/* Located in dfcoords.c */
extern void df_decode_coords(Datafile *df, Variable *v, char *coords);
extern void df_decode_coord_type(char *ctype, VariableType *type, char **coord_domain);
void df_free_coord_system(Datafile *df, CoordinateSystem *csystem);
void dfSetRecord(Datafile *df, int varid);

extern int df_default_geotype;
extern char *df_default_projection;

/** Read and set up a datafile.
 *
 * There are several ways of defining the data file.
 *
 * 1. Ascii file of values stored in a column array.
 * If the file is an ascii file, it must have the
 * following format:
 *
 \begin{verbatim}
 # Comments
 ## COLUMNS n
 ##
 ## COLUMN1.name XXXX
 ## COLUMN1.long_name XXXX
 ## COLUMN1.units XXXX
 ## COLUMN1.missing_value XXXX
 ## COLUMN1.fill_value XXXX
 ##
 ## COLUMN2.name XXXX
 ## COLUMN2.long_name XXXX
 ## COLUMN2.units XXXX
 ## COLUMN2.missing_value XXXX
 ## COLUMN2.fill_value XXXX
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
 \end{verbatim}
 * Note that at least the older .ts files do not have the fill_value
 * variable and so this was commented out of the code here (makes no
 * difference to the output as not needed and was only put in for
 * completeness sake by Jason Waring)
 *
 * 2. netCDF file with 0, 1 or 2 spatial
 * dimensions. The file may support contain one
 * unlimited dimension. This dimension is handled
 * specially, and it is assumed that the values
 * associated with the dimension monotonically
 * increase/decrease.
 *
 * @param name file containing the data file data.
 * @param df pointer to datafile structure (assumed
 * 	 not previously initialised).
 *
 * @see quit() If anything goes wrong.
 */
void dfRead(char *folderPath, char *name, Datafile *df) {
	int fid;
	FILE *fp;
	char tempFileName[BMSLEN];

	/* Clear the structure */
	memset(df, 0, sizeof(Datafile));

	/* Store the name */
	if ((df->name = (char *) malloc(strlen(name) + 1)) == NULL)
		quit("dfRead: Can't allocate memory for name\n");
	strcpy(df->name, name);


	snprintf(tempFileName, sizeof(tempFileName), "%s%s", folderPath, name);
	/* Try to open the file */
	if (nc_open(tempFileName, NC_NOWRITE, &fid) == NC_NOERR) {
		/* It is a netCDF file! */
		df_netcdf_read(fid, df);
	}
	else if ((fp = Open_Input_File(folderPath, name, "r")) != NULL) {
		/* Assume it is an ascii file */
		df_ascii_read(fp, df);
	}

	else
		quit("dfRead: Can't open %s\n", tempFileName);

	if (df->records != NULL)
		dfCheckRecords(df);
}

/** Set the record variable. This can be quite expensive as it
 * requires all variables that depend on the record dimension
 * to be adjusted.
 *
 * The record variable MUST be one dimensional, and the
 * record dimension must be the first dimension of ALL
 * variables in the Datafile that use this dimension.
 *
 * @param df pointer to datafile.
 * @param varid variable index/identifier.
 */
void dfSetRecord(Datafile *df, int varid) {
	Variable *rv = NULL;
	int recdimid = -1;
	int i, n;
	double *data;

	/* Check if this is a valid variable */
	if ((varid < 0) && (varid >= df->nv))
		quit("dfSetRecord: Invalid variable identifier.\n");
	else
		rv = &df->variables[varid];

	/* Confirm that the variable contains only one dimension. */
	if ((rv->nd == 1) && (df->dimensions[rv->dimids[0]].size > 0))
		recdimid = rv->dimids[0];
	else
		quit(
				"dfSetRecord: Record variable must contain single non-zero dimension.\n");

	/* Read in the record data and populate the record info.
	 * and record variable in the Datafile structure. */
	if (df->type == DFT_NETCDF) {
		if (rv->data != NULL)
			free(rv->data);
		rv->data = (double *) malloc(sizeof(double) * df->dimensions[recdimid].size);
		data = (double *) malloc(sizeof(double) * df->dimensions[recdimid].size);
		//nc_get_var_double(df->ncid, varid, rv->data);
		nc_get_var_double(df->ncid, varid, data);
		for(n = 0; n < (int)df->dimensions[recdimid].size; n++){
			rv->data[n] = data[n];

		}
		free(data);
	}
	rv->start_record = 0;
	rv->nrecords = (int)df->dimensions[recdimid].size;

	/* Copy over the record information. */
	df->records = rv->data;
	df->nrecords = (int) df->dimensions[recdimid].size;
	df->rec_name = rv->name;
	df->rec_units = rv->units;
	df->rec_longname = rv->longname;
	df->ri = varid;
	df->geotype = df_default_geotype;
	df->projection = df_default_projection;

	/* Sweep through all of the variables and reduce any that
	 * contain the recdimid as the first dimension. Nullify
	 * the data. */
	for (i = 0; i < df->nv; ++i) {
		Variable *v = &df->variables[i];
		if (v->dimids[0] == recdimid) {
			int i;

			v->dim_as_record = 1;
			if (v->data != NULL) {
				v->start_record = 0;
				v->nrecords = (int)df->nrecords;
			} else {
				v->start_record = 0;
				v->nrecords = 0;
			}

			--(v->nd);
			for (i = 0; i < v->nd; ++i)
				v->dimids[i] = v->dimids[i + 1];

		} else
			v->dim_as_record = 0;
	}
}

/**
 * Find record indices which bracket a requested record value.
 *
 * @param df pointer to datafile structure
 * @param r specified sample record
 * @param before index value just before record specified
 * @param after index value just after record specified
 * @param frac fraction of record interval
 */
int dfFindRecord(Datafile *df, double r, int *before, int *after, double *frac) {
	int imid;
	int ilow = 0;
	int ihigh = (int)df->nrecords - 1;

	if (df->records == NULL) {
		*before = 0;
		*after = 0;
		*frac = 0.0;
		return (0);
	}
    
	/* first check whether t is within the table range */
	if (r <= df->records[ilow]) {
		*before = ilow;
		*after = ilow;
		*frac = 0.0;
		return (0);
	}
	if (r >= df->records[ihigh]) {
		*before = ihigh;
		*after = ihigh;
		*frac = 0.0;
		return (0);
	}

	/* perform binary chop to determine values either side of t */
	while (ihigh - ilow > 1) {
		imid = (ilow + ihigh) / 2;
		if (r >= df->records[imid])
			ilow = imid;
		else
			ihigh = imid;
	}

	/* Store results and return */
	*before = ilow;
	*after = ihigh;
	*frac = (r - df->records[ilow]) / (df->records[ihigh] - df->records[ilow]);
	return (1);
}

/**
 * If recycling forcing files and current time iutside time in datafile
 * determine appropriate time to use.
 *
 * @param df pointer to datafile structure
 * @param r specified sample record
 * @param before index value just before record specified
 * @param after index value just after record specified
 */
int dfFindTime(Datafile *df, double r, int rwind, double *newt) {
	int ilow = 0;
	int ihigh = (int)df->nrecords - 1;

	if (df->records == NULL) {
		*newt = r;
		return (0);
	}

	/* Check whether t is within the table range */
	if (r <= df->records[ilow]) {
		*newt = r;
		return (0);
	}
	if (r >= df->records[ihigh]) {
		if (!rwind) {
			*newt = r;
			return (0);
		} else {
			/* If not in range and use cycling of forcing then recycle */
			double lengthdf = df->records[ihigh] - df->records[ilow];
			double leftover = floor(r / lengthdf);

			*newt = r - leftover * df->records[ilow];
			return (1);
		}
	}

	/* Otherwise use r as is */
	*newt = r;
	return (0);
}

/**
 * Read a region of data records into a memory buffer.
 *
 * If the variable has not been read before, then memory
 * will be allocated.
 *
 * If the variable data has already been read then the
 * function will not re-read the data.
 *
 * If a portion of the specified record region has already
 * been read, then the buffers will be adjusted, and only the
 * new data read in.
 *
 * @param df pointer to datafile structure
 * @param v pointer to variable
 * @param start_rec record number (negative if var has not record info).
 * @param nrecs record number (must be 1 if start_rec is negative).
 */
void dfReadRecords(Datafile *df, Variable *v, int start_rec, int nrecs) {
	int i;

	if (df->type == DFT_ASCII)
		return;

	/* Check whether data has been allocated. If one of the
	 * dimension is a record, check that the size is the same
	 * as requested. If not clear the data, it's too much effort
	 * to regig.
	 */
	if ((v->data != NULL) && (v->dim_as_record)) {
		if (v->nrecords != nrecs) {
			df_free_data_records(df, v);
			v->data = NULL;
			v->start_record = 0;
			v->nrecords = 0;
		}
	}

	/* Check whether the record range is appropriate for the
	 * specified variable. Quit if incorrect values were passed through.
	 */
	if (start_rec >= 0) {
		if (v->dim_as_record) {
			if (start_rec >= df->nrecords)
				quit(
						"dfReadRecords: Start buffer record (%d) out of bounds for variable '%s'.\n",
						start_rec, v->name);

			if ((start_rec + nrecs - 1) >= df->nrecords)
				quit("dfReadRecords: End buffer record (%d) out of bounds for variable '%s'.\n", start_rec + nrecs - 1, v->name);
		} else if (nrecs != 1)
			quit("dfReadRecords: nrecs should be 1 for variable '%s' not %d\n", v->name, nrecs);
	}

	else
		quit("dfReadRecords: start_rec was negative.");

	/* Check to see whether the data has already been read for the
	 * range specified. If so, then ignore.
	 */
	if (!((v->data != NULL) && (v->start_record == start_rec) && (v->nrecords == nrecs))) {
		int sr = start_rec;
		int er = start_rec + nrecs - 1;

		/* Define the range (sr and er) which need to be
		 * read from the file. By default read the whole
		 * region specified. */
		if ((v->data == NULL) || (!v->dim_as_record)) {

			/* Variable with non-record dimension or NULL data.
			 * Don't need to worry about swapping. */
			if (v->data == NULL)
				df_alloc_data_records(df, v, nrecs);
		} else {
			/* Variable with record dimension and previously
			 * allocated memory. If new data is entirely
			 * outside of the existing range, then re-read. If overlap
			 * then swap buffers and read non-overlapping regions. */
			int csr = v->start_record;
			int cer = v->start_record + v->nrecords - 1;
			if (!((er < csr) || (sr > cer))) {

				if (sr < csr) {
					/* Overlap at start - Shift right */
					df_right_shift_data(df, v, csr - sr);
					er = csr - 1;
				}

				else {
					/* Overlap at end - Shift left */
					df_left_shift_data(df, v, sr - csr);
					sr = cer + 1;
				}
			}
		}

		v->start_record = start_rec;
		v->nrecords = nrecs;
		for (i = sr; i <= er; ++i)
			df_read_data(df, v, i);
	}

	/* Is there a coordinate system. If so, then for a read of
	 * all of the variables in the coordinate system. */
	if (v->csystem != NULL) {
		CoordinateSystem *cs = v->csystem;
		for (i = 0; i < cs->nc; ++i) {
			Variable *cv = &df->variables[cs->coordids[i]];
			if (cv->dim_as_record)
				dfReadRecords(df, cv, start_rec, nrecs);
			else
				dfReadRecords(df, cv, 0, 1);
		}
	}

}

/**
 * Free memory associated with a data file.
 *
 * @param df pointer to data file structure.
 *
 * @see quit() If anything goes wrong.
 */
void dfFree(Datafile *df) {
	if (df != NULL) {
		int i, j;

		/* Free any memory specifically associated with
		 * the file type */
		if (df->type == DFT_ASCII)
			df_ascii_free(df);
		else
			df_netcdf_free(df);

		/* Deallocate memory associated with variables */
		if (df->variables != NULL) {
			for (i = 0; i < df->nv; ++i) {
				Variable *v = &df->variables[i];

				free(v->name);

				if (v->longname != NULL)
					free(v->longname);

				if (v->units != NULL)
					free(v->units);

				if (v->ncdimids != NULL) {
					free(v->ncdimids);
				}

				if (v->coord_domain != NULL)
					free(v->coord_domain);

				if (v->coordids != NULL)
					free(v->coordids);

				if (v->attributes != NULL) {
					for (j = 0; j < v->na; ++j) {

						/* Sweet sweet ugly code TODO: Try to clean this up*/
						if ((strcasecmp(v->attributes[j].name, "long_name") != 0) && (strcasecmp(v->attributes[j].name, "units") != 0)) {
							free(v->attributes[j].value);
						}

						free(v->attributes[j].name);

					}
					free(v->attributes);
				}

				if (v->csystem != NULL)
					df_free_coord_system(df, v->csystem);

				if (v->dimids != NULL)
					free(v->dimids);

				if (v->data != NULL)
					df_free_data_records(df, v);
			}

			free(df->variables);
		}

		/* Deallocate memory associated with the dimensions */
		if (df->dimensions != NULL) {
			/* Free the first name as this is always allocated*/
			free(df->dimensions[0].name);
			for (i = 1; i < df->nd; ++i) {
				free(df->dimensions[i].name);
			}
			free(df->dimensions);
		}

		/* Deallocate memory associated with the attributes */
		if (df->attributes != NULL) {
			for (i = 0; i < df->na; ++i) {
				if (df->attributes[i].name != NULL)
					free(df->attributes[i].name);
				if (df->attributes[i].value != NULL)
					free(df->attributes[i].value);
			}
			free(df->attributes);
		}
		free(df->name);
		free(df);
	}
}

/**
 * Read a data record into memory buffer.
 * This function is effectively an interface to dfReadRecords.
 *
 * @param df pointer to datafile structure
 * @param df pointer to datafile structure
 * @param rec record number (Use 0 if variable is indep of record dim).
 *
 * @see dfReadRecords
 */
void dfReadRecord(Datafile *df, Variable *v, int rec) {
	dfReadRecords(df, v, rec, 1);
}

/** Find the index of a variable in a data file.
 *
 * @param df pointer to data file structure
 * @param name name of variable to find
 * @return variable index. -1 if unsuccessful.
 */
int dfIndex(Datafile *df, const char *name) {
	int i;

	if (name == NULL)
		return (-1);

	for (i = 0; i < df->nv; i++)
		if (strcasecmp(name, df->variables[i].name) == 0 || ((df->variables[i].longname != NULL)
				&& (strcasecmp(name, df->variables[i].longname) == 0)))
			return (i);

	return (-1);
}

/** Convienience function to find the number of variables.
 *
 * @param df pointer to data file structure.
 * @return Number of variables in the datafile structure.
 */
int dfGetNumVariables(Datafile *df) {
	return df->nv;
}

/** Convienience function to find the variable structure for the
 * specified variable id.
 *
 * @param df pointer to data file structure.
 * @param varid variable index.
 * @return pointer to variable. NULL if out of bounds.
 */
Variable *dfGetVariable(Datafile *df, int varid) {
	if ((varid < 0) || (varid >= df->nv))
		return NULL;

	return &df->variables[varid];
}

/** Convienience function to find the variable structure by name.
 *
 * @param df pointer to data file structure.
 * @param name variable name.
 * @return pointer to variable. NULL if out of bounds.
 */
Variable *dfGetVariableByName(Datafile *df, const char *name) {
	return dfGetVariable(df, dfIndex(df, name));
}

/** Convienience function to find the dimension structure for the
 * specified dimension id.
 *
 * @param df pointer to data file structure.
 * @param dimid dimension index.
 * @return pointer to dimension. NULL if out of bounds.
 */
Dimension *dfGetDimension(Datafile *df, int dimid) {
	if ((dimid < 0) || (dimid >= df->nd))
		return NULL;

	return &df->dimensions[dimid];
}

/** Find the global attribute by name, and return the attribute
 * structure.
 *
 * @param df pointer to datafile structure.
 * @param name name of the global attribute variable.
 * @return pointer to the attribute structure. NULL if unsuccessful.
 */
Attribute *dfGetGlobalAttribute(Datafile *df, const char *name) {
	int i;

	for (i = 0; i < df->na; ++i) {
		if (strcasecmp(df->attributes[i].name, name) == 0)
			return &df->attributes[i];
	}

	return NULL;
}

/** Find the global attribute by name, and return the attribute
 * structure.
 *
 * @param df pointer to datafile structure.
 * @param v pointer to the variable structure.
 * @param name name of the global attribute variable.
 * @return pointer to the attribute structure. NULL if unsuccessful.
 */
Attribute *dfGetAttribute(Datafile *df, Variable *v, const char *name) {
	int i;

	for (i = 0; i < v->na; ++i) {
		if (strcasecmp(v->attributes[i].name, name) == 0)
			return &v->attributes[i];
	}

	return NULL;
}

/** Add a text attribute to the variable. This is quite an
 * expensive routine as it requires the table to be reallocated.
 *
 * @param df pointer to the datafile structure.
 * @param v pointer to the variable structure.
 * @param name attribute name.
 * @param text attribute text.
 */
void dfAddTextAttribute(Datafile *df, Variable *v, const char *name, const char *text) {
	int i;
	int na = v->na + 1;
	Attribute *atts = NULL;

	atts = (Attribute*) malloc((size_t)na * sizeof(Attribute));
	memset(atts, 0, (size_t)na * sizeof(Attribute));

	/* Copy over the attribute information to the new atts array */
	for (i = 0; i < v->na; ++i) {
		atts[i].name = v->attributes[i].name;
		atts[i].n = v->attributes[i].n;
		atts[i].type = v->attributes[i].type;
		atts[i].value = v->attributes[i].value;
	}

	/* Now add the new information to the last element in the
	 * table. */
	atts[v->na].name = (char *) malloc((strlen(name) + 1) * sizeof(char));
	strcpy(atts[v->na].name, name);
	atts[v->na].n = (int)strlen(text);
	atts[v->na].type = AT_TEXT;
	atts[v->na].value = (void*) malloc(sizeof(char) * (size_t)(atts[v->na].n + 1));
	strcpy((char*) atts[v->na].value, text);

	/* Swizzle the old for the new, and free the old. */
	if (v->attributes) {
		free(v->attributes);
	}
	v->na = na;
	v->attributes = atts;

}

/**
 * Check that record values in a data file are
 * monotonic increasing or decreasing.
 *
 * @param df pointer to data file structure
 *
 * @see quit() If anything goes wrong.
 */
void dfCheckRecords(Datafile *df) {
	int i;
	int failed = 0;

	for (i = 1; (i < df->nrecords) && (failed == 0); i++) {
		if (df->records[i] <= df->records[i - 1])
			failed = 1;
	}

	if (failed) {
		for (i = (int)df->nrecords - 1; i > 0; i--) {
			if (df->records[i] >= df->records[i - 1]){
				quit("tsCheckRecords: Records out of order, %s, record=%g\n", df->name, df->records[i]);
			}
		}
	}
}

/**
 * Print summary information about a data file.
 *
 * @param df pointer to data file structure.
 * @param fp output FILE pointer.
 *
 * @see quit() If anything goes wrong.
 */
void dfPrintInfo(Datafile *df, FILE *fp, int level) {
	int last = (int)df->nrecords - 1;
	int i, j, n;

	fprintf(fp, "Data file: %s\n", df->name);
	if ((df->nrecords > 0) && (df->records != NULL)) {
		fprintf(fp, "Units %s\n", df->rec_units);
		fprintf(fp, "Starts=%.12g - ", df->records[0]);
		fprintf(fp, "Ends=%.12g\n", df->records[last]);

		if (level > 2) {
			for (i = 0; i < df->na; ++i) {
				Attribute *a = &df->attributes[i];
				fprintf(fp, "Global attribute %d: %s = ", i, a->name);
				if (a->type != AT_TEXT) {
					for (j = 0; j < (int)a->n; ++j) {
						switch (a->type) {
						case AT_BYTE:
							fprintf(fp, "%c", ATT_BYTE(a,j));
							break;

						case AT_FLOAT:
							fprintf(fp, "%g", ATT_FLOAT(a, j));
							break;

						case AT_DOUBLE:
							fprintf(fp, "%g", ATT_DOUBLE(a, j));
							break;

						case AT_SHORT:
							fprintf(fp, "%d", (int) ATT_SHORT(a, j));
							break;

						case AT_INT:
							fprintf(fp, "%d", ATT_INT(a, j));
							break;

						default:
							break;
						}
					}
				} else
					fprintf(fp, "%s", (char *) a->value);
				fprintf(fp, "\n");
			}
		}
	} else
		fprintf(fp, "No named record available.\n");

	if (level <= 0) {
		if (df->type == DFT_ASCII)
			fprintf(fp, "Column\tVariable\n");
		else
			fprintf(fp, "ID\tVariable\n");

		for (n = 0; n < df->nv; n++) {
			fprintf(fp, "%d\t%s\n", n + (df->type == DFT_ASCII), df->variables[n].name);
		}
	} else {
		for (n = 0; n < df->nv; n++) {
			Variable *v = &df->variables[n];

			if (df->type == DFT_ASCII)
				fprintf(fp, "Column:       %d\n", n + 1);
			else
				fprintf(fp, "ID:           %d\n", n);

			fprintf(fp, "Variable:     %s\n", v->name);
			fprintf(fp, "Long name:    %s\n", (v->longname) ? v->longname
					: v->name);
			fprintf(fp, "Units:        %s\n", (v->units) ? v->units
					: "no units");
			fprintf(fp, "Missing:      %g\n", v->missing);
			fprintf(fp, "Fill value:   %g\n", v->fillvalue);
			if (level > 2) {
				for (i = 0; i < v->na; ++i) {
					Attribute *a = &v->attributes[i];
					fprintf(fp, "Attrib %d:     %s = ", i, a->name);
					if (a->type != AT_TEXT) {
						for (j = 0; j < (int) a->n; ++j) {

							switch (a->type) {
							case AT_BYTE:
								fprintf(fp, "%c", ATT_BYTE(a, j));
								break;

							case AT_FLOAT:
								fprintf(fp, "%g", ATT_FLOAT(a, j));
								break;

							case AT_DOUBLE:
								fprintf(fp, "%g", ATT_DOUBLE(a, j));
								break;

							case AT_SHORT:
								fprintf(fp, "%d", (int) ATT_SHORT(a, j));
								break;

							case AT_INT:
								fprintf(fp, "%d", ATT_INT(a, j));
								break;

							default:
								break;
							}
						}
					} else
						fprintf(fp, "%s", (char *) a->value);
					fprintf(fp, "\n");
				}
			}
			if (level > 1) {
				fprintf(fp, "Dimensions:  ");
				if (v->dim_as_record) {
					fprintf(
							fp,
							" %s",
							df->dimensions[df->variables[df->ri].dimids[0]].name);

				}
				for (i = 0; i < v->nd; ++i)
					fprintf(fp, " %s", df->dimensions[v->dimids[i]].name);
				fprintf(fp, "\n");

				fprintf(fp, "Coordinates: ");
				for (i = 0; i < v->nc; ++i) {
					fprintf(fp, " %s", df->variables[v->coordids[i]].name);
					if (level > 2) {
						switch (df->variables[v->coordids[i]].type) {
						case VT_X:
							fprintf(fp, "(X)");
							break;

						case VT_Y:
							fprintf(fp, "(Y)");
							break;

						case VT_Z:
							fprintf(fp, "(Z)");
							break;

						case VT_TIME:
							fprintf(fp, "(TIME)");
							break;

						case VT_LONGITUDE:
							fprintf(fp, "(LONGITUDE)");
							break;

						case VT_LATITUDE:
							fprintf(fp, "(LATITUDE)");
							break;

						default:
							break;
						}
					}
				}
				fprintf(fp, "\n");
			}
			fprintf(fp, "\n\n");
		}
	}

	fprintf(fp, "\n");
}

/** Get the data value for the specified record and indices.
 *
 * @param df pointer to datafile structure.
 * @param v pointer to variable structure.
 * @param record record index (not relative record index).
 * @param is array of indice values.
 * @return Value at specified record and index.
 *
 * @see dfGetNumDimensions
 */
double dfGetDataValue(Datafile *df, Variable *v, int record, int *is) {
	int ri = record - v->start_record;
	double val = 0.0;

	if (ri < 0)
		ri = 0;
	if (ri >= v->nrecords)
		ri = v->nrecords - 1;

	switch (v->nd) {
	case 0:
		val = VAR_0D(v)[ri];
		break;

	case 1:
		val = VAR_1D(v)[ri][is[0]];
		break;

	case 2:
		val = VAR_2D(v)[ri][is[0]][is[1]];
		break;

	case 3:
		val = VAR_3D(v)[ri][is[0]][is[1]][is[2]];
		break;

	case 4:
		val = VAR_4D(v)[ri][is[0]][is[1]][is[2]][is[3]];
		break;

	default:
		quit("dfgetDataValue: Bad number of dimensions\n");
		break;
	}
	return val;
}

/* PRIVATE or PROTECTED functions */

/*
 Routine to read data file from a netCDF file. The netcdf read
 overrides any record name already specified.

 Arguments:
 fid     -    netCDF file id
 df      -    pointer to datafile structure (assumed
 not previously initialised)

 This routine calls quit() if anything goes wrong
 */
void df_netcdf_read(int fid, Datafile *df) {
	int i, j;
	char line[MAXLINELEN];
	int unlimited = -1;

	/* Query the netCDF file for the total number of dimensions,
	 * allocate the array, and populate.
	 *
	 * Special care must be taken define which dimension is the special
	 * record (or unlimited dimension) */
	df->type = DFT_NETCDF;
	df->ncid = fid;
	df->ri = -1;
	nc_inq_ndims(df->ncid, &df->nd);
	if (df->nd > 0) {
		df->dimensions = (Dimension *) malloc((size_t)df->nd * sizeof(Dimension));
		memset(df->dimensions, 0, (size_t)df->nd * sizeof(Dimension));
		for (i = 0; i < df->nd; ++i) {
			df->dimensions[i].dimid = i;
			nc_inq_dim(df->ncid, i, line, &df->dimensions[i].size);
			df->dimensions[i].name = (char *) malloc(sizeof(char) * (strlen(
					line) + 1));
			strcpy(df->dimensions[i].name, line);
		}

		/* Is there an unlimited dimension ? */
		nc_inq_unlimdim(df->ncid, &unlimited);
	} else {
		warn("No dimensions were specified in this netCDF file.\n");
		return;
	}

	/* Query the netCDF file for the total number of variables,
	 * and allocate the array */
	nc_inq_nvars(df->ncid, &df->nv);
	if (df->nv > 0) {
		df->variables = (Variable *) malloc((size_t)df->nv * sizeof(Variable));
		memset(df->variables, 0, (size_t)df->nv * sizeof(Variable));
		for (i = 0; i < df->nv; ++i) {
			Variable *v = &df->variables[i];

			/* Variable type and netCDF id - default to data */
			df->variables[i].type = VT_DATA;
			v->varid = i;
			v->dim_as_record = 0;

			/* Variable name */
			nc_inq_varname(df->ncid, v->varid, line);
			v->name = (char *) malloc(sizeof(char*) * (strlen(line) + 1));
			strcpy(v->name, line);

			/* Extract the dimension information for each variable */
			nc_inq_varndims(df->ncid, v->varid, &v->nd);
			v->dimids = (int *) malloc((size_t)v->nd * sizeof(int));
			nc_inq_vardimid(df->ncid, v->varid, v->dimids);

			/* Loop through all of the attributes. Take special care to
			 * with the long_name, units, missing, _FillValue, and
			 * coord type.  Coordinates are checked later. */
			nc_inq_varnatts(df->ncid, v->varid, &v->na);
			if (v->na > 0) {
				v->attributes = (Attribute*) malloc((size_t)v->na * sizeof(Attribute));
				memset(v->attributes, 0, (size_t)v->na * sizeof(Attribute));
				for (j = 0; j < v->na; ++j) {
					Attribute *a = &v->attributes[j];

					df_netcdf_read_attrib(df, v->varid, j, a);

					/* Check for known attribute types */
					if (strcasecmp(a->name, "long_name") == 0)
						v->longname = ATT_TEXT(a);

					else if (strcasecmp(a->name, "units") == 0)
						v->units = ATT_TEXT(a);

					else if (strcasecmp(a->name, "missing_value") == 0)
						v->missing = ATT_DOUBLE(a, 0);

					else if (strcasecmp(a->name, "_FillValue") == 0)
						v->fillvalue = ATT_DOUBLE(a, 0);

					else if (strcasecmp(a->name, "coordinate_type") == 0) {
						/* Decode the coordinate types */
						df_decode_coord_type(ATT_TEXT(a), &v->type, &v->coord_domain);
					}
				}
			}

		}

		/* Read the global attributes */
		nc_inq_natts(df->ncid, &df->na);
		if (df->na > 0) {
			df->attributes = (Attribute*) malloc((size_t)df->na * sizeof(Attribute));
			memset(df->attributes, 0, (size_t)df->na * sizeof(Attribute));
			for (j = 0; j < df->na; ++j)
				df_netcdf_read_attrib(df, NC_GLOBAL, j, &df->attributes[j]);
		}

		/* Search for a variable that has the same name as the unlimited
		 * dimension name. This will become the coordinate dimension.
		 */
		for (i = 0; i < df->nv; ++i) {
			Variable *v = &df->variables[i];

			if ((v->nd == 1) && (v->dimids[0] == unlimited)) {
				if (strcasecmp(v->name, df->dimensions[v->dimids[0]].name) == 0) {
					dfSetRecord(df, i);
					break;
				}
			}
		}

		/* Sweep back trough the variables and decode the associated
		 * coordinates information if the attribute is present. This
		 * has to be left until the end to ensure all variables were
		 * read. */
		for (i = 0; i < df->nv; ++i) {
			Variable *v = &df->variables[i];
			char *text = NULL;

			for (j = 0; j < v->na; ++j) {
				Attribute *a = &v->attributes[j];
				if (strcasecmp(a->name, "coordinates") == 0) {
					text = ATT_TEXT(a);
				}
			}

			df_decode_coords(df, v, text);
		}
	} else {
		warn("No variables were specified in this netCDF file.\n");
		return;
	}

}

/*
 * Free up the memory associated with a netcdf data file.
 */
void df_netcdf_free(Datafile *df) {
	nc_close(df->ncid);

}

void trim(char *s)
{
	// Trim spaces and tabs from beginning:
	int i=0,j;

	if(s == NULL)
		return;
	if(strlen(s) <= 0)
		return;

	while((s[i]==' ')||(s[i]=='\t') ||(s[i]=='\n')) {
		i++;
	}
	if(i>0) {
		for(j=0;j<(int)strlen(s);j++) {
			s[j]=s[j+i];
		}
		s[j]='\0';
	}

	if(s == NULL)
		return;
	if(strlen(s) <= 0)
		return;

	// Trim spaces and tabs from end:
	i=(int)strlen(s)-1;

	while((s[i]==' ')||(s[i]=='\t') || (s[i]=='\n')|| (s[i]=='\r')) {
		i--;
	}
	if(i<((int)strlen(s)-1)) {
		s[i + 1]='\0';
	}
}



/*
 Routine to read column data from an ascii file

 Arguments:
 fp      -    FILE pointer for ascii input file
 df      -    pointer to data file structure (assumed
 not previously initialised)

 This routine calls quit() if anything goes wrong
 */
void df_ascii_read(FILE *fp, Datafile *df) {
	int n;
	int i;
	char line[MAXLINELEN];
	char *str[MAXLINELEN];
	char key[MAXLINELEN];

	df->type = DFT_ASCII;

	/* Set the dimensions there can only be 1 */
	df->dimensions = (Dimension*) malloc(sizeof(Dimension));
	memset(df->dimensions, 0, sizeof(Dimension));
	df->dimensions[0].name = (char *) malloc(sizeof(char) * MAXLINELEN);
	strcpy(df->dimensions[0].name, "row");

	/* Get number of variables, allocate memory and zero the array */
	set_keyprm_errfn(quit);
	skipToKeyEnd(fp, "## COLUMNS");
	fscanf(fp, "%d", &n);

	df->variables = (Variable*) malloc((size_t)n * sizeof(Variable));
	memset(df->variables, 0, (size_t)n * sizeof(Variable));

	/* Populate the arrays with meta data */
	if (n < 1)
		quit("df_ascii_read: No variables!\n");

	/* Read variable info */
	for (i = 0; i < n; i++) {

		/* Variable type - default to data */
		df->variables[i].type = VT_DATA;

		/* Variable name */
		snprintf(key, sizeof(key), "## COLUMN%d.name", i + 1);
		readkeyprm_s(fp, key, line);
		trim(line);
		if ((df->variables[i].name = (char *) malloc(strlen(line) + 1)) == NULL)
			quit("df_ascii_read: No memory for variable names\n");
		strcpy(df->variables[i].name, line);
		set_keyprm_errfn(warn);

		/* Variable long name */
		snprintf(key, sizeof(key), "## COLUMN%d.long_name", i + 1);
		readkeyprm_s(fp, key, line);
		if ((df->variables[i].longname = (char *) malloc(strlen(line) + 1))
				== NULL)
			quit("df_ascii_read: No memory for long variable names\n");
		strcpy(df->variables[i].longname, line);

		/* Variable units */
		snprintf(key, sizeof(key), "## COLUMN%d.units", i + 1);
		readkeyprm_s(fp, key, line);
		if ((df->variables[i].units = (char *) malloc(strlen(line) + 1))
				== NULL)
			quit("df_ascii_read: No memory for variable units\n");
		trim(line);
		strcpy(df->variables[i].units, line);

		/* Missing values */
		snprintf(key, sizeof(key), "## COLUMN%d.missing_value", i + 1);
		readkeyprm_d(fp, key, &df->variables[i].missing);

		/* Fill value values */
		/*snprintf(key, sizeof(key),"## COLUMN%d.fill_value",i+1);
		 readkeyprm_d(fp,key,&df->variables[i].fillvalue);*/

		set_keyprm_errfn(quit);
	}

	/* Store number of variables and pointer to record units */
	df->nv = n;
	df->ri = -1;

	/* Count number of data lines */
	fseek(fp, 0L, 0);
	for (n = 0; nextline(line, MAXLINELEN, fp); n++)
		/* loop */;
	if (n < 1)
		quit("df_ascii_read: No data in file\n");
	df->dimensions[0].size = (size_t)n;

	/* Allocate data memory and store pointer to record value (if applic) */
	for (i = 0; i < df->nv; ++i) {
		df->variables[i].nd = 1;
		df->variables[i].dimids = malloc(sizeof(int) * (size_t)df->variables[i].nd);
		df->variables[i].dimids[0] = 0;
		df->variables[i].start_record = 0;
		df->variables[i].nrecords = 1;
		df->variables[i].dim_as_record = 0;
		df->variables[i].data = (double*) malloc(df->dimensions[0].size
				* sizeof(double));
	}

	/* Read data */
	fseek(fp, 0L, 0);
	for (n = 0; nextline(line, MAXLINELEN, fp); n++) {
		/* Check for correct number of strings on each line */
		if (parseline(line, str, MAXLINELEN) != df->nv)
			quit("df_ascii_read: Wrong number of data values on line %d in file %s\n", n, df->name);
		/* Loop over each value */
		for (i = 0; i < df->nv; i++) {
			if (sscanf(str[i], "%lf", &df->variables[i].data[n]) != 1)
				quit(
						"df_ascii_read: Can't read data value for column %d, line %d\n",
						i + 1, n);
		}
	}

	fclose(fp);
}

void df_ascii_write(FILE *fp, Datafile *df) {
	int i, n;

	/* Write variable info */
	fprintf(fp, "## COLUMNS %d\n##\n", df->nv);
	for (i = 0; i < df->nv; i++) {
		fprintf(fp, "## COLUMN%d.name  %s\n", i + 1, df->variables[i].name);
		fprintf(fp, "## COLUMN%d.long_name  %s\n", i + 1,
				df->variables[i].longname);
		fprintf(fp, "## COLUMN%d.units  %s\n", i + 1, df->variables[i].units);
		fprintf(fp, "## COLUMN%d.missing_value  %g\n", i + 1,
				df->variables[i].missing);
		/*fprintf(fp,"## COLUMN%d.fill_value  %g\n",i+1,df->variables[i].fillvalue);*/
		fprintf(fp, "##\n");
	}

	/* Write values */
	for (i = 0; i < df->nrecords; i++) {
		for (n = 0; n < df->nv; n++) {
			fprintf(fp, "%.10g", df->variables[n].data[i]);
			if (n < df->nv - 1)
				fprintf(fp, " ");
		}
		fprintf(fp, "\n");
	}
}

/*
 * Free up the memory associated with an ascii column data file.
 */
void df_ascii_free(Datafile *df) {
}

/*
 Routine to read an attribute from the netCDF file.

 Arguments:
 df          -    pointer to data file structure
 varid       -    Variable id or NC_GLOBAL.
 attnum      -    Attribute number.
 att         -    Attribute structure to be populated.

 This routine calls quit() if anything goes wrong
 */
void df_netcdf_read_attrib(Datafile *df, int varid, int attnum, Attribute *a) {
	char line[MAXLINELEN];
	nc_type type;

	/* Read attribute name */
	nc_inq_attname(df->ncid, varid, attnum, line);
	a->name = (char *) malloc((sizeof(line) + 1) * sizeof(char));
	strcpy(a->name, line);

	/* Get the attribute type and length */
	nc_inq_att(df->ncid, varid, a->name, &type, (size_t *) &a->n);
	if (type == NC_CHAR) {
		a->value = (void*) malloc(sizeof(char) * (size_t)(a->n + 1));
		memset(a->value, 0, sizeof(char) * (size_t)(a->n + 1));
		nc_get_att_text(df->ncid, varid, a->name, (char*) a->value);
		a->type = AT_TEXT;
	}

	else if (a->n > 0) {
		switch (type) {
		case NC_BYTE:
			a->value = (void*) malloc(sizeof(char) * (size_t)a->n);
			nc_get_att_schar(df->ncid, varid, a->name, (signed char*) a->value);
			a->type = AT_BYTE;
			break;

		case NC_FLOAT:
			a->value = (void*) malloc(sizeof(float) * (size_t)a->n);
			nc_get_att_float(df->ncid, varid, a->name, (float*) a->value);
			a->type = AT_FLOAT;
			break;

		case NC_DOUBLE:
//			a->value = (void*) malloc(sizeof(double) * a->n);
//			nc_get_att_double(df->ncid, varid, a->name, (double*) a->value);
			a->value = (void*) malloc(sizeof(double) *(size_t)a->n);
			nc_get_att_double(df->ncid, varid, a->name, (double*) a->value);
			a->type = AT_DOUBLE;
			break;

		case NC_SHORT:
			a->value = (void*) malloc(sizeof(short) * (size_t)a->n);
			nc_get_att_short(df->ncid, varid, a->name, (short*) a->value);
			a->type = AT_SHORT;
			break;

		case NC_INT:
			a->value = (void*) malloc(sizeof(int) * (size_t)a->n);
			nc_get_att_int(df->ncid, varid, a->name, (int*) a->value);
			a->type = AT_INT;
			break;

		default:
			break;
		}
	}
}

/* Allocate a data record. */
void df_alloc_data_record(Datafile *df, Variable *v, int r) {
	int e1 = 0;
	int e2 = 0;
	int e3 = 0;
	int e4 = 0;

	switch (v->nd) {
	case 0:
		break;

	case 1:
		e1 = (int)df->dimensions[v->dimids[0]].size;
		VAR_1D(v)[r] = alloc1d(e1);
		break;

	case 2:
		e1 = (int)df->dimensions[v->dimids[0]].size;
		e2 = (int)df->dimensions[v->dimids[1]].size;
		VAR_2D(v)[r] = alloc2d(e2, e1);
		break;

	case 3:
		e1 = (int)df->dimensions[v->dimids[0]].size;
		e2 = (int)df->dimensions[v->dimids[1]].size;
		e3 = (int)df->dimensions[v->dimids[2]].size;
		VAR_3D(v)[r] = alloc3d(e3, e2, e1);
		break;

	case 4:
		e1 = (int)df->dimensions[v->dimids[0]].size;
		e2 = (int)df->dimensions[v->dimids[1]].size;
		e3 = (int)df->dimensions[v->dimids[2]].size;
		e4 = (int)df->dimensions[v->dimids[3]].size;
		VAR_4D(v)[r] = alloc4d(e4, e3, e2, e1);
		break;

	default:
		quit(
				"df_alloc_data_record: Unable to allocate memory for '%d' dimensional array.\n",
				v->nd);
		break;
	}
}

double* df_alloc_data(Datafile *df, Variable *v) {
	double *data = 0;

	switch (v->nd) {
	case 0:
		data = (double*) malloc(sizeof(double) * (size_t)v->nrecords);
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		data = (double*) malloc(sizeof(double *) * (size_t)v->nrecords);
		break;

	default:
		quit(
				"df_alloc_data: Unable to allocate memory for '%d' dimensional array.\n",
				v->nd);
		break;
	}

	return data;
}

void df_alloc_data_records(Datafile *df, Variable *v, int nrecs) {
	int i = 0;

	v->nrecords = nrecs;
	v->data = df_alloc_data(df, v);
	for (i = 0; i < nrecs; ++i)
		df_alloc_data_record(df, v, i);
}

void df_free_data_record(Datafile *df, Variable *v, int r) {

	if (v->data != NULL) {
		switch (v->nd) {
		case 0:
			break;

		case 1:
			if (VAR_1D(v)[r] != NULL)
				free1d(VAR_1D(v)[r]);
			VAR_1D(v)[r] = NULL;
			break;

		case 2:
			if (VAR_2D(v)[r] != NULL)
				free2d(VAR_2D(v)[r]);
			VAR_2D(v)[r] = NULL;
			break;

		case 3:
			if (VAR_3D(v)[r] != NULL)
				free3d(VAR_3D(v)[r]);
			VAR_3D(v)[r] = NULL;
			break;

		case 4:
			if (VAR_4D(v)[r] != NULL)
				free4d(VAR_4D(v)[r]);
			VAR_4D(v)[r] = NULL;
			break;
		}
	}
}

void df_free_data_records(Datafile *df, Variable *v) {
	int i;

	if (v->data != NULL) {
		for (i = 0; i < v->nrecords; ++i)
			df_free_data_record(df, v, i);
	}

	if (v->data != NULL)
		free(v->data);

	v->data = NULL;
	v->nrecords = 0;
	v->start_record = 0;
}

/* Read a data record into the specified variable from a netCDF
 * file. It is assumed that the memory has already been allocated.
 */
void df_read_data(Datafile *df, Variable *v, int r) {
	size_t start[5];
	size_t count[5];
	int i = 0;
	int fid = df->ncid;
	int index = r - v->start_record;

	if ((r < 0) || (r >= df->nrecords))
		quit(
				"df_read_data: Attempt to read an invalid record for variable '%s'.\n",
				v->name);

	if (v->dim_as_record) {
		start[i] = (size_t)r;
		count[i++] = 1;
	}

	switch (v->nd) {
	case 0:
		nc_get_vara_double(fid, v->varid, start, count, &v->data[index]);
		break;

	case 1:
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[0]].size;
		nc_get_vara_double(fid, v->varid, start, count, VAR_1D(v)[index]);
		break;

	case 2:
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[0]].size;
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[1]].size;
		nc_get_vara_double(fid, v->varid, start, count, VAR_2D(v)[index][0]);
		break;

	case 3:
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[0]].size;
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[1]].size;
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[2]].size;
		nc_get_vara_double(fid, v->varid, start, count, VAR_3D(v)[index][0][0]);
		break;

	case 4:
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[0]].size;
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[1]].size;
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[2]].size;
		start[i] = 0;
		count[i++] = df->dimensions[v->dimids[3]].size;
		nc_get_vara_double(fid, v->varid, start, count, VAR_4D(v)[index][0][0][0]);
		break;

	default:
		quit("df_read_data: Bad number of dimensions\n");
		break;
	}
}

/* Shift the data to the left by nrec places */
void df_left_shift_data(Datafile *df, Variable *v, int nrecs) {
	int i;
	double *newdata = df_alloc_data(df, v);

	for (i = 0; i < v->nrecords; ++i) {
		int j = (i - nrecs + v->nrecords) % v->nrecords;

		/* Copy the data */
		switch (v->nd) {
		case 0:
			newdata[j] = v->data[i];
			break;

		case 1:
			((double**) newdata)[j] = VAR_1D(v)[i];
			break;

		case 2:
			((double***) newdata)[j] = VAR_2D(v)[i];
			break;

		case 3:
			((double****) newdata)[j] = VAR_3D(v)[i];
			break;

		case 4:
			((double*****) newdata)[j] = VAR_4D(v)[i];
			break;
		}
	}

	free(v->data);
	v->data = newdata;
}

/* Shift the data to the right by nrec places */
void df_right_shift_data(Datafile *df, Variable *v, int nrecs) {
	int i;
	double *newdata = df_alloc_data(df, v);

	for (i = 0; i < v->nrecords; ++i) {
		int j = (i + nrecs + v->nrecords) % v->nrecords;

		/* Copy the data */
		switch (v->nd) {
		case 0:
			newdata[j] = v->data[i];
			break;

		case 1:
			((double**) newdata)[j] = VAR_1D(v)[i];
			break;

		case 2:
			((double***) newdata)[j] = VAR_2D(v)[i];
			break;

		case 3:
			((double****) newdata)[j] = VAR_3D(v)[i];
			break;

		case 4:
			((double*****) newdata)[j] = VAR_4D(v)[i];
			break;
		}
	}

	free(v->data);
	v->data = newdata;
}

/* Shift the data record along. A negative number of records implies
 * a shift to the left, a positive to the right.
 */
void df_shift_data(Datafile *df, Variable *v, int nrecs) {
	int i = 0;
	int j = 0;
	int src_start = 0;
	int src_end = 0;
	int dst_start = 0;
	int dir = (nrecs < 0) ? 1 : -1;
	int nstart = v->start_record;

	nrecs = (int) abs(nrecs);  // This was fabs but it was tripping a compile warning as nrecs is an int so chaned it to abs
	/* Only scroll if necessary */
	if ((nrecs == 0) || (!v->dim_as_record) || (v->data == NULL))
		return;

	/* Adjust so we don't scroll past the ends. */
	nstart += dir * nrecs;
	if (nstart < 0) {
		nrecs += nstart;
		nstart = 0;
	} else if ((nstart + v->nrecords) > df->nrecords) {
		nrecs -= ((int)df->nrecords - (nstart + (int)v->nrecords));
		nstart = (int)df->nrecords - (int)v->nrecords - 1;
	}

	if (nrecs < v->nrecords) {
		if (dir > 0) {
			src_start = nrecs;
			src_end = v->nrecords - 1;
			dst_start = 0;
		} else {
			src_start = v->nrecords - nrecs - 1;
			src_end = 0;
			dst_start = v->nrecords - 1;
		}

		for (i = src_start, j = dst_start; (dir < 0) ? (i >= src_end) : (i <= src_end); i += dir, j += dir) {

			/* Swap the values or addresses */
			switch (v->nd) {
			case 0: {
				double tmp = v->data[j];
				v->data[j] = v->data[i];
				v->data[i] = tmp;
			}
				break;

			case 1: {
				double *tmp = VAR_1D(v)[j];
				VAR_1D(v)[j] = VAR_1D(v)[i];
				VAR_1D(v)[i] = tmp;
			}
				break;

			case 2: {
				double **tmp = VAR_2D(v)[j];
				VAR_2D(v)[j] = VAR_2D(v)[i];
				VAR_2D(v)[i] = tmp;
			}
				break;

			case 3: {
				double ***tmp = VAR_3D(v)[j];
				VAR_3D(v)[j] = VAR_3D(v)[i];
				VAR_3D(v)[i] = tmp;
			}
				break;

			case 4: {
				double ****tmp = VAR_4D(v)[j];
				VAR_4D(v)[j] = VAR_4D(v)[i];
				VAR_4D(v)[i] = tmp;
			}
				break;
			}
		}
	} else
		quit("df_shift_data: Attempt to shift more records that in buffer.\n");

	v->start_record = nstart;
}


/**
 * Open the input file with the name given in the given input folder.
 *
 */
FILE *Open_Input_File(const char *inputFolder, const char *name, const char *mode) {
	char fileName[BMSLEN];
	//char newFileName[BMSLEN];

	snprintf(fileName, sizeof(fileName), "%s%s", inputFolder, name);

	if (strstr(fileName, ".nc") != NULL) {
		return fopen(fileName, mode);

	} else {

		//if (Convert_File_Format(fileName, newFileName, sizeof(newFileName)) < 0) {
		//	quit("Cannot open Atlantis input file %s. \n", fileName);
		//	return NULL;
		//}
		return fopen(fileName, mode);
		//return fopen(newFileName, mode);
	}
}

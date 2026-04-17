/*********************************************************************

    File:           datafile.h
    
    Created:        Wed Jun 18 13:30:31 EST 1997
    
    Author:         Jason Waring/Stephen Walker
                    CSIRO Division of Marine Research
    
    Purpose:        Include file for routines which deal with
		    data files that have coordinate variable data.
    
    Revisions:      none

    $Id: datafile.h 3369 2012-08-29 06:16:46Z gor171 $

*********************************************************************/

#ifndef _DATAFILE_H
#define _DATAFILE_H

/* Enumerations */
typedef enum { DFT_ASCII, DFT_NETCDF } DataFileType;
typedef enum { AT_TEXT, AT_BYTE, AT_FLOAT, AT_DOUBLE, AT_SHORT, AT_INT }
		AttributeType;

/* Defines the set of variables understood by the datafile package.
 */
#define VT_UNKNOWN_COORD 0x00000001	/* Unsupported coordinate type */
#define VT_DATA		 0x00000002	/* Data var. */
#define VT_TIME		 0x00000004	/* Time information */
#define VT_X 		 0x00000008	/* X cartesian coord var. */
#define VT_Y 		 0x00000010	/* Y cartesian coord var. */
#define VT_Z 		 0x00000020	/* Z cartesian coord var. */
#define VT_LATITUDE	 0x00000040	/* Lat. geographic coord var. */
#define VT_LONGITUDE	 0x00000080	/* Long. geographic coord var. */
typedef int VariableType;

/* Define the geographic types understood by datafile */
#define GT_NONE		 0x00000001	/* No-geographic info. */
#define GT_GEOGRAPHIC	 0x00000002	/* Lat/Long. */
#define GT_PROJECTION	 0x00000004	/* Projection */
typedef int GeoType;

#define GEOGRAPHIC_TAG "geographic"

#if !defined(MAXNUMDIMS)
#define MAXNUMDIMS 10
#endif

#if !defined(MAXNUMCOORDS)
#define MAXNUMCOORDS (MAXNUMDIMS)	/* must be same as dims */
#endif

#if !defined(MAXNUMCMAPS)
#define MAXNUMCMAPS (MAXNUMCOORDS)	/* must be same as coords */
#endif



/* Structures for the supported analytic coordinate systems. */

/* Structure for rectangular grid */
typedef struct {
    double x0;     /* X origin */
    double y0;     /* Y origin */
    double dx;     /* X increment */
    double dy;     /* Y increment */
    double th;     /* rotation (from X axis, in degrees) */
    double sinth;
    double costh;
} RectGrid;

/* Structure for polar grid */
typedef struct {
    double x0;     /* X origin (m) */
    double y0;     /* Y origin (m) */
    double arc;    /* azimuthal span of grid (in degrees) */
    double rmin;   /* inside radius of polar grid (m). */
    double rotation; /* rotation (from -ve X axis, in degrees) */
} PolarGrid;

typedef struct {
    double ioffset; /* Grid offset in i direction */
    double joffset; /* Grid offset in j direction */
    int ni; /* Number grid points in i direction */
    int nj; /* Number grid points in j direction */
    double x0;     /* X origin */
    double y0;     /* Y origin */
    double dx;     /* X increment */
    double dy;     /* Y increment */
    double th;     /* rotation (from X axis, in degrees) */
    double sinth;
    double costh;
} RectGridInfo;


typedef struct {
    double ioffset; /* Grid offset in i direction */
    double joffset; /* Grid offset in j direction */
    int ni; /* Number grid points in i direction */
    int nj; /* Number grid points in j direction */
    double x0;     /* X origin (m) */
    double y0;     /* Y origin (m) */
    double arc;    /* azimuthal span of grid (in degrees) */
    double rmin;   /* inside radius of polar grid (m). */
    double rotation; /* rotation (from +ve X axis, in degrees) */
    double fac;    /* Scale factor */
    double dth;    /* delta theta */
    double logrmin; /* log of rmin */
} PolarGridInfo;



typedef struct CoordinateSystem CoordinateSystem;
typedef struct CoordinateMapping CoordinateMapping;
typedef struct Attribute Attribute;
typedef struct Dimension Dimension;
typedef struct Variable Variable;
typedef struct Datafile Datafile;


/* An attribute contains name information, type, and number.
 */
#define CHK_TYPE(a,t) ((a)->type == (t))
#define CHK_TYPE_RANGE(a, t, i) (((a)->type == (t)) && ((i) >= 0) && ((i) < (a)->n))
#define ATT_TEXT(a) ((char*)(CHK_TYPE(a, AT_TEXT) ? (a)->value: 0))
#define ATT_BYTE(a,i) (CHK_TYPE_RANGE(a,AT_BYTE,i) ? ((char*)((a)->value))[(i)] : 0)
#define ATT_FLOAT(a,i) (CHK_TYPE_RANGE(a,AT_FLOAT,i) ? ((float*)((a)->value))[(i)] : 0.0)
//#define ATT_DOUBLE(a,i) (CHK_TYPE_RANGE(a,AT_DOUBLE,i) ? ((double*)((a)->value))[(i)] : 0.0)
#define ATT_DOUBLE(a,i) (CHK_TYPE_RANGE(a,AT_DOUBLE,i) ? ((double*)((a)->value))[(i)] : 0.0)
#define ATT_INT(a,i) (CHK_TYPE_RANGE(a,AT_INT,i) ? ((int*)((a)->value))[(i)] : 0)
#define ATT_SHORT(a,i) (CHK_TYPE_RANGE(a,AT_SHORT,i) ? ((short*)((a)->value))[(i)] : 0)

struct Attribute {
   char *name;		/* Attribute name */
   //size_t  n;		/* Number of values for this attribute */
   int n;
   AttributeType type;  /* Attribute type */
   void *value;		/* Attribute value(s) */
};



/* A dimension contains a name and size.
 */
struct Dimension {
   char *name;		/* Dimension name */
   size_t size;		/* Size of dimension */

   int  dimid;		/* netCDF dim id */
};

/* A variable can either be a data variable or a coordinate variable.
 */
//#define VAR_0D(v) ((double*)((v)->data))
//#define VAR_1D(v) ((double**)((v)->data))
//#define VAR_2D(v) ((double***)((v)->data))
//#define VAR_3D(v) ((double****)((v)->data))
//#define VAR_4D(v) ((double*****)((v)->data))
/*
#define VAR_5D(v) ((double******)((v)->data))
#define VAR_6D(v) ((double*******)((v)->data))
*/

#define VAR_0D(v) ((doubleINPUT*)((v)->data))
#define VAR_1D(v) ((doubleINPUT**)((v)->data))
#define VAR_2D(v) ((doubleINPUT***)((v)->data))
#define VAR_3D(v) ((doubleINPUT****)((v)->data))
#define VAR_4D(v) ((doubleINPUT*****)((v)->data))



struct Variable {

    VariableType type;		/* Enumerated variable type */

    /* Primary information */
    char *name;			/* Variable name */
    char *longname;		/* Long name */
    char *units;		/* Units */
    double missing;		/* Missing value */
    double fillvalue;		/* Fill value */
    int na;			/* Number of attributes */
    Attribute *attributes;	/* Unsupported attributes */

    /* Dimensional information */
    int  nd;			/* Number of dimensions */
    int *dimids;		/* Dimensions */
    int dim_as_record;		/* If non-zero then dims include record dim */

    /* Data buffer - Always double but maybe multi-dimensional,
     * so casting maybe required. */
    int  start_record;		/* Record number corresponding to start of
				 * data array.  */
    int  nrecords;		/* Number of records in record buffer */
    double *data;		/* Array of data values (records by
				 * array dims) */

    /* Coordinate specific information */
    char *coord_domain;		/* Defines the coordinate domain */
				/* e.g. projection=UTM(55), spheroid=WGS84 */
    int nc;			/* Number of coordinates */
    int *coordids;		/* List of coord ids associated with
				 * variable. NULL if none */
    CoordinateSystem *csystem;	/* Coordinate systems associated with
				 * this variable. NULL if none */
    int	nncd;			/* Number of non-coordinate dimensions */
    int *ncdimids;		/* The number of non-coordinate dimension ids */

    /* netCDF identifiers - used for speed ups */
    int varid;			/* Identfiers for var of netCDF file */

};


/* Defines data and methods for transforming a coordinate coordinate
 * variables.
 */
struct CoordinateTransform {
   void *private_data;	/* Transformation data. */
   void (*forward)(void *data, int n, double from[], double to[]);
   void (*inverse)(void *data, int n, double from[], double to[]);
   void (*free)(void *data);
};

/** Geographic transformation data.
  *
  * Transforms that data from the internal 'file' projection to
  * an external 'user' projection.
  */
struct GeoTransformData {
    int int_geotype;	        /* The internal data type the file data should
				 * be interpreted as. */
    MapProjection *int_mp;      /* Projection (if required) of internal data */
    int ext_geotype;	        /* The external data the data should be
				 * tranformed into. */
    MapProjection *ext_mp;      /* Projection (if required) of external data */
};


/* A coordinate map allows the definition of coordinate groupings
 * and the methods by which the mapping to and from indice
 * space occurs.
 */
struct CoordinateMapping {
    int nc;			/* Number of coordinates in map */
    int *coordids;		/* Coordinate ids */
    VariableType *coordtypes;	/* Coordinate type */
    struct CoordinateTransform *transform; /* Transforming coordinates */

    /* This is private to the dfSetCoordSystem, do not set these. */
    void (*free)(Datafile *df,	/* Deallocate memory assoc with mapping */
		CoordinateMapping *cm);
    int (*coords_to_indices)(	/* Function to convert from coords to indices */
		Datafile * df,
		CoordinateMapping *cm,
		const int depindices[],   /* Dependent indices */
		const double coords[],	   /* Coordinate values */
		double indices[]);	   /* Coordinate indices */
    int (*indices_to_coords)(	/* Function to convert from indices to coords */
		Datafile * df,
		CoordinateMapping *cm,
		const int depindices[], /* Dependent indices */
		const double indices[],
		double coords[]);	   /* Coordinate values */
    void *special_data;		/* Special data which might be used by
			         * the transformation functions */
    int  nd;			/* Number of dimensions these coords
			         * define */
    int  *dimids;		/* Dimension ids of defined coords */
    int  ndepd;			/* Number of dependent dimensions without
				 * which the coordinate values cannot be
			         * evaluated. */
    int  *depdimids;		/* Dimension ids of dependent dimensions */
};


/* A group of coordinates defines a coordinate system.
 * To convert between indice space and coordinate space
 * may be non-unique for a particular coordinate, and it
 * may be necessary to group coordinates (e.g. XY to IJ).
 * These coordinate mappings are defines as CoordinateMappings.
 *
 * Example:
 * If we have four variable T(time), X, Y, Z, and each is defined
 * using with the dimensions:
 *
 *   T(nrecords)
 *   X(ni, nj)
 *   Y(ni, nj)
 *   Z(ni, nj, nk)
 *
 * then we would have three coordinatate mappings.
 * 1 - Maps the time to record and record to time.
 * 2 - Maps XY to IJ and IJ to XY.
 * 3 - Maps Z to K knowing IJ from the XY to IJ calculation.
 *
 * In this case the simplist case is evaluated first (T),
 * followed by XY and then Z which relies on the ni and nj
 * (or X and Y) to compute Z (or K). This internal dependency
 * is critical to efficiently evaluating coordinates.
 */
struct CoordinateSystem {
    int nc;			/* Number of coordinates */
    int *coordids;		/* Array of coordinate ids */
    int *coordtypes;		/* Array of coordinate types */
    int ncm;			/* Number of coordinate mappings */
    CoordinateMapping *cmaps;	/* Coordinate maps. */
    int *cmorder;		/* Defines the order in which the
				 * coordinate should be evaluated. */
};



/* Data file data. The principle information is contained in the
 * record (if appropriate), but the data variables are stored separately.
 */
struct Datafile {
    char *name;         /* Data file name */
    DataFileType type;	/* Data file type */
    long nrecords;      /* Number of records */
    int ri;             /* netCDF index of record variable */
    char *rec_name;     /* Record name */
    char *rec_units;    /* Record units string */
    char *rec_longname; /* Record long name */
    int geotype;	/* Geographic transform type */
    char *projection;   /* Projection information */
    double *records;    /* record values (if appropriate) */
    int ncid;           /* netCDF file id, if appropriate */
    int nv;             /* Number of variables */
    Variable *variables;/* Variables */
    int nd;		/* Number of dimensions */
    Dimension *dimensions; /* Dimensions */
    int na;		/* Number of attributes */
    Attribute *attributes;	/* Unsupported attributes */
};


void    dfRead(char *folderName, char *name, Datafile *df);
void    dfFree(Datafile *df);
void    dfSetRecord(Datafile *df, int varid);
double  dfEval(Datafile *df, Variable *v, double r);
double  dfEvalEx(Datafile *df, Variable *v, double r);
double  dfEvalCoords(Datafile *df, Variable *v, double r, double coords[]);
int	dfGetNumVariables(Datafile *df);
Variable *dfGetVariable(Datafile *df, int varid);
Variable *dfGetVariableByName(Datafile *df, const char *name);
Dimension *dfGetDimension(Datafile *df, int dimid);
Attribute *dfGetGlobalAttribute(Datafile *df, const char *name);
Attribute *dfGetAttribute(Datafile *df, Variable *v, const char *name);
void    dfAddTextAttribute(Datafile *df, Variable *v,
			   const char *name, const char *text);
int     dfIndex(Datafile *df, const char *varname);
void    dfCheckRecords(Datafile *df);
void    dfPrintInfo(Datafile *df, FILE *fp, int level);
int     dfFindRecord(Datafile *df, double r, int *before, int *after, double *frac);
int     dfFindTime(Datafile *df, double r, int rwind, double *newt);
void    dfReadRecords(Datafile *df, Variable *v, int start_rec, int nrecs);
void    dfReadRecord(Datafile *df, Variable *v, int start_rec);
double dfGetDataValue(Datafile *df, Variable *v, int record, int *is);

int	dfSetCoordSystem(Datafile *df, Variable *v,
		 int ncr, CoordinateMapping *requests);
int     dfInferAndSetCoordSystem(Datafile *df, Variable *v);
int     dfGetNumDimensions(Datafile *df, Variable *v);
int     *dfGetDimIds(Datafile *df, Variable *v);
int     dfGetNumCoordinates(Datafile *df, Variable *v);
int     *dfGetCoordIds(Datafile *df, Variable *v);
int     *dfGetCoordTypes(Datafile *df, Variable *v);
void    dfSetGeoType(Datafile *df, char *type);
void    dfSetDefaultGeoType(char *type);
int     dfCoordsToIndices(Datafile *df, Variable *v,
                      const double coords[], double indices[]);
int     dfIndicesToCoords(Datafile *df, Variable *v,
                      const double indices[], double coords[]);


#endif /* _DATAFILE_H */

/******************************************************************//**
\file atgeomIO.c
\brief Input and output routines for general box model geometry types

	File:           atgeomIO.c

    Created:        Mon Oct 10 11:16:07 EST 1994

    Author:         Stephen Walker
                    CSIRO Division of Oceanography

    Purpose:        Input and output routines for general box model geometry types

    Arguments:      See each routine below

    Returns:        void

    Revisions:      8/8/2004 EA Fulton
					Ported across the code from the southeast (sephys) model

					17/11/2004 EA Fulton
					Converted original routine definitions from
					void
					routine_name(blah,blahblah)
					int blah;
					double blahblah;

					to

					void routine_name(int blah, double blahblah)

					14-04-2009 Bec Gorton
					Added support for projections in the input bgm file. These projections use the proj4
					projection library. The projections are used to accurately calculate the number
					of hours of sunlight for primary production light limitation.

*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sjwlib.h>
#include <atlantisboxmodel.h>
#include <atUtilLib.h>

void parseProjectionString(MSEBoxModel *bm, FILE *fp);
static int getUniqueIBox( MSEBoxModel *bm, Box *b);
static void Get_DistID(MSEBoxModel *bm);

static void Compute_Box_Widths(MSEBoxModel *bm); /* JMK */
static void CopyBGMFile(MSEBoxModel *bm, char *name){

	char str[STRLEN];

	if(strlen(bm->destFolder) > 0){
	#ifdef _WIN32
		sprintf(str, "copy %s %s", name, bm->destFolder);
	#else
		sprintf(str, "cp %s %s", name, bm->destFolder);
	#endif
		system(str);
	}
}

/*******************************************************************//**
 * \brief Read in the model box geometry.
 *
This routine reads a box model geometry
from an ascii file. It does not return if an error is encountered.
Input file format for this part is shown below, where N is an integer
and X and Y are floating point numbers.

    # Number of boxes in horizontal plane <br>
    nbox        N<br>

    # Number of faces in horizontal plane<br>
    nface       N<br>

    # Box Model polyline (should be closed)<br>
    bnd_vert    X    Y<br>
    bnd_vert    X    Y<br>
    bnd_vert    X    Y<br>
       .<br>
       .<br>
       .<br>

The boxes and faces are read using readBoxGeom() and
readFaceGeom() below. This routine assumes that the
boxes and faces are each in order in the input file
(ie, data for box0 comes before data for box1 etc). This
minimises the number of times which the file has to be rewound
and re-read.

	@param name   input file name
	@param bm     box model pointer
*********************************************************************/
void readMSEBoxModelGeom(char *name, MSEBoxModel *bm)
{
	double maxeast, maxsouth, maxwest, maxnorth, xdist, ydist;
    dpoint p;
    int i = 0;
    FILE *fp;

    /* Copy bgm file to destination folder */
    CopyBGMFile(bm, name);
    /* open the file */
    if( (fp=Open_Input_File(bm->inputFolder, name,"r")) == NULL )
		quit("readMSEBoxModelGeom: Can't open %s%s\n",bm->inputFolder, name);

    if( verbose > 1)
		fprintf(stderr,"readMSEBoxModelGeom: reading from file %s\n",name);

    /* rewind file */
    fseek(fp, 0L, SEEK_SET);

    /* Read number of boxes and faces */
    skipToKeyEnd(fp,"nbox");
    if( fscanf(fp,"%d",&bm->nbox) != 1 || bm->nbox < 1 || bm->nbox > 10000 )
		quit("readMSEBoxModelGeom: Can't read nbox\n");

    skipToKeyEnd(fp,"nface");
    if( fscanf(fp,"%d",&bm->nface) != 1 || bm->nface < 0 || bm->nface > 20000 )
		quit("readMSEBoxModelGeom: Can't read nface\n");

    /* If the lim_sum_hours flag is true then read in the projection information. */
    /* read in the projection string - use to calculate the number of sun hours in a day */

    /* Set the error function to warn so that we don't quit here if not projection is specified */
    set_keyprm_errfn(warn);
    if(skipToKeyEnd(fp,"projection") > 0){
    	parseProjectionString(bm, fp);
    }
    else{
    	bm->projection = NULL;
    }

    set_keyprm_errfn(quit);

    /* allocate space for boundary polyline */
    bm->bnd = createpolyline();

    /* read boundary polyline */
    skipToKeyStart(fp,"bnd_vert");
    while( fscanf(fp,"bnd_vert %lf %lf ",&p.x,&p.y) == 2 ){
		addtoend(bm->bnd, p);
    }


    if( bm->bnd->np < 3 )
		quit("readMSEBoxModelGeom: Only %d points in model boundary!\n",bm->bnd->np);

    if( verbose > 1)
		fprintf(stderr,"Model boundary has %ld points\n",bm->bnd->np);

	/* Read and store maximum water column depth */
    skipToKeyEnd(fp,"maxwcbotz");
    if(fscanf(fp,"%lf",&bm->maxwcbotz) != 1 || bm->maxwcbotz >= 0.0 )
		quit("readBoxGeom: Can't read maxwcbotz\n");

    /* allocate box and face arrays */
    bm->boxes = (Box *)malloc((size_t)bm->nbox*sizeof(Box));
    bm->is_boundary = (int *)i_alloc1d(bm->nbox);

    if( bm->boxes==NULL )
		quit("readMSEBoxModelGeom: No memory for box array\n");

    if( bm->nface > 0 ) {
		bm->faces = (Face *)malloc((size_t)bm->nface*sizeof(Face));
		if( bm->faces==NULL )
			quit("readMSEBoxModelGeom: No memory for face array\n");
    }
    else{
		bm->faces = NULL;
	}

    /* Set up boxes - note external numbering starts from 0 */
	bm->nboundary = 0;
    bm->nland = 0;
    for(i=0; i<bm->nbox; i++) {
		bm->boxes[i].n = i;
		readBoxGeom(fp,name, &bm->boxes[i]);
	    getUniqueIBox(bm, &bm->boxes[i]);

		/* Check to see if we have land */
		// if(!bm->terrestrial_on && bm->boxes[i].botz > 0.0 )
	//	    	quit("readBoxGeom: Box %d has a botz of greater than 0 which indicates land but you do not have the land mode turned on (flagAllowLand = 0 in yout run.prm file)\n", i);

		/* Decide on box type by checking whether point inside
		* box is also inside the model boundary
		*/
        bm->is_boundary[i] = 0;
		if( pointinpoly(bm->boxes[i].inside, *bm->bnd) ) {
			bm->boxes[i].type = INTERNALBOX;
		}
		else {
			bm->boxes[i].type = BOUNDARY;
			bm->nboundary++;
			if( verbose > 1)
				fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box\n",i,bm->boxes[i].label);
            bm->is_boundary[i] = 1;
		}

		if( bm->boxes[i].botz == 0.0 ){
			bm->boxes[i].type = BOUNDARY;
			bm->nboundary++;
			if( verbose > 1)
				fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box as it has 0.0 depth\n",i,bm->boxes[i].label);
            bm->is_boundary[i] = 1;
		}

		if( bm->boxes[i].botz > 0.0){
			if(bm->terrestrial_on == TRUE){
				bm->boxes[i].type = LAND;
                bm->nland++;
				if( verbose > 1)
					fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a land box as it has %f depth\n",i,bm->boxes[i].label, bm->boxes[i].botz);
			} else {
				bm->boxes[i].type = BOUNDARY;
				bm->nboundary++;
				if( verbose > 1)
					fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box as it has 0.0 depth\n",i,bm->boxes[i].label);
                bm->is_boundary[i] = 1;
			}
		}
	}

	/* Check box 0 is a boundary box and box 1 isn't */
	if(bm->boxes[0].type != BOUNDARY)
		quit("Box 0 must be a BOUNDARY box, yours is not so please redo the bgm file\n");

	if(bm->boxes[1].type != INTERNALBOX)
		quit("Box 1 must be an INTERNAL box, yours is not so please redo the bgm file\n");


    printf("%d of %d boxes are boundary boxes\n", bm->nboundary, bm->nbox);

    /* Set up faces */
    fseek(fp, 0L, SEEK_SET);
    for(i=0; i<bm->nface; i++) {
		bm->faces[i].n = i;
		readFaceGeom(fp,&bm->faces[i]);
    }

    /* Close the file */
    fclose(fp);
	


	/* Determine maximum width of model domain  - first get lower left and upper right
	corners and then get the distance between them */
	maxeast = MAXDOUBLE;
	maxsouth = MAXDOUBLE;
	maxwest = -MAXDOUBLE;
	maxnorth = -MAXDOUBLE;
    for(i=0; i<bm->nbox; i++){
		if(bm->boxes[i].inside.x < maxeast)
			maxeast = bm->boxes[i].inside.x;
		if(bm->boxes[i].inside.y < maxsouth)
			maxsouth = bm->boxes[i].inside.y;
		if(bm->boxes[i].inside.x > maxwest)
			maxwest = bm->boxes[i].inside.x;
		if(bm->boxes[i].inside.y > maxnorth)
			maxnorth = bm->boxes[i].inside.y;
	}
    
	xdist = maxwest - maxeast;
	ydist = maxnorth - maxsouth;
	bm->width = sqrt(xdist * xdist + ydist * ydist);
    
    if(verbose){
        printf("Got model width, now do Get_DistID\n");
    }

    /* Get ordered list of boxes */
    Compute_Box_Widths(bm);
	Get_DistID(bm);
    return;
}



/**//**
 /brief Free the memory associated with the geometry.
 ****/
void freeMSEBoxModelGeom(MSEBoxModel *bm)
{
	int i = 0;

	/**** Free the faces and the boundary polylines of each box, and
	**** the box and face memory.
	****/
	for(i=0; i<bm->nbox; ++i) {
	   clearpolyline(bm->boxes[i].bnd);
	   free(bm->boxes[i].bnd);
	   free(bm->boxes[i].iface);
	   free(bm->boxes[i].ibox);
	}

	free(bm->faces);
	free(bm->boxes);

	/**** Free the bounary polyline memory.
	****/
	clearpolyline(bm->bnd);
	free(bm->bnd);

	/* Free up the projection */
	if(bm->projection != NULL)
	    mpFree(bm->projection);
}


/*******************************************************************//**
This routine writes a box model geometry to an ascii file.
Output file format for this part is as shown in the comments
for readMSEBoxModelGeom above.

The boxes and faces are written using writeBoxGeom() and
writeFaceGeom() below.

    Arguments:     fp   output file pointer
		   bm    box model pointer
*********************************************************************/
void writeMSEBoxModelGeom(FILE *fp, MSEBoxModel *bm)
{
    int i = 0;
    int writecomments;
    linepoint *ptr;

    /* Sanity checks */
    if( bm == NULL )
		quit("writeMSEBoxModelGeom: NULL MSEBoxModel pointer\n");
    if( bm->nbox < 1 )
		quit("writeMSEBoxModelGeom: nbox < 1\n");
    if( bm->nface < 0 )
		quit("writeMSEBoxModelGeom: nface < 0\n");

    fprintf(fp,"# Box Model Geometry\n");

    /* Always write comments here */
    writecomments = 1;

    if( writecomments )
		fprintf(fp,"# Number of boxes in horizontal plane\n");

    fprintf(fp,"nbox  %d\n",bm->nbox);

    if( writecomments )
		fprintf(fp,"# Number of faces in horizontal plane\n");

    fprintf(fp,"nface  %d\n",bm->nface);

    if( writecomments )
		fprintf(fp,"# Box Model boundary\n");

    fprintf(fp,"# %ld points\n",bm->bnd->np);

    for(ptr=bm->bnd->start; ptr != NULL; ptr = ptr->next )
		fprintf(fp,"bnd_vert %.10g %.10g\n",ptr->p.x,ptr->p.y);

    fprintf(fp,"\n");

    /* Write geometry for each box */
    for(i=0; i<bm->nbox; i++)
		writeBoxGeom(fp,&bm->boxes[i]);

    /* Write geometry for each face */
    for(i=0; i<bm->nface; i++)
		writeFaceGeom(fp,&bm->faces[i]);
}


/*******************************************************************//**
This routine reads a box geometry from an ascii file.
It assumes that the box already contains valid values for
n. It also assumes that the file is positioned at or before
the data for the box requested. No file positioning
is done here, apart from reading data. This means that
my parameter library (in sjwlib) should not be used,
as its routines do explicit file positioning.
Input file format for data for 1 box is shown below, where
boxnn means box followed by the integer box number nn
(with the minimum number of digits needed). Here, S is
a string (which must not contain whitespace),
N is an integer and X, Y, Z and A are floating point
numbers.

# Geometry for box nn (this comment should always be present)
# Box label
boxnn.label     S

# Position of station or point inside box
boxnn.inside	X    Y

# Number of box faces (equals number of neighbouring boxes)
boxnn.nconn	N

# List of face numbers for this box
boxnn.iface     N  N  N  N  N ...

# List of corresponding neignbouring boxes
boxnn.ibox	N  N  N  N  N  ...

# Depth for this box
boxnn.botz      Z

# Area of this box
boxnn.area      A

# Boundary of this box (should always be last)
boxnn.vert	X    Y<br>
boxnn.vert	X    Y<br>
boxnn.vert	X    Y<br>
    .<br>
    .<br>
    .<br>


    Arguments:     fp    input file pointer
		   b    box pointer
*********************************************************************/
void readBoxGeom(FILE *fp, char *name, Box *b)
{
    char key[BMSLEN];
    dpoint p;
    int i = 0;

    /* Sanity checks */
    if( b == NULL )
		quit("readBoxGeom: %s - NULL box pointer!\n", name);
    if( b->n < 0 )
		quit("readBoxGeom: Box number < 0!\n");

    if( verbose > 1)
		fprintf(stderr,"readBoxGeom: reading box %d in bgm file %s\n",b->n, name);

    /* Read box label */
    sprintf(key,"box%d.label",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%s",b->label) != 1 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    /* Read station location */
    sprintf(key,"box%d.inside",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf %lf",&b->inside.x,&b->inside.y) != 2 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    /* Check the points are not negative */
  	if(b->inside.x < 0)
  		warn("readBoxGeom: Box %d has negative inside x points %e in bgm file %s.\n", b->n, b->inside.x, name);
  	if(b->inside.y < 0)
  		warn("readBoxGeom: Box %d has negative inside y points %e in bgm file %s.\n", b->n, b->inside.y, name);

    /* Read number of connections */
    sprintf(key,"box%d.nconn",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%d",&b->nconn) != 1 || b->nconn < 0 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    if(b->nconn == 0)
    	warn("Box %d has 0 connections with other boxes. Please check this is correct.\n", b->n);

    /* Allocate memory for face and neighbour box lists */
    b->iface = (int *)malloc((size_t)b->nconn*sizeof(int));
    b->ibox = (int *)malloc((size_t)b->nconn*sizeof(int));
    if( b->iface==NULL || b->ibox==NULL )
		quit("readBoxGeom: Can't allocate memory for iface or ibox\n");

    /* Read the face index list */
    sprintf(key,"box%d.iface",b->n);
    skipToKeyEnd(fp,key);
    for(i=0; i<b->nconn; i++)
		if( fscanf(fp,"%d",&b->iface[i]) != 1 )
			quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    /* Read the box index list */
    sprintf(key,"box%d.ibox",b->n);
    skipToKeyEnd(fp,key);
    for(i=0; i<b->nconn; i++)
		if( fscanf(fp,"%d",&b->ibox[i]) != 1 )
			quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    /* Read the bottom z-coord for this box */
    sprintf(key,"box%d.botz",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf",&b->botz) != 1 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);



	b->current_botz = b->botz;

	/* Read box area */
    sprintf(key,"box%d.area",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf",&b->area) != 1 || b->area <= 0.0 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    /* Read box vertical mixing constant */
    sprintf(key,"box%d.vertmix",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf",&b->vertmix) != 1 || b->vertmix < 0.0 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

    /* Check the vertmix value is valid */
    if(isnan(b->vertmix)){
    	quit("Vertmix value for box %d is nan in bgm file %s\n", b->n, name);
    }

    if((b->vertmix == 0) && (b->botz < 0.0)){
    	warn("Vertmix value for box %d is 0 in bgm file %s> This means you will have no vertical mixing in this box.\n", b->n, name);

    }

    /* Read box specific horizontal mixing constant */
    sprintf(key,"box%d.horizmix",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf",&b->horizmix) != 1 || b->horizmix < 0.0 )
		quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);

	/* Check the vertmix value is valid */
	if(isnan(b->horizmix)){
		quit("horizmix value for box %d is nan in bgm file %s\n", b->n, name);
	}

	 if((b->horizmix == 0) && (b->botz < 0.0)){
		warn("Horizmix value for box %d is 0 in bgm file %s> This means you will have no horizontal mixing in this box.\n", b->n, name);
	}

    /* Read whether this is a box where may have to relax the tolerance of the adpative difference method */
    sprintf(key,"box%d.relax_tol",b->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%d",&b->relax_tol) != 1 || b->relax_tol < 0 )
        quit("readBoxGeom: Can't read %s in bgm file %s\n",key, name);
    
    
    /* Read box boundary, assumes polyline has not already
     * been allocated
     */
    b->bnd = createpolyline();

	sprintf(key,"box%d.vert",b->n);
    skipToKeyStart(fp,key);

    sprintf(key,"box%d.vert %%lf %%lf ",b->n);

	while( fscanf(fp,key,&p.x,&p.y) == 2 ){
		addtoend(b->bnd, p);
	}

    if( b->bnd->np < 3 )
		quit("readBoxGeom: less than 3 points in boundary in bgm file %s!\n", name);

    /* Check that the inside point is inside the box boundary */

    if(pointinpoly(b->inside, *b->bnd) <= 0){
    	quit("readBoxGeom: inside point in box %d is not inside the box boundary. Please hand edit your bgm file and fix\n", b->n);
    }

}

/*******************************************************************//**
This routine writes a box geometry to an ascii file.
Output file format for data for 1 box is as shown in the comments
for readBoxGeom() above.


    Arguments:     fp   output file pointer
		   b    box pointer
*********************************************************************/
void writeBoxGeom(FILE *fp, Box *b)
{
    int i = 0;
    int writecomments;
    linepoint *ptr;

    /* Sanity checks */
    if( b == NULL )
	quit("writeBoxGeom: NULL box pointer!\n");
    if( b->n < 0 )
	quit("writeBoxGeom: Box number < 0!\n");

    if( verbose > 1)
		fprintf(stderr,"writeBoxGeom: writing box %d\n",b->n);

    fprintf(fp,"# Geometry for box%d\n",b->n);

    /* Write detailed comments for box number 0 only */
    writecomments = (b->n == 0);

    if( writecomments )
        fprintf(fp,"# Box label\n");
    fprintf(fp,"box%d.label  %s\n",b->n,b->label);

    if( writecomments )
        fprintf(fp,"# Position of station or point inside box\n");
    fprintf(fp,"box%d.inside  %.10g %.10g\n",b->n,b->inside.x,b->inside.y);

    if( writecomments )
        fprintf(fp,"# Number of faces (number of neighbouring boxes)\n");
    fprintf(fp,"box%d.nconn  %d\n",b->n,b->nconn);

    if( writecomments )
        fprintf(fp,"# List of face numbers for box%d\n",b->n);
    fprintf(fp,"box%d.iface ",b->n);
    for(i=0; i<b->nconn; i++)
        fprintf(fp," %d",b->iface[i]);
    fprintf(fp,"\n");

    if( writecomments )
        fprintf(fp,"# List of corresponding neignbouring boxes\n");
    fprintf(fp,"box%d.ibox ",b->n);
    for(i=0; i<b->nconn; i++)
        fprintf(fp," %d",b->ibox[i]);
    fprintf(fp,"\n");

    if( writecomments )
        fprintf(fp,"# Depth for box%d\n",b->n);
    fprintf(fp,"box%d.botz  %.10g\n",b->n,b->botz);

    if( writecomments )
        fprintf(fp,"# Area of box%d\n",b->n);
    fprintf(fp,"box%d.area  %.10g\n",b->n,b->area);

    if( writecomments )
        fprintf(fp,"# Boundary of box%d\n",b->n);
    fprintf(fp,"# %ld points\n",b->bnd->np);
    for(ptr=b->bnd->start; ptr != NULL; ptr = ptr->next )
        fprintf(fp,"box%d.vert %.10g %.10g\n",b->n,ptr->p.x,ptr->p.y);
    fprintf(fp,"\n");
}


/*******************************************************************//**
This routine reads a face geometry from an ascii file.
It assumes that the face already contains valid values for
n. It also assumes that the file is positioned at or before
the data for the face requested. No file positioning
is done here, apart from reading data. This means that
my parameter library (in sjwlib) should not be used,
as its routines do explicit file positioning.
Input file format for data for 1 face is shown below.
Here X, Y, L, C and S are all floating point values.

# Geometry for face nn
# Start point of face
facenn.p1	X    Y

# End point of face
facenn.p2	X    Y

# Length of face
facenn.length   L

# Cos and Sin of angle from +ve x axis to normal to face
facenn.cs   C    S

# Indices of boxes to left and right of this face
facenn.lr       N    N


    Arguments:     fp    input file pointer
		   f     face pointer
*********************************************************************/
void readFaceGeom(FILE *fp, Face *f)
{
    char key[BMSLEN];

    /* Sanity checks */
    if( f == NULL )
		quit("readFaceGeom: NULL face pointer!\n");
    if( f->n < 0 )
		quit("readFaceGeom: Face number < 0!\n");

    if( verbose > 1)
		fprintf(stderr,"readFaceGeom: reading face %d\n",f->n);

    /* Read face endpoints */
    sprintf(key,"face%d.p1",f->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf %lf",&f->p1.x,&f->p1.y) != 2 )
		quit("readFaceGeom: Can't read %s\n",key);

    sprintf(key,"face%d.p2",f->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf %lf",&f->p2.x,&f->p2.y) != 2 )
		quit("readFaceGeom: Can't read %s\n",key);

    /* Read face length */
    sprintf(key,"face%d.length",f->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf",&f->len) != 1 || f->len <= 0.0 )
		quit("readFaceGeom: Can't read %s\n",key);

    /* Read face orientation */
    sprintf(key,"face%d.cs",f->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%lf %lf",&f->cos,&f->sin) != 2 )
		quit("readFaceGeom: Can't read %s\n",key);

    /* Read indices of boxes to left and right */
    sprintf(key,"face%d.lr",f->n);
    skipToKeyEnd(fp,key);
    if( fscanf(fp,"%d %d",&f->ibl,&f->ibr) != 2 || f->ibl < 0 || f->ibr < 0 )
		quit("readFaceGeom: Can't read %s\n",key);
}


/*******************************************************************//**
This routine writes a face geometry to an ascii file.
File format for data for 1 face is as shown above in the
readFaceGeom() comments.

    Arguments:     fp    output file pointer
		   f     face pointer
*********************************************************************/
void writeFaceGeom(FILE *fp, Face *f)
{
    int writecomments = 0;

    /* Sanity checks */
    if( f == NULL )
	quit("writeFaceGeom: NULL face pointer!\n");
    if( f->n < 0 )
	quit("writeFaceGeom: Face number < 0!\n");

    if( verbose > 1)
	 fprintf(stderr,"writeFaceGeom: writing face %d\n",f->n);

    fprintf(fp,"# Geometry for face%d\n",f->n);

    /* Write detailed comments for face number 0 only */
    writecomments = (f->n == 0);

    if( writecomments )
        fprintf(fp,"# Start point of face\n");
    fprintf(fp,"face%d.p1  %.10g %.10g\n",f->n,f->p1.x,f->p1.y);

    if( writecomments )
        fprintf(fp,"# End point of face\n");
    fprintf(fp,"face%d.p2  %.10g %.10g\n",f->n,f->p2.x,f->p2.y);

    if( writecomments )
        fprintf(fp,"# Length of face\n");
    fprintf(fp,"face%d.length  %.10g\n",f->n,f->len);

    if( writecomments )
        fprintf(fp,"# Cos and Sin of angle from +ve x axis to normal to face\n");
    fprintf(fp,"face%d.cs  %.10g %.10g\n",f->n,f->cos,f->sin);

    if( writecomments )
        fprintf(fp,"# Indices of boxes to left and right of face\n");
    fprintf(fp,"face%d.lr  %d %d\n",f->n,f->ibl,f->ibr);

    fprintf(fp,"\n");
}

/**
 * \brief Get the projection value from the given string.
 *
 * The string will be of the format lat_0=-36. This is parsed and the double value is passed back.
 * If the string format is not correct an error message is printed and Atlantis will quit.
 *
 *
 */
double getProjectionValue(char *str)
{
	char *valueStr;

	valueStr = strstr(str, "=");

	if(valueStr == NULL){
		quit("Box Model Projection format is incorrect.\n The format must be like: \nprojection proj=alb, lon_0=134. ,lat_1=-36. ,lat_2=-18. x_0=3000000., y_0=6000000\n");
	}
	return atof(valueStr);

}
/*******************************************************************//*
 *
 * \brief Parse the projection string into the box model variables.
 *
 * The projection string must be in the following format:
 *
 * 	projection proj=alb lon_0=134. lat_1=-36. lat_2=-18. x_0=3000000. y_0=6000000
 *
 * 	Tokenise the string based on ','
 */
void parseProjectionString(MSEBoxModel *bm, FILE *fp)
{
	 char buf[200];
	 int  buflen = 200;
	 char str[100][10];
	 int count = 0;

	 /* read in the line from the input file */
	 fgets(buf, buflen, fp);
     if (strcmp(buf, "") == 0)
//	 if(buf == NULL )  // This was tripping Wtautological-pointer-compare error
		 quit("Box Model Projection format is incorrect.\n The format must be like: \nprojection proj=aec lon_0=134. lat_1=-36. lat_2=-18. x_0=3000000. y_0=6000000\n");

	 /* Make sure the projction isn't longlat*/

	 if(strstr(buf, "longlat") != NULL){
		 quit("Box Model Projection contains co-ordinates that are geographic. Your bgm file should contain projected values that are in meters.");
	 }
	 count = parseline(buf, (char **)str, 20);

	 bm->projection = mpInitialise(count, (char **)str);

	 if( bm->projection == NULL)
		 quit("ERROR:Projection initialisation failed. Please check the projection string in your bgm file.\n\n\n");
}

/**
 * Get the unqiue adjacent boxes.
 *
 *
 */
static int getUniqueIBox( MSEBoxModel *bm, Box *b){
	int i, count, nb;
	int *checkBox = (int *)malloc(sizeof(int) * bm->nbox);

	for(i = 0; i < bm->nbox; i++){
		checkBox[i] = 0;
	}
	count = 0;
	for(i = 0; i < b->nconn; i++){
		nb = b->ibox[i];
		if(checkBox[nb] == 0){
			checkBox[nb] = 1;
			count++;
		}
	}

	b->uniqueAdjBoxes = (int *)malloc(sizeof(int) * count);
	b->nUniqueAdj = count;

	count = 0;
	for(i = 0; i < bm->nbox; i++){
		if(checkBox[i] != 0){
			b->uniqueAdjBoxes[count] = i;
			count++;
		}
	}
	b->nUniqueAdj = count;

	free(checkBox);
	return 1;
}


/* JMK Compute per-box widths as the diagonal of each box's bounding box,
   and print geometry for each box unconditionally. */
#include <float.h>
#include <math.h>

/* JMK: width-from-center = max distance from inside point to any polygon vertex (radius).
   Stores the radius in bm->box_width[i] and prints both radius and diameter. */
static void Compute_Box_Widths(MSEBoxModel *bm)
{
    if (!bm || bm->nbox <= 0) return;

    if (bm->box_width) { free(bm->box_width); bm->box_width = NULL; }
    bm->box_width = (double*)malloc((size_t)bm->nbox * sizeof(double));
    if (!bm->box_width) {
        quit("Compute_Box_Widths: no memory for box_width (%d)\n", bm->nbox);
    }

    for (int i = 0; i < bm->nbox; i++) {
        polyline *poly = bm->boxes[i].bnd;

        if (!poly || poly->np <= 0 || !poly->start) {
            bm->box_width[i] = 0.0;  /* no geometry */
/*            fprintf(stderr,
                    "JMK Geometry for box %d (%s):\n"
                    "   Inside point: (%.6f, %.6f)\n"
                    "   No boundary vertices found.\n"
                    "   Width-from-center (radius) = %.6f, diameter = %.6f\n",
                    i, bm->boxes[i].label,
                    bm->boxes[i].inside.x, bm->boxes[i].inside.y,
                    bm->box_width[i], 2.0*bm->box_width[i]);*/
            continue;
        }

        /* center (inside point) */
        const double cx = bm->boxes[i].inside.x;
        const double cy = bm->boxes[i].inside.y;

        /* find max distance from center to any vertex */
        double max_r = 0.0;
        int    max_v = -1;
        int    v     = 0;

        for (linepoint *lp = poly->start; lp != NULL; lp = lp->next, v++) {
            const double dx = lp->p.x - cx;
            const double dy = lp->p.y - cy;
            const double r  = sqrt(dx*dx + dy*dy);
            if (r > max_r) { max_r = r; max_v = v; }
        }

        /* store radius in box_width (distance from center to farthest boundary point) */
        bm->box_width[i] = max_r;

        /* --- JMK logging (always) --- */
        /* fprintf(stderr, "JMK Geometry for box %d (%s):\n", i, bm->boxes[i].label);
        fprintf(stderr, "   Inside point: (%.6f, %.6f)\n", cx, cy);*/

        v = 0;
        for (linepoint *lp = poly->start; lp != NULL; lp = lp->next, v++) {
            /*fprintf(stderr, "   Vert %d: (%.6f, %.6f)\n", v, lp->p.x, lp->p.y);*/
        }

        fprintf(stderr,
        "JMK   Box %d: Width-from-center (radius) = %.6f, diameter = %.6f (max at vertex %d)\n",
        i, bm->box_width[i], 2.0 * bm->box_width[i], max_v);
		}
}/* JMK geometry */


static void Get_DistID(MSEBoxModel *bm) {
    int ij, k;
    double *ax, *bx, *cx, *dx, *ex;
    double xdist, ydist;
    
    if(verbose) {
        printf("Sorting distances - get DistID\n");
    }
    
    /* Create arrays to use in the sort */
    ax = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    bx = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    cx = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    dx = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    ex = Util_Alloc_Init_1D_Double(bm->nbox, 0.0);
    
    for (ij=0; ij < bm->nbox; ij++) {
        bm->boxes[ij].distID = (int *)malloc(sizeof(int) * bm->nbox);

        for (k=0; k < bm->nbox; k++) {
            xdist = bm->boxes[ij].inside.x - bm->boxes[k].inside.x;
            ydist = bm->boxes[ij].inside.y - bm->boxes[k].inside.y;
            ax[k] = sqrt(xdist * xdist + ydist * ydist);
            bx[k] = k; // IDs that want sorted based on distance and then load into DistID
            cx[k] = 1.0; // Not needed in this case
            dx[k] = 1.0; // Not needed in this case
            ex[k] = 1.0; // Not needed in this case
        }
        
        Quicksort_Dir(ax, bx, cx, dx, ex, bm->nbox, 1);
        
        for (k=0; k < bm->nbox; k++) {
            bm->boxes[ij].distID[k] = (int)(bx[k]);
      }
	  
	  
         
    
    }
    
    /* Clean up */
    free1d(ax);
    free1d(bx);
    free1d(cx);
    free1d(dx);
    free1d(ex);
    
    return;
}

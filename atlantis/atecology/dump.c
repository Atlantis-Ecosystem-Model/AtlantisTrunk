/*******************************************************************//**

\ingroup atecology

    File:           dump.c

    Created         17/07/2000

    Author:         Pavel Sakov
                    CSIRO Marine Research

    Purpose:        to be added

    Description:    to be added

    Revisions:      EAF 16/09/2003 modified to cope with alternative MECO
                    as available for the south-east (Scott Condie) runs

*********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <values.h>
#include <netcdf.h>
#include <sjwlib.h>
#include <nc.h>
#include "tr2ex.h"

extern int errno;
extern errfn keyprm_errfn;

char* cuserid(char *string);
void set_boxes(tr2ex* e, FILE* f, double* dz);
int xy2b(tr2ex* e, dpoint* p);
int z2k(tr2ex* e, int bindex, int strict, double z);

/* from "mparams.h" */
#define SOLID 0x00010000
#define OUTSIDE 0x00020000

/* some initializations */
void init_from_meco_dump(char* fname, tr2ex* e)
{
    int fid = -1;
    char buf[512];
    int quit_later = 0;
    int iid, jid, kid;
    int ni, nj, nk;
    int topk;
    int nxy;
    int zcid, xcid, ycid, validuid;
    // int flagid, h1acellid, h2acellid, zgridid,;
    int start[4] = {0, 0, 0, 0};
    int count[4];
    meco_dump* meco = malloc(sizeof(meco_dump));
    double* xc = NULL;
    double* yc = NULL;
    double* zc = NULL;
    //double* h1acell = NULL;
    //double* h2acell = NULL;
    double* validu = NULL;
    int* flag = NULL;
    int xyz_index, xy_index;
    int i, j, k;

    e->meco = meco;
    e->meco->fname = strdup(fname);

    NC_open(fname, NC_NOWRITE, &fid);
    meco->fid = fid;

    if (verbose)
	printf("MECO dumpfile:\n  file = \"%s\"\n", fname);

    NC_inq_dimid(fname, fid, "i", &iid);
    NC_inq_dimid(fname, fid, "j", &jid);
    NC_inq_dimid(fname, fid, "k", &kid);

    NC_inq_dimlen(fname, fid, iid, &ni);
    NC_inq_dimlen(fname, fid, jid, &nj);
    NC_inq_dimlen(fname, fid, kid, &nk);

    if (verbose)
	printf("  dimensions = %d x %d x %d\n", ni, nj, nk);

    NC_inq_varid(fname, fid, "zc", &zcid);
    NC_inq_varid(fname, fid, "xc", &xcid);
    NC_inq_varid(fname, fid, "yc", &ycid);
    //NC_inq_varid(fname, fid, "h1acell", &h1acellid); Need to calculate these directly unfortunately!
    //NC_inq_varid(fname, fid, "h2acell", &h2acellid);
    //NC_inq_varid(fname, fid, "flag", &flagid);       Need to use isnan and u field to figure out flags instead
    //NC_inq_varid(fname, fid, "z_grid", &zgridid);    Need to set zgrid = zcid

    NC_inq_varid(fname, fid, "u", &validuid); // Read in some data so that can use its values to set the flag up from that rather than using a flag field defined in the model output

    xc = malloc(ni * nj * sizeof(double));
    yc = malloc(ni * nj * sizeof(double));
    zc = malloc(nk * sizeof(double));
    //h1acell = malloc(ni * nj * sizeof(double));
    //h2acell = malloc(ni * nj * sizeof(double));
    flag = malloc(ni * nj * nk * sizeof(int));
    validu = malloc(ni * nj * nk * sizeof(double));
    nxy = ni * nj;
    meco->count[0] = 1;
    meco->count[1] = nk;
    meco->count[2] = nj;
    meco->count[3] = ni;
    meco->ncells = nk * nj * ni;
    meco->zgrid = malloc((nk+1) * sizeof(double));

    /* read cell positions */
    count[0] = nk;
    NC_get_vara_double(fname, fid, zcid, start, count, zc);
    count[0] = nk + 1;
    //NC_get_vara_double(fname, fid, zgridid, start, count, meco->zgrid);
    count[0] = nj;
    count[1] = ni;
    NC_get_vara_double(fname, fid, xcid, start, count, xc);
    NC_get_vara_double(fname, fid, ycid, start, count, yc);
    //NC_get_vara_double(fname, fid, h1acellid, start, count, h1acell);
    //NC_get_vara_double(fname, fid, h2acellid, start, count, h2acell);

    count[0] = 1;
    count[1] = nk;
    count[2] = nj;
    count[3] = ni;

    NC_get_vara_double(fname, fid, validuid, start, count, validu);

    //NC_get_vara_int(fname, fid, flagid, start, count, flag);

    /* read projections and convert XY data to BM projection if necessary */
    memset(buf, 0, 512);
    NC_get_att_text(fname, fid, xcid, "projection", buf);
    if (strcasecmp(buf, e->projection)) {
	char* args[64];
	int nargs;
	MapProjection* mp_meco = NULL;
	MapProjection* mp_bm = NULL;
	if (strcasecmp(buf, "geographic")) {
	    nargs = parseline(buf, args, 64);
	    mp_meco = mpInitialise(nargs, args);
	    for (i = 0; i < nxy; ++i)
		mpInverse(mp_meco, xc[i], yc[i], &yc[i], &xc[i]);
	}
	if (strcasecmp(e->projection, "geographic")) {
	    printf("projection = %s\n", e->projection);
	    nargs = parseline(e->projection, args, 64);
	    mp_bm = mpInitialise(nargs, args);
	    for (i = 0; i < nxy; ++i)
		mpForward(mp_bm, yc[i], xc[i], &xc[i], &yc[i]);
	}
	mpFree(mp_meco);
	mpFree(mp_bm);
    }

    /* create mapping bm -> meco */
    /* count cells */
    for (k = 0, xyz_index = 0; k < nk; ++k) {
        for (j = 0, xy_index = 0; j < nj; ++j) {
            for (i = 0; i < ni; ++i, xy_index++, xyz_index++) {
                /* Set flag */
                if(!isnan(validu[xyz_index])){
              // if (!(flag[xyz_index] & (SOLID|OUTSIDE))) {
              dpoint p = {xc[xy_index], yc[xy_index]};
              int b = xy2b(e, &p);
              int kk = (b >= 0) ? z2k(e, b, 0, -zc[k]) : -1;
              if (kk >= 0)
                 e->boxes[b].cells[kk].nmecocells++;
              flag[xyz_index] = 1;
                }
            else flag[xyz_index] = 0;
            }
        }
        if (k == 0) meco->zgrid[k] = 0;
        else meco->zgrid[k] = (zc[k] + zc[k-1])/2;
    }

    /* allocate memory */
    for (i = 0; i < e->nb; ++i) {
        box* b = &e->boxes[i];
        for (j = 0; j < b->nlayers; ++j) {
            cell* c = &e->boxes[i].cells[j];
            int n = c->nmecocells;
            if (n > 0)
            c->mecocells = malloc(n * sizeof(mecocell));
            else if (!c->is_boundary) {
            warn("error: BM cell (b,k) = (%d,%d) contains 0 MECO cells\n", i, j);
            //quit_later = 1;
            } else
            c->mecocells = NULL;
            c->nmecocells = 0;
        }
    }
    if (quit_later)
	exit(0);

    /* fill the mapping
    note that the area used was given using the great circle method
    D = 60*acos(sin(L1*pi/180)*sin(L2*pi/180)+cos(L1*pi/180)*cos(L2*pi/180)*cos(DG*pi/180))*180/pi*1.852*1000
    with D distance in metres, L1 latitude point 1, L2 latitude point 2, DG differnece in longitude,
    (from http://www.ga.gov.au/nmd/geodesy/datums/distance.jsp)*/
    topk = nk - 1;
    for (k = 0, xyz_index = 0; k < nk; ++k) {
	for (j = 0, xy_index = 0; j < nj; ++j) {
	    for (i = 0; i < ni; ++i, xy_index++, xyz_index++) {
	         if (flag[xyz_index]){
		    // if (!(flag[xyz_index] & (SOLID|OUTSIDE))) {
		    dpoint p = {xc[xy_index], yc[xy_index]};
		    int b = xy2b(e, &p);
		    int kk = (b >= 0) ? z2k(e, b, 0, -zc[k]) : -1;
		    if (kk >= 0) {
		       cell* c = &e->boxes[b].cells[kk];
		       mecocell* mc = &c->mecocells[c->nmecocells];
		       mc->index = xyz_index;
		       //mc->area = h1acell[xy_index] * h2acell[xy_index];
		       mc->area = 22200*22200;
		       e->boxes[b].cells[kk].nmecocells++;
		       if (k == topk)
		          c->b->mecoarea += mc->area;
		    }
		 }
	    }
	}
    }

    if (verbose) {
	printf("  bm mapping:\n");
	for (i = 0; i < e->nb; ++i) {
	    box* b = &e->boxes[i];
	    printf("    box %d:\n", i);
	    for (j = 0; j < b->nlayers; ++j)
		printf("      layer %d: %d cell(s)\n", j, b->cells[j].nmecocells);
	    printf("      mecoarea = %.3g\n", b->mecoarea);
	}
    }

    /* find and store topzid & botzid*/
    if (nc_inq_varid(fid, "eta", &meco->topzid) != NC_NOERR && nc_inq_varid(fid, "topz", &meco->topzid) != NC_NOERR)
	quit("error: %s: could not get id for variables \"eta\" or \"topz\"\n", fname);
    NC_inq_varid(fname, fid, "botz", &meco->botzid);

    fflush(stdout);

    free(xc);
    free(yc);
    free(zc);
    //free(h1acell);
    //free(h2acell);
    free(flag);
}

#undef SOLID
#undef OUTSIDE

void init_from_bm_dump(char* fname, tr2ex* e)
{
    int fid;
    FILE *f = NULL;
    char buf[512] = "";
    int tid = -1;
    int zid = -1;
    double* dz = NULL;
    int start[2] = {0,0};
    int count[2];
    dpoint p;

    e->bm = malloc(sizeof(bm_dump));
    e->bm->fname = strdup(fname);

    NC_open(fname, NC_NOWRITE, &fid);

    if (verbose)
	printf("BM dumpfile:\n  file = \"%s\"\n", fname);

    /* time units */
    NC_inq_varid(fname, fid, "time", &tid);
    memset(buf, 0, 512);
    NC_get_att_text(fname, fid, tid, "units", buf);

    e->bm->t_units = strdup(buf);

    /* geometry */
    memset(buf, 0, 512);
    NC_get_att_text(fname, fid, NC_GLOBAL, "geometry", buf);

    if ((f = Open_Input_File(bm->inputFolder,buf, "r")) == NULL)
	quit("error: %s%s: %s\n", bm->inputFolder, buf, strerror(errno));

    if (verbose)
	printf("geometry:\n  file = \"%s\"\n", buf);

    set_keyprm_errfn(quit);

    /* read boundary polyline */
    e->boundary = createpolyline();
    skipToKeyStart(f,"bnd_vert");
    while (fscanf(f, "bnd_vert %e %e ", &p.x, &p.y) == 2)
	addtoend(e->boundary, p);

    if (verbose)
	printf("  boundary: %ld points\n", e->boundary->np);

    /* read number of boxes*/
    readkeyprm_i(f, "nbox", &e->nb);
    if (e->nb <= 0)
	quit("error: %s: nbox = 0\n", buf);
    if (verbose)
	printf("  %d boxes\n", e->nb);

    /* read layer thicknesses */
    NC_get_att_int(fname, fid, NC_GLOBAL, "wcnz", &e->nk);
    NC_get_att_int(fname, fid, NC_GLOBAL, "sednz", &e->nktot);
    e->nktot += e->nk;
    dz = malloc(e->nktot * e->nb * sizeof(double));
    NC_inq_varid(fname, fid, "nominal_dz", &zid);
    count[0] = e->nb;
    count[1] = e->nktot;
    NC_get_vara_double(fname, fid, zid, start, count, dz);

    set_boxes(e, f, dz);

    free(dz);
    fclose(f);
    nc_close(fid);
}

void create_exchange_dump(FILE* f, tr2ex* e)
{
    errfn errfn_orig = keyprm_errfn;
    char fname[512];
    int fid = -1;
    meco_dump* meco = e->meco;
    exchange_dump* hd = NULL;
    int ncells = e->ncells;
    cell** cells = e->cells;
    char* t_units = e->t_units;
    double dt = e->dt;
    int dims[4];
    double fill = -1.0;
    int ifill = -1;
    int i;

    set_keyprm_errfn(quiet);
    if (!readkeyprm_s(f, "OUTHDdumpfile", fname)) {
	keyprm_errfn = errfn_orig;
	return;
    }

    keyprm_errfn = errfn_orig;

    hd = malloc(sizeof(exchange_dump));

    hd->fname = strdup(fname);
    hd->r = 0;
    hd->nb = e->nb;
    hd->nk = e->nk;

    e->hd = hd;

    /* create the netCDF file */
    NC_create(fname, NC_WRITE, &fid);
    hd->fid = fid;

    /* set dimensions */
    NC_def_dim(fname, fid, "t", NC_UNLIMITED, &hd->tid);
    NC_def_dim(fname, fid, "b", hd->nb, &hd->bid);
    NC_def_dim(fname, fid, "z", hd->nk, &hd->zid);

    /* determine maximal number of destination cells */
    hd->nd = 0;
    for (i = 0; i < ncells; ++i) {
	cell* c = cells[i];
	if (c->nto > hd->nd)
    	    hd->nd = c->nto;
    }
    if (verbose)
    	fprintf(stderr, "%d destination cells\n", hd->nd);
    NC_def_dim(fname, fid, "dest", hd->nd, &hd->did);

    /* variables */

    /* t */
    dims[0] = hd->tid;
    NC_def_var(fname, fid, "t", NC_DOUBLE, 1, dims, &hd->tvid);
    NC_put_att_text(fname, fid, hd->tvid, "units", t_units);
    NC_put_att_double(fname, fid, hd->tvid, "dt", 1, &dt);

    /* exchange */
    dims[0] = hd->tid;
    dims[1] = hd->bid;
    dims[2] = hd->zid;
    dims[3] = hd->did;
    NC_def_var(fname, fid, "exchange", NC_DOUBLE, 4, dims, &hd->evid);
    NC_put_att_double(fname, fid, hd->evid, "_FillValue", 1, &fill);

    /* dest_b */
    NC_def_var(fname, fid, "dest_b", NC_INT, 4, dims, &hd->bvid);
    NC_put_att_int(fname, fid, hd->bvid, "_FillValue", 1, &ifill);

    /* dest_k */
    NC_def_var(fname, fid, "dest_k", NC_INT, 4, dims, &hd->kvid);
    NC_put_att_int(fname, fid, hd->kvid, "_FillValue", 1, &ifill);

    NC_put_att_text(fname, fid, NC_GLOBAL, "description", DESCRIPTION);
    {
	char uname[L_cuserid];
	cuserid(uname);
	NC_put_att_text(fname, fid, NC_GLOBAL, "author", uname);
    }
    {
	time_t t;
	char* tstr;
	time(&t);
	tstr = strdup(ctime(&t));
	tstr[strlen(tstr)-1] = 0; /* strip \n */
	NC_put_att_text(fname, fid, NC_GLOBAL, "date", tstr);
	free(tstr);
    }
    NC_put_att_text(fname, fid, NC_GLOBAL, "MECOdumpfile", meco->fname);
    NC_put_att_text(fname, fid, NC_GLOBAL, "BMdumpfile", e->bm->fname);
    NC_put_att_double(fname, fid, NC_GLOBAL, "scale_exchanges", 1, &e->scale_exchanges);
    NC_put_att_double(fname, fid, NC_GLOBAL, "scale_tracers", 1, &e->scale_tracers);
    NC_put_att_double(fname, fid, NC_GLOBAL, "weight_tracers", 1, &e->weight_tracers);
    NC_put_att_int(fname, fid, NC_GLOBAL, "volpower", 1, &e->volpower);
    for (i = 0; i < e->ntracers; ++i) {
	trinfo* t = &e->tracers[i];
	char buf[512];
	snprintf(buf, sizeof(buf), "tracer%d.name", i);
	NC_put_att_text(fname, fid, NC_GLOBAL, buf, t->name);
	snprintf(buf, sizeof(buf), "tracer%d.scale", i);
	NC_put_att_double(fname, fid, NC_GLOBAL, buf, 1, &t->scale);
	snprintf(buf, sizeof(buf), "tracer%d.weight", i);
	NC_put_att_double(fname, fid, NC_GLOBAL, buf, 1, &t->weight);
    }

    nc_enddef(fid);
    nc_sync(fid);
}

void write_exchange_dump(tr2ex* e)
{
    exchange_dump* hd = e->hd;
    char* fname = hd->fname;
    int fid = hd->fid;
    cell** cells = e->cells;
    link* links = e->links;
    double* exchanges = e->x;
    int nexchanges = e->nlinks;
    int nb = hd->nb;
    int nk = hd->nk;
    int nd = hd->nd;
    int b, k, d, i;
    size_t start[4];
    size_t count[4];
    int* cdest = NULL;
    double*** exchange = NULL;
    int*** dest_k = NULL;
    int*** dest_b = NULL;

    if (hd == NULL || e->t < e->toutstart)
	return;

    cdest = i_alloc1d(nb * nk);
    exchange = alloc3d(nd, nk, nb);
    dest_k = i_alloc3d(nd, nk, nb);
    dest_b = i_alloc3d(nd, nk, nb);

    start[0] = hd->r;
    count[0] = 1;
    NC_put_vara_double(fname, fid, hd->tvid, &start[0], &count[0], &e->t);

    for (b = 0; b < nb; ++b)
	for (k = 0; k < nk; ++k)
	    for (d = 0; d < nd; ++d) {
		exchange[b][k][d] = -1.0;
		dest_b[b][k][d] = -1;
		dest_k[b][k][d] = -1;
	    }

    for (i = 0; i < nexchanges; ++i) {
	int oindex = cells[links[i].from]->oindex;
	int bb = oindex / nk;
	int kk = oindex % nk;
	exchange[bb][kk][cdest[oindex]] = exchanges[i];
	dest_b[bb][kk][cdest[oindex]] = cells[links[i].to]->oindex / nk;
	dest_k[bb][kk][cdest[oindex]] = cells[links[i].to]->oindex % nk;
	cdest[oindex]++;
    }

    start[1] = 0;
    start[2] = 0;
    start[3] = 0;
    count[1] = nb;
    count[2] = nk;
    count[3] = nd;
    NC_put_vara_double(fname, fid, hd->evid, start, count, exchange[0][0]);
    NC_put_vara_int(fname, fid, hd->bvid, start, count, dest_b[0][0]);
    NC_put_vara_int(fname, fid, hd->kvid, start, count, dest_k[0][0]);
    nc_sync(fid);

    free(cdest);
    free3d(exchange);
    i_free3d(dest_k);
    i_free3d(dest_b);

    hd->r++;
}

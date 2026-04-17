///*******************************************************************//**
//\ingroup atecology
//    File:           init.c
//
//    Created         17/07/2000
//
//    Author:         Pavel Sakov
//                    CSIRO Marine Research
//
//    Purpose:        to be added
//
//    Description:    to be added
//
//    Revisions:      none.
//
//*********************************************************************/
//
//#include <stdlib.h>
//#include <stdio.h>
//#include <sjwlib.h>
//#include <netcdf.h>
//#include <nc.h>
//#include "tr2ex.h"
//
//extern int do_run;
//extern int nc;
//extern int* cindices;
//extern int nl;
//extern int* lindices;
//
//void calculate_dvol(tr2ex* e);
//void check_cell(tr2ex* e, int b, int k);
//void describe_box(box* b);
//void describe_cell(tr2ex* e, int b, int k);
//void describe_link(tr2ex* m2b, int i);
//double get_meco_record_number(tr2ex* e, double t);
//double get_tscale(const char* t_units);
//const char* get_tunit(const char* t_units);
//void init_from_bm_dump(char* fname, tr2ex* e);
//void init_from_meco_dump(char* fname, tr2ex* e);
//void create_exchange_dump(FILE* f, tr2ex* e);
//int xyz2cell(void* _e, double x, double y, double z, int* i, int* j, int* k);
//int find_trindex(void* _e, char* name);
//
//extern errfn keyprm_errfn;
//
//static void init_tr2ex(tr2ex* e)
//{
//    e->fname = NULL;
//    e->meco = NULL;
//    e->bm = NULL;
//    e->hd = NULL;
//    e->nb = -1;
//    e->boxes = NULL;
//    e->nk = -1;
//    e->nktot = -1;
//    e->use_polyarea = USE_POLYAREA_DEF;
//    e->boundary = NULL;
//    e->projection = NULL;
//    e->weight_tracers = -1.0;
//    e->scale_tracers = -1.0;
//    e->scale_exchanges = -1.0;
//    e->volpower = VOLPOWER_DEF;
//    e->handle_tide = HANDLE_TIDE_DEF;
//    e->exchangeweights_into_E = EXCHANGEWEIGHTS_INTO_E_DEF;
//    e->volfrac_max = VOLFRAC_MAX_DEF;
//    e->periodic = 0;
//    e->period = NaN;
//    e->ntracers = -1;
//    e->tracers = NULL;
//    e->ncells = -1;
//    e->cells = NULL;
//    e->nacells = -1;
//    e->acells = NULL;
//    e->nbtscells = 0;
//    e->btscells = NULL;
//    e->bts = NULL;
//    e->nlinks = -1;
//    e->links = NULL;
//    e->nflinks = 0;
//    e->flinks = NULL;
//    e->t_units = NULL;
//    e->t_units_out = NULL;
//    e->tstart = NaN;
//    e->tstop = NaN;
//    e->dt = NaN;
//    e->t = NaN;
//    e->nt = -1;
//    e->npss = -1;
//    e->pss = NULL;
//    e->precipitation = NULL;
//    e->precip_id = -1;
//    e->evaporation = NULL;
//    e->evap_id = -1;
//    e->trv0 = NULL;
//    e->trv1 = NULL;
//    e->cvol0 = NULL;
//    e->cvol1 = NULL;
//    e->dtrv = NULL;
//    e->nv = -1;
//    e->nc = -1;
//    e->nec = -1;
//    e->x = NULL;
//    e->vfrom = NULL;
//    e->vto = NULL;
//    e->emax = NaN;
//    e->iemax = -1;
//    e->dtrmax = NULL;
//    e->idtrmax = NULL;
//    e->ntotal = 0;
//    e->Etotal = NaN;
//    e->DTRtotal = NULL;
//    e->remove_z_trend = 0;
//    e->dvol = NULL;
//}
//
//static void build_box(FILE* f, box* b) {
//    char buf[512];
//    char key[512];
//    int nd = 0;
//    dpoint p;
//    errfn errfn_orig = keyprm_errfn;
//
//    snprintf(key, sizeof(key), "box%d.label", b->index);
//    readkeyprm_s(f, key, buf);
//    b->label = strdup(buf);
//
//    {
//	int i, j;
//	int n = 0;
//	int *nv = NULL;
//	snprintf(key, sizeof(key), "box%d.nconn", b->index);
//	readkeyprm_i(f, key, &n);
//	nv = malloc(n * sizeof(int));
//	snprintf(key, sizeof(key), "box%d.ibox", b->index);
//	skipToKeyEnd(f, key);
//	for (i = 0; i < n; ++i)
//	    if (fscanf(f, "%d", &nv[i]) != 1)
//		keyprm_errfn("error: could not read \"%s\"\n", key, b->index);
//	for (i = 0; i < n; ++i) {
//	    int v = nv[i];
//	    for (j = 0; j < i; ++j)
//		if (nv[j] == v)
//		    break;
//	    if (j == i) {
//		nv[nd] = v;
//		nd++;
//	    }
//	}
//	b->nneighbours = nd;
//	b->neighbours = malloc(nd * sizeof(int));
//	for (i = 0; i < nd; ++i)
//	    b->neighbours[i] = nv[i];
//	free(nv);
//    }
//
//    b->boundary = createpolyline();
//    snprintf(key, sizeof(key), "box%d.vert", b->index);
//    skipToKeyStart(f, key);
//    snprintf(key, sizeof(key), "box%d.vert %%e %%e ", b->index);
//    while (fscanf(f, key, &p.x, &p.y) == 2)
//	addtoend(b->boundary, p);
//
//    snprintf(key, sizeof(key), "box%d.inside", b->index);
//    skipToKeyEnd(f,key);
//    if (fscanf(f,"%e %e",&b->inside.x,&b->inside.y) != 2)
//	keyprm_errfn("error: could not read \"%s\"\n", key, b->index);
//
//    if (!pointinpoly(b->boundary, b->inside))
//	quit("error: box %d: inside point is not inside the boundary\n", b->index);
//
//    /* decide on box type by checking whether "inside" point for the
//     * box is also inside the model boundary */
//    if (pointinpoly(b->e->boundary, b->inside))
//	b->is_boundary = 0;
//    else
//	b->is_boundary = 1;
//
//    set_keyprm_errfn(quiet);
//    snprintf(key, sizeof(key), "box%d.area", b->index);
//    if (b->e->use_polyarea || !readkeyprm_d(f, key, &b->area))
//	if ((b->area = polyarea(*b->boundary)) <= 0 )
//	    quit("error: could not either read \"%s\" or calculate bounding polygon area\n", key);
//    set_keyprm_errfn(errfn_orig);
//
//    b->mecoarea = 0.0;
//}
//
///** Sets array of boxes in `tr2ex' in the end of initialization from BM dump.
// * @param e tr2ex data structure
// * @param f geometry file
// * @param dz array of cell depths [(nwc+nsed) * nz] read from "nominal_dz"
// */
//void set_boxes(tr2ex* e, FILE* f, double* dz)
//{
//    int i, j, index;
//
//    e->boxes = (box*) calloc(e->nb, sizeof(box));
//    for (i = 0; i < e->nb; i++) {
//	box* b = &e->boxes[i];
//	b->e = e;
//	b->index = i;
//	build_box(f, b);
//
//	/* layers */
//	b->nlayers = 0;
//	index = b->index * e->nktot;
//	for (j = 0; j < e->nk; ++j, ++index) {
//	    if (dz[index] > 0.0)
//		b->nlayers++;
//	    else
//		break;
//	}
//	if (b->nlayers <= 0)
//	    quit("error: box %d: nlayers == 0\n", b->index);
//	b->cells = malloc(b->nlayers * sizeof(cell));
//
//	index = b->index * e->nktot;
//	for (j = 0; j < b->nlayers; ++j, ++index)
//	    b->cells[j].thickness = dz[index];
//
//	if (verbose)
//	    describe_box(b);
//    }
//    fflush(stdout);
//}
//
//static void set_cells(tr2ex* e)
//{
//    int i, j, index, aindex;
//
//    for (i = 0, index = 0, aindex = 0; i < e->nb; ++i) {
//	box* b = &e->boxes[i];
//	for (j = 0; j < b->nlayers; ++j, ++index) {
//	    cell* c = &b->cells[j];
//	    c->b = b;
//	    c->k = j;
//	    c->layer = b->nlayers - 1 - j;
//	    c->index = index;
//	    c->is_upper = (c->layer == 0);
//	    c->is_boundary = b->is_boundary;
//	    c->area = b->area;
//	    c->volume = c->thickness * b->area;
//	    c->volume_before = c->volume;
//	    c->volume_after = c->volume;
//	    c->oindex = i * e->nk + j;
//	    if (!c->is_boundary)
//		aindex++;
//
//	    /* to be set later */
//	    c->nmecocells = 0;
//	    c->mecocells = NULL;
//	    c->nto = 0;
//	    c->nfrom = 0;
//	    c->to = NULL;
//	    c->from = NULL;
//	    c->lto = NULL;
//	    c->lfrom = NULL;
//	}
//    }
//
//    e->ncells = index;
//    e->cells = malloc(e->ncells * sizeof(cell *));
//
//    e->nacells = aindex;
//    e->acells = malloc(e->nacells * sizeof(cell *));
//
//    for (i = 0, index = 0, aindex = 0; i < e->nb; ++i) {
//	box* b = &e->boxes[i];
//	for (j = 0; j < b->nlayers; ++j, ++index) {
//	    cell* c = &b->cells[j];
//	    e->cells[index] = c;
//	    if (!c->is_boundary) {
//		e->acells[aindex] = c;
//		aindex++;
//	    }
//	}
//    }
//
//    if (verbose) {
//	printf("%d cells\n", index);
//	printf("%d non-boundary cells\n", aindex);
//    }
//    fflush(stdout);
//}
//
//static void set_btscells(FILE* f, tr2ex* e)
//{
//    errfn errfn_orig = keyprm_errfn;
//    int n = 0;
//    int i, t;
//
//    set_keyprm_errfn(quiet);
//    readkeyprm_i(f, "nbtscells", &n);
//
//    e->nbtscells = n;
//    e->btscells = malloc(n * sizeof(cell*));
//    e->bts = malloc(n * sizeof(cellTimeSeries));
//
//    set_keyprm_errfn(quit);
//    for (i = 0; i < n; ++i) {
//	cellTimeSeries* bts = &e->bts[i];
//	char buf[512];
//	char key[32];
//	char tsfname[512];
//	int b, k;
//
//	snprintf(key, sizeof(key), "btscell%d", i);
//	readkeyprm_s(f, key, buf);
//	if (sscanf(buf, "(%d,%d) %s", &b, &k, tsfname) != 3)
//	    quit("error: %s: %s: \"%s\" does not match format \"(%%d,%%d) %%s\"\n", e->fname, key, buf);
//	check_cell(e, b, k);
//	e->btscells[i] = &e->boxes[b].cells[k];
//
//	tsRead(tsfname, &bts->ts);
//	if (strcmp(bts->ts.t_units, e->t_units) != 0)
//	    tsNewTimeUnits(&bts->ts, e->t_units);
//
//	bts->tstrindices = malloc(e->ntracers * sizeof(int));
//
//	for (t = 0; t < e->ntracers; ++t) {
//	    bts->tstrindices[t] = tsIndex(&bts->ts, e->tracers[t].name);
//	    if (bts->tstrindices[t] < 0)
//		quit("error: %s: \"%s\" not found\n", tsfname, e->tracers[t].name);
//	}
//    }
//
//    set_keyprm_errfn(errfn_orig);
//}
//
//static void check_boundary_cells(tr2ex* e)
//{
//    int i;
//    int j;
//    int quit_later = 0;
//    int done = FALSE;
//
//    for (i = 0; i < e->ncells; ++i) {
//        if(done == TRUE)
//            break;
//	cell* c = e->cells[i];
//	if (!c->is_boundary)
//	    continue;
//	for (j = 0; j < e->nbtscells; ++j) {
//	    if (c == e->btscells[j])
//		done = TRUE;//goto ok;
//	}
//	if (c->nmecocells == 0) {
//	    warn("error: boundary cell (%d,%d) contains no MECO cells and no TS data\n", c->b->index, c->k);
//	    //quit_later = 1;
//	}
//    //ok:
//    }
//
//    if (quit_later)
//	exit(1);
//}
//
//static int find_link(tr2ex* e, int b0, int k0, int b1, int k1)
//{
//    cell* c0 = &e->boxes[b0].cells[k0];
//    cell* c1 = &e->boxes[b1].cells[k1];
//    int i;
//
//    for (i = 0; i < c0->nto; ++i) {
//	link* l = &e->links[c0->lto[i]];
//	if (l->from == c0->index && l->to == c1->index)
//	    return c0->lto[i];
//    }
//
//    return -1;
//}
//
//static void read_link_weights(FILE* f, tr2ex* e)
//{
//    errfn errfn_orig = keyprm_errfn;
//    int n = 0;
//    int i;
//
//    set_keyprm_errfn(quiet);
//    readkeyprm_i(f, "nexchangeweights", &n);
//
//    set_keyprm_errfn(quit);
//    for (i = 0; i < n; ++i) {
//	char buf[512];
//	char key[32];
//	int b0, k0, b1, k1;
//	int l;
//	double w;
//
//	snprintf(key, sizeof(key), "exchangeweight%d", i);
//	readkeyprm_s(f, key, buf);
//	if (sscanf(buf, "(%d,%d)->(%d,%d) %e", &b0, &k0, &b1, &k1, &w) != 5)
//	    quit("error: %s: %s: \"%s\" does not match format \"(%%d,%%d)->(%%d,%%d) %%e\"\n", e->fname, key, buf);
//	check_cell(e, b0, k0);
//	check_cell(e, b1, k1);
//	l = find_link(e, b0, k0, b1, k1);
//	if (l == -1)
//	    quit("error: %s: %s: exchange (%d,%d)->(%d,%d) not found\n", e->fname, key, b0, k0, b1, k1);
//	e->links[l].w = w;
//	if (verbose)
//	    printf("exchange %d: w = %.3g\n", l, w);
//    }
//
//    set_keyprm_errfn(errfn_orig);
//}
//
//static void add_links(FILE* f, tr2ex* e, int* n, link** links)
//{
//    errfn errfn_orig = keyprm_errfn;
//    int i;
//
//    *n = 0;
//    *links = NULL;
//
//    set_keyprm_errfn(quiet);
//    readkeyprm_i(f, "naddexchanges", n);
//
//    if (*n <= 0)
//	return;
//
//    *links = malloc(*n * sizeof(link));
//
//    set_keyprm_errfn(quit);
//    for (i = 0; i < *n; ++i) {
//	link* l = &(*links)[i];
//	char buf[512];
//	char key[32];
//	int b0, k0, b1, k1;
//	cell* cfrom;
//	cell* cto;
//
//	snprintf(key, sizeof(key), "addexchange%d", i);
//	readkeyprm_s(f, key, buf);
//	if (sscanf(buf, "(%d,%d)->(%d,%d)", &b0, &k0, &b1, &k1) != 4) {
//	    printf("error: %s: %s: \"%s\" does not match format \"(%%d,%%d)->(%%d,%%d)\"\n", e->fname, key, buf);
//	    exit(1);
//	}
//	check_cell(e, b0, k0);
//	check_cell(e, b1, k1);
//	cfrom = e->cells[e->boxes[b0].cells[k0].index];
//	cto = e->cells[e->boxes[b1].cells[k1].index];
//	l->from = cfrom->index;
//	l->to = cto->index;
//	l->is_fixed = 0;
//	l->w = 1.0;
//	e->nlinks++;
//	cfrom->nto++;
//	cto->nfrom++;
//    }
//
//    set_keyprm_errfn(errfn_orig);
//}
//
//static void set_fixed_links(FILE* f, tr2ex* e)
//{
//    errfn errfn_orig = keyprm_errfn;
//    double tscale = 86400.0 / get_tscale(e->t_units);
//    int n = 0;
//    int i;
//
//    set_keyprm_errfn(quiet);
//    readkeyprm_i(f, "nfixedexchanges", &n);
//
//    e->nflinks = n;
//    e->flinks = malloc(n * sizeof(link*));
//
//    set_keyprm_errfn(quit);
//    for (i = 0; i < n; ++i) {
//	char buf[512];
//	char key[32];
//	int b0, k0, b1, k1;
//	int l;
//	double w;
//
//	snprintf(key, sizeof(key), "fixedexchange%d", i);
//	readkeyprm_s(f, key, buf);
//	if (sscanf(buf, "(%d,%d)->(%d,%d) %e", &b0, &k0, &b1, &k1, &w) != 5) {
//	    printf("error: %s: %s: \"%s\" does not match format \"(%%d,%%d)->(%%d,%%d) %%e\"\n", e->fname, key, buf);
//	    exit(1);
//	}
//	check_cell(e, b0, k0);
//	check_cell(e, b1, k1);
//	l = find_link(e, b0, k0, b1, k1);
//	if (l == -1)
//	    quit("error: %s: %s: exchange (%d,%d)->(%d,%d) not found\n", e->fname, key, b0, k0, b1, k1);
//	e->links[l].is_fixed = 1;
//	e->links[l].value = w * e->dt * tscale;
//	e->flinks[i] = &e->links[l];
//	if (verbose)
//	    printf("exchange %d: fixed, %.3g per second\n", l, w);
//    }
//
//    set_keyprm_errfn(errfn_orig);
//}
//
//static void set_links(FILE* f, tr2ex* e)
//{
//    int i, j, k, n;
//    link* links_a = NULL;
//    int nlinks_a = 0;
//
//    e->nlinks = 0;
//
//    /* count */
//    printf("BLAH BLAH BLAH!!!\n");
//
//    for (i = 0; i < e->ncells; ++i) {
//	cell* c = e->cells[i];
//	box* b = c->b;
//	printf("i: %d\n", i);
//	for (j = 0; j < b->nneighbours; ++j) {
//	    box* nb = &e->boxes[b->neighbours[j]];
//	    for (k = 0; k < nb->nlayers; ++k) {
//		cell* nc = &nb->cells[k];
//		if (c->layer == nc->layer) {
//		    e->nlinks++;
//		    c->nto++;
//		    nc->nfrom++;
//		}
//	    }
//	}
//	if(i > 14) quit("got through the count for cell %d\n", i);
//	for (j = 0; j < b->nlayers; ++j) {
//	    cell* nc = &b->cells[j];
//	    if (abs(nc->layer - c->layer) == 1) {
//		e->nlinks++;
//		c->nto++;
//		nc->nfrom++;
//	    }
//	}
//    }
//
//    add_links(f, e, &nlinks_a, &links_a);
//
//    /* allocate memory */
//    e->links = malloc(e->nlinks * sizeof(link));
//    for (i = 0; i < e->ncells; ++i) {
//	cell* c = e->cells[i];
//	if (c->nfrom > 0) {
//	    c->from = malloc(c->nfrom * sizeof(int));
//	    c->lfrom = malloc(c->nfrom * sizeof(int));
//	    c->nfrom = 0;
//	}
//	if (c->nto > 0) {
//	    c->to = malloc(c->nto * sizeof(int));
//	    c->lto = malloc(c->nto * sizeof(int));
//	    c->nto = 0;
//	}
//    }
//
//    /* create */
//    e->nlinks = 0;
//    for (i = 0; i < e->ncells; ++i) {
//	cell* c = e->cells[i];
//	box* b = c->b;
//	for (j = 0; j < b->nneighbours; ++j) {
//	    box* nb = &e->boxes[b->neighbours[j]];
//	    for (k = 0; k < nb->nlayers; ++k) {
//		cell* nc = &nb->cells[k];
//		if (c->layer == nc->layer) {
//		    link* l = &e->links[e->nlinks];
//		    l->index = e->nlinks;
//		    l->from = c->index;
//		    l->to = nc->index;
//		    l->is_fixed = 0;
//		    l->w = 1.0;
//		    l->value = NaN;
//
//		    c->to[c->nto] = nc->index;
//		    c->lto[c->nto] = e->nlinks;
//		    c->nto++;
//
//		    nc->from[nc->nfrom] = c->index;
//		    nc->lfrom[nc->nfrom] = e->nlinks;
//		    nc->nfrom++;
//
//		    e->nlinks++;
//		}
//	    }
//	}
//	for (j = 0; j < b->nlayers; ++j) {
//	    cell* nc = &b->cells[j];
//	    if (abs(nc->layer - c->layer) == 1) {
//		link* l = &e->links[e->nlinks];
//		l->index = e->nlinks;
//		l->from = c->index;
//		l->to = nc->index;
//		l->is_fixed = 0;
//	        l->w = 1.0;
//		l->value = NaN;
//
//		c->to[c->nto] = nc->index;
//		c->lto[c->nto] = e->nlinks;
//		c->nto++;
//
//		nc->from[nc->nfrom] = c->index;
//		nc->lfrom[nc->nfrom] = e->nlinks;
//		nc->nfrom++;
//
//		e->nlinks++;
//	    }
//	}
//    }
//
//    if (verbose)
//	printf("%d exchanges created automatically\n", e->nlinks);
//
//    /* add user specified links */
//    n = nlinks_a;
//    nlinks_a = 0;
//    for (i = 0; i < n; ++i) {
//	link* l = &links_a[i];
//	cell* cfrom = e->cells[l->from];
//	cell* cto = e->cells[l->to];
//
//	if (find_link(e, cfrom->b->index, cfrom->k, cto->b->index, cto->k) >= 0) {
//	    if (verbose)
//		printf("user specified exchange (%d,%d)->(%d,%d) skipped: already created\n", cfrom->b->index, cfrom->k, cto->b->index, cto->k);
//	    continue;
//	}
//
//	e->links[e->nlinks].index = e->nlinks;
//	e->links[e->nlinks].to = l->to;
//	e->links[e->nlinks].from = l->from;
//	e->links[e->nlinks].w = l->w;
//	e->links[e->nlinks].is_fixed = 0;
//	e->links[e->nlinks].value = NaN;
//
//	cfrom->to[cfrom->nto] = l->to;
//	cfrom->lto[cfrom->nto] = e->nlinks;
//	cfrom->nto++;
//
//	cto->from[cto->nfrom] = l->from;
//	cto->lfrom[cto->nfrom] = e->nlinks;
//	cto->nfrom++;
//
//	e->nlinks++;
//	nlinks_a++;
//    }
//    if (links_a != NULL)
//	free(links_a);
//
//    if (verbose)
//	printf("%d user specified exchange(s) added\n", nlinks_a);
//
//    read_link_weights(f, e);
//
//    set_fixed_links(f, e);
//
//    for (i = 0; i < nl; ++i)
//	describe_link(e, lindices[i]);
//
//    fflush(stdout);
//}
//
//static void read_tracers(FILE* f, tr2ex* e)
//{
//    errfn errfn_orig = keyprm_errfn;
//    int fid = e->meco->fid;
//    char* fname = e->meco->fname;
//    char buf[512];
//    char key[512];
//    int i;
//
//    readkeyprm_i(f, "ntracers", &e->ntracers);
//    e->tracers = malloc(e->ntracers * sizeof(trinfo));
//
//    if (verbose)
//	printf("%d tracers\n", e->ntracers);
//
//    for (i = 0; i < e->ntracers; ++i) {
//	trinfo* t = &e->tracers[i];
//
//	t->index = i;
//
//	snprintf(key, sizeof(key), "tracer%d.name", i);
//	readkeyprm_s(f, key, buf);
//	t->name = strdup(buf);
//
//	/* optional parameters */
//	set_keyprm_errfn(quiet);
//	snprintf(key, sizeof(key), "tracer%d.weight", i);
//	if (!(readkeyprm_d(f, key, &t->weight)))
//	    t->weight = 1.0;
//	snprintf(key, sizeof(key), "tracer%d.scale", i);
//	if (!(readkeyprm_d(f, key, &t->scale)))
//	    t->scale = 1.0;
//
//	set_keyprm_errfn(errfn_orig);
//    }
//
//    if (verbose)
//	for (i = 0; i < e->ntracers; ++i) {
//	    trinfo* t = & e->tracers[i];
//	    printf("  tracer%d: name = %s, weight = %.3g, scaling factor = %.3g\n", i, t->name, t->weight, t->scale);
//	}
//
//    /* each tracer must ne found in the MECO netcdf file */
//    for (i = 0; i < e->ntracers; ++i) {
//	trinfo* t = &e->tracers[i];
//	int ndims;
//	NC_inq_varid(fname, fid, t->name, &t->id_meco);
//	NC_inq_varndims(fname, fid, t->id_meco, &ndims);
//	if (ndims != 4)
//	    quit("error: %s: \"%s\" not a 4-dimensional variable\n", fname, t->name);
//    }
//    fflush(stdout);
//}
//
//static void read_forcing(char* fname, tr2ex* e)
//{
//    FILE* f = fopen(fname, "r");
//    errfn warn_orig = warn;
//    errfn errfn_orig = keyprm_errfn;
//    char buf[512];
//
//    if (f == NULL)
//	quit("error: %s: %s\n", fname, strerror(errno));
//
//    set_keyprm_warnfn(quiet);
//    readPointSourceSink(fname, e->t_units, &e->pss, &e->npss, e, xyz2cell, find_trindex);
//    set_keyprm_warnfn(warn_orig);
//
//    set_keyprm_errfn(quiet);
//    if (readkeyprm_s(f, "Precipitation", buf)) {
//	e->precipitation = malloc(sizeof(TimeSeries));
//	tsRead(bm->inputFolder, buf, e->precipitation);
//	if (strcmp(e->precipitation->t_units, e->t_units) != 0)
//	    tsNewTimeUnits(e->precipitation, e->t_units);
//	e->precip_id = tsIndex(e->precipitation, "precipitation");
//	if (e->precip_id < 0)
//	    quit("error: %s: \"precipitation\" variable not found\n", e->precipitation->name);
//    } else if (verbose)
//	printf("warning: no precipitation\n");
//    set_keyprm_errfn(errfn_orig);
//
//    set_keyprm_errfn(quiet);
//    if (readkeyprm_s(f, "Evaporation", buf)) {
//	e->evaporation = malloc(sizeof(TimeSeries));
//	tsRead(bm->inputFolder, buf, e->evaporation);
//	if (strcmp(e->evaporation->t_units, e->t_units) != 0)
//	    tsNewTimeUnits(e->evaporation, e->t_units);
//	e->evap_id = tsIndex(e->evaporation, "evaporation");
//	if (e->precip_id < 0)
//	    quit("error: %s: \"evaporation\" variable not found\n", e->evaporation->name);
//    } else if (verbose)
//	printf("warning: no evaporation\n");
//    set_keyprm_errfn(errfn_orig);
//    fflush(stdout);
//}
//
//static void read_weights(FILE* f, tr2ex* e)
//{
//    errfn errfn_orig = keyprm_errfn;
//
//    readkeyprm_d(f, "scale_exchanges", &e->scale_exchanges);
//    readkeyprm_d(f, "scale_tracers", &e->scale_tracers);
//    readkeyprm_d(f, "weight_tracers", &e->weight_tracers);
//
//    set_keyprm_errfn(quiet);
//    readkeyprm_i(f, "volpower", &e->volpower);
//    readkeyprm_d(f, "volfrac_max", &e->volfrac_max);
//    set_keyprm_errfn(errfn_orig);
//
//    if (verbose) {
//	printf("scaling factor for exchanges = %.3g\n", e->scale_exchanges);
//	printf("scaling factor for tracers = %.3g\n", e->scale_tracers);
//	printf("weighting factor for tracers = %.3g\n", e->weight_tracers);
//	printf("weighting by volume power = %d\n", e->volpower);
//    }
//    fflush(stdout);
//}
//
//static void set_time(FILE* f, tr2ex* e)
//{
//    char buf[512];
//    double dtscale;
//    errfn errfn_orig = keyprm_errfn;
//
//    readkeyprm_d(f, "tstart", &e->tstart);
//
//    set_keyprm_errfn(quiet);
//    if (!readkeyprm_d(f, "toutstart", &e->toutstart))
//	e->toutstart = e->tstart;
//    set_keyprm_errfn(errfn_orig);
//
//    readkeyprm_d(f, "tstop", &e->tstop);
//    readkeyprm_d(f, "dt", &e->dt);
//    readkeyprm_s(f, "dt", buf);
//    dtscale = get_tscale(buf);
//    e->dt /= dtscale;
//    readkeyprm_s(f, "OUTtunits", buf); /* model time units = BM hd time units
//					*/
//    {
//	char hdu[512];
//	char bmu[512];
//	sscanf(e->bm->t_units, "%s", bmu);
//	sscanf(buf, "%s", hdu);
//	if (strcmp(hdu, bmu))
//	    quit("error: %s: to match BM, OUTtunits must be in %s\n", e->fname, bmu);
//    }
//    e->t_units = strdup(buf);
//    readkeyprm_s(f, "tunits", buf);
//    if (verbose) {
//	fprintf(stderr, "tstart = %.2f %s\n", e->tstart, buf);
//	fprintf(stderr, "tend = %.2f %s \n", e->tstop, buf);
//	fprintf(stderr, "dt = %.2f %s\n", e->dt, buf);
//    }
//    e->t_units_out = strdup(buf); /* screen output time units */
//    ChangeTimeUnits(e->t_units_out, e->t_units, &e->tstart, 1);
//    ChangeTimeUnits(e->t_units_out, e->t_units, &e->toutstart, 1);
//    ChangeTimeUnits(e->t_units_out, e->t_units, &e->tstop, 1);
//    e->dt *= get_tscale(e->t_units) / get_tscale(e->t_units_out);
//    fflush(stdout);
//}
//
//static void check_time_range(FILE* f, tr2ex* e)
//{
//    /* make sure that the requested time period is presented */
//    (void) get_meco_record_number(e, e->tstart);
//    (void) get_meco_record_number(e, e->tstop);
//}
//
//static void set_work_variables(tr2ex* e)
//{
//    int i;
//    e->trv0 = malloc(e->ntracers * sizeof(double*));
//    e->trv1 = malloc(e->ntracers * sizeof(double*));
//    e->dtrv = alloc2d(e->nacells,  e->ntracers);
//    for (i = 0; i < e->ntracers; ++i) {
//	e->trv0[i] = malloc(e->ncells * sizeof(double));
//	e->trv1[i] = malloc(e->ncells * sizeof(double));
//    }
//    e->cvol0 = malloc(e->ncells * sizeof(double));
//    e->cvol1 = malloc(e->ncells * sizeof(double));
//    e->nv = e->nlinks;
//    e->nc = e->nacells * 2 + e->nflinks;
//    e->nec = e->nacells + e->nflinks;
//    e->x = calloc(e->nv, sizeof(double));
//    e->vfrom = malloc(e->ncells * sizeof(double));
//    e->vto = malloc(e->ncells * sizeof(double));
//    e->dtrmax = malloc(e->ntracers * sizeof(double));
//    e->idtrmax = malloc(e->ntracers * sizeof(int));
//    e->Etotal = 0.0;
//    e->DTRtotal = calloc(e->ntracers, sizeof(double));
//}
//
//void init(char* fname, tr2ex* e)
//{
//    FILE* f = fopen(fname, "r");
//    char buf[512];
//    int i;
//
//    if (f == NULL)
//	quit("error: %s: %s\n", fname, strerror(errno));
//
//    init_tr2ex(e);
//
//    e->fname = strdup(fname);
//
//    /* a couple of optional flags */
//    set_keyprm_errfn(quiet);
//    readkeyprm_i(f, "BMuse_polyarea", &e->use_polyarea);
//    readkeyprm_i(f, "handle_tide", &e->handle_tide);
//    readkeyprm_i(f, "weights_into_E", &e->exchangeweights_into_E);
//    set_keyprm_errfn(quit);
//
//    /* some basic initializations from the BM dumpfile */
//    readkeyprm_s(f, "BMdumpfile", buf);
//    init_from_bm_dump(buf, e);
//
//    /* need to know the BM projection (somehow it is not stored anywhere in
//     * BM files) */
//    readkeyprm_s(f, "BMprojection", buf);
//    e->projection = strdup(buf);
//    //    if (verbose)
//	printf("projection = %s\n", e->projection);
//
//    set_keyprm_errfn(quiet);
//    if (readkeyprm_d(f, "period", &e->period))
//	e->periodic = 1;
//    set_keyprm_errfn(quit);
//
//    set_time(f, e);
//
//    if (e->periodic) {
//	double pscale;
//	char buf[512];
//	readkeyprm_s(f, "period", buf);
//	pscale = get_tscale(buf);
//	e->period /= pscale;
//	e->period *= get_tscale(e->t_units) / get_tscale(e->t_units_out);
//    }
//
//    /* connectivity */
//    set_cells(e);
//    set_links(f, e);
//
//    set_btscells(f, e);
//
//    /* some basic initializations from the MECO dump file */
//    readkeyprm_s(f, "MECOdumpfile", buf);
//    init_from_meco_dump(buf, e);
//
//    for (i = 0; i < nc; ++i)
//	describe_cell(e, cindices[i] >> 16, cindices[i] << 16 >> 16);
//
//    check_time_range(f, e);
//
//    read_weights(f, e);
//
//    read_tracers(f, e);
//
//    set_btscells(f, e);
//    check_boundary_cells(e);
//
//    readkeyprm_s(f, "BMforcingfile", buf);
//    read_forcing(buf, e);
//
//    set_work_variables(e);
//
//    if (e->handle_tide) {
//	readkeyprm_i(f, "remove_z_trend", &e->remove_z_trend);
//	if (e->remove_z_trend)
//	    calculate_dvol(e);
//    }
//
//    if (do_run)
//	create_exchange_dump(f, e);
//
//    fclose(f);
//
//    fflush(stdout);
//}

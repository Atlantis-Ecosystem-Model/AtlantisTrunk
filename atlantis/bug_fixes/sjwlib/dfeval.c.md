# sjwlib/dfeval.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-01](#sjw3-01) | 65-77, 105-119 | H/M | wrong index / buffer overrun | `dfReadRecords(df, v, before, after - before + 1); ... v->data[before] ... v->data[after]` — for netCDF datafi… |  | [ ] |
| [SJW3-02](#sjw3-02) | 264-266, 294-296 | M/L | buffer overrun (latent) | `int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS]; ... ncorners = 1 << nd` — with MAXNUMDIMS=10 the array… |  | [ ] |

---

## SJW3-01

**wrong index / buffer overrun** · Conf H / Sev M  
**Lines (current code):** 65-77, 105-119  
**Lines at review (Oct 2026):** 65-77, 105-119

**Why it's thought to be a bug**

`dfReadRecords(df, v, before, after - before + 1); ... v->data[before] ... v->data[after]` — for netCDF datafiles `dfReadRecords` (datafile.c:348-430) allocates a buffer of only `nrecs` (1 or 2) records starting at `v->start_record = before`, and `df_read_data` stores record r at `data[r - v->start_record]` (datafile.c:1505). Indexing with the absolute record number `before`/`after` therefore reads past the buffer for any record > 0 (and returns the wrong value). Compare `dfGetDataValue` (datafile.c:912) and `df_interp_1d_inv_weight` (line 227) which correctly subtract `v->start_record`. Only ASCII files work because `dfReadRecords` returns early and all records are in memory. Affects every 0-D netCDF time series evaluated via `tsEval`/`tsEvalR`/`tsEvalEx`. Functions: `dfEval()`, `dfEvalEx()`.

**Current code** (`sjwlib/dfeval.c` lines 63–102 (truncated))

```c
   63: 		/* NULL STATEMENT - Should take some action here */;
   64: 	}
   65: 	dfReadRecords(df, v, before, after - before + 1);
   66: 
   67: 	val = v->missing;
   68: 
   69: 	/* Value exactly at before record */
   70: 	if (frac == 0.0 && (fabs(v->data[before] - v->missing) > 1e-10)) {
   71: 		val = v->data[before];
   72: 		/* Value exactly at after record */
   73: 	} else if (frac == 1.0 && (fabs(v->data[after] - v->missing) > 1e-10)) {
   74: 		val = v->data[after];
   75: 		/* Value inbetween - need to interpolate */
   76: 	} else if ((fabs(v->data[after] - v->missing) > 1e-10) && (fabs(v->data[before] - v->missing) > 1e-10)) {
   77: 		val = v->data[before] * (1.0 - frac) + v->data[after] * frac;
   78: 	}
   79: 
   80: 	return (val);
   81: }
   82: 
   83: double dfEvalEx(Datafile *df, Variable *v, double r) {
   84: 	double frac;
   85: 	int before;
   86: 	int after;
   87: 	double val;
   88: 
   89: 	/* Sanity checks */
   90: 	if (df == NULL)
   91: 		quit("dfEvalEx: NULL Datafile pointer\n");
   92: 	if (v == NULL)
   93: 		quit("dfEvalEx: NULL Variable pointer\n");
   94: 	if (v->nd > 0)
   95: 		quit("dfEvalEx: Ambiguous data - it is multi-dimensional, use dfEvalCoords.\n");
   96: 
   97: 	if ((df->records == NULL) || (!v->dim_as_record))
   98: 		return v->data[0];
   99: 
  100: 	/* Find nearest records in table */
  101: 	if (dfFindRecord(df, r, &before, &after, &frac) == 0){
  102: 		/* Requested record not in range of those in file */
```

**Suggested fix (from review):** Use `v->data[before - v->start_record]` / `v->data[after - v->start_record]` (or `dfGetDataValue(df, v, before, NULL)`).

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## SJW3-02

**buffer overrun (latent)** · Conf M / Sev L  
**Lines (current code):** 264-266, 294-296  
**Lines at review (Oct 2026):** 264-266, 294-296

**Function:** `df_interp_linear()`

**Why it's thought to be a bug**

`int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS]; ... ncorners = 1 << nd` — with MAXNUMDIMS=10 the array has 99 rows, but `ncorners` = 2^nd exceeds 99 for nd >= 7, overrunning the stack array. Should be sized `1 << MAXNUMDIMS`. Function: `df_interp_linear()`.

**Current code** (`sjwlib/dfeval.c` lines 262–298)

```c
  262: 		int iindices[MAXNUMDIMS];
  263: 		int corner[MAXNUMDIMS];
  264: 		int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS];
  265: 		int dsize[MAXNUMDIMS];
  266: 		int ncorners = 1 << nd;
  267: 
  268: #if 0
  269: 		fprintf(stderr, "Coords =");
  270: 		for (i=0; i<v->csystnc; ++i)
  271: 		fprintf(stderr, " %g", coords[i]);
  272: 		fprintf(stderr, "\n");
  273: 
  274: 		fprintf(stderr, "Indices =");
  275: 		for (i=0; i<nd; ++i)
  276: 		fprintf(stderr, " %g", indices[i]);
  277: 		fprintf(stderr, "\n");
  278: #endif
  279: 
  280: 		/* Compute the fraction indices and integer indices.
  281: 		 * Trim at boundaries if necessary. */
  282: 		for (i = 0; i < nd; ++i) {
  283: 			dsize[i] = (int)df->dimensions[dimids[i]].size - 1;
  284: 			findices[i] = indices[i];
  285: 			if (findices[i] < 0)
  286: 				findices[i] = 0.0;
  287: 			if (findices[i] > dsize[i])
  288: 				findices[i] = (double) dsize[i];
  289: 			iindices[i] = (int) floor(findices[i]);
  290: 			findices[i] -= iindices[i];
  291: 		}
  292: 
  293: 		/* Create the corner indice offsets (0 or 1) */
  294: 		for (i = 0; i < ncorners; ++i)
  295: 			for (j = 0; j < nd; ++j)
  296: 				coffsets[i][j] = (i >> j) & 0x01;
  297: 
  298: 		/* Now step though all of the corners and perform
```

**Suggested fix (from review):** Declare `coffsets[1 << MAXNUMDIMS][MAXNUMDIMS]` or quit if nd > 6.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

dfeval.c `dfEvalCoords()`: when `dfFindRecord` returns an out-of-range record (r0==r1, rfrac=0) the result is fine, but `recvals[1]` is used with weight `rfrac` even when only one record was read; harmless because rfrac==0 there.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

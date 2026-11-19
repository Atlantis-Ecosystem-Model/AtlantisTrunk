# ateconomic/ateconts.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [QUOT-28](#quot-28) | 84-128 | M/L | partial initialisation | It allocates `nEconts` FisheryTimeSeries but reads only index `i=0`; entries 1..n-1 stay uninitialised. A nega… |  | [ ] |

---

## QUOT-28

**partial initialisation** · Conf M / Sev L  
**Lines (current code):** 84-128  
**Lines at review (Oct 2026):** 84-128

**Function:** `Read_Econ_Time_Series()`

**Why it's thought to be a bug**

It allocates `nEconts` FisheryTimeSeries but reads only index `i=0`; entries 1..n-1 stay uninitialised. A negative nEconts is not rejected before malloc. It is latent because the only caller (ateconomicsetup.c:503) uses `tsGDP[0]`. Function: `Read_Econ_Time_Series()`.

**Current code** (`ateconomic/ateconts.c` lines 82–121 (truncated))

```c
   82: 		fprintf(stderr, "Reading Economic Time Series %s\n", name);
   83: 
   84:     readkeyprm_i(fp,"nEconts",&nEconts);
   85:     if(!nEconts)
   86:         quit("Economic model currently expects a GDP file, if you don't have one please create a dummy file (filled with zeroes for example)\n");
   87: 
   88:     nnts = nEconts;
   89: 
   90: 	/* Allocate memory for list of time series */
   91:     if( (fts=(FisheryTimeSeries *)malloc((size_t)nnts*sizeof(FisheryTimeSeries))) == NULL )
   92:         quit("Read_Econ_Time_Series: Can't allocate memory for economic %s time series list\n", tsname);
   93: 
   94:     /** Read each economic time series input **/
   95: 	/* Location */
   96:     snprintf(key, sizeof(key), "%sts%d.location",tsname,i);
   97: 	readkeyprm_s(fp,key,buf);
   98: 	if( sscanf(buf,"%lf %lf",&fts[i].x,&fts[i].y) != 2 )
   99: 		quit("Incorrect information specified for %s, need x y format", key);
  100: 
  101: 	/* Box entry for FisheryTimeSeries template not actually needed for economic time
  102: 	   series as yet so set to default value of 0
  103: 	*/
  104: 	fts[i].b = 0;
  105: 
  106: 	/* Data */
  107:     snprintf(key, sizeof(key),"%sts%d.data",tsname,i);
  108: 
  109: 	readkeyprm_s(fp,key,buf);
  110:     tsRead(folderPath, buf,&fts[i].ts);
  111: 	if(verbose > 1)
  112: 		tsPrintInfo(&fts[i].ts,stderr);
  113: 
  114: 	/* Check data time units */
  115: 	if( strcmp(fts[i].ts.t_units,t_units) != 0 ) {
  116: 		if( verbose > 1) {
  117: 		  fprintf(stderr,"Converting %s time series time units\n",tsname);
  118: 		  fprintf(stderr,"    Was %s\n",fts[i].ts.t_units);
  119: 		  fprintf(stderr,"    Now %s\n",t_units);
  120: 		}
  121: 		tsNewTimeUnits(&fts[i].ts,t_units);
```

**Suggested fix (from review):** Loop i over nEconts (or allocate 1) and quit if `nEconts < 1`

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

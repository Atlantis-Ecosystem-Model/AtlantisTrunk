# sjwlib/netcdf.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-03](#sjw3-03) | 277 | H/L | format specifier mismatch | `quit("ncvarread: %s has %d bytes per value in file, code expects %d\n",n,size);` — three specifiers but two a… |  | [ ] |
| [SJW3-04](#sjw3-04) | 117, 138 | M/L | buffer overrun | `char aval[MAXLINELEN]; ... nc_get_att_text(fid,id,attr,aval)` — attribute length is never checked (no `nc_inq… |  | [ ] |

---

## SJW3-03

**format specifier mismatch** · Conf H / Sev L  
**Lines (current code):** 277  
**Lines at review (Oct 2026):** 277

**Function:** `ncvarread()`

**Why it's thought to be a bug**

`quit("ncvarread: %s has %d bytes per value in file, code expects %d\n",n,size);` — three specifiers but two args, and `%s` consumes the int `n` (crash/garbage on this error path). Function: `ncvarread()`.

**Current code** (`sjwlib/netcdf.c` lines 275–279)

```c
  275: 
  276:     if( (n=ncvarsize(fid,vid)) != size )
  277:         quit("ncvarread: %s has %d bytes per value in file, code expects %d\n",n,size);
  278:     nc_inq_vartype(fid,vid,&datatype);
  279: 
```

**Suggested fix (from review):** `quit("... %s has %d ... %d\n", name, n, size);`

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

## SJW3-04

**buffer overrun** · Conf M / Sev L  
**Lines (current code):** 117, 138  
**Lines at review (Oct 2026):** 117, 138

**Function:** `ncvarfind()`

**Why it's thought to be a bug**

`char aval[MAXLINELEN]; ... nc_get_att_text(fid,id,attr,aval)` — attribute length is never checked (no `nc_inq_attlen`), so a text attribute >= 4000 chars overflows the stack buffer; also a value of exactly MAXLINELEN chars is not NUL-terminated for the `strcmp`. Function: `ncvarfind()`.

**Current code** (`sjwlib/netcdf.c` lines 115–140)

```c
  115: 		int d[NC_MAX_VAR_DIMS];
  116: 		int na;
  117: 		char aval[MAXLINELEN];
  118: 		int attnum;
  119: 		int match;
  120: 
  121: 		match = 1;
  122: 		memset(aval,0,MAXLINELEN);
  123: 		/* Check number of dimensions */
  124: 	        nc_inq_var(fid,id,name,&dt,&nd,d,&na);
  125: 		if( (nvdims>0) && (nd!=nvdims) ) match = 0;
  126: 
  127: 		/* Check each dimension */
  128: 		for(i=0; vdims!=NULL && match && i<nd; i++)
  129: 		    if( (vdims[i]>=0) && (d[i]!=vdims[i]) ) match = 0;
  130: 
  131: 		/* Check attribute existence */
  132: 		if( match && (attr != NULL) &&
  133: 		    (nc_inq_attid(fid,id,attr, &attnum) != NC_NOERR) )
  134: 		    match = 0;
  135: 
  136: 		/* Check attribute value */
  137: 		if( match && (attr != NULL) && (attval != NULL)) {
  138: 		    if (nc_get_att_text(fid,id,attr,aval) == NC_NOERR) {
  139: 		       if (strcmp(aval,attval) != 0 )
  140: 		          match = 0;
```

**Suggested fix (from review):** Query `nc_inq_attlen` first and skip/limit if `len >= MAXLINELEN`.

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

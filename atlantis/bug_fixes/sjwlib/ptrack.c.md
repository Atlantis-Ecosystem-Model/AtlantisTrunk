# sjwlib/ptrack.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-05](#sjw3-05) | 57, 87 | H/L | type mismatch / stack corruption | `int n; ... nc_inq_dimlen(fid,p_did,(size_t *)&n);` writes an 8-byte `size_t` into a 4-byte `int` on 64-bit pl… |  | [ ] |
| [SJW3-06](#sjw3-06) | 154-158 | H/L | wrong logic / buffer overrun / NULL deref | `if( t ) nc_get_vara_double(..., count(=1), b); for(n = 0; n < rec; n++){ t[n] = (double)b[n]; }` — only one t… |  | [ ] |
| [SJW3-07](#sjw3-07) | 100 | M/L | format specifier mismatch | `quit("... (only %d records)\n",rec,name,nrec)` — `nrec` is `size_t` passed to `%d`. Function: `readparticles(… |  | [ ] |

## Reviewer notes for this file

(Not called anywhere in the Atlantis tree — all latent.)

---

## SJW3-05

**type mismatch / stack corruption** · Conf H / Sev L  
**Listed lines:** 57, 87

**Function:** `readparticles()`

**Why it's thought to be a bug**

`int n; ... nc_inq_dimlen(fid,p_did,(size_t *)&n);` writes an 8-byte `size_t` into a 4-byte `int` on 64-bit platforms, clobbering adjacent stack. Function: `readparticles()`.

**Current code** (`sjwlib/ptrack.c` lines 54–88)

```c
   54:     size_t start[2];
   55:     size_t count[2];
   56:     //size_t n;
   57:     int n;
   58:     int t_vid;
   59:     int x_vid;
   60:     int y_vid;
   61:     int z_vid;
   62:     int f_vid;
   63:     doubleINPUT *b;
   64:     short *f;
   65:     size_t nrec;
   66: 
   67:     /* Open the netCDF file */
   68:     if( (nc_open(name,NC_NOWRITE,&fid)) != NC_NOERR )
   69:     	quit("readparticles: Can't open %s\n",name);
   70:     
   71:     /* Inquire about this file */
   72:     if( nc_inq(fid,&ndims,&nvars,&natts,&recdim) == -1 )
   73:     	quit("readparticles: nc_inq failed\n");
   74:     if( ndims != 2 )
   75:     	quit("readparticles: not enough dimensions\n");
   76:     if( nvars < 5 )
   77:     	quit("readparticles: not enough variables\n");
   78: 
   79:     /* Check dimensions are as expected */
   80:     if( (nc_inq_dimid(fid,"t", &t_did)) != NC_NOERR )
   81:     	quit("readparticles: no t dimension");
   82:     if( t_did != recdim )
   83:     	quit("readparticles: t dimension not unlimited\n");
   84:     if( (nc_inq_dimid(fid,"n",&p_did)) != NC_NOERR )
   85:     	quit("readparticles: no n dimension");
   86:     /* Get number of particles */
   87:     nc_inq_dimlen(fid,p_did,(size_t *)&n);
   88: 
```

**Suggested fix (from review):** Use a `size_t` temporary and cast to int afterwards.

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

## SJW3-06

**wrong logic / buffer overrun / NULL deref** · Conf H / Sev L  
**Listed lines:** 154-158

**Function:** `readparticles()`

**Why it's thought to be a bug**

`if( t ) nc_get_vara_double(..., count(=1), b); for(n = 0; n < rec; n++){ t[n] = (double)b[n]; }` — only one time value is read, but the loop writes `rec` values into `t` (a pointer to a single double per the API doc), reading uninitialised `b[1..]`; the loop is also outside the `if(t)`, so `t==NULL` with `rec>0` dereferences NULL, and for `rec==0` `*t` is never set. Function: `readparticles()`.

**Current code** (`sjwlib/ptrack.c` lines 152–160)

```c
  152:     start[0] = (size_t)rec;
  153:     count[0] = 1;
  154:     if( t )
  155: 	nc_get_vara_double(fid,t_vid,start,count, (doubleINPUT *)b);
  156:     for(n = 0; n < rec; n++){
  157:     	t[n] = (double)b[n];
  158:     }
  159:     if( t_units )
  160: 	nc_get_att_text(fid,t_vid,"units",t_units);
```

**Suggested fix (from review):** Replace loop with `if (t) *t = b[0];` inside the `if(t)` block.

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

## SJW3-07

**format specifier mismatch** · Conf M / Sev L  
**Listed lines:** 100

**Function:** `readparticles()`

**Why it's thought to be a bug**

`quit("... (only %d records)\n",rec,name,nrec)` — `nrec` is `size_t` passed to `%d`. Function: `readparticles()`.

**Current code** (`sjwlib/ptrack.c` lines 98–102)

```c
   98:     nc_inq_dimlen(fid,t_did,&nrec);
   99:     if( rec >= (int)nrec )
  100:     	quit("readparticles: Record %d not in %s (only %d records)\n",rec,name,nrec);
  101:     if( ndump )
  102: 	*ndump = (int)nrec;
```

**Suggested fix (from review):** Cast `(int)nrec`.

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

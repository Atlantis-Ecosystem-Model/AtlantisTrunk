# atSS3Link/atSS3Util.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-41](#ss3-41) | 73-76 | H/M | off-by-one (Fortran port) | `a1 = (int)(ran3*np + 1)` gives the range 1..np: `dum[0]` is never chosen and `dum[np]` (unfilled, 0) can be. … |  | [ ] |
| [SS3-42](#ss3-42) | 36-39, 45-49, 64 | M/L | leak / div0 / fixed buffer | `dum` is allocated before the `tc==0` early return, which leaks it. `tot==0` produces NaN props. `dum[15000]` … |  | [ ] |
| [SS3-43](#ss3-43) | 235-241 | H/L | off-by-one (NR port) | In the 0-based port, `ii=0` serves both as the "not started" flag and as a valid index: if the first nonzero i… |  | [ ] |
| [SS3-44](#ss3-44) | 165, 174-177 | M/L | leak / div0 | `VV` is never freed, and on a singular row `VV[i] = 1.0/big` is computed with big==0 after only a printf. Late… |  | [ ] |

---

## SS3-41

**off-by-one (Fortran port)** · Conf H / Sev M  
**Listed lines:** 73-76

**Function:** `GenMnomial()`

**Why it's thought to be a bug**

`a1 = (int)(ran3*np + 1)` gives the range 1..np: `dum[0]` is never chosen and `dum[np]` (unfilled, 0) can be. `if (a>0) c[a] += 1` then discards category 0 (age 0 or the first length bin) completely; the 0-means-empty sentinel only made sense with 1-based indexing. Function: `GenMnomial()`.

**Current code** (`atSS3Link/atSS3Util.c` lines 71–78)

```c
   71:         a = 0;
   72: 		do{
   73: 			a1 = (int) (ran3(iseed)*(float)np+1);
   74: 			a = dum[a1];
   75: 		} while (a<=0 && withrep);
   76: 		if (a>0) c[a] += 1;   // add if (a>0) -  a can be 0 if proportion=0 8.Apr.08
   77: 		if (withrep) dum[a1] = -1;
   78: 	}
```

**Suggested fix (from review):** `a1 = (int)(ran3*np)`. Use -1 as the empty sentinel and accept `a >= 0`

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

## SS3-42

**leak / div0 / fixed buffer** · Conf M / Sev L  
**Listed lines:** 36-39, 45-49, 64

**Function:** `GenMnomial()`

**Why it's thought to be a bug**

`dum` is allocated before the `tc==0` early return, which leaks it. `tot==0` produces NaN props. `dum[15000]` is fixed while np can reach about 10000+ndim/2, with no bound check. Function: `GenMnomial()`.

**Current code** (`atSS3Link/atSS3Util.c` lines 30–62) — code appears to have moved -4 lines since the review

```c
   30: //
   31: // ******************************************************************************
   32: void GenMnomial(double *prop, int ndim, int tc, double *c, int *iseed, int withrep)
   33: {
   34: 	int i,j,icount,np,a,a1;
   35: 	double tot,ntot;
   36: 	int *dum = Util_Alloc_Init_1D_Int(15000, 0);
   37: 
   38:  	// check if some catches were made
   39: 	if (tc==0) return;
   40: 
   41: 	Util_Init_1D_Double(c, ndim, 0.0);
   42: 
   43: 	// first total the probabilities and then normalise
   44: 	tot = 0.0;
   45: 	for (i=0;i<ndim;i++)
   46: 		tot = tot + prop[i];
   47: 
   48: 	for (i=0;i<ndim;i++)
   49: 		prop[i] = prop[i]/tot;
   50: 
   51: 	// check whether this is stochastic
   52: 	// don't put this code because not used ?
   53: 
   54: 	// adjust to handle 'without-replacement sampling'
   55: 	ntot = 10000.0;
   56: 	if (withrep && tot <14000) ntot = tot;
   57: 
   58: 	// do special set up for index matrix
   59: 	np = 0;
   60: 	for (i=0;i<ndim;i++){
   61: 		icount = (int) (prop[i]*ntot+0.5);
   62: 		if (icount>0){
```

**Suggested fix (from review):** Allocate after the check, free on all paths, guard tot, bound np

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

## SS3-43

**off-by-one (NR port)** · Conf H / Sev L  
**Listed lines:** 235-241

**Function:** `LUBKSB()`

**Why it's thought to be a bug**

In the 0-based port, `ii=0` serves both as the "not started" flag and as a valid index: if the first nonzero is at i=0, forward substitution never starts. The loop `j < (i-1)` should be `j < i`. INVM gives wrong inverses. INVM/LUBKSB are currently never called (latent). Function: `LUBKSB()`.

**Current code** (`atSS3Link/atSS3Util.c` lines 225–235) — code appears to have moved -8 lines since the review

```c
  225: //
  226: // ******************************************************************************
  227: void LUBKSB(double **AA,int n, int *INDX, double *B) {
  228:     int i,ii=0,ip,j;
  229:     double sum;
  230: 
  231:     for ( i=0; i < n; i++) {
  232:         ip = INDX[i];
  233:         sum = B[ip];
  234:         B[ip] = B[i];
  235:         if (ii) {
```

**Suggested fix (from review):** Use `ii=-1` as the flag and `for (j=ii; j<i; j++)`

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

## SS3-44

**leak / div0** · Conf M / Sev L  
**Listed lines:** 165, 174-177

**Function:** `LUDCMP()`

**Why it's thought to be a bug**

`VV` is never freed, and on a singular row `VV[i] = 1.0/big` is computed with big==0 after only a printf. Latent (unused). Function: `LUDCMP()`.

**Current code** (`atSS3Link/atSS3Util.c` lines 157–173) — code appears to have moved -6 lines since the review

```c
  157: //
  158: // ******************************************************************************
  159: void LUDCMP (double **AA, int n, int *INDX, double *d){
  160:     int i, j, k, imax = 0;
  161:     double tiny = 1.0e-20;
  162:     double big, dum, sum, temp;
  163:     double *VV;
  164: 
  165:     VV = Util_Alloc_Init_1D_Double(n, 0.0);
  166: 
  167:     *d = 1.0;
  168:     for( i=0; i < n; i++) {
  169:         big = 0.;
  170:         for( j=0; j < n; j++) {
  171:             if ((temp=fabs(AA[i][j])) > big)
  172:             	big = temp;
  173:         }
```

**Suggested fix (from review):** `free1d(VV)`. Return or quit on a singular matrix

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

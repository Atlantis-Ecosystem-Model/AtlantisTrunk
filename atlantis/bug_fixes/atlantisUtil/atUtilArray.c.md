# atlantisUtil/atUtilArray.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-08](#utl2-08) | 142, 166 | H/M | prototype/definition type mismatch | The definitions are `Util_Alloc_Init_3D_Long_Double(int,int,int, double value)` and `Util_Alloc_Init_2D_Long_D… |  | [ ] |
| [UTL2-09](#utl2-09) | 243 | H/L | prototype mismatch | The definition is `Util_Init_5D_Int(int *****array, ...)` but atUtilLib.h:112 declares `int ****array`. The po… |  | [ ] |
| [UTL2-10](#utl2-10) | 396-403, 439-444 | M/L | buffer overrun (latent) | The copy runs if ANY new dim is larger, then copies the full old extent in every dimension: `for(i<olddim3) fo… |  | [ ] |

---

## UTL2-08

**prototype/definition type mismatch** · Conf H / Sev M  
**Listed lines:** 142, 166

**Why it's thought to be a bug**

The definitions are `Util_Alloc_Init_3D_Long_Double(int,int,int, double value)` and `Util_Alloc_Init_2D_Long_Double(int,int, double value)`, but atUtilLib.h:102-103 declares the last argument as `long double value`. atUtilArray.c doesn't include atUtilLib.h, so the compiler never catches this. Callers (e.g. atlantisUtil/atUtilGroupIO.c:821-822, 894-896, which include atUtilLib.h) pass a long double. On x86-64 Linux, long double goes on the stack and double in xmm0, so the callee reads a garbage init value and arrays such as `preyEaten` and `ratioLost` may not start at 0. On ARM64 macOS long double == double, so the problem stays hidden there. This is undefined behaviour. Functions: `Util_Alloc_Init_2D_Long_Double()`, `Util_Alloc_Init_3D_Long_Double()`.

**Current code** (`atlantisUtil/atUtilArray.c` lines 140–168)

```c
  140: }
  141: 
  142: long double ***Util_Alloc_Init_3D_Long_Double(int dim1, int dim2, int dim3, double value)
  143: {
  144: 	long double ***array;
  145: 
  146: 	//printf("Creating long double array with dimensions %d, %d, %d\n", dim1, dim2, dim3);
  147: 
  148: 	array = (long double ***)d_alloc3longd(dim1, dim2, dim3);
  149: 	Util_Init_3D_Long_Double(array, dim3, dim2, dim1, value);
  150: 
  151: 	return array;
  152: }
  153: 
  154: double **Util_Alloc_Init_2D_Double(int dim1, int dim2,  double value)
  155: {
  156: 	double **array;
  157: 
  158: 	//printf("Creating double array with dimensions %d, %d\n", dim1, dim2);
  159: 
  160: 	array = (double **)alloc2d(dim1, dim2);
  161: 	Util_Init_2D_Double(array, dim2, dim1, value);
  162: 
  163: 	return array;
  164: }
  165: 
  166: long double **Util_Alloc_Init_2D_Long_Double(int dim1, int dim2,  double value)
  167: {
  168: 	long double **array;
```

**Suggested fix (from review):** Change both definitions to `long double value`, and include atUtilLib.h in atUtilArray.c so the compiler checks prototypes.

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

## UTL2-09

**prototype mismatch** · Conf H / Sev L  
**Listed lines:** 243

**Function:** `Util_Init_5D_Int()`

**Why it's thought to be a bug**

The definition is `Util_Init_5D_Int(int *****array, ...)` but atUtilLib.h:112 declares `int ****array`. The pointer ABI is the same and there are no current callers, so this is latent; including atUtilLib.h (see UTL2-08) would turn it into a compile error. Function: `Util_Init_5D_Int()`.

**Current code** (`atlantisUtil/atUtilArray.c` lines 241–245)

```c
  241: 
  242: 
  243: void Util_Init_5D_Int(int *****array, int dim1, int dim2, int dim3, int dim4, int dim5, int value)
  244: {
  245: 	int i, j, k, l, m;
```

**Suggested fix (from review):** Fix the header to `int *****array`.

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

## UTL2-10

**buffer overrun (latent)** · Conf M / Sev L  
**Listed lines:** 396-403, 439-444

**Why it's thought to be a bug**

The copy runs if ANY new dim is larger, then copies the full old extent in every dimension: `for(i<olddim3) for(j<olddim2) for(k<olddim1) array[i][j][k] = oldarray[i][j][k]`. If one dim grows while another shrinks, it writes past the new allocation. It also quits when all dims are equal. Util_ReAlloc_2D_Int has the same issue. There are no current callers. Functions: `Util_ReAlloc_3D_Double()`, `Util_ReAlloc_2D_Int()`.

**Current code** (`atlantisUtil/atUtilArray.c` lines 394–433 (truncated))

```c
  394: 	array = Util_Alloc_Init_3D_Double(newdim1, newdim2, newdim3, value);
  395: 
  396: 	if (( newdim1 > olddim1 ) || ( newdim2 > olddim2 ) || ( newdim3 > olddim3 )){
  397: 		for(i = 0; i < olddim3; i++){
  398: 			for(j = 0; j < olddim2; j++){
  399: 				for(k = 0; k < olddim1; k++){
  400: 					array[i][j][k] = oldarray[i][j][k];
  401: 				}
  402: 			}
  403: 		}
  404: 
  405: 	} else {
  406: 		quit("Util_ReAlloc_3D_Double: Should we realloc if the dimension is smaller?\n");
  407: 	}
  408: 	free3d(oldarray);
  409: 
  410: 	return array;
  411: }
  412: 
  413: int *Util_ReAlloc_1D_Int(int newdim, int olddim, int *oldarray, int value)
  414: {
  415: 	int i;
  416: 	int *array;
  417: 	
  418: 	array = Util_Alloc_Init_1D_Int(newdim, value);
  419: 
  420: 	if ( newdim > olddim ) {
  421: 		for(i = 0; i < olddim; i++){
  422: 			array[i]= oldarray[i];
  423: 		}
  424: 	} else {
  425: 		quit("Util_ReAlloc_1D_Int: Should we realloc if the dimension is smaller?\n");
  426: 	}
  427: 	i_free1d(oldarray);
  428: 
  429: 	return array;
  430: }
  431: 
  432: int **Util_ReAlloc_2D_Int(int newdim1, int olddim1, int newdim2, int olddim2, int **oldarray, int value)
  433: {
```

**Suggested fix (from review):** Loop to `min(old,new)` in each dimension, and only quit when no dimension grows.

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

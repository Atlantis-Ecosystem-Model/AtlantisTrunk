# atlantisUtil/atUtilArray.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-08](#utl2-08) | 142, 166 | H/M | prototype/definition type mismatch | The definitions are `Util_Alloc_Init_3D_Long_Double(int,int,int, double value)` and `Util_Alloc_Init_2D_Long_D… |  | [ ] |
| [UTL2-09](#utl2-09) | 243, 260 | H/L | prototype mismatch | The definition is `Util_Init_5D_Int(int *****array, ...)` but atUtilLib.h:112 declares `int ****array`. The po… |  | [ ] |
| [UTL2-10](#utl2-10) | 389 | M/L | buffer overrun (latent) | The copy runs if ANY new dim is larger, then copies the full old extent in every dimension: `for(i<olddim3) fo… |  | [ ] |

---

## UTL2-08

**prototype/definition type mismatch** · Conf H / Sev M  
**Found at (current code):** `atUtilArray.c` 142, `atUtilArray.c` 166
**Lines at review (Oct 2026):** 142, 166

**Why it's thought to be a bug**

The definitions are `Util_Alloc_Init_3D_Long_Double(int,int,int, double value)` and `Util_Alloc_Init_2D_Long_Double(int,int, double value)`, but atUtilLib.h:102-103 declares the last argument as `long double value`. atUtilArray.c doesn't include atUtilLib.h, so the compiler never catches this. Callers (e.g. atlantisUtil/atUtilGroupIO.c:820-821, 893-895, which include atUtilLib.h) pass a long double. On x86-64 Linux, long double goes on the stack and double in xmm0, so the callee reads a garbage init value and arrays such as `preyEaten` and `ratioLost` may not start at 0. On ARM64 macOS long double == double, so the problem stays hidden there. This is undefined behaviour. Functions: `Util_Alloc_Init_2D_Long_Double()`, `Util_Alloc_Init_3D_Long_Double()`.

**Code at issue** — `atUtilArray.c` line 142 (closest match to the quoted code; wording differs slightly)  
Search for: `long double ***Util_Alloc_Init_3D_Long_Double(int dim1, int dim2, int dim3, double value)`

```c
  140: }
  141: 
  142→ long double ***Util_Alloc_Init_3D_Long_Double(int dim1, int dim2, int dim3, double value)
  143: {
  144: 	long double ***array;
```

**Code at issue** — `atUtilArray.c` line 166 (closest match to the quoted code; wording differs slightly)  
Search for: `long double **Util_Alloc_Init_2D_Long_Double(int dim1, int dim2,  double value)`

```c
  164: }
  165: 
  166→ long double **Util_Alloc_Init_2D_Long_Double(int dim1, int dim2,  double value)
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
**Found at (current code):** `atUtilArray.c` 243, `atUtilArray.c` 260
**Lines at review (Oct 2026):** 243

**Function:** `Util_Init_5D_Int()`

**Why it's thought to be a bug**

The definition is `Util_Init_5D_Int(int *****array, ...)` but atUtilLib.h:112 declares `int ****array`. The pointer ABI is the same and there are no current callers, so this is latent; including atUtilLib.h (see UTL2-08) would turn it into a compile error. Function: `Util_Init_5D_Int()`.

**Code at issue** — `atUtilArray.c` line 243  
Search for: `Util_Init_5D_Int(int *****array,`

```c
  241: 
  242: 
  243→ void Util_Init_5D_Int(int *****array, int dim1, int dim2, int dim3, int dim4, int dim5, int value)
  244: {
  245: 	int i, j, k, l, m;
```

**Code at issue** — `atUtilArray.c` line 260  
Search for: `int ****array`

```c
  258: }
  259: 
  260→ void Util_Init_4D_Int(int ****array, int dim1, int dim2, int dim3, int dim4, int value)
  261: {
  262: 	int i, j, k, l;
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
**Found at (current code):** `atUtilArray.c` 389
**Lines at review (Oct 2026):** 396-403, 439-444

**Why it's thought to be a bug**

The copy runs if ANY new dim is larger, then copies the full old extent in every dimension: `for(i<olddim3) for(j<olddim2) for(k<olddim1) array[i][j][k] = oldarray[i][j][k]`. If one dim grows while another shrinks, it writes past the new allocation. It also quits when all dims are equal. Util_ReAlloc_2D_Int has the same issue. There are no current callers. Functions: `Util_ReAlloc_3D_Double()`, `Util_ReAlloc_2D_Int()`.

**Code at issue** — `atUtilArray.c` line 389 (located by name only; check this is the right place)  
Search for: `double ***Util_ReAlloc_3D_Double(int newdim1, int olddim1, int newdim2, int olddim2, int n`

```c
  387: }
  388: 
  389→ double ***Util_ReAlloc_3D_Double(int newdim1, int olddim1, int newdim2, int olddim2, int newdim3, int olddim3, double ***oldarray, double value)
  390: {
  391: 	int i, j, k;
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

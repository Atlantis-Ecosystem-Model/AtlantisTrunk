# sjwlib/memory.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW1-01](#sjw1-01) | 46, 86, 51 | H/L | format specifier | `quit("...size %d, %d\n", n1, n2)` where `n1..n6` are `long int`, and `quit("...size = %d\n", size)` where `si… |  | [ ] |
| [SJW1-02](#sjw1-02) | 609, 191, 283 | M/M | zero-size / free mismatch | The allocators accept a dimension of 0 (they only reject `n<0`). If an outer dimension is 0 (e.g. `d_alloc2d(n… |  | [ ] |
| [SJW1-03](#sjw1-03) | 683, 156 | L/L | integer overflow | `size = (size_t)(n1 * n2 * n3) * sizeof(double)`: the product is formed in `long` before the cast. That is fin… |  | [ ] |
| [SJW1-04](#sjw1-04) | 308 | H/L | copy-paste (message) | The 4d free routines report `"free3d: attempt to free NULL pointer"` for the main-storage check (copied from t… |  | [ ] |

---

## SJW1-01

**format specifier** · Conf H / Sev L  
**Found at (current code):** `memory.c` 46, `memory.c` 86, `memory.c` 51
**Lines at review (Oct 2026):** 46, 51, 86, 150, 232, 336, 456, 574, 579, 616, 680, 762, 865, 985, 1104, 1145, 1210, 1291, 1384, 1426, 1490, 1564, 1606, 1669, 1751, 1845, 1885, 1948, 2024, 2110, 2149, 2212, 2288

**Why it's thought to be a bug**

`quit("...size %d, %d\n", n1, n2)` where `n1..n6` are `long int`, and `quit("...size = %d\n", size)` where `size` is `size_t`. `%d` with a 64-bit argument is undefined behaviour and prints the wrong value on LP64, so the error message reports the wrong size just when someone needs it to debug a bad allocation. Functions: every `*_alloc1d..6d` / `d_alloc*longd`.

**Code at issue** — `memory.c` line 46  
Search for: `quit("`

```c
   44: 
   45: 	if (n1 < 0)
   46→ 		quit("alloc1d: you have asked to allocate an array of size %d\n", n1);
   47: 
   48: 	/* allocate storage */
```

**Code at issue** — `memory.c` line 86  
Search for: `size %d, %d\n", n1, n2)`

```c
   84: 
   85: 	if (n1 < 0 || n2 < 0)
   86→ 		quit("alloc2longd: you have asked to allocate an array of size %d, %d\n", n1, n2);
   87: 
   88: 	/* first allocate main storage */
```

**Code at issue** — `memory.c` line 51  
Search for: `size = %d\n", size)`

```c
   49: 	size = (size_t)n1 * sizeof(long double);
   50: 	if ((p = (long double *) malloc(size)) == NULL)
   51→ 		quit("alloc1d: Not enough memory size = %d\n", size);
   52: 	for (i = 0; i < n1; i++)
   53: 		p[i] = 0.0;
```

**Suggested fix (from review):** Use `%ld` for the `long` dims and `%zu` for `size`.

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

## SJW1-02

**zero-size / free mismatch** · Conf M / Sev M  
**Found at (current code):** `memory.c` 609, `memory.c` 191, `memory.c` 283
**Lines at review (Oct 2026):** 118, 191-193, 283-287, 395-401, 523-531, 648, 721-723, 813-817, 924-930, 1052-1060, 1178, 1250-1252, 1342-1346, 1458, 1531-1533, 1637, 1710-1712, 1802-1806, 1916, 1987-1989, 2073-2077, 2180, 2251-2253, 2337-2341

**Why it's thought to be a bug**

The allocators accept a dimension of 0 (they only reject `n<0`). If an outer dimension is 0 (e.g. `d_alloc2d(n1,0)`, or `n2`/`n3` == 0 for 3d), the pointer block is `malloc(0)` and none of its entries are set. The matching free then reads `pp[0]` / `ppp[0][0]` / `pppp[0][0][0]` out of that zero-byte block and passes the garbage to `free()`: a heap over-read and an invalid free (crash at shutdown). On platforms where `malloc(0)` returns NULL the allocation instead quits with a false "Not enough memory". Functions: all `*_free2d..6d` paired with the `*_alloc2d..6d` routines.

**Code at issue** — `memory.c` line 609 (closest match to the quoted code; wording differs slightly)  
Search for: `d_alloc2d(long int n1, long int n2) {`

```c
  607:  */
  608: double **
  609→ d_alloc2d(long int n1, long int n2) {
  610: 	double *p;
  611: 	double **pp;
```

**Code at issue** — `memory.c` line 191  
Search for: `ppp[0][0]`

```c
  189: 	}
  190: 	/* extract pointer to main storage */
  191→ 	p = ppp[0][0];
  192: 	/* extract pointer to row pointer storage */
  193: 	pp = ppp[0];
```

**Code at issue** — `memory.c` line 283  
Search for: `pppp[0][0][0]`

```c
  281: 	}
  282: 	/* extract pointer to main storage */
  283→ 	p = pppp[0][0][0];
  284: 	/* extract pointer to row pointer storage */
  285: 	pp = pppp[0][0];
```

**Suggested fix (from review):** Either reject zero dims in the allocators, or store the base pointers so the frees don't dereference empty blocks (e.g. always allocate `max(n,1)` pointer slots and set them).

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

## SJW1-03

**integer overflow** · Conf L / Sev L  
**Found at (current code):** `memory.c` 683, `memory.c` 156
**Lines at review (Oct 2026):** 153-160, 235-250, 339-362, 459-490, 619, 683-690, 765-780, 868-891, 988-1019, 1148, 1213-1220, 1294-1309, 1429, 1493-1500, 1609, 1672-1679, 1754-1769, 1888, 1951-1958, 2027-2042, 2152, 2215-2222, 2291-2306

**Why it's thought to be a bug**

`size = (size_t)(n1 * n2 * n3) * sizeof(double)`: the product is formed in `long` before the cast. That is fine on LP64 (macOS/Linux), but on LLP64 Windows (the code has `WIN32` branches) `long` is 32-bit, so more than 2^31 elements overflow and under-allocate. The loop bound `i < n1*n2*n3` overflows the same way. Only `d_alloc2longd` (line 89) casts each factor correctly. Functions: all typed `*_alloc2d..6d`.

**Code at issue** — `memory.c` line 683  
Search for: `size = (size_t)(n1 * n2 * n3) *sizeof(double)`

```c
  681: 
  682: 	/* first allocate main storage */
  683→ 	size = (size_t)(n1 * n2 * n3) *sizeof(double);
  684: 	if ((p = (double *) malloc(size)) == NULL)
  685: 		quit("alloc3d: Not enough memory\n");
```

**Code at issue** — `memory.c` line 156  
Search for: `i < n1 * n2 * n3`

```c
  154: 	if ((p = (long double *) malloc(size)) == NULL)
  155: 		quit("alloc3d: Not enough memory\n");
  156→ 	for (i = 0; i < n1 * n2 * n3; i++)
  157: 		p[i] = 0.0;
  158: 
```

**Suggested fix (from review):** Cast each factor: `(size_t)n1 * (size_t)n2 * (size_t)n3`, and use a `size_t` loop index.

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

## SJW1-04

**copy-paste (message)** · Conf H / Sev L  
**Found at (current code):** `memory.c` 308
**Lines at review (Oct 2026):** 308, 1827, 2093

**Why it's thought to be a bug**

The 4d free routines report `"free3d: attempt to free NULL pointer"` for the main-storage check (copied from the 3d version). Functions: `d_free4longd()`, `i_free4d()`, `s_free4d()`.

**Code at issue** — `memory.c` line 308 (closest match to the quoted code; wording differs slightly)  
Search for: `quit("free3d: attempt to free NULL pointer\n");`

```c
  306: 	if (checkMemory) {
  307: 		if (p == NULL)
  308→ 			quit("free3d: attempt to free NULL pointer\n");
  309: 	}
  310: 	free((void *) p);
```

**Suggested fix (from review):** Change the text to `free4d` / `free4longd`.

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

memory.c: several pointer-block `sizeof`s name the wrong pointer level (e.g. `sizeof(long double *)` for the `long double ****` cube block at lines 258/362/370, and the equivalents in `d_alloc4d` 788, `d_alloc5d` 891/899, `f_alloc4d` 1317, `i_alloc4d` 1777, `s_alloc4d` 2050, `c_alloc4d` 2314, `d_alloc6*` 482/490/498/1011/1019/1027). All data-pointer types are the same size on supported platforms, so these are not functional bugs. The zero-initialisation, strides (`p + i*n1`, `pp + i*n2`, ...) and free order were checked for every typed variant and are correct. `checkMemory` is honoured inconsistently (`d_free1longd`, `*_free6*`, `s_free1d/3d/4d`, `c_free1d/3d/4d` always quit on NULL), but `checkMemory = 1` by default, so the behaviour is the same.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

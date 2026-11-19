# atSS3Link/atSS3Util.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SS3-41](#ss3-41) | 73, 64 | H/M | off-by-one (Fortran port) | `a1 = (int)(ran3*np + 1)` gives the range 1..np: `dum[0]` is never chosen and `dum[np]` (unfilled, 0) can be. … |  | [ ] |
| [SS3-42](#ss3-42) | 46, 49 | M/L | leak / div0 / fixed buffer | `dum` is allocated before the `tc==0` early return, which leaks it. `tot==0` produces NaN props. `dum[15000]` … |  | [ ] |
| [SS3-43](#ss3-43) | 236 | H/L | off-by-one (NR port) | In the 0-based port, `ii=0` serves both as the "not started" flag and as a valid index: if the first nonzero i… |  | [ ] |
| [SS3-44](#ss3-44) | 177 | M/L | leak / div0 | `VV` is never freed, and on a singular row `VV[i] = 1.0/big` is computed with big==0 after only a printf. Late… |  | [ ] |

---

## SS3-41

**off-by-one (Fortran port)** · Conf H / Sev M  
**Found at (current code):** `atSS3Util.c` 73, `atSS3Util.c` 64
**Lines at review (Oct 2026):** 73-76

**Function:** `GenMnomial()`

**Why it's thought to be a bug**

`a1 = (int)(ran3*np + 1)` gives the range 1..np: `dum[0]` is never chosen and `dum[np]` (unfilled, 0) can be. `if (a>0) c[a] += 1` then discards category 0 (age 0 or the first length bin) completely; the 0-means-empty sentinel only made sense with 1-based indexing. Function: `GenMnomial()`.

**Code at issue** — `atSS3Util.c` line 73 (closest match to the quoted code; wording differs slightly)  
Search for: `a1 = (int) (ran3(iseed)*(float)np+1);`

```c
   71:         a = 0;
   72: 		do{
   73→ 			a1 = (int) (ran3(iseed)*(float)np+1);
   74: 			a = dum[a1];
   75: 		} while (a<=0 && withrep);
```

**Code at issue** — `atSS3Util.c` line 64 (closest match to the quoted code; wording differs slightly)  
Search for: `dum[np+j] = i;`

```c
   62: 		if (icount>0){
   63: 			for (j=0;j<icount;j++)
   64→ 				dum[np+j] = i;
   65: 			np = np + icount;
   66: 		}
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
**Found at (current code):** `atSS3Util.c` 46, `atSS3Util.c` 49
**Lines at review (Oct 2026):** 36-39, 45-49, 64

**Function:** `GenMnomial()`

**Why it's thought to be a bug**

`dum` is allocated before the `tc==0` early return, which leaks it. `tot==0` produces NaN props. `dum[15000]` is fixed while np can reach about 10000+ndim/2, with no bound check. Function: `GenMnomial()`.

**Code at issue** — `atSS3Util.c` line 46 (the line the review pointed to, found by its text)  
Search for: `tot = tot + prop[i];`

```c
   44: 	tot = 0.0;
   45: 	for (i=0;i<ndim;i++)
   46→ 		tot = tot + prop[i];
   47: 
   48: 	for (i=0;i<ndim;i++)
```

**Code at issue** — `atSS3Util.c` line 49 (the line the review pointed to, found by its text)  
Search for: `prop[i] = prop[i]/tot;`

```c
   47: 
   48: 	for (i=0;i<ndim;i++)
   49→ 		prop[i] = prop[i]/tot;
   50: 
   51: 	// check whether this is stochastic
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
**Found at (current code):** `atSS3Util.c` 236
**Lines at review (Oct 2026):** 235-241

**Function:** `LUBKSB()`

**Why it's thought to be a bug**

In the 0-based port, `ii=0` serves both as the "not started" flag and as a valid index: if the first nonzero is at i=0, forward substitution never starts. The loop `j < (i-1)` should be `j < i`. INVM gives wrong inverses. INVM/LUBKSB are currently never called (latent). Function: `LUBKSB()`.

**Code at issue** — `atSS3Util.c` line 236 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `for( j=ii; j < (i-1); j++) {`

```c
  234:         B[ip] = B[i];
  235:         if (ii) {
  236→             for( j=ii; j < (i-1); j++) {
  237: 				sum -= AA[i][j] * B[j];
  238:             }
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
**Found at (current code):** `atSS3Util.c` 177
**Lines at review (Oct 2026):** 165, 174-177

**Function:** `LUDCMP()`

**Why it's thought to be a bug**

`VV` is never freed, and on a singular row `VV[i] = 1.0/big` is computed with big==0 after only a printf. Latent (unused). Function: `LUDCMP()`.

**Code at issue** — `atSS3Util.c` line 177  
Search for: `VV[i] = 1.0 / big`

```c
  175:             printf("Singular matrix in routine LUDCMP\n");
  176:         }
  177→         VV[i] = 1.0 / big;
  178:     }
  179: 
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

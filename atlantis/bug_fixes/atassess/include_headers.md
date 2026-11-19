# atassess/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HDR-1](#hdr-1) | 33 | M/L |  | Lowercase `#define alpha/beta/gamma` can collide with nrcdflib.h parameters or libm `gamma()`. |  | [ ] |
| [HDR-2](#hdr-2) | 186 | H/L |  | `totselectnums_id 4` but `totnums` has 4 rows (0-3); unused today. |  | [ ] |
| [HDR-3](#hdr-3) | 69 | H/L |  | The comment on phys ids contradicts the defines (o2_wc/TSS inserted at 6/7). This is the root of WRT-02. |  | [ ] |
| [HDR-4](#hdr-4) | 301 | H/L |  | Dangling `extern double **food;` with no definition. |  | [ ] |

---

## HDR-1

**finding** · Conf M / Sev L  
**Found at (current code):** `atsample.h` 33
**Lines at review (Oct 2026):** atsample.h:103-105

**Why it's thought to be a bug**

Lowercase `#define alpha/beta/gamma` can collide with nrcdflib.h parameters or libm `gamma()`.

**Code at issue** — `atsample.h` line 33 (closest match to the quoted code; wording differs slightly)  
Search for: `"#define" definitions for alpha, beta and gammaF`

```c
   31: 
   32:  15/9/2004 added prototypes for Amoeba() and amotry(). Also added
   33→  "#define" definitions for alpha, beta and gammaF
   34: 
   35:  16/9/2004 added prototypes for BackCalc(), SolvePlus(),
```

**Suggested fix (from review):** rename

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

## HDR-2

**finding** · Conf H / Sev L  
**Found at (current code):** `atsample.h` 186
**Lines at review (Oct 2026):** atsample.h:186

**Why it's thought to be a bug**

`totselectnums_id 4` but `totnums` has 4 rows (0-3); unused today.

**Code at issue** — `atsample.h` line 186 (located by name only; check this is the right place)  
Search for: `#define totselectnums_id 4`

```c
  184: #define totcatchnums_id 2
  185: #define totdiscardnums_id 3
  186→ #define totselectnums_id 4
  187: 
  188: /* Network characteristics */
```

**Suggested fix (from review):** allocate 5 / remove

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

## HDR-3

**finding** · Conf H / Sev L  
**Found at (current code):** `atAssessLib.h` 69
**Lines at review (Oct 2026):** atAssessLib.h:50-75

**Why it's thought to be a bug**

The comment on phys ids contradicts the defines (o2_wc/TSS inserted at 6/7). This is the root of WRT-02.

**Code at issue** — `atAssessLib.h` line 69 (located by name only; check this is the right place)  
Search for: `#define o2_wc_id 6`

```c
   67: #define light_sed_id 4
   68: #define o2_sed_id 5
   69→ #define o2_wc_id 6
   70: #define TSS_id 7
   71: #define don_wc_id 8
```

**Suggested fix (from review):** update comment

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

## HDR-4

**finding** · Conf H / Sev L  
**Found at (current code):** `atsample.h` 301
**Lines at review (Oct 2026):** atsample.h:301

**Why it's thought to be a bug**

Dangling `extern double **food;` with no definition.

**Code at issue** — `atsample.h` line 301  
Search for: `extern double **food`

```c
  299: extern double ***cvsample;
  300: extern double ***cvphys; /* Array to store coefficient variance values for the physical parameters that are sampled */
  301→ extern double **food;
  302: extern double **globalfledge;
  303: extern double **globalpd;
```

**Suggested fix (from review):** delete

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

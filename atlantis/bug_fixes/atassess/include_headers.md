# atassess/include/*.h — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HDR-1](#hdr-1) | atsample.h:103-105 | M/L |  | Lowercase `#define alpha/beta/gamma` can collide with nrcdflib.h parameters or libm `gamma()`. |  | [ ] |
| [HDR-2](#hdr-2) | atsample.h:186 | H/L |  | `totselectnums_id 4` but `totnums` has 4 rows (0-3); unused today. |  | [ ] |
| [HDR-3](#hdr-3) | atAssessLib.h:50-75 | H/L |  | The comment on phys ids contradicts the defines (o2_wc/TSS inserted at 6/7). This is the root of WRT-02. |  | [ ] |
| [HDR-4](#hdr-4) | atsample.h:301 | H/L |  | Dangling `extern double **food;` with no definition. |  | [ ] |

---

## HDR-1

**finding** · Conf M / Sev L  
**Lines (current code):** atsample.h:103-105  
**Lines at review (Oct 2026):** atsample.h:103-105

**Why it's thought to be a bug**

Lowercase `#define alpha/beta/gamma` can collide with nrcdflib.h parameters or libm `gamma()`.

**Current code** (`atassess/include/atsample.h` lines 101–107)

```c
  101: #define survey_id 1
  102: 
  103: #define alpha 1.0
  104: #define beta 0.5
  105: #define gamma 2.0
  106: 
  107: #define est1_id 0
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
**Lines (current code):** atsample.h:186  
**Lines at review (Oct 2026):** atsample.h:186

**Why it's thought to be a bug**

`totselectnums_id 4` but `totnums` has 4 rows (0-3); unused today.

**Current code** (`atassess/include/atsample.h` lines 184–188)

```c
  184: #define totcatchnums_id 2
  185: #define totdiscardnums_id 3
  186: #define totselectnums_id 4
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
**Lines (current code):** atAssessLib.h:50-75  
**Lines at review (Oct 2026):** atAssessLib.h:50-75

**Why it's thought to be a bug**

The comment on phys ids contradicts the defines (o2_wc/TSS inserted at 6/7). This is the root of WRT-02.

**Current code** (`atassess/include/atAssessLib.h` lines 48–77)

```c
   48: #define gamma_q 4
   49: 
   50: /* For phys[id][box][record_in_time]
   51:  property                 id
   52:  water column depth        0
   53:  sediment depth            1
   54:  salinity                  2
   55:  light in water column     3
   56:  light at sediment surface 4
   57:  oxygen in sediment        5
   58:  don in water column       6
   59:  din in water column       7
   60:  volume                    8
   61:  */
   62: 
   63: #define wc_depth_id 0
   64: #define sed_depth_id 1
   65: #define salinity_id 2
   66: #define light_wc_id 3
   67: #define light_sed_id 4
   68: #define o2_sed_id 5
   69: #define o2_wc_id 6
   70: #define TSS_id 7
   71: #define don_wc_id 8
   72: #define din_wc_id 9
   73: #define vol_id 10
   74: #define temperature_id 11
   75: #define num_sampled_phy_id 12
   76: 
   77: //#define temperature_id 11
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
**Lines (current code):** atsample.h:301  
**Lines at review (Oct 2026):** atsample.h:301

**Why it's thought to be a bug**

Dangling `extern double **food;` with no definition.

**Current code** (`atassess/include/atsample.h` lines 299–303)

```c
  299: extern double ***cvsample;
  300: extern double ***cvphys; /* Array to store coefficient variance values for the physical parameters that are sampled */
  301: extern double **food;
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

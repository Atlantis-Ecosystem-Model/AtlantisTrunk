# ateconomic/include/ (prototypes vs definitions) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-25](#ecio-25) | 170 | M/H | definition in header | `int g_effortHistoryCount;` is a (tentative) definition, not an `extern` declaration, in a header that every a… |  | [ ] |
| [ECIO-26](#ecio-26) | 111 | L/L | duplicate definition (latent) | `Quicksort_Dir` is defined in both ateconomic/ateconhelp.c:42 and atlantisUtil/atUtilhelp.c:43. ateconhelp.c i… |  | [ ] |
| [ECIO-27](#ecio-27) | 32 | L/L | missing prototypes | `Economic_Output_ExpectedCatchPerMonth()` is public but declared ad hoc in ateconomic.c:35 instead of in atEco… |  | [ ] |

---

## ECIO-25

**definition in header** · Conf M / Sev H  
**Found at (current code):** `atFishMIP_OSP_Bridge.h` 170
**Lines at review (Oct 2026):** atFishMIP_OSP_Bridge.h:170

**Why it's thought to be a bug**

`int g_effortHistoryCount;` is a (tentative) definition, not an `extern` declaration, in a header that every ateconomic TU includes via atEconomic.h:38, as does atlantismain.c:139. Under `-fno-common`, which is the default in GCC>=10 and Clang>=11 (no `-fcommon` in PreRules.am/configure.ac), this gives duplicate-symbol link errors. It is also never used.

**Code at issue** — `atFishMIP_OSP_Bridge.h` line 170 (located by name only; check this is the right place)  
Search for: `extern int g_effortHistoryCount; /* number of species slots allocated in g_effortHistory *`

```c
  168: extern EffortHistoryRecord *g_catch_n_effort_History;  /* per fished species */
  169: 
  170→ extern int g_effortHistoryCount; /* number of species slots allocated in g_effortHistory */
  171: 
  172: 
```

**Suggested fix (from review):** Delete it, or make it `extern` and define it once.

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

## ECIO-26

**duplicate definition (latent)** · Conf L / Sev L  
**Found at (current code):** `atEconomic.h` 111
**Lines at review (Oct 2026):** atEconomic.h:111; atlantisboxmodel.h:6255

**Why it's thought to be a bug**

`Quicksort_Dir` is defined in both ateconomic/ateconhelp.c:42 and atlantisUtil/atUtilhelp.c:43. ateconhelp.c is not in libateconomic_a_SOURCES at the moment, so it does not link today, but adding it back would collide.

**Code at issue** — `atEconomic.h` line 111 (located by name only; check this is the right place)  
Search for: `void Quicksort_Dir(double *x, double *bbx, double *ccx, double *ddx, double *eex, int n, i`

```c
  109: 
  110: /* Numerical routines */
  111→ void Quicksort_Dir(double *x, double *bbx, double *ccx, double *ddx, double *eex, int n, int ascendflag);
  112: 
  113: /* Util functions */
```

**Suggested fix (from review):** Delete ateconhelp.c's copy.

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

## ECIO-27

**missing prototypes** · Conf L / Sev L  
**Found at (current code):** `ateconomic.c` 32
**Lines at review (Oct 2026):** atEconomicLib.h; ateconomic.c:35; ateconio.c:1003

**Why it's thought to be a bug**

`Economic_Output_ExpectedCatchPerMonth()` is public but declared ad hoc in ateconomic.c:35 instead of in atEconomicLib.h (next to the other Economic_Output_*). `Write_Expected_CatchPerMonth()` is non-static and has no prototype anywhere, unlike every other Write_* in ateconio.c. All other prototypes in atEconomic.h, atEconomicLib.h, atFishMIP_OSP_Bridge.h and atFishMIP_OSP_PluginAdapter.h match their definitions in signature.

**Code at issue** — `ateconomic.c` line 32 (located by name only; check this is the right place)  
Search for: `#include <atEconomic.h>`

```c
   30: #include <math.h>
   31: #include <sjwlib.h>
   32→ #include <atEconomic.h>
   33: 
   34: void Economic_Output_Temporal_Catches(MSEBoxModel *bm);
```

**Suggested fix (from review):** Add the prototype to atEconomicLib.h and make Write_Expected_CatchPerMonth static.

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

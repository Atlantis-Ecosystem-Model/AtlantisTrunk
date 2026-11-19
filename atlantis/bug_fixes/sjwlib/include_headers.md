# sjwlib/include/*.h (sjwlib.h, sjwalloc.h, datafile.h, colourtable.h, gridlib.h, mapproj.h, nrcdflib.h, pointsourcesink.h, polyline.h, ptrack.h, timeseries.h, txt_param.h, win32.h, xytoij.h, sjwconfig.h, common.h, dos2unix.h, unix2dos.h) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-15](#sjw3-15) | sjwlib.h:80-82 | H/M | macro multiple evaluation / wrong rounding | `#if !defined(round)` / `#define round(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))` — `round` in <math.h> is a … |  | [ ] |
| [SJW3-16](#sjw3-16) | sjwlib.h:71-72 | L/L | macro multiple evaluation | `#define max(x,y) ((x)>(y) ? (x) : (y))` / `min` evaluate arguments twice; any caller passing a side-effecting… |  | [ ] |

## Reviewer notes for this file

No issues found in the remaining headers: struct/prototype declarations checked against the definitions in the assigned .c files (gridlib.h, ptrack.h, pointsourcesink.h, colourtable.h, polyline.h, timeseries.h, datafile.h, sjwalloc.h prototypes match); common.h/dos2unix.h/unix2dos.h are third-party dos2unix headers not used by the build.

---

## SJW3-15

**macro multiple evaluation / wrong rounding** · Conf H / Sev M  
**Lines (current code):** sjwlib.h:80-82  
**Lines at review (Oct 2026):** sjwlib.h:80-82

**Why it's thought to be a bug**

`#if !defined(round)` / `#define round(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))` — `round` in <math.h> is a function, not a macro, so this macro is always defined and silently replaces C99 `round()` in every file that includes math.h then sjwlib.h. (a) `x` is evaluated 3 times: in atassess/atsurvey.c:640, :1260 and atassess/atattribute.c:858 the argument is `(chrt + drandom(0.0, 1.0)) * ageClassSize + 0.5`, so `drandom` is called three times and the comparison and the floor/ceil use three different random numbers — the assigned age `cage` is not a rounding of any single draw, distorting age-composition samples and potentially exceeding the intended age range. (b) Even with a pure argument it rounds halves down (`round(2.5)=2`, `round(-2.5)=-3`) unlike C `round`. (c) `x` is not parenthesised.

**Current code** (`sjwlib/include/sjwlib.h` lines 78–84)

```c
   78: //#//endif
   79: 
   80: #if !defined(round)
   81: #define	round(x)	((x-floor(x))>0.5 ? ceil(x) : floor(x))
   82: #endif
   83: 
   84: /* Maximum length of an input line */
```

**Suggested fix (from review):** Remove the macro (C99 `round` is available on all supported compilers incl. MSVC 2013+), or make it a static inline function. In callers, compute the drandom expression into a temporary first.

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

## SJW3-16

**macro multiple evaluation** · Conf L / Sev L  
**Lines (current code):** sjwlib.h:71-72  
**Lines at review (Oct 2026):** sjwlib.h:71-72

**Why it's thought to be a bug**

`#define max(x,y) ((x)>(y) ? (x) : (y))` / `min` evaluate arguments twice; any caller passing a side-effecting argument (e.g. `drandom()`) gets inconsistent results. Not verified that any caller does.

**Current code** (`sjwlib/include/sjwlib.h` lines 69–74)

```c
   69: #ifndef _WIN32
   70: //#if !defined(max)
   71: #define max(x,y) ((x)>(y) ? (x) : (y) )
   72: #define min(x,y) ((x)<(y) ? (x) : (y) )
   73: #define _hypot( x, y ) (hypot( x,y ) )
   74: #endif
```

**Suggested fix (from review):** Use inline functions.

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

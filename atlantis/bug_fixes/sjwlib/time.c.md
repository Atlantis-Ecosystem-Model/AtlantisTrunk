# sjwlib/time.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-25](#sjw2-25) | 160-164 | H/M | off-by-one day | When rounding pushes the time to 24:00, the code computes `j = tojul(y,mo,d,0,0,0) + 1`. `tojul` returns `juld… |  | [ ] |
| [SJW2-26](#sjw2-26) | 373-377 | M/L | silent data loss | If the date has `HH:MM` without seconds, `sscanf` returns 2. The fallback then zeroes `h` and `mi` too, silent… |  | [ ] |

---

## SJW2-25

**off-by-one day** · Conf H / Sev M  
**Listed lines:** 160-164

**Function:** `todat()`

**Why it's thought to be a bug**

When rounding pushes the time to 24:00, the code computes `j = tojul(y,mo,d,0,0,0) + 1`. `tojul` returns `julday - 0.5`, so `j = julday + 0.5` and `(long)j == julday`. `caldat` then returns the same date, and `h` is reset to 0, so the result is midnight at the start of the same day, 24 h too early. This happens whenever a float time sits just below midnight (e.g. 86399.9999 s), which is common for whole-day times, and affects `TimeToDate()` strings. Function: `todat()`.

**Current code** (`sjwlib/time.c` lines 158–166)

```c
  158: 	    *h += 1;
  159: 	}
  160: 	if( *h >= 24 ) {
  161: 	    double j = tojul(*y,*mo,*d,0,0,0) + 1;
  162: 	    caldat((long)j,mo,d,y);
  163: 	    *h -= 24;
  164: 	}
  165: }
  166: 
```

**Suggested fix (from review):** `caldat((long)floor(tojul(*y,*mo,*d,0,0,0) + 1 + 0.5), mo, d, y);` (i.e. `julday(...)+1`).

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

## SJW2-26

**silent data loss** · Conf M / Sev L  
**Listed lines:** 373-377

**Function:** `DateToJulian()`

**Why it's thought to be a bug**

If the date has `HH:MM` without seconds, `sscanf` returns 2. The fallback then zeroes `h` and `mi` too, silently dropping the time of day from the epoch or date. Function: `DateToJulian()`.

**Current code** (`sjwlib/time.c` lines 371–379)

```c
  371: 
  372:     /* Read hours, minutes and seconds if present */
  373:     if( sscanf(p,"%*d-%*d-%*d %d:%d:%d",&h,&mi,&s) != 3 ) {
  374: 	h = 0;
  375: 	mi = 0;
  376: 	s = 0;
  377:     }
  378: 
  379:     /* Convert to Julian days */
```

**Suggested fix (from review):** Accept a return of >= 2 (set `s=0` when only 2 are read).

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

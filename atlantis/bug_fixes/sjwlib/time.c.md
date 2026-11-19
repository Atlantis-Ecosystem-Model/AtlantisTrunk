# sjwlib/time.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-25](#sjw2-25) | 161 | H/M | off-by-one day | When rounding pushes the time to 24:00, the code computes `j = tojul(y,mo,d,0,0,0) + 1`. `tojul` returns `juld… |  | [ ] |
| [SJW2-26](#sjw2-26) | 373 | M/L | silent data loss | If the date has `HH:MM` without seconds, `sscanf` returns 2. The fallback then zeroes `h` and `mi` too, silent… |  | [ ] |

---

## SJW2-25

**off-by-one day** · Conf H / Sev M  
**Found at (current code):** `time.c` 161
**Lines at review (Oct 2026):** 160-164

**Function:** `todat()`

**Why it's thought to be a bug**

When rounding pushes the time to 24:00, the code computes `j = tojul(y,mo,d,0,0,0) + 1`. `tojul` returns `julday - 0.5`, so `j = julday + 0.5` and `(long)j == julday`. `caldat` then returns the same date, and `h` is reset to 0, so the result is midnight at the start of the same day, 24 h too early. This happens whenever a float time sits just below midnight (e.g. 86399.9999 s), which is common for whole-day times, and affects `TimeToDate()` strings. Function: `todat()`.

**Code at issue** — `time.c` line 161 (closest match to the quoted code; wording differs slightly)  
Search for: `double j = tojul(*y,*mo,*d,0,0,0) + 1;`

```c
  159: 	}
  160: 	if( *h >= 24 ) {
  161→ 	    double j = tojul(*y,*mo,*d,0,0,0) + 1;
  162: 	    caldat((long)j,mo,d,y);
  163: 	    *h -= 24;
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
**Found at (current code):** `time.c` 373
**Lines at review (Oct 2026):** 373-377

**Function:** `DateToJulian()`

**Why it's thought to be a bug**

If the date has `HH:MM` without seconds, `sscanf` returns 2. The fallback then zeroes `h` and `mi` too, silently dropping the time of day from the epoch or date. Function: `DateToJulian()`.

**Code at issue** — `time.c` line 373 (the line the review pointed to, found by its text)  
Search for: `if( sscanf(p,"%*d-%*d-%*d %d:%d:%d",&h,&mi,&s) != 3 ) {`

```c
  371: 
  372:     /* Read hours, minutes and seconds if present */
  373→     if( sscanf(p,"%*d-%*d-%*d %d:%d:%d",&h,&mi,&s) != 3 ) {
  374: 	h = 0;
  375: 	mi = 0;
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

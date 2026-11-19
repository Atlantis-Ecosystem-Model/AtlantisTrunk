# ateconomic/ateconts.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [QUOT-28](#quot-28) | 84, 88 | M/L | partial initialisation | It allocates `nEconts` FisheryTimeSeries but reads only index `i=0`; entries 1..n-1 stay uninitialised. A nega… |  | [ ] |

---

## QUOT-28

**partial initialisation** · Conf M / Sev L  
**Found at (current code):** `ateconts.c` 84, `ateconts.c` 88
**Lines at review (Oct 2026):** 84-128

**Function:** `Read_Econ_Time_Series()`

**Why it's thought to be a bug**

It allocates `nEconts` FisheryTimeSeries but reads only index `i=0`; entries 1..n-1 stay uninitialised. A negative nEconts is not rejected before malloc. It is latent because the only caller (ateconomicsetup.c:503) uses `tsGDP[0]`. Function: `Read_Econ_Time_Series()`.

**Code at issue** — `ateconts.c` line 84 (the line the review pointed to, found by its text)  
Search for: `readkeyprm_i(fp,"nEconts",&nEconts);`

```c
   82: 		fprintf(stderr, "Reading Economic Time Series %s\n", name);
   83: 
   84→     readkeyprm_i(fp,"nEconts",&nEconts);
   85:     if(!nEconts)
   86:         quit("Economic model currently expects a GDP file, if you don't have one please create a dummy file (filled with zeroes for example)\n");
```

**Code at issue** — `ateconts.c` line 88 (the line the review pointed to, found by its text)  
Search for: `nnts = nEconts;`

```c
   86:         quit("Economic model currently expects a GDP file, if you don't have one please create a dummy file (filled with zeroes for example)\n");
   87: 
   88→     nnts = nEconts;
   89: 
   90: 	/* Allocate memory for list of time series */
```

**Suggested fix (from review):** Loop i over nEconts (or allocate 1) and quit if `nEconts < 1`

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

# sjwlib/dfeval.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-01](#sjw3-01) | 65, 70, 73 | H/M | wrong index / buffer overrun | `dfReadRecords(df, v, before, after - before + 1); ... v->data[before] ... v->data[after]` — for netCDF datafi… |  | [ ] |
| [SJW3-02](#sjw3-02) | 264 | M/L | buffer overrun (latent) | `int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS]; ... ncorners = 1 << nd` — with MAXNUMDIMS=10 the array… |  | [ ] |

---

## SJW3-01

**wrong index / buffer overrun** · Conf H / Sev M  
**Found at (current code):** `dfeval.c` 65, `dfeval.c` 70, `dfeval.c` 73
**Lines at review (Oct 2026):** 65-77, 105-119

**Why it's thought to be a bug**

`dfReadRecords(df, v, before, after - before + 1); ... v->data[before] ... v->data[after]` — for netCDF datafiles `dfReadRecords` (datafile.c:348-430) allocates a buffer of only `nrecs` (1 or 2) records starting at `v->start_record = before`, and `df_read_data` stores record r at `data[r - v->start_record]` (datafile.c:1505). Indexing with the absolute record number `before`/`after` therefore reads past the buffer for any record > 0 (and returns the wrong value). Compare `dfGetDataValue` (datafile.c:912) and `df_interp_1d_inv_weight` (line 227) which correctly subtract `v->start_record`. Only ASCII files work because `dfReadRecords` returns early and all records are in memory. Affects every 0-D netCDF time series evaluated via `tsEval`/`tsEvalR`/`tsEvalEx`. Functions: `dfEval()`, `dfEvalEx()`.

**Code at issue** — `dfeval.c` line 65  
Search for: `dfReadRecords(df, v, before, after - before + 1)`

```c
   63: 		/* NULL STATEMENT - Should take some action here */;
   64: 	}
   65→ 	dfReadRecords(df, v, before, after - before + 1);
   66: 
   67: 	val = v->missing;
```

**Code at issue** — `dfeval.c` line 70  
Search for: `v->data[before]`

```c
   68: 
   69: 	/* Value exactly at before record */
   70→ 	if (frac == 0.0 && (fabs(v->data[before] - v->missing) > 1e-10)) {
   71: 		val = v->data[before];
   72: 		/* Value exactly at after record */
```

**Code at issue** — `dfeval.c` line 73  
Search for: `v->data[after]`

```c
   71: 		val = v->data[before];
   72: 		/* Value exactly at after record */
   73→ 	} else if (frac == 1.0 && (fabs(v->data[after] - v->missing) > 1e-10)) {
   74: 		val = v->data[after];
   75: 		/* Value inbetween - need to interpolate */
```

**Suggested fix (from review):** Use `v->data[before - v->start_record]` / `v->data[after - v->start_record]` (or `dfGetDataValue(df, v, before, NULL)`).

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

## SJW3-02

**buffer overrun (latent)** · Conf M / Sev L  
**Found at (current code):** `dfeval.c` 264
**Lines at review (Oct 2026):** 264-266, 294-296

**Function:** `df_interp_linear()`

**Why it's thought to be a bug**

`int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS]; ... ncorners = 1 << nd` — with MAXNUMDIMS=10 the array has 99 rows, but `ncorners` = 2^nd exceeds 99 for nd >= 7, overrunning the stack array. Should be sized `1 << MAXNUMDIMS`. Function: `df_interp_linear()`.

**Code at issue** — `dfeval.c` line 264  
Search for: `int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS]`

```c
  262: 		int iindices[MAXNUMDIMS];
  263: 		int corner[MAXNUMDIMS];
  264→ 		int coffsets[MAXNUMDIMS * MAXNUMDIMS - 1][MAXNUMDIMS];
  265: 		int dsize[MAXNUMDIMS];
  266: 		int ncorners = 1 << nd;
```

**Suggested fix (from review):** Declare `coffsets[1 << MAXNUMDIMS][MAXNUMDIMS]` or quit if nd > 6.

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

dfeval.c `dfEvalCoords()`: when `dfFindRecord` returns an out-of-range record (r0==r1, rfrac=0) the result is fine, but `recvals[1]` is used with weight `rfrac` even when only one record was read; harmless because rfrac==0 there.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

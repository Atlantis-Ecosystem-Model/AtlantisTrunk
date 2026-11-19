# sjwlib/timeseries.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-14](#sjw2-14) | 350, 337 | M/M | NULL dereference | `ATT_DOUBLE(x0,0)` etc. expand to `(a)->type` (datafile.h:126,131), but `dfGetGlobalAttribute` returns NULL wh… |  | [ ] |
| [SJW2-15](#sjw2-15) | 604, 612, 167 | M/L | stale copy | After converting units, `ts->t_units`, `df->rec_units` and `df->variables[ri].units` are updated, but the sepa… |  | [ ] |
| [SJW2-16](#sjw2-16) | 515, 120 | L/L | NULL dereference / uninitialised | If the file has no record variable and no `t`/`time` variable, `ts->t` stays NULL, and `tsMultiCheck`/`tsMulti… |  | [ ] |

---

## SJW2-14

**NULL dereference** · Conf M / Sev M  
**Found at (current code):** `timeseries.c` 350, `timeseries.c` 337
**Lines at review (Oct 2026):** 337-361

**Function:** `tsEvalXYR()`

**Why it's thought to be a bug**

`ATT_DOUBLE(x0,0)` etc. expand to `(a)->type` (datafile.h:126,131), but `dfGetGlobalAttribute` returns NULL when `xorigin`/`dx`/`rotation`/`arc`/`rmin` is missing, which segfaults. `str = ATT_TEXT(gt)` is NULL if `gridtype` is not text, and is then passed to `strcasecmp`. Function: `tsEvalXYR()`.

**Code at issue** — `timeseries.c` line 350  
Search for: `ATT_DOUBLE(x0,0)`

```c
  348: 							Attribute *dy = dfGetGlobalAttribute(df, "dy");
  349: 							Attribute *rot = dfGetGlobalAttribute(df, "rotation");
  350→ 							snprintf(analytic, sizeof(analytic), "rectangular 0.0 0.0 %d %d %g %g %g %g %g", dim1size, dim2size, ATT_DOUBLE(x0,0), ATT_DOUBLE(y0,0),
  351: 									ATT_DOUBLE(dx,0), ATT_DOUBLE(dy,0), ATT_DOUBLE(rot,0));
  352: 						}
```

**Code at issue** — `timeseries.c` line 337  
Search for: `str = ATT_TEXT(gt)`

```c
  335: 					if (gt != NULL) {
  336: 						char analytic[MAXLINELEN];
  337→ 						str = ATT_TEXT(gt);
  338: 
  339: 						analytic[0] = 0;
```

**Suggested fix (from review):** Check each attribute for NULL (and `str != NULL`) before use; quit with a clear message.

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

## SJW2-15

**stale copy** · Conf M / Sev L  
**Found at (current code):** `timeseries.c` 604, `timeseries.c` 612, `timeseries.c` 167
**Lines at review (Oct 2026):** 601-614

**Function:** `tsNewTimeUnits()`

**Why it's thought to be a bug**

After converting units, `ts->t_units`, `df->rec_units` and `df->variables[ri].units` are updated, but the separate copy `ts->varunit[df->ri]` (strcpy'd in `tsRead`, lines 162-168) still holds the old units. Function: `tsNewTimeUnits()`.

**Code at issue** — `timeseries.c` line 604  
Search for: `ts->t_units`

```c
  602: 	Datafile *df = ts->df;
  603: 
  604→ 	ChangeTimeUnits(ts->t_units, newunits, df->records, (int)df->nrecords);
  605: 
  606: 	/* Store new units in time series */
```

**Code at issue** — `timeseries.c` line 612  
Search for: `df->rec_units`

```c
  610: 
  611: 	/* Propogate the change into the DataFile */
  612→ 	df->rec_units = ts->t_units;
  613: 	df->variables[df->ri].units = ts->t_units;
  614: }
```

**Code at issue** — `timeseries.c` line 167 (closest match to the quoted code; wording differs slightly)  
Search for: `strcpy(ts->varunit[i], df->variables[i].units);`

```c
  165: 			if (ts->varunit[i] == NULL)
  166: 				quit("tsRead: Failed to allocate memory.\n");
  167→ 			strcpy(ts->varunit[i], df->variables[i].units);
  168: 		}
  169: 	}
```

**Suggested fix (from review):** Also update `ts->varunit[df->ri]`.

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

## SJW2-16

**NULL dereference / uninitialised** · Conf L / Sev L  
**Found at (current code):** `timeseries.c` 515, `timeseries.c` 120
**Lines at review (Oct 2026):** 108-114, 117-139, 515-516, 563

**Function:** `tsRead()`

**Why it's thought to be a bug**

If the file has no record variable and no `t`/`time` variable, `ts->t` stays NULL, and `tsMultiCheck`/`tsMultiEvalXYZ` dereference `ts->t[0]`. For NetCDF with `nd > 3`, `ts->type` is left unset (`default: break`). Function: `tsRead()`.

**Code at issue** — `timeseries.c` line 515  
Search for: `ts->t[0]`

```c
  513: 			if (varid >= 0) {
  514: 				found_var = 1;
  515→ 				f_tstart = (ts->t[0] < f_tstart) ? ts->t[0] : f_tstart;
  516: 				f_tstop = (ts->t[ts->nt - 1] > f_tstop) ? ts->t[ts->nt - 1] : f_tstop;
  517: 			}
```

**Code at issue** — `timeseries.c` line 120  
Search for: `ts->type`

```c
  118: 		switch (df->nd) {
  119: 		case 0:
  120→ 			ts->type = TS_NC0;
  121: 			break;
  122: 
```

**Suggested fix (from review):** Quit if `ts->t == NULL`; set a type or quit in the default branch.

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

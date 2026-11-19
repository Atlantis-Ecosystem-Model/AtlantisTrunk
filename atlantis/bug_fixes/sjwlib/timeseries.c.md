# sjwlib/timeseries.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-14](#sjw2-14) | 337-361 | M/M | NULL dereference | `ATT_DOUBLE(x0,0)` etc. expand to `(a)->type` (datafile.h:126,131), but `dfGetGlobalAttribute` returns NULL wh… |  | [ ] |
| [SJW2-15](#sjw2-15) | 601-614 | M/L | stale copy | After converting units, `ts->t_units`, `df->rec_units` and `df->variables[ri].units` are updated, but the sepa… |  | [ ] |
| [SJW2-16](#sjw2-16) | 108-114, 117-139, 515-516, 563 | L/L | NULL dereference / uninitialised | If the file has no record variable and no `t`/`time` variable, `ts->t` stays NULL, and `tsMultiCheck`/`tsMulti… |  | [ ] |

---

## SJW2-14

**NULL dereference** · Conf M / Sev M  
**Listed lines:** 337-361

**Function:** `tsEvalXYR()`

**Why it's thought to be a bug**

`ATT_DOUBLE(x0,0)` etc. expand to `(a)->type` (datafile.h:126,131), but `dfGetGlobalAttribute` returns NULL when `xorigin`/`dx`/`rotation`/`arc`/`rmin` is missing, which segfaults. `str = ATT_TEXT(gt)` is NULL if `gridtype` is not text, and is then passed to `strcasecmp`. Function: `tsEvalXYR()`.

**Current code** (`sjwlib/timeseries.c` lines 335–363)

```c
  335: 					if (gt != NULL) {
  336: 						char analytic[MAXLINELEN];
  337: 						str = ATT_TEXT(gt);
  338: 
  339: 						analytic[0] = 0;
  340: 						/* We have a analytic expression. Determine the type
  341: 						 * and add this as as an attribute to the x and y
  342: 						 * variables in the datafile structure.
  343: 						 * Always assume cell edge. */
  344: 						if (strcasecmp(str, "rectangular") == 0) {
  345: 							Attribute *x0 = dfGetGlobalAttribute(df, "xorigin");
  346: 							Attribute *y0 = dfGetGlobalAttribute(df, "yorigin");
  347: 							Attribute *dx = dfGetGlobalAttribute(df, "dx");
  348: 							Attribute *dy = dfGetGlobalAttribute(df, "dy");
  349: 							Attribute *rot = dfGetGlobalAttribute(df, "rotation");
  350: 							snprintf(analytic, sizeof(analytic), "rectangular 0.0 0.0 %d %d %g %g %g %g %g", dim1size, dim2size, ATT_DOUBLE(x0,0), ATT_DOUBLE(y0,0),
  351: 									ATT_DOUBLE(dx,0), ATT_DOUBLE(dy,0), ATT_DOUBLE(rot,0));
  352: 						}
  353: 
  354: 						else if (strcasecmp(str, "polar") == 0) {
  355: 							Attribute *x0 = dfGetGlobalAttribute(df, "xorigin");
  356: 							Attribute *y0 = dfGetGlobalAttribute(df, "yorigin");
  357: 							Attribute *arc = dfGetGlobalAttribute(df, "arc");
  358: 							Attribute *rmin = dfGetGlobalAttribute(df, "rmin");
  359: 							Attribute *rot = dfGetGlobalAttribute(df, "rotation");
  360: 							snprintf(analytic, sizeof(analytic), "polar 0.0 0.0 %d %d %g %g %g %g %g", dim1size, dim2size, ATT_DOUBLE(x0,0), ATT_DOUBLE(y0,0),
  361: 									ATT_DOUBLE(arc,0), ATT_DOUBLE(rmin,0), ATT_DOUBLE(rot,0));
  362: 						}
  363: 
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
**Listed lines:** 601-614

**Function:** `tsNewTimeUnits()`

**Why it's thought to be a bug**

After converting units, `ts->t_units`, `df->rec_units` and `df->variables[ri].units` are updated, but the separate copy `ts->varunit[df->ri]` (strcpy'd in `tsRead`, lines 162-168) still holds the old units. Function: `tsNewTimeUnits()`.

**Current code** (`sjwlib/timeseries.c` lines 599–616)

```c
  599:  * This routine calls quit() if anything goes wrong
  600:  */
  601: void tsNewTimeUnits(TimeSeries *ts, char *newunits) {
  602: 	Datafile *df = ts->df;
  603: 
  604: 	ChangeTimeUnits(ts->t_units, newunits, df->records, (int)df->nrecords);
  605: 
  606: 	/* Store new units in time series */
  607: 	if ((ts->t_units = (char *) realloc((void *) ts->t_units, strlen(newunits) + 1)) == NULL)
  608: 		quit("tsNewTimeUnits: Can't allocate memory for new units\n");
  609: 	strcpy(ts->t_units, newunits);
  610: 
  611: 	/* Propogate the change into the DataFile */
  612: 	df->rec_units = ts->t_units;
  613: 	df->variables[df->ri].units = ts->t_units;
  614: }
  615: 
  616: /** Free memory associated with a time series.
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
**Listed lines:** 108-114, 117-139, 515-516, 563

**Function:** `tsRead()`

**Why it's thought to be a bug**

If the file has no record variable and no `t`/`time` variable, `ts->t` stays NULL, and `tsMultiCheck`/`tsMultiEvalXYZ` dereference `ts->t[0]`. For NetCDF with `nd > 3`, `ts->type` is left unset (`default: break`). Function: `tsRead()`.

**Current code** (`sjwlib/timeseries.c` lines 133–143) — code appears to have moved +27 lines since the review

```c
  133: 			break;
  134: 
  135: 		default:
  136: 			break;
  137: 		}
  138: 	} else
  139: 		ts->type = TS_ASCII;
  140: 
  141: 	ts->name = df->name;
  142: 	ts->nt = df->nrecords;
  143: 	ts->ti = df->ri;
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

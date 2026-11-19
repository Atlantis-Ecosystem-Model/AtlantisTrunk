# atecology/dump.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-09](#emisc-09) | 300 | H/L | format specifier | `fscanf(f, "bnd_vert %e %e ", &p.x, &p.y)` reads into `dpoint` members that are `double` (sjwlib/include/polyl… |  | [ ] |
| [EMISC-10](#emisc-10) | 446, 452, 463 | H/L | NULL dereference | `hd->fname`, `hd->fid`, `hd->nb`... are read in the declarations before the `if (hd == NULL ...)` check. Funct… |  | [ ] |
| [EMISC-11](#emisc-11) | 111, 188 | M/L | leak / uninitialised | `validu` is never freed; `meco->zgrid[nk]` is never set; for a non-boundary cell with 0 MECO cells, `c->mecoce… |  | [ ] |

## Reviewer notes for this file

(Note: this file is not in `libatecology_a_SOURCES` and `tr2ex.h` does not exist, so it is dead code. Severities are latent.)

---

## EMISC-09

**format specifier** · Conf H / Sev L  
**Found at (current code):** `dump.c` 300
**Lines at review (Oct 2026):** 300

**Function:** `init_from_bm_dump()`

**Why it's thought to be a bug**

`fscanf(f, "bnd_vert %e %e ", &p.x, &p.y)` reads into `dpoint` members that are `double` (sjwlib/include/polyline.h:19-20); `%e` writes a float. Function: `init_from_bm_dump()`.

**Code at issue** — `dump.c` line 300  
Search for: `fscanf(f, "bnd_vert %e %e ", &p.x, &p.y)`

```c
  298:     e->boundary = createpolyline();
  299:     skipToKeyStart(f,"bnd_vert");
  300→     while (fscanf(f, "bnd_vert %e %e ", &p.x, &p.y) == 2)
  301: 	addtoend(e->boundary, p);
  302: 
```

**Suggested fix (from review):** Use `%le`

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

## EMISC-10

**NULL dereference** · Conf H / Sev L  
**Found at (current code):** `dump.c` 446, `dump.c` 452, `dump.c` 463
**Lines at review (Oct 2026):** 445-463

**Function:** `write_exchange_dump()`

**Why it's thought to be a bug**

`hd->fname`, `hd->fid`, `hd->nb`... are read in the declarations before the `if (hd == NULL ...)` check. Function: `write_exchange_dump()`.

**Code at issue** — `dump.c` line 446  
Search for: `hd->fname`

```c
  444: {
  445:     exchange_dump* hd = e->hd;
  446→     char* fname = hd->fname;
  447:     int fid = hd->fid;
  448:     cell** cells = e->cells;
```

**Code at issue** — `dump.c` line 452 (closest match to the quoted code; wording differs slightly)  
Search for: `int nb = hd->nb;`

```c
  450:     double* exchanges = e->x;
  451:     int nexchanges = e->nlinks;
  452→     int nb = hd->nb;
  453:     int nk = hd->nk;
  454:     int nd = hd->nd;
```

**Code at issue** — `dump.c` line 463  
Search for: `if (hd == NULL`

```c
  461:     int*** dest_b = NULL;
  462: 
  463→     if (hd == NULL || e->t < e->toutstart)
  464: 	return;
  465: 
```

**Suggested fix (from review):** Move the NULL check before the dereferences

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

## EMISC-11

**leak / uninitialised** · Conf M / Sev L  
**Found at (current code):** `dump.c` 111, `dump.c` 188
**Lines at review (Oct 2026):** 104, 111, 189-194, 247-252

**Function:** `init_from_meco_dump()`

**Why it's thought to be a bug**

`validu` is never freed; `meco->zgrid[nk]` is never set; for a non-boundary cell with 0 MECO cells, `c->mecocells` is left uninitialised (only the boundary branch sets NULL). Function: `init_from_meco_dump()`.

**Code at issue** — `dump.c` line 111 (closest match to the quoted code; wording differs slightly)  
Search for: `meco->zgrid = malloc((nk+1) * sizeof(double));`

```c
  109:     meco->count[3] = ni;
  110:     meco->ncells = nk * nj * ni;
  111→     meco->zgrid = malloc((nk+1) * sizeof(double));
  112: 
  113:     /* read cell positions */
```

**Code at issue** — `dump.c` line 188  
Search for: `c->mecocells`

```c
  186:             int n = c->nmecocells;
  187:             if (n > 0)
  188→             c->mecocells = malloc(n * sizeof(mecocell));
  189:             else if (!c->is_boundary) {
  190:             warn("error: BM cell (b,k) = (%d,%d) contains 0 MECO cells\n", i, j);
```

**Suggested fix (from review):** free(validu); set mecocells=NULL in both branches

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

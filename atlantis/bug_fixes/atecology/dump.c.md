# atecology/dump.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [EMISC-09](#emisc-09) | 300 | H/L | format specifier | `fscanf(f, "bnd_vert %e %e ", &p.x, &p.y)` reads into `dpoint` members that are `double` (sjwlib/include/polyl… |  | [ ] |
| [EMISC-10](#emisc-10) | 445-463 | H/L | NULL dereference | `hd->fname`, `hd->fid`, `hd->nb`... are read in the declarations before the `if (hd == NULL ...)` check. Funct… |  | [ ] |
| [EMISC-11](#emisc-11) | 104, 111, 189-194, 247-252 | M/L | leak / uninitialised | `validu` is never freed; `meco->zgrid[nk]` is never set; for a non-boundary cell with 0 MECO cells, `c->mecoce… |  | [ ] |

## Reviewer notes for this file

(Note: this file is not in `libatecology_a_SOURCES` and `tr2ex.h` does not exist, so it is dead code. Severities are latent.)

---

## EMISC-09

**format specifier** · Conf H / Sev L  
**Lines (current code):** 300  
**Lines at review (Oct 2026):** 300

**Function:** `init_from_bm_dump()`

**Why it's thought to be a bug**

`fscanf(f, "bnd_vert %e %e ", &p.x, &p.y)` reads into `dpoint` members that are `double` (sjwlib/include/polyline.h:19-20); `%e` writes a float. Function: `init_from_bm_dump()`.

**Current code** (`atecology/dump.c` lines 298–302)

```c
  298:     e->boundary = createpolyline();
  299:     skipToKeyStart(f,"bnd_vert");
  300:     while (fscanf(f, "bnd_vert %e %e ", &p.x, &p.y) == 2)
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
**Lines (current code):** 445-463  
**Lines at review (Oct 2026):** 445-463

**Function:** `write_exchange_dump()`

**Why it's thought to be a bug**

`hd->fname`, `hd->fid`, `hd->nb`... are read in the declarations before the `if (hd == NULL ...)` check. Function: `write_exchange_dump()`.

**Current code** (`atecology/dump.c` lines 443–465)

```c
  443: void write_exchange_dump(tr2ex* e)
  444: {
  445:     exchange_dump* hd = e->hd;
  446:     char* fname = hd->fname;
  447:     int fid = hd->fid;
  448:     cell** cells = e->cells;
  449:     link* links = e->links;
  450:     double* exchanges = e->x;
  451:     int nexchanges = e->nlinks;
  452:     int nb = hd->nb;
  453:     int nk = hd->nk;
  454:     int nd = hd->nd;
  455:     int b, k, d, i;
  456:     size_t start[4];
  457:     size_t count[4];
  458:     int* cdest = NULL;
  459:     double*** exchange = NULL;
  460:     int*** dest_k = NULL;
  461:     int*** dest_b = NULL;
  462: 
  463:     if (hd == NULL || e->t < e->toutstart)
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
**Lines (current code):** 104, 111, 189-194, 247-252  
**Lines at review (Oct 2026):** 104, 111, 189-194, 247-252

**Function:** `init_from_meco_dump()`

**Why it's thought to be a bug**

`validu` is never freed; `meco->zgrid[nk]` is never set; for a non-boundary cell with 0 MECO cells, `c->mecocells` is left uninitialised (only the boundary branch sets NULL). Function: `init_from_meco_dump()`.

**Current code** (`atecology/dump.c` lines 102–113)

```c
  102:     //h2acell = malloc(ni * nj * sizeof(double));
  103:     flag = malloc(ni * nj * nk * sizeof(int));
  104:     validu = malloc(ni * nj * nk * sizeof(double));
  105:     nxy = ni * nj;
  106:     meco->count[0] = 1;
  107:     meco->count[1] = nk;
  108:     meco->count[2] = nj;
  109:     meco->count[3] = ni;
  110:     meco->ncells = nk * nj * ni;
  111:     meco->zgrid = malloc((nk+1) * sizeof(double));
  112: 
  113:     /* read cell positions */
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

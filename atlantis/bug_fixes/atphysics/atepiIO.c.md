# atphysics/atepiIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-05](#phy3-05) | 224-227 vs 139,173-182 | M/M | inconsistent metadata | `writeBMEpiInfo` defines `is_boundary(t,b)` (NC_INT) with `bmtype="epibenthos"` but no `dtype`/`sumtype` attri… |  | [ ] |

---

## PHY3-05

**inconsistent metadata** · Conf M / Sev M  
**Lines (current code):** 224-227 vs 139,173-182  
**Lines at review (Oct 2026):** 224-227 vs 139,173-182

**Why it's thought to be a bug**

`writeBMEpiInfo` defines `is_boundary(t,b)` (NC_INT) with `bmtype="epibenthos"` but no `dtype`/`sumtype` attributes. If an output file is later read back (e.g. restart from output.nc), `readBMEpiInfo` picks it up through `ncvarfind(...,"bmtype","epibenthos")`. The `ncattinq(...,"dtype")` call then fails, and with NC_FATAL set the run exits. Even if it did not, the variable would be read as a double epi tracer. Functions: `writeBMEpiInfo()`, `readBMEpiInfo()`.

**Current code** (`atphysics/atepiIO.c` lines 222–229)

```c
  222:     
  223:     /* Note that nominal_dz is not time dependent */
  224:     i = ncvardef(fid, "is_boundary", NC_INT, 2, dim);
  225:     ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");
  226:     ncattput(fid, i, "units", NC_CHAR, (int) strlen("") + 1, "");
  227:     ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Whether box is a boundary box") + 1, "Whether box is a boundary box");
  228: 
  229: 	/* Loop over the epi variables */
```

**Suggested fix (from review):** Give is_boundary a different bmtype (e.g. "phys") or add dtype/sumtype and skip it on read

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

`quit(... "%d" ..., sizeof(doubleINPUT))` passes a size_t to `%d` (atsummaryIO.c:310,408; atepiIO.c:386; atLandIO.c:299; atdiagIO.c:283; atfishstatIO.c:268; atdietIO.c:227). This only matters on the error path (L).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atepiIO.c:303-316: the scaling match uses `strstr(name, groupName)`, so a short group code that is a substring of another (e.g. "BC" in "BCX_N") can pick the wrong init_scalar.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

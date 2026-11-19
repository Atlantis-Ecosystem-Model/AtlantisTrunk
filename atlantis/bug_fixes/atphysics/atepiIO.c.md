# atphysics/atepiIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-05](#phy3-05) | 79, 139, 225 | M/M | inconsistent metadata | `writeBMEpiInfo` defines `is_boundary(t,b)` (NC_INT) with `bmtype="epibenthos"` but no `dtype`/`sumtype` attri… |  | [ ] |

---

## PHY3-05

**inconsistent metadata** · Conf M / Sev M  
**Found at (current code):** `atepiIO.c` 79, `atepiIO.c` 139, `atepiIO.c` 225
**Lines at review (Oct 2026):** 224-227 vs 139,173-182

**Why it's thought to be a bug**

`writeBMEpiInfo` defines `is_boundary(t,b)` (NC_INT) with `bmtype="epibenthos"` but no `dtype`/`sumtype` attributes. If an output file is later read back (e.g. restart from output.nc), `readBMEpiInfo` picks it up through `ncvarfind(...,"bmtype","epibenthos")`. The `ncattinq(...,"dtype")` call then fails, and with NC_FATAL set the run exits. Even if it did not, the variable would be read as a double epi tracer. Functions: `writeBMEpiInfo()`, `readBMEpiInfo()`.

**Code at issue** — `atepiIO.c` line 79  
Search for: `bmtype = "epibenthos"`

```c
   77:  // A typical epibenthos entry
   78:  double weed(t,b);                // Weed values
   79→  weed:bmtype = "epibenthos";    // This is an epibenthos variable
   80:  weed:units = "Percent";        // Units for weed
   81:  weed:long_name = "Weed Cover"; // Long name of weed
```

**Code at issue** — `atepiIO.c` line 139  
Search for: `ncvarfind(`

```c
  137: 	idlist = i_alloc1d(nvars);
  138: 
  139→ 	n = ncvarfind(fid, 2, dims, "bmtype", "epibenthos", idlist);
  140: 	if (n < 1)
  141: 		warn("readBMEpiInfo: No epibenthos variables\n");
```

**Code at issue** — `atepiIO.c` line 225 (closest match to the quoted code; wording differs slightly)  
Search for: `ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");`

```c
  223:     /* Note that nominal_dz is not time dependent */
  224:     i = ncvardef(fid, "is_boundary", NC_INT, 2, dim);
  225→     ncattput(fid, i, "bmtype", NC_CHAR, (int) strlen("epibenthos") + 1, "epibenthos");
  226:     ncattput(fid, i, "units", NC_CHAR, (int) strlen("") + 1, "");
  227:     ncattput(fid, i, "long_name", NC_CHAR, (int) strlen("Whether box is a boundary box") + 1, "Whether box is a boundary box");
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

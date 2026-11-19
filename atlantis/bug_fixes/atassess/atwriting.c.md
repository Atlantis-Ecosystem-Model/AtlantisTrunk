# atassess/atwriting.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [WRT-01](#wrt-01) | 593 | M/M | div-by-zero | CPUE = catch/effort with no guard (line 1099 adds small_num). |  | [ ] |
| [WRT-02](#wrt-02) ⭐ | 914 | H/H | stale layout | Header reads `Salt LightWc LightSed O2 DON DIN`, but samplephys[0..5] = salinity, light_wc, light_sed, o2_sed,… |  | [ ] |
| [WRT-03](#wrt-03) | 1647, 1700 | M/M | size mismatch | Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped. |  | [ ] |
| [WRT-04](#wrt-04) | 854 | H/L | header text | Hard-coded bin counts in headers; "Location" label; missing `#`/space/newline. |  | [ ] |
| [WRT-05](#wrt-05) | 2648 | H/L | dead code | `WriteDietOut` is never called, so indiet/offdiet/oi files are never produced. |  | [ ] |

---

## WRT-01

**div-by-zero** · Conf M / Sev M  
**Found at (current code):** `atwriting.c` 593
**Lines at review (Oct 2026):** 768

**Why it's thought to be a bug**

CPUE = catch/effort with no guard (line 1099 adds small_num).

**Code at issue** — `atwriting.c` line 593 (located by name only; check this is the right place)  
Search for: `area_correct = (zonearea[z] / (1000.0 * 1000.0)) + small_num; /* So can get output per km2`

```c
  591: 		/* Write Info */
  592: 		if (bm->flagareacorrect)
  593→ 			area_correct = (zonearea[z] / (1000.0 * 1000.0)) + small_num; /* So can get output per km2 from m2 total areas*/
  594: 		else
  595: 			area_correct = 1.0;
```

**Suggested fix (from review):** `+ small_num`

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

## WRT-02

**stale layout** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atwriting.c` 914
**Lines at review (Oct 2026):** 914, 940-941

**Why it's thought to be a bug**

Header reads `Salt LightWc LightSed O2 DON DIN`, but samplephys[0..5] = salinity, light_wc, light_sed, o2_sed, o2_wc, TSS. DON/DIN/Temp (rows 6, 7, 9) are never written.

**Code at issue** — `atwriting.c` line 914 (the line the review pointed to, found by its text)  
Search for: `fprintf(fid, "#Time Zone Salt LightWc LightSed O2 DON DIN Chla nitrif denitrif #\n");`

```c
  912: 
  913:     /* Column definitions */
  914→ 	fprintf(fid, "#Time Zone Salt LightWc LightSed O2 DON DIN Chla nitrif denitrif #\n");
  915: 
  916: 	/* Return file pointer */
```

**Suggested fix (from review):** write named ids and match the header

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

## WRT-03

**size mismatch** · Conf M / Sev M  
**Found at (current code):** `atwriting.c` 1647, `atwriting.c` 1700
**Lines at review (Oct 2026):** 1647-1650, 1700-1707

**Why it's thought to be a bug**

Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped.

**Code at issue** — `atwriting.c` line 1647 (the line the review pointed to, found by its text)  
Search for: `for (j = 0; j < bm->K_num_abcbin; j++)`

```c
 1645:     /* Column definitions */
 1646: 	fprintf(fid, "#Time Zone");
 1647→ 	for (j = 0; j < bm->K_num_abcbin; j++)
 1648: 		fprintf(fid, " B%d", j + 1);
 1649: 	for (j = 0; j < bm->K_num_abcbin; j++)
```

**Code at issue** — `atwriting.c` line 1700 (the line the review pointed to, found by its text)  
Search for: `for (j = 0; j < bm->K_num_abcbin; j++) {`

```c
 1698: 
 1699: 		/* Write Biomass Info */
 1700→         for (j = 0; j < bm->K_num_abcbin; j++) {
 1701: 			fprintf(fid, "%.8g ", endnums[j][bioindex_id][z]);
 1702: 		}
```

**Suggested fix (from review):** cap in ParamIO

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

## WRT-04

**header text** · Conf H / Sev L  
**Found at (current code):** `atwriting.c` 854
**Lines at review (Oct 2026):** 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782

**Why it's thought to be a bug**

Hard-coded bin counts in headers; "Location" label; missing `#`/space/newline.

**Code at issue** — `atwriting.c` line 854 (the review quoted no code: this is the line it pointed to, found by its text; check)  
Search for: `fprintf(fid, "#Time Zone Species Age Bin0 Bin1 Bin2 Bin3 Bin4 Bin5 Bin6 Bin7 Bin8 Bin9 Bin`

```c
  852: 
  853:     /* Column definitions */
  854→ 	fprintf(fid, "#Time Zone Species Age Bin0 Bin1 Bin2 Bin3 Bin4 Bin5 Bin6 Bin7 Bin8 Bin9 Bin10 #\n");
  855: 
  856: 	/* Return file pointer */
```

**Suggested fix (from review):** build headers from loop bounds

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

## WRT-05

**dead code** · Conf H / Sev L  
**Found at (current code):** `atwriting.c` 2648
**Lines at review (Oct 2026):** 2648-2669

**Why it's thought to be a bug**

`WriteDietOut` is never called, so indiet/offdiet/oi files are never produced.

**Code at issue** — `atwriting.c` line 2648 (the line the review pointed to, found by its text)  
Search for: `void WriteDietOut(MSEBoxModel *bm, int ij) {`

```c
 2646:  *
 2647:  */
 2648→ void WriteDietOut(MSEBoxModel *bm, int ij) {
 2649: 
 2650: 	if (verbose)
```

**Suggested fix (from review):** confirm intent

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

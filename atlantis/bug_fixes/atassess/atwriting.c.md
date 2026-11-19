# atassess/atwriting.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [WRT-01](#wrt-01) | 768 | M/M | div-by-zero | CPUE = catch/effort with no guard (line 1099 adds small_num). |  | [ ] |
| [WRT-02](#wrt-02) ⭐ | 914, 940-941 | H/H | stale layout | Header reads `Salt LightWc LightSed O2 DON DIN`, but samplephys[0..5] = salinity, light_wc, light_sed, o2_sed,… |  | [ ] |
| [WRT-03](#wrt-03) | 1647-1650, 1700-1707 | M/M | size mismatch | Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped. |  | [ ] |
| [WRT-04](#wrt-04) | 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782 | H/L | header text | Hard-coded bin counts in headers; "Location" label; missing `#`/space/newline. |  | [ ] |
| [WRT-05](#wrt-05) | 2648-2669 | H/L | dead code | `WriteDietOut` is never called, so indiet/offdiet/oi files are never produced. |  | [ ] |

---

## WRT-01

**div-by-zero** · Conf M / Sev M  
**Lines (current code):** 768  
**Lines at review (Oct 2026):** 768

**Why it's thought to be a bug**

CPUE = catch/effort with no guard (line 1099 adds small_num).

**Current code** (`atassess/atwriting.c` lines 766–770) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  766: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  767: 			if (FunctGroupArray[sp].isVertebrate == TRUE) {
  768: 				fprintf(fid, "%.8g ", stockinfo[stotcatch_id][sp][z][id] / stockinfo[stoteffort_id][sp][z][id]);
  769: 			}
  770: 		}
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
**Lines (current code):** 914, 940-941  
**Lines at review (Oct 2026):** 914, 940-941

**Why it's thought to be a bug**

Header reads `Salt LightWc LightSed O2 DON DIN`, but samplephys[0..5] = salinity, light_wc, light_sed, o2_sed, o2_wc, TSS. DON/DIN/Temp (rows 6, 7, 9) are never written.

**Current code** (`atassess/atwriting.c` lines 912–943)

```c
  912: 
  913:     /* Column definitions */
  914: 	fprintf(fid, "#Time Zone Salt LightWc LightSed O2 DON DIN Chla nitrif denitrif #\n");
  915: 
  916: 	/* Return file pointer */
  917: 	return (fid);
  918: }
  919: 
  920: /**
  921:  *
  922:  *	\brief Routine to write data to the physical and nutrient information file
  923:  *
  924:  */
  925: void Write_Physs(FILE *fid, MSEBoxModel *bm, int ij, int id) {
  926: 	int j, z;
  927: 
  928: 	if (verbose > 1)
  929:         printf( "Write physics\n");
  930: 
  931: 	for (z = 0; z < bm->nfzones; z++) {
  932: 
  933: 		/* Write Time */
  934: 		fprintf(fid, "%d ", ij);
  935: 
  936: 		/* Write Zone */
  937: 		fprintf(fid, "%d ", z);
  938: 
  939: 		/* Write Info */
  940: 		for (j = 0; j < 6; j++)
  941: 			fprintf(fid, "%.8g ", samplephys[j][z][id]);
  942: 		for (j = 2; j >= 0; j--)
  943: 			fprintf(fid, "%.8g ", sampleprocess[j][z][id]);
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
**Lines (current code):** 1647-1650, 1700-1707  
**Lines at review (Oct 2026):** 1647-1650, 1700-1707

**Why it's thought to be a bug**

Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped.

**Current code** (`atassess/atwriting.c` lines 1645–1652)

```c
 1645:     /* Column definitions */
 1646: 	fprintf(fid, "#Time Zone");
 1647: 	for (j = 0; j < bm->K_num_abcbin; j++)
 1648: 		fprintf(fid, " B%d", j + 1);
 1649: 	for (j = 0; j < bm->K_num_abcbin; j++)
 1650: 		fprintf(fid, " A%d", j + 1);
 1651: 
 1652: 	index = 0;
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
**Lines (current code):** 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782  
**Lines at review (Oct 2026):** 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782

**Why it's thought to be a bug**

Hard-coded bin counts in headers; "Location" label; missing `#`/space/newline.

**Current code** (`atassess/atwriting.c` lines 852–856)

```c
  852: 
  853:     /* Column definitions */
  854: 	fprintf(fid, "#Time Zone Species Age Bin0 Bin1 Bin2 Bin3 Bin4 Bin5 Bin6 Bin7 Bin8 Bin9 Bin10 #\n");
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
**Lines (current code):** 2648-2669  
**Lines at review (Oct 2026):** 2648-2669

**Why it's thought to be a bug**

`WriteDietOut` is never called, so indiet/offdiet/oi files are never produced.

**Current code** (`atassess/atwriting.c` lines 2646–2671)

```c
 2646:  *
 2647:  */
 2648: void WriteDietOut(MSEBoxModel *bm, int ij) {
 2649: 
 2650: 	if (verbose)
 2651: 		fprintf(stderr, "Writing diet output\n");
 2652: 
 2653: 	/* Initialise files if necessary */
 2654: 	if (!indietfp && bm->samplediet)
 2655: 		indietfp = Init_InDiet_File(bm, "indietinfo.txt");
 2656: 	if (!offdietfp && bm->samplediet)
 2657: 		offdietfp = Init_OffDiet_File(bm, "offdietinfo.txt");
 2658: 	if (!oifp)
 2659: 		oifp = Init_Oi_File(bm, "oiinfo.txt");
 2660: 
 2661: 	/* Write files */
 2662: 	if (bm->samplediet) {
 2663: 		Write_InDiets(indietfp, bm, ij, inshorediet);
 2664: 		Write_OffDiets(offdietfp, bm, ij, offshorediet);
 2665: 	}
 2666: 	Write_Ois(oifp, bm, ij);
 2667: 
 2668: 	return;
 2669: }
 2670: 
 2671: /**
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

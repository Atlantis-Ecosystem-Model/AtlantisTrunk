# atassess/atwriting.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **5 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [WRT-01](#wrt-01) | 768 | M/M | div-by-zero | CPUE = catch/effort with no guard (line 1099 adds small_num). |  | [ ] |
| [WRT-02](#wrt-02) ⭐ | 914, 940-941 | H/H | stale layout | Header reads `Salt LightWc LightSed O2 DON DIN`, but samplephys[0..5] = salinity, light_wc, light_sed, o2_sed,… |  | [ ] |
| [WRT-03](#wrt-03) | 1647-1650, 1700-1707 | M/M | size mismatch | Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped. |  | [ ] |
| [WRT-04](#wrt-04) | 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782 | H/L | header text | Hard-coded bin counts in headers; "Location" label; missing `#`/space/newline. |  | [ ] |
| [WRT-05](#wrt-05) | 2648-2669 | H/L | dead code | `WriteDietOut` is never called, so indiet/offdiet/oi files are never produced. |  | [ ] |

---

## WRT-01

**div-by-zero** · Conf M / Sev M  
**Listed lines:** 768

**Why it's thought to be a bug**

CPUE = catch/effort with no guard (line 1099 adds small_num).

**Current code** (`atassess/atwriting.c` lines 591–595) — code appears to have moved -175 lines since the review

```c
  591: 		/* Write Info */
  592: 		if (bm->flagareacorrect)
  593: 			area_correct = (zonearea[z] / (1000.0 * 1000.0)) + small_num; /* So can get output per km2 from m2 total areas*/
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
**Listed lines:** 914, 940-941

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
**Listed lines:** 1647-1650, 1700-1707

**Why it's thought to be a bug**

Loops to `K_num_abcbin` over `endnums` sized K_num_tot_sp; the parameter is not capped.

**Current code** (`atassess/atwriting.c` lines 1645–1684 (truncated))

```c
 1645:     /* Column definitions */
 1646: 	fprintf(fid, "#Time Zone");
 1647: 	for (j = 0; j < bm->K_num_abcbin; j++)
 1648: 		fprintf(fid, " B%d", j + 1);
 1649: 	for (j = 0; j < bm->K_num_abcbin; j++)
 1650: 		fprintf(fid, " A%d", j + 1);
 1651: 
 1652: 	index = 0;
 1653: 	for (j = 0; j < bm->K_num_tot_sp; j++) {
 1654: 		if (FunctGroupArray[j].isMacroFauna == TRUE) {
 1655: 			fprintf(fid, " BB%d", index + 1);
 1656: 			index++;
 1657: 		}
 1658: 	}
 1659: 	fprintf(fid, " BB%d", index + 1);
 1660: 
 1661: 	index = 0;
 1662: 	for (j = 0; j < bm->K_num_tot_sp; j++) {
 1663: 		if (FunctGroupArray[j].isMacroFauna == TRUE) {
 1664: 			fprintf(fid, " BA%d", index + 1);
 1665: 			index++;
 1666: 		}
 1667: 	}
 1668: 	fprintf(fid, " BA%d", index + 1);
 1669: 
 1670: 	//	for (j = 0; j < bm->K_num_abcbin - bm->K_num_microfauna; j++)
 1671: 	//		fprintf(fid, " BA%d", j + 1);
 1672: 
 1673: 	/* New line */
 1674: 	fprintf(fid, "\n");
 1675: 	/* Return file pointer */
 1676: 	return (fid);
 1677: }
 1678: 
 1679: /**
 1680:  *
 1681:  *	\brief Routine to write data to the ABC curve indices file
 1682:  *
 1683:  */
 1684: void Write_Abcs(FILE *fid, MSEBoxModel *bm, int ij) {
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
**Listed lines:** 854, 1540, 1593, 2179, 1286/1323/1361, 2473, 1486, 2708, 2782

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
**Listed lines:** 2648-2669

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

# ateconomic/ateconio.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-01](#ecio-01) | 169-170 | H/L | copy-paste (message) | `quit("Init_Econ_Meas_File: Can't open %s\n",fname)` sits inside `Init_Econ_Ind_File()` (EconSubIndx.txt), so … |  | [ ] |
| [ECIO-02](#ecio-02) | 313-341 | M/L | output ambiguity / copy-paste | `Write_Real_Effort_Schedule()` writes a block of `EffortSchedule[..][expect_id]` rows (a copy of `Write_Effort… |  | [ ] |
| [ECIO-03](#ecio-03) | 195, 615, 690, 952, 1011 vs 290, 315, 330 | L/L | inconsistent loop bound | The effort-schedule writers loop `ns < max(1, nsubfleets)`, but the econ-index, price, deemed-value, catch-per… |  | [ ] |
| [ECIO-04](#ecio-04) | 119-132, 906-907, 1039-1040 | L/L | dangling static pointer | `Close_Econ_Indx_Files()` closes every static FILE* but does not set it to NULL. `Economic_Output_Temporal_Cat… |  | [ ] |

---

## ECIO-01

**copy-paste (message)** · Conf H / Sev L  
**Listed lines:** 169-170

**Function:** `Init_Econ_Ind_File()`

**Why it's thought to be a bug**

`quit("Init_Econ_Meas_File: Can't open %s\n",fname)` sits inside `Init_Econ_Ind_File()` (EconSubIndx.txt), so the error names the wrong function/file. Function: `Init_Econ_Ind_File()`.

**Current code** (`ateconomic/ateconio.c` lines 167–172)

```c
  167: 
  168:     /** Create file **/
  169:     if( (fid=Util_fopen(bm, fname, "w")) == NULL )
  170:     	quit("Init_Econ_Meas_File: Can't open %s\n",fname);
  171: 
  172:     /** Column definitions **/
```

**Suggested fix (from review):** Change text to `Init_Econ_Ind_File`.

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

## ECIO-02

**output ambiguity / copy-paste** · Conf M / Sev L  
**Listed lines:** 313-341

**Function:** `Write_Real_Effort_Schedule()`

**Why it's thought to be a bug**

`Write_Real_Effort_Schedule()` writes a block of `EffortSchedule[..][expect_id]` rows (a copy of `Write_Effort_Schedule()`) and then a block of `[current_id]` rows. Both blocks have the same key columns (Time Fishery Subfleet) and the header (l.276) has no column saying which is which, so the expected and realised rows can't be told apart in RealEffortSchedIndx.txt. The expected values are already written to EffortSchedIndx.txt. Function: `Write_Real_Effort_Schedule()`.

**Current code** (`ateconomic/ateconio.c` lines 305–337) — code appears to have moved -6 lines since the review

```c
  305: }
  306: 
  307: void Write_Real_Effort_Schedule(FILE *fid, MSEBoxModel *bm) {
  308: 	int nf, ns, month;
  309: 
  310: 	if (verbose > 1)
  311:         printf( "Write realised gross effort schedule\n");
  312: 
  313: 	/* Write predicted effort schedule values */
  314: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  315: 		for (ns = 0; ns < max(1,bm->FISHERYprms[nf][nsubfleets_id]); ns++) {
  316: 
  317: 			/* Write time and port */
  318: 			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);
  319: 
  320: 			/* Economic effort schedule */
  321: 			for (month = 0; month < 12; month++) {
  322: 				fprintf(fid, " %e", bm->EffortSchedule[nf][ns][month][expect_id]);
  323: 			}
  324: 			fprintf(fid, "\n");
  325: 		}
  326: 	}
  327: 
  328: 	/* Write realised effort schedule values */
  329: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  330: 		for (ns = 0; ns < max(1,bm->FISHERYprms[nf][nsubfleets_id]); ns++) {
  331: 
  332: 			/* Write time and port */
  333: 			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);
  334: 
  335: 			/* Realised effort schedule */
  336: 			for (month = 0; month < 12; month++) {
  337: 				fprintf(fid, " %e", bm->EffortSchedule[nf][ns][month][current_id]);
```

**Suggested fix (from review):** Drop the first (expect_id) block, or add a "Type" column.

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

## ECIO-03

**inconsistent loop bound** · Conf L / Sev L  
**Listed lines:** 195, 615, 690, 952, 1011 vs 290, 315, 330

**Why it's thought to be a bug**

The effort-schedule writers loop `ns < max(1, nsubfleets)`, but the econ-index, price, deemed-value, catch-per-month and expected-catch writers loop `ns < nsubfleets`. A fishery with 0 subfleets therefore gets rows in some output files and not in others. Functions: `Write_Econ_Ind()`, `Write_Prices()`, `Write_Deemed_Values()`, `Write_Temporal_Catches()`, `Write_Expected_CatchPerMonth()`.

**Current code** (`ateconomic/ateconio.c` lines 193–197)

```c
  193: 	/* Write catch values */
  194: 	for (nf = 0; nf < bm->K_num_fisheries; nf++) {
  195: 		for (ns = 0; ns < bm->FISHERYprms[nf][nsubfleets_id]; ns++) {
  196: 			/* Write time, fishery and subfleet */
  197: 			fprintf(fid, "%e %s %d", bm->dayt, FisheryArray[nf].fisheryCode, ns);
```

**Suggested fix (from review):** Use one convention everywhere.

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

## ECIO-04

**dangling static pointer** · Conf L / Sev L  
**Listed lines:** 119-132, 906-907, 1039-1040

**Why it's thought to be a bug**

`Close_Econ_Indx_Files()` closes every static FILE* but does not set it to NULL. `Economic_Output_Temporal_Catches()` and `Economic_Output_ExpectedCatchPerMonth()` lazily reopen only `if (!fp)`, so any write after the close would use a freed FILE*. Latent: depends on call order. Functions: `Close_Econ_Indx_Files()`.

**Current code** (`ateconomic/ateconio.c` lines 117–134)

```c
  117: }
  118: 
  119: void Close_Econ_Indx_Files(MSEBoxModel *bm) {
  120: 	Util_Close_Output_File(anneconindfp);
  121: 	Util_Close_Output_File(pricesfp);
  122: 	Util_Close_Output_File(deemedValuefp);
  123: 	Util_Close_Output_File(efftschedfp);
  124: 	Util_Close_Output_File(realefftschedfp);
  125: 	Util_Close_Output_File(tradefp);
  126: 	Util_Close_Output_File(anneconfp);
  127: 	Util_Close_Output_File(annportfp);
  128: 	Util_Close_Output_File(quotaallocfp);
  129: 	Util_Close_Output_File(tradematrixfp);
  130: 	Util_Close_Output_File(tempcatchfp);
  131: 	Util_Close_Output_File(expectedcatchpermonthfp);
  132: }
  133: 
  134: /**
```

**Suggested fix (from review):** Set each pointer to NULL after closing.

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

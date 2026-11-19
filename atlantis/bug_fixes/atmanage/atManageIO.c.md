# atmanage/atManageIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-42](#mngio-42) | 158-160, 177 | H/L | header/column mismatch | The header order is `Blim BrefB BrefA`, but the row prints `Blim, BrefA, BrefB` (matching the argument order a… |  | [ ] |
| [MNGIO-43](#mngio-43) | 58-66 | H/L | resource leak | `cpuefp` and `grosscpuefp` are opened lazily in Write_CPUEreport and Write_CPUE but never closed or flushed in… |  | [ ] |

---

## MNGIO-42

**header/column mismatch** · Conf H / Sev L  
**Lines (current code):** 158-160, 177  
**Lines at review (Oct 2026):** 158-160, 177

**Why it's thought to be a bug**

The header order is `Blim BrefB BrefA`, but the row prints `Blim, BrefA, BrefB` (matching the argument order and the callers in atManageAnnual.c:1334), so the BrefA and BrefB columns are mislabelled. Functions: `initAnnBrokenStickFile()`, `WriteAnnBrokenStickFile()`.

**Current code** (`atmanage/atManageIO.c` lines 156–179)

```c
  156:     fprintf(fid,"FrefA ");
  157:     fprintf(fid,"FrefH ");
  158:     fprintf(fid,"Blim ");
  159:     fprintf(fid,"BrefB ");
  160:     fprintf(fid,"BrefA ");
  161:     fprintf(fid,"FCurr ");
  162:     fprintf(fid,"FTARG ");
  163:     fprintf(fid,"CurrentBiomass ");
  164:     fprintf(fid,"BrokenStickScaler ");
  165: 	fprintf(fid,"\n");
  166:     /* Return file pointer */
  167:     return(fid);
  168: }
  169: /**
  170:  * \brief Write out info to the broken stick output file.
  171:  */
  172: void WriteAnnBrokenStickFile(MSEBoxModel *bm,  int species, int nf, int tier, double FrefLim, double FrefA, double FrefH, double Blim, double BrefA, double BrefB, double FCurr, double FTARG, double Bcurr, double scalar)
  173: {
  174: 	if(annBrokenfp == NULL){
  175: 		annBrokenfp = initAnnBrokenStickFile(bm);
  176: 	}
  177: 	fprintf(annBrokenfp, "%e %s %s %d %e %e %e %e %e %e %e %e %e %e\n", bm->dayt, FunctGroupArray[species].groupCode,  FisheryArray[nf].fisheryCode, tier, FrefLim, FrefA, FrefH, Blim, BrefA, BrefB, FCurr, FTARG, Bcurr, scalar);
  178: }
  179: 
```

**Suggested fix (from review):** Swap the header labels to `BrefA BrefB`.

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

## MNGIO-43

**resource leak** · Conf H / Sev L  
**Lines (current code):** 58-66  
**Lines at review (Oct 2026):** 58-66

**Why it's thought to be a bug**

`cpuefp` and `grosscpuefp` are opened lazily in Write_CPUEreport and Write_CPUE but never closed or flushed in `Close_Manage_Output_Files()`, so trailing output may be lost.

**Current code** (`atmanage/atManageIO.c` lines 56–68)

```c
   56:  * \brief Close the management output text files.
   57:  */
   58: void Close_Manage_Output_Files(MSEBoxModel *bm) {
   59: 
   60: 	/* Close the annual TAC reporting file */
   61: 	Util_Close_Output_File(anntacfp);
   62: 	/* Close the broken stick output file */
   63: 	Util_Close_Output_File(annBrokenfp);
   64:     /* Close the ecosystem caps output file */
   65:     Util_Close_Output_File(annCapResultfp);
   66: }
   67: 
   68: /**
```

**Suggested fix (from review):** Add Util_Close_Output_File for both.

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

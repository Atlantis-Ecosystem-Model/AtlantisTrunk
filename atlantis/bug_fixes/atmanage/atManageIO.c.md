# atmanage/atManageIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNGIO-42](#mngio-42) | 172 | H/L | header/column mismatch | The header order is `Blim BrefB BrefA`, but the row prints `Blim, BrefA, BrefB` (matching the argument order a… |  | [ ] |
| [MNGIO-43](#mngio-43) | 58 | H/L | resource leak | `cpuefp` and `grosscpuefp` are opened lazily in Write_CPUEreport and Write_CPUE but never closed or flushed in… |  | [ ] |

---

## MNGIO-42

**header/column mismatch** · Conf H / Sev L  
**Found at (current code):** `atManageIO.c` 172
**Lines at review (Oct 2026):** 158-160, 177

**Why it's thought to be a bug**

The header order is `Blim BrefB BrefA`, but the row prints `Blim, BrefA, BrefB` (matching the argument order and the callers in atManageAnnual.c:1334), so the BrefA and BrefB columns are mislabelled. Functions: `initAnnBrokenStickFile()`, `WriteAnnBrokenStickFile()`.

**Code at issue** — `atManageIO.c` line 172 (located by name only; check this is the right place)  
Search for: `void WriteAnnBrokenStickFile(MSEBoxModel *bm,  int species, int nf, int tier, double FrefL`

```c
  170:  * \brief Write out info to the broken stick output file.
  171:  */
  172→ void WriteAnnBrokenStickFile(MSEBoxModel *bm,  int species, int nf, int tier, double FrefLim, double FrefA, double FrefH, double Blim, double BrefA, double BrefB, double FCurr, double FTARG, double Bcurr, double scalar)
  173: {
  174: 	if(annBrokenfp == NULL){
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
**Found at (current code):** `atManageIO.c` 58
**Lines at review (Oct 2026):** 58-66

**Why it's thought to be a bug**

`cpuefp` and `grosscpuefp` are opened lazily in Write_CPUEreport and Write_CPUE but never closed or flushed in `Close_Manage_Output_Files()`, so trailing output may be lost.

**Code at issue** — `atManageIO.c` line 58 (the line the review pointed to, found by its text)  
Search for: `void Close_Manage_Output_Files(MSEBoxModel *bm) {`

```c
   56:  * \brief Close the management output text files.
   57:  */
   58→ void Close_Manage_Output_Files(MSEBoxModel *bm) {
   59: 
   60: 	/* Close the annual TAC reporting file */
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

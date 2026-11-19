# atphysics/atswr.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-26](#phy3-26) | 56 | H/L | format/argument mismatch | `quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr)`: an int is pas… |  | [ ] |
| [PHY3-27](#phy3-27) | 80, 86, 114 | M/L | buffer overrun | `char buf[200]` is filled by `readkeyprm_s(fp,"Solar_radiation",buf)`, while the caller `readSolar` uses `buf[… |  | [ ] |

---

## PHY3-26

**format/argument mismatch** · Conf H / Sev L  
**Found at (current code):** `atswr.c` 56
**Lines at review (Oct 2026):** 56

**Function:** `swrForcingBM()`

**Why it's thought to be a bug**

`quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr)`: an int is passed for `%s` and a double for `%d`, so the error path crashes. Function: `swrForcingBM()`.

**Code at issue** — `atswr.c` line 56  
Search for: `quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr)`

```c
   54: 				bp->swr = inputData->dataBuffer[b][0];
   55: 				if (!_finite(bp->swr)) {
   56→ 					quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr);
   57: 				}
   58: 				/*fprintf(bm->logFile, "Time %e: %s in box %d is set to %e, inputData->tleft= %e, rewind = %d\n",
```

**Suggested fix (from review):** Reorder args: `inputData->variableName, b, bm->dayt, bp->swr`

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

## PHY3-27

**buffer overrun** · Conf M / Sev L  
**Found at (current code):** `atswr.c` 80, `atswr.c` 86, `atswr.c` 114
**Lines at review (Oct 2026):** 80,86,96

**Function:** `init_SolarData()`

**Why it's thought to be a bug**

`char buf[200]` is filled by `readkeyprm_s(fp,"Solar_radiation",buf)`, while the caller `readSolar` uses `buf[STRLEN]` (2000) for the same key. A long path overflows. Function: `init_SolarData()`.

**Code at issue** — `atswr.c` line 80  
Search for: `char buf[200]`

```c
   78: 
   79: void init_SolarData(MSEBoxModel *bm, FILE *fp, PhyPropertyData *propInput, char *variableName, char *shortName, char *longName, double minValue, double maxValue){
   80→ 	char buf[200];
   81: 
   82: 	if (propInput->tracerID == -1)
```

**Code at issue** — `atswr.c` line 86  
Search for: `readkeyprm_s(fp, "Solar_radiation", buf)`

```c
   84: 
   85: 	propInput->is_valid_z = FALSE;
   86→ 	readkeyprm_s(fp, "Solar_radiation", buf);
   87: 
   88: 	/* Will hard code for now - might want more than one solar nc file in the future but they shouldn't be large files as they don't include z */
```

**Code at issue** — `atswr.c` line 114  
Search for: `buf[STRLEN]`

```c
  112: 
  113: void readSolar(MSEBoxModel *bm, char *tunit){
  114→ 	char buf[STRLEN];
  115: 	FILE *fp;
  116: 
```

**Suggested fix (from review):** Use `char buf[STRLEN]`

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

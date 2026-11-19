# atphysics/atswr.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-26](#phy3-26) | 56 | H/L | format/argument mismatch | `quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr)`: an int is pas… |  | [ ] |
| [PHY3-27](#phy3-27) | 80,86,96 | M/L | buffer overrun | `char buf[200]` is filled by `readkeyprm_s(fp,"Solar_radiation",buf)`, while the caller `readSolar` uses `buf[… |  | [ ] |

---

## PHY3-26

**format/argument mismatch** · Conf H / Sev L  
**Listed lines:** 56

**Function:** `swrForcingBM()`

**Why it's thought to be a bug**

`quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr)`: an int is passed for `%s` and a double for `%d`, so the error path crashes. Function: `swrForcingBM()`.

**Current code** (`atphysics/atswr.c` lines 54–58)

```c
   54: 				bp->swr = inputData->dataBuffer[b][0];
   55: 				if (!_finite(bp->swr)) {
   56: 					quit("%s in %d at time %e is not finite %e\n", b, bm->dayt, inputData->variableName, bp->swr);
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
**Listed lines:** 80,86,96

**Function:** `init_SolarData()`

**Why it's thought to be a bug**

`char buf[200]` is filled by `readkeyprm_s(fp,"Solar_radiation",buf)`, while the caller `readSolar` uses `buf[STRLEN]` (2000) for the same key. A long path overflows. Function: `init_SolarData()`.

**Current code** (`atphysics/atswr.c` lines 84–104) — code appears to have moved +6 lines since the review

```c
   84: 
   85: 	propInput->is_valid_z = FALSE;
   86: 	readkeyprm_s(fp, "Solar_radiation", buf);
   87: 
   88: 	/* Will hard code for now - might want more than one solar nc file in the future but they shouldn't be large files as they don't include z */
   89: 	propInput->nFiles = 1;
   90: 
   91: 	/* Allocate memory for file names if necessary */
   92: 	propInput->fname = c_alloc2d(BMSLEN, propInput->nFiles);
   93:     propInput->use_resets = (int *)i_alloc1d(propInput->nFiles);
   94:     propInput->use_resets[0] = 0;
   95:     
   96: 	strcpy(propInput->fname[0], buf);
   97: 	propInput->variableName = c_alloc1d(BMSLEN);
   98: 	strcpy(propInput->variableName, variableName);
   99: 	snprintf(buf, sizeof(buf), "%s_rewind", shortName);
  100: 	readkeyprm_i(fp, buf, &propInput->rewind);
  101: 
  102:     if(propInput->rewind == FALSE) {  // Just inc case looking for swr_rewind and its Solar_radiation_rewind
  103:         snprintf(buf, sizeof(buf), "Solar_radiation_rewind");
  104:         readkeyprm_i(fp, buf, &propInput->rewind);
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

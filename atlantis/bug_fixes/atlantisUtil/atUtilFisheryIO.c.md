# atlantisUtil/atUtilFisheryIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-13](#utl2-13) | 106 | L/L | NULL dereference | `FisheryArray = malloc(...)` is never checked for NULL before use. Function: `Util_Read_Fisheries_XML()`. |  | [ ] |
| [UTL2-14](#utl2-14) | 92 | L/M | logic | `bm->K_max_num_subfleet = 5; // -1;` sets a floor of 5 on the max subfleet count, so arrays sized by it are la… |  | [ ] |

---

## UTL2-13

**NULL dereference** · Conf L / Sev L  
**Found at (current code):** `atUtilFisheryIO.c` 106
**Lines at review (Oct 2026):** 106

**Function:** `Util_Read_Fisheries_XML()`

**Why it's thought to be a bug**

`FisheryArray = malloc(...)` is never checked for NULL before use. Function: `Util_Read_Fisheries_XML()`.

**Code at issue** — `atUtilFisheryIO.c` line 106 (closest match to the quoted code; wording differs slightly)  
Search for: `FisheryArray = (FisheryStruct *) malloc(sizeof(FisheryStruct) * (size_t)(bm->K_num_fisheri`

```c
  104: 		}
  105: 
  106→ 		FisheryArray = (FisheryStruct *) malloc(sizeof(FisheryStruct) * (size_t)(bm->K_num_fisheries + 1));
  107: 
  108: 
```

**Suggested fix (from review):** Check for NULL and quit.

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

## UTL2-14

**logic** · Conf L / Sev M  
**Found at (current code):** `atUtilFisheryIO.c` 92
**Lines at review (Oct 2026):** 92, 124-125

**Function:** `Util_Read_Fisheries_XML()`

**Why it's thought to be a bug**

`bm->K_max_num_subfleet = 5; // -1;` sets a floor of 5 on the max subfleet count, so arrays sized by it are larger than needed. It is probably deliberate (the -1 is commented out), but any loop over 0..K_max_num_subfleet will visit subfleets that don't exist for fisheries with fewer. Function: `Util_Read_Fisheries_XML()`.

**Code at issue** — `atUtilFisheryIO.c` line 92  
Search for: `K_max_num_subfleet= 5; // -1`

```c
   90: 
   91: 	}
   92→ 	bm->K_max_num_subfleet= 5; // -1;
   93: 
   94: 	/* Read in each of the functional groups */
```

**Suggested fix (from review):** Confirm intent. Otherwise initialise to 0 (or -1).

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

atUtilFisheryIO.c:60: `strstr(convertedXMLFileName, ".csv")` matches the first ".csv" anywhere in the path, not the extension. The overlong-string strcpy into `fisheryCode[20]` (Util_XML_Get_Value_String, atUtilXML.c:1363) is unbounded.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`xmlCleanupParser()` at atUtilFisheryIO.c:136 is called mid-run, and libxml2 documents it as unsafe if the library is used afterwards. This is a common pattern in the codebase.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

# sjwlib/convertFileFormats.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-20](#sjw2-20) | 431 | H/L | wrong mode (copy-paste) | On `_WIN32`, `pFlag->FromToMode = FROMTO_DOS2UNIX` is set, but then `ConvertUnixToDos()` is called. That funct… |  | [ ] |
| [SJW2-21](#sjw2-21) | 422 | M/L | unchecked fopen / leak / ignored error | The `outputFile = fopen(...)` result is not checked (NULL then goes to fputc/fclose). `pFlag` is never freed. … |  | [ ] |

---

## SJW2-20

**wrong mode (copy-paste)** · Conf H / Sev L  
**Found at (current code):** `convertFileFormats.c` 431
**Lines at review (Oct 2026):** 425-439

**Function:** `Convert_File_Format()`

**Why it's thought to be a bug**

On `_WIN32`, `pFlag->FromToMode = FROMTO_DOS2UNIX` is set, but then `ConvertUnixToDos()` is called. That function's switch only handles `FROMTO_UNIX2DOS`/`FROMTO_UNIX2MAC` (lines 291-404), so it hits `default:` and writes nothing, leaving the temp file empty. The only caller (datafile.c:1736) is commented out, so this is latent. Function: `Convert_File_Format()`.

**Code at issue** — `convertFileFormats.c` line 431  
Search for: `pFlag->FromToMode = FROMTO_DOS2UNIX`

```c
  429: 	pFlag->KeepDate = 0;
  430: 	pFlag->ConvMode = CONVMODE_ASCII; /* default ascii */
  431→ 	pFlag->FromToMode = FROMTO_DOS2UNIX; /* default dos2unix */
  432: 	pFlag->NewLine = 0;
  433: 	pFlag->Force = 0;
```

**Suggested fix (from review):** Set `FROMTO_UNIX2DOS` on Windows (or call the matching converter).

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

## SJW2-21

**unchecked fopen / leak / ignored error** · Conf M / Sev L  
**Found at (current code):** `convertFileFormats.c` 422
**Lines at review (Oct 2026):** 420-463

**Function:** `Convert_File_Format()`

**Why it's thought to be a bug**

The `outputFile = fopen(...)` result is not checked (NULL then goes to fputc/fclose). `pFlag` is never freed. The conversion return value is ignored, so a "binary symbol" abort leaves a silently truncated temp file while the function returns TRUE. Function: `Convert_File_Format()`.

**Code at issue** — `convertFileFormats.c` line 422  
Search for: `outputFile = fopen(`

```c
  420: 	if ((inputFile = fopen(fileName, "r")) == NULL)
  421: 		return -1;
  422→ 	outputFile = fopen(tempFileName, "w");
  423: 
  424: 	/* What operating system is this? */
```

**Suggested fix (from review):** Check fopen, free pFlag, and propagate the conversion's return value.

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

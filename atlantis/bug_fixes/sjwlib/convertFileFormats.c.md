# sjwlib/convertFileFormats.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW2-20](#sjw2-20) | 425-439 | H/L | wrong mode (copy-paste) | On `_WIN32`, `pFlag->FromToMode = FROMTO_DOS2UNIX` is set, but then `ConvertUnixToDos()` is called. That funct… |  | [ ] |
| [SJW2-21](#sjw2-21) | 420-463 | M/L | unchecked fopen / leak / ignored error | The `outputFile = fopen(...)` result is not checked (NULL then goes to fputc/fclose). `pFlag` is never freed. … |  | [ ] |

---

## SJW2-20

**wrong mode (copy-paste)** · Conf H / Sev L  
**Listed lines:** 425-439

**Function:** `Convert_File_Format()`

**Why it's thought to be a bug**

On `_WIN32`, `pFlag->FromToMode = FROMTO_DOS2UNIX` is set, but then `ConvertUnixToDos()` is called. That function's switch only handles `FROMTO_UNIX2DOS`/`FROMTO_UNIX2MAC` (lines 291-404), so it hits `default:` and writes nothing, leaving the temp file empty. The only caller (datafile.c:1736) is commented out, so this is latent. Function: `Convert_File_Format()`.

**Current code** (`sjwlib/convertFileFormats.c` lines 423–441)

```c
  423: 
  424: 	/* What operating system is this? */
  425: #ifdef _WIN32
  426: 	pFlag = (CFlag*)malloc(sizeof(CFlag));
  427: 	pFlag->NewFile = 0;
  428: 	pFlag->Quiet = 0;
  429: 	pFlag->KeepDate = 0;
  430: 	pFlag->ConvMode = CONVMODE_ASCII; /* default ascii */
  431: 	pFlag->FromToMode = FROMTO_DOS2UNIX; /* default dos2unix */
  432: 	pFlag->NewLine = 0;
  433: 	pFlag->Force = 0;
  434: 	pFlag->Follow = SYMLINK_SKIP;
  435: 	pFlag->status = 0;
  436: 	pFlag->stdio_mode = 1;
  437: 	pFlag->error = 0;
  438: 
  439: 	ConvertUnixToDos(inputFile, outputFile, pFlag, "atlantis");
  440: 
  441: #else
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
**Listed lines:** 420-463

**Function:** `Convert_File_Format()`

**Why it's thought to be a bug**

The `outputFile = fopen(...)` result is not checked (NULL then goes to fputc/fclose). `pFlag` is never freed. The conversion return value is ignored, so a "binary symbol" abort leaves a silently truncated temp file while the function returns TRUE. Function: `Convert_File_Format()`.

**Current code** (`sjwlib/convertFileFormats.c` lines 418–457 (truncated))

```c
  418: #endif
  419: 
  420: 	if ((inputFile = fopen(fileName, "r")) == NULL)
  421: 		return -1;
  422: 	outputFile = fopen(tempFileName, "w");
  423: 
  424: 	/* What operating system is this? */
  425: #ifdef _WIN32
  426: 	pFlag = (CFlag*)malloc(sizeof(CFlag));
  427: 	pFlag->NewFile = 0;
  428: 	pFlag->Quiet = 0;
  429: 	pFlag->KeepDate = 0;
  430: 	pFlag->ConvMode = CONVMODE_ASCII; /* default ascii */
  431: 	pFlag->FromToMode = FROMTO_DOS2UNIX; /* default dos2unix */
  432: 	pFlag->NewLine = 0;
  433: 	pFlag->Force = 0;
  434: 	pFlag->Follow = SYMLINK_SKIP;
  435: 	pFlag->status = 0;
  436: 	pFlag->stdio_mode = 1;
  437: 	pFlag->error = 0;
  438: 
  439: 	ConvertUnixToDos(inputFile, outputFile, pFlag, "atlantis");
  440: 
  441: #else
  442: 
  443: 	pFlag = (CFlag*) malloc(sizeof(CFlag));
  444: 	pFlag->NewFile = 0;
  445: 	pFlag->Quiet = 0;
  446: 	pFlag->KeepDate = 0;
  447: 	pFlag->ConvMode = CONVMODE_ASCII; /* default ascii */
  448: 	pFlag->FromToMode = FROMTO_UNIX2DOS; /* default unix2dos */
  449: 	pFlag->NewLine = 0;
  450: 	pFlag->Force = 0;
  451: 	pFlag->Follow = SYMLINK_SKIP;
  452: 	pFlag->status = 0;
  453: 	pFlag->stdio_mode = 1;
  454: 	pFlag->error = 0;
  455: 
  456: 	ConvertDosToUnix(inputFile, outputFile, pFlag, "atlantis");
  457: 
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

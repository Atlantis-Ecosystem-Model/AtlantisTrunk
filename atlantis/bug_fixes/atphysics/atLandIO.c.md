# atphysics/atLandIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-08](#phy3-08) | 37, 40 | H/L | NULL dereference | If `tracerName` does not contain "Land_", `name` stays NULL and is passed to `strcmp` (crash). If "Land_" appe… |  | [ ] |
| [PHY3-09](#phy3-09) | 276 | H/L | copy-paste log text | `"Entering writeBMEpiData\n"` printed in `writeBMLandData()`. |  | [ ] |

---

## PHY3-08

**NULL dereference** · Conf H / Sev L  
**Found at (current code):** `atLandIO.c` 37, `atLandIO.c` 40
**Lines at review (Oct 2026):** 37-46

**Function:** `Find_WC_Tracer()`

**Why it's thought to be a bug**

If `tracerName` does not contain "Land_", `name` stays NULL and is passed to `strcmp` (crash). If "Land_" appears but not at the start, the offset is wrong. The function is currently unused (latent). Function: `Find_WC_Tracer()`.

**Code at issue** — `atLandIO.c` line 37 (the line the review pointed to, found by its text)  
Search for: `name = strstr(tracerName, "Land_");`

```c
   35: 	char *name;
   36: 
   37→ 	name = strstr(tracerName, "Land_");
   38: 
   39: 	if(name != NULL){
```

**Code at issue** — `atLandIO.c` line 40 (the line the review pointed to, found by its text)  
Search for: `name = tracerName + strlen("Land_");`

```c
   38: 
   39: 	if(name != NULL){
   40→ 		name = tracerName + strlen("Land_");
   41: 	}
   42: 
```

**Suggested fix (from review):** `name = (strncmp(tracerName,"Land_",5)==0) ? tracerName+5 : tracerName;`

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

## PHY3-09

**copy-paste log text** · Conf H / Sev L  
**Found at (current code):** `atLandIO.c` 276
**Lines at review (Oct 2026):** 276

**Why it's thought to be a bug**

`"Entering writeBMEpiData\n"` printed in `writeBMLandData()`.

**Code at issue** — `atLandIO.c` line 276  
Search for: `"Entering writeBMEpiData\n"`

```c
  274: 
  275: 	if (verbose > 0)
  276→ 		fprintf(stderr, "Entering writeBMEpiData\n");
  277: 
  278: 	/* Set netCDF library error handling */
```

**Suggested fix (from review):** Change to writeBMLandData

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

# atphysics/atLandIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-08](#phy3-08) | 37-46 | H/L | NULL dereference | If `tracerName` does not contain "Land_", `name` stays NULL and is passed to `strcmp` (crash). If "Land_" appe… |  | [ ] |
| [PHY3-09](#phy3-09) | 276 | H/L | copy-paste log text | `"Entering writeBMEpiData\n"` printed in `writeBMLandData()`. |  | [ ] |

---

## PHY3-08

**NULL dereference** · Conf H / Sev L  
**Listed lines:** 37-46

**Function:** `Find_WC_Tracer()`

**Why it's thought to be a bug**

If `tracerName` does not contain "Land_", `name` stays NULL and is passed to `strcmp` (crash). If "Land_" appears but not at the start, the offset is wrong. The function is currently unused (latent). Function: `Find_WC_Tracer()`.

**Current code** (`atphysics/atLandIO.c` lines 35–48)

```c
   35: 	char *name;
   36: 
   37: 	name = strstr(tracerName, "Land_");
   38: 
   39: 	if(name != NULL){
   40: 		name = tracerName + strlen("Land_");
   41: 	}
   42: 
   43: 
   44: 	for(i = 0; i < bm->ntracer; i++){
   45: 
   46: 		if( strcmp(name, bm->tinfo[i].name) == 0 ){
   47: 		  index = bm->tinfo[i].n;
   48: 		  return index;
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
**Listed lines:** 276

**Why it's thought to be a bug**

`"Entering writeBMEpiData\n"` printed in `writeBMLandData()`.

**Current code** (`atphysics/atLandIO.c` lines 274–278)

```c
  274: 
  275: 	if (verbose > 0)
  276: 		fprintf(stderr, "Entering writeBMEpiData\n");
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

# atecology/atfluxbreakdown.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-12](#bmsg-12) | 67-70, 88-91, 100-102, 113-130, 146-148, 159-175, 195-198, 214-217, 228-247, 258-292 | H/L | wrong logic | Each test is `if (fabs(x) > maxflux)` but then stores `maxflux = x` (signed). After a large negative flux wins… |  | [ ] |
| [BMSG-13](#bmsg-13) | 53, 295-296 | H/M | missing case | The switch handles only WC, SED and EPIFAUNA. atecology.c:3640 calls `Max_Flux_Check(bm, ICE_BASED, ...)` when… |  | [ ] |
| [BMSG-14](#bmsg-14) | 637-647, 663-684, 697-713, 725-735, 748-758, 771-781, 869-882 | M/L | missing case | `Print_Eat_Diagnostics()` passes `ICE_BASED`/`LAND_BASED` as `habitat` to the Print_* helpers, but every helpe… |  | [ ] |
| [BMSG-15](#bmsg-15) | 805-826 | M/L | wrong file / NULL deref | The header lines go to the global `logfp` but the values go to `llogfp`. Also `fp` stays NULL if `diagType` is… |  | [ ] |
| [BMSG-16](#bmsg-16) | 322-327 | L/L | wrong label | Any `level_id` other than WC or SED is labelled "epi", so ICE_BASED calls (atecology.c:3613) are logged as epi… |  | [ ] |
| [BMSG-17](#bmsg-17) | 572-580 | L/L | always-true condition | `if(numCohorts > 0)` is always true, so the else branch never runs. The WC branch (432) uses `> 1`. The output… |  | [ ] |

---

## BMSG-12

**wrong logic** · Conf H / Sev L  
**Listed lines:** 67-70, 88-91, 100-102, 113-130, 146-148, 159-175, 195-198, 214-217, 228-247, 258-292

**Function:** `Max_Flux_Check()`

**Why it's thought to be a bug**

Each test is `if (fabs(x) > maxflux)` but then stores `maxflux = x` (signed). After a large negative flux wins, `maxflux` is negative, so every later candidate passes the test and the last one checked is reported as the culprit. Function: `Max_Flux_Check()`.

**Current code** (`atecology/atfluxbreakdown.c` lines 65–104)

```c
   65: 						valcheck = (FunctGroupArray[fgIndex].grow[n][SN_id] + FunctGroupArray[fgIndex].grow[n][RN_id])
   66: 								* VERTinfo[fgIndex][n][DEN_id] / bm->cell_vol;
   67: 						if (fabs(valcheck) > maxflux) {
   68: 							ans = FunctGroupArray[fgIndex].totNTracers[n];
   69: 							maxflux = valcheck;
   70: 							flux_sp = fgIndex;
   71: 							fluxnum = WC; // Our should this be growth_id ?
   72: 							fcnum = n;
   73: 						}
   74:                         
   75:                         /* Biomass check 2 - never used as first check will trip it so don't both doing the test
   76:                         if (fabs(valcheck) > maxflux) {
   77:                             ans = -fgIndex;
   78:                             maxflux = valcheck;
   79:                             flux_sp = fgIndex;
   80:                             fluxnum = growth_id;
   81:                             fcnum = n;
   82:                         }
   83:                          */
   84:                         
   85:                         /* Death check */
   86: 						valcheck = (FunctGroupArray[fgIndex].dead[n] + (double)FunctGroupArray[fgIndex].preyEaten[n][WC]) * (VERTinfo[fgIndex][n][SN_id]
   87: 								+ VERTinfo[fgIndex][n][RN_id]) / bm->cell_vol;
   88: 						if (fabs(valcheck) > maxflux) {
   89: 							ans = -fgIndex;
   90: 							maxflux = valcheck;
   91: 							flux_sp = fgIndex;
   92: 							fluxnum = death_id;
   93: 							fcnum = n;
   94: 						}
   95: 					}
   96: 					break;
   97: 				case BIOMASS:	/* Intentional */
   98: 				case AGE_STRUCTURED_BIOMASS:
   99: 					for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
  100: 						if (fabs(localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]]) > maxflux) {
  101: 							ans = FunctGroupArray[fgIndex].totNTracers[n];
  102: 							maxflux = localWCFlux[FunctGroupArray[fgIndex].totNTracers[n]];
  103: 							flux_sp = fgIndex;
  104: 							fluxnum = WC;
```

**Suggested fix (from review):** Store `maxflux = fabs(x)` everywhere.

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

## BMSG-13

**missing case** · Conf H / Sev M  
**Listed lines:** 53, 295-296

**Function:** `Max_Flux_Check()`

**Why it's thought to be a bug**

The switch handles only WC, SED and EPIFAUNA. atecology.c:3640 calls `Max_Flux_Check(bm, ICE_BASED, ...)` when the ice flux balance fails (`ecotest > 1`), which hits `default: quit("No such callerid ...")`. The ice diagnostic path ends the run instead of reporting the culprit. Function: `Max_Flux_Check()`.

**Current code** (`atecology/atfluxbreakdown.c` lines 51–55)

```c
   51: 	fcnum = 0;
   52: 
   53: 	switch (callerid) {
   54: 	case WC: /* Watercolumn box */
   55: 
```

**Suggested fix (from review):** Add an `ICE_BASED` case that scans `localICEFlux` (the caller passes it in all three slots), or stop calling it for ice.

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

## BMSG-14

**missing case** · Conf M / Sev L  
**Listed lines:** 637-647, 663-684, 697-713, 725-735, 748-758, 771-781, 869-882

**Why it's thought to be a bug**

`Print_Eat_Diagnostics()` passes `ICE_BASED`/`LAND_BASED` as `habitat` to the Print_* helpers, but every helper only switches on WC/SED/EPIFAUNA, so nothing is printed. It is called that way at atecology.c:3684. Functions: `Print_PreyInfo_Value()`, `Print_PreyEaten_Value()`, etc.

**Current code** (`atecology/atfluxbreakdown.c` lines 635–649)

```c
  635: 
  636: 	for (cohort = 0; cohort < FunctGroupArray[guild].numCohorts * FunctGroupArray[guild].numGeneTypes; cohort++) {
  637: 		switch (habitat) {
  638: 		case WC:
  639: 			fprintf(llogfp, "grazeInfo %s (%d) wc %.14e\n", FunctGroupArray[guild].groupCode, guild, GRAZEinfo[guild][cohort][habitat]);
  640: 			break;
  641: 		case SED:
  642: 			fprintf(llogfp, "grazeInfo %s (%d) sed %.14e\n", FunctGroupArray[guild].groupCode, guild, GRAZEinfo[guild][cohort][habitat]);
  643: 			break;
  644: 		case EPIFAUNA:
  645: 			fprintf(llogfp, "grazeInfo %s (%d) epi %.14e\n", FunctGroupArray[guild].groupCode, guild, GRAZEinfo[guild][cohort][habitat]);
  646: 			break;
  647: 		}
  648: 	}
  649: }
```

**Suggested fix (from review):** Add ICE_BASED/LAND_BASED cases to the helpers (the arrays are sized `num_active_habitats`).

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

## BMSG-15

**wrong file / NULL deref** · Conf M / Sev L  
**Listed lines:** 805-826

**Function:** `Print_Eat_Diagnostics()`

**Why it's thought to be a bug**

The header lines go to the global `logfp` but the values go to `llogfp`. Also `fp` stays NULL if `diagType` is not 0-4, and is then called (NULL function pointer). Function: `Print_Eat_Diagnostics()`.

**Current code** (`atecology/atfluxbreakdown.c` lines 803–828)

```c
  803: 	void (*fp)(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) = NULL; // Function pointer
  804: 
  805: 	switch (diagType) {
  806: 	case 0:
  807: 		fp = Print_PreyInfo_Value;
  808: 		fprintf(logfp, "Prey info values\n");
  809: 		break;
  810: 	case 1:
  811: 		fp = Print_PreyEaten_Value;
  812: 		fprintf(logfp, "Prey eaten values after %d, %s\n", guild, FunctGroupArray[guild].groupCode);
  813: 		break;
  814: 	case 2:
  815: 		fp = Print_GrazeInfo_Values;
  816: 		fprintf(logfp, "GrazeInfo values after %d, %s\n", guild, FunctGroupArray[guild].groupCode);
  817: 		break;
  818: 	case 3:
  819: 		fp = Print_SPEAT_Value;
  820: 		fprintf(logfp, "spEat values\n");
  821: 		break;
  822: 	case 4:
  823: 		fp = Print_EATINGinfo_Value;
  824: 		fprintf(logfp, "EATINGinfo values\n");
  825: 		break;
  826: 	}
  827: 
  828: 	switch (habitatType) {
```

**Suggested fix (from review):** Use `llogfp` for the headers; add `default: quit(...)`.

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

## BMSG-16

**wrong label** · Conf L / Sev L  
**Listed lines:** 322-327

**Function:** `Call_Diagnostics()`

**Why it's thought to be a bug**

Any `level_id` other than WC or SED is labelled "epi", so ICE_BASED calls (atecology.c:3613) are logged as epi. Function: `Call_Diagnostics()`.

**Current code** (`atecology/atfluxbreakdown.c` lines 320–329)

```c
  320: 	char *location;
  321: 
  322: 	if (level_id == WC)
  323: 		location = loc1;
  324: 	else if (level_id == SED)
  325: 		location = loc2;
  326: 	else
  327: 		location = loc3;
  328: 
  329: 	if (bm->checkNH || qnancall) {
```

**Suggested fix (from review):** Add an "ice" label for ICE_BASED (and "land").

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

## BMSG-17

**always-true condition** · Conf L / Sev L  
**Listed lines:** 572-580

**Function:** `Print_Flux()`

**Why it's thought to be a bug**

`if(numCohorts > 0)` is always true, so the else branch never runs. The WC branch (432) uses `> 1`. The output is still right because the loop covers all cohorts. Function: `Print_Flux()`.

**Current code** (`atecology/atfluxbreakdown.c` lines 570–582)

```c
  570: 				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
  571: 				case BIOMASS:
  572: 					if(FunctGroupArray[fgIndex].numCohorts > 0){
  573: 						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
  574: 							fprintf(llogfp, "%s-%d (%s-%d) N epiflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
  575: 									localEPIFlux[FunctGroupArray[fgIndex].totNTracers[n]]);
  576: 						}
  577: 					} else {
  578: 						fprintf(llogfp, "%s (%s) N epiflux: %.20e\n", FunctGroupArray[fgIndex].fullName, FunctGroupArray[fgIndex].groupCode,
  579: 								localEPIFlux[FunctGroupArray[fgIndex].totNTracers[0]]);
  580: 					}
  581: 					break;
  582: 				}
```

**Suggested fix (from review):** Use `> 1` to match line 432.

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

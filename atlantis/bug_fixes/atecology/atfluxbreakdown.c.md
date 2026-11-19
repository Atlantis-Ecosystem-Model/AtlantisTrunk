# atecology/atfluxbreakdown.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **6 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-12](#bmsg-12) | 67 | H/L | wrong logic | Each test is `if (fabs(x) > maxflux)` but then stores `maxflux = x` (signed). After a large negative flux wins… |  | [ ] |
| [BMSG-13](#bmsg-13) | 295 | H/M | missing case | The switch handles only WC, SED and EPIFAUNA. atecology.c:3640 calls `Max_Flux_Check(bm, ICE_BASED, ...)` when… |  | [ ] |
| [BMSG-14](#bmsg-14) | 801 | M/L | missing case | `Print_Eat_Diagnostics()` passes `ICE_BASED`/`LAND_BASED` as `habitat` to the Print_* helpers, but every helpe… |  | [ ] |
| [BMSG-15](#bmsg-15) | 801 | M/L | wrong file / NULL deref | The header lines go to the global `logfp` but the values go to `llogfp`. Also `fp` stays NULL if `diagType` is… |  | [ ] |
| [BMSG-16](#bmsg-16) | 869 | L/L | wrong label | Any `level_id` other than WC or SED is labelled "epi", so ICE_BASED calls (atecology.c:3613) are logged as epi… |  | [ ] |
| [BMSG-17](#bmsg-17) | 572 | L/L | always-true condition | `if(numCohorts > 0)` is always true, so the else branch never runs. The WC branch (432) uses `> 1`. The output… |  | [ ] |

---

## BMSG-12

**wrong logic** · Conf H / Sev L  
**Found at (current code):** `atfluxbreakdown.c` 67
**Lines at review (Oct 2026):** 67-70, 88-91, 100-102, 113-130, 146-148, 159-175, 195-198, 214-217, 228-247, 258-292

**Function:** `Max_Flux_Check()`

**Why it's thought to be a bug**

Each test is `if (fabs(x) > maxflux)` but then stores `maxflux = x` (signed). After a large negative flux wins, `maxflux` is negative, so every later candidate passes the test and the last one checked is reported as the culprit. Function: `Max_Flux_Check()`.

**Code at issue** — `atfluxbreakdown.c` line 67 (closest match to the quoted code; wording differs slightly)  
Search for: `if (fabs(valcheck) > maxflux) {`

```c
   65: 						valcheck = (FunctGroupArray[fgIndex].grow[n][SN_id] + FunctGroupArray[fgIndex].grow[n][RN_id])
   66: 								* VERTinfo[fgIndex][n][DEN_id] / bm->cell_vol;
   67→ 						if (fabs(valcheck) > maxflux) {
   68: 							ans = FunctGroupArray[fgIndex].totNTracers[n];
   69: 							maxflux = valcheck;
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
**Found at (current code):** `atfluxbreakdown.c` 295
**Lines at review (Oct 2026):** 53, 295-296

**Function:** `Max_Flux_Check()`

**Why it's thought to be a bug**

The switch handles only WC, SED and EPIFAUNA. atecology.c:3640 calls `Max_Flux_Check(bm, ICE_BASED, ...)` when the ice flux balance fails (`ecotest > 1`), which hits `default: quit("No such callerid ...")`. The ice diagnostic path ends the run instead of reporting the culprit. Function: `Max_Flux_Check()`.

**Code at issue** — `atfluxbreakdown.c` line 295  
Search for: `default:`

```c
  293: 		}
  294: 		break;
  295→ 	default:
  296: 		quit("No such callerid defined for Max_Flux_Check - how did it get here?\n");
  297: 		break;
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
**Found at (current code):** `atfluxbreakdown.c` 801
**Lines at review (Oct 2026):** 637-647, 663-684, 697-713, 725-735, 748-758, 771-781, 869-882

**Why it's thought to be a bug**

`Print_Eat_Diagnostics()` passes `ICE_BASED`/`LAND_BASED` as `habitat` to the Print_* helpers, but every helper only switches on WC/SED/EPIFAUNA, so nothing is printed. It is called that way at atecology.c:3684. Functions: `Print_PreyInfo_Value()`, `Print_PreyEaten_Value()`, etc.

**Code at issue** — `atfluxbreakdown.c` line 801 (located by name only; check this is the right place)  
Search for: `void Print_Eat_Diagnostics(MSEBoxModel *bm, FILE *llogfp, int guild, HABITAT_TYPES habitat`

```c
  799:  *
  800:  */
  801→ void Print_Eat_Diagnostics(MSEBoxModel *bm, FILE *llogfp, int guild, HABITAT_TYPES habitatType, int diagType) {
  802: 	int sp;
  803: 	void (*fp)(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) = NULL; // Function pointer
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
**Found at (current code):** `atfluxbreakdown.c` 801
**Lines at review (Oct 2026):** 805-826

**Function:** `Print_Eat_Diagnostics()`

**Why it's thought to be a bug**

The header lines go to the global `logfp` but the values go to `llogfp`. Also `fp` stays NULL if `diagType` is not 0-4, and is then called (NULL function pointer). Function: `Print_Eat_Diagnostics()`.

**Code at issue** — `atfluxbreakdown.c` line 801 (located by name only; check this is the right place)  
Search for: `void Print_Eat_Diagnostics(MSEBoxModel *bm, FILE *llogfp, int guild, HABITAT_TYPES habitat`

```c
  799:  *
  800:  */
  801→ void Print_Eat_Diagnostics(MSEBoxModel *bm, FILE *llogfp, int guild, HABITAT_TYPES habitatType, int diagType) {
  802: 	int sp;
  803: 	void (*fp)(MSEBoxModel *bm, FILE *llogfp, int guild, int habitat, HABITAT_TYPES habitatType) = NULL; // Function pointer
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
**Found at (current code):** `atfluxbreakdown.c` 869
**Lines at review (Oct 2026):** 322-327

**Function:** `Call_Diagnostics()`

**Why it's thought to be a bug**

Any `level_id` other than WC or SED is labelled "epi", so ICE_BASED calls (atecology.c:3613) are logged as epi. Function: `Call_Diagnostics()`.

**Code at issue** — `atfluxbreakdown.c` line 869 (located by name only; check this is the right place)  
Search for: `case ICE_BASED:`

```c
  867: 
  868: 		break;
  869→ 	case ICE_BASED:
  870: 		for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
  871: 			if (FunctGroupArray[sp].isVertebrate == FALSE && FunctGroupArray[sp].habitatCoeffs[ICE_BASED] > 0) {
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
**Found at (current code):** `atfluxbreakdown.c` 572
**Lines at review (Oct 2026):** 572-580

**Function:** `Print_Flux()`

**Why it's thought to be a bug**

`if(numCohorts > 0)` is always true, so the else branch never runs. The WC branch (432) uses `> 1`. The output is still right because the loop covers all cohorts. Function: `Print_Flux()`.

**Code at issue** — `atfluxbreakdown.c` line 572 (the line the review pointed to, found by its text)  
Search for: `if(FunctGroupArray[fgIndex].numCohorts > 0){`

```c
  570: 				case AGE_STRUCTURED_BIOMASS:	/* Intentional */
  571: 				case BIOMASS:
  572→ 					if(FunctGroupArray[fgIndex].numCohorts > 0){
  573: 						for (n = 0; n < FunctGroupArray[fgIndex].numCohortsXnumGenes; n++) {
  574: 							fprintf(llogfp, "%s-%d (%s-%d) N epiflux: %.20e\n", FunctGroupArray[fgIndex].fullName, n, FunctGroupArray[fgIndex].groupCode, n,
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

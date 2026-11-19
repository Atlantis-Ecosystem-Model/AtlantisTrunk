# atecology/atPhysChemIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BMSG-20](#bmsg-20) | 77, 138, 40 | M/M | inconsistent condition | The count uses `if(bm->track_rugosity_arag)` (truthy), but the add uses `if(bm->track_rugosity_arag == TRUE)`.… |  | [ ] |

---

## BMSG-20

**inconsistent condition** · Conf M / Sev M  
**Found at (current code):** `atPhysChemIO.c` 77, `atPhysChemIO.c` 138, `atPhysChemIO.c` 40
**Lines at review (Oct 2026):** 77-79 vs 138-141

**Function:** `Ecology_Assign_Physio_Chem()`

**Why it's thought to be a bug**

The count uses `if(bm->track_rugosity_arag)` (truthy), but the add uses `if(bm->track_rugosity_arag == TRUE)`. The flag is read with `integer_check` (atparamIO.c:209), so a value such as 2 reserves 2 slots that are never filled. Their `tracerIndex` pointers are garbage and get dereferenced in `Print_Flux()` and `Check_Layer_Initial_Biomass()` (`*PhysioChemArray[n].tracerIndex`). Function: `Ecology_Assign_Physio_Chem()`.

**Code at issue** — `atPhysChemIO.c` line 77  
Search for: `if(bm->track_rugosity_arag)`

```c
   75: 		bm->K_num_physiochem += 14;  // The raw C and P nutrient tracers
   76: 	}
   77→ 	if(bm->track_rugosity_arag) {
   78: 		bm->K_num_physiochem += 2;
   79: 	}
```

**Code at issue** — `atPhysChemIO.c` line 138  
Search for: `if(bm->track_rugosity_arag == TRUE)`

```c
  136: 	}
  137: 
  138→ 	if(bm->track_rugosity_arag == TRUE) {
  139: 		Add_Physio_Chem_Tracer(bm, llogfp, i++, "Rugosity", &Rugosity_i, 0, 0, 0, NULL, NULL);
  140: 		Add_Physio_Chem_Tracer(bm, llogfp, i++, "AragoniteSaturation", &AragoniteSat_i, 0, 0, 0, NULL, NULL);
```

**Code at issue** — `atPhysChemIO.c` line 40 (closest match to the quoted code; wording differs slightly)  
Search for: `PhysioChemArray[index].tracerIndex = tracerIndex;`

```c
   38: 	strcpy(PhysioChemArray[index].name, name);
   39: 
   40→ 	PhysioChemArray[index].tracerIndex = tracerIndex;
   41: 	PhysioChemArray[index].diagTol = tol;
   42: 	PhysioChemArray[index].diagFlux = flux;
```

**Suggested fix (from review):** Use the same test in both places (e.g. `if (bm->track_rugosity_arag)` at line 138).

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

atPhysChemIO.c 88-93: if `flagIsEstuary` and `flagpassivetracer` are both set, two PhysioChemArray entries named "SED" share `&SED_i`, so SED is printed and checked twice. The comment says the overload is intentional, but turning both on probably was not anticipated.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

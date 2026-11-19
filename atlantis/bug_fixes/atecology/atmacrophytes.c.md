# atecology/atmacrophytes.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 3 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [BIOL-25](#biol-25) | 88 | H/L | format specifier mismatch | `quit("This is not coded for %s group types ...", FunctGroupArray[sp_id].groupType, ...)` — `groupType` is a `… |  | [ ] |
| [BIOL-26](#biol-26) | 190-192 vs 118 | L/M | wrong parameter | Epiphyte nutrient limitation uses `KNepi_id` (l.118), but epiphyte NH/NO uptake partitioning uses `KN_id` (the… |  | [ ] |

---

## BIOL-25

**format specifier mismatch** · Conf H / Sev L  
**Lines (current code):** 88  
**Lines at review (Oct 2026):** 88

**Function:** `Grow_Macrophytes()`

**Why it's thought to be a bug**

`quit("This is not coded for %s group types ...", FunctGroupArray[sp_id].groupType, ...)` — `groupType` is a `GROUP_TYPES` enum (int) (atFunctGroup.h:152), but it is passed to `%s`. This is undefined behaviour and probably a segfault instead of the intended error message (ersem_ben_lim path). Function: `Grow_Macrophytes()`.

**Current code** (`atecology/atmacrophytes.c` lines 86–90)

```c
   86:             hSPACE_sp *= min(1.0, max(0.0, (1.0 - sp_biom / (SPmax * area_hab + small_num))));
   87:         } else if (FunctGroupArray[sp_id].speciesParams[flag_lim_id] == ersem_ben_lim) {
   88:             quit("This is not coded for %s group types as yet - see Beth for a fix or change type of %s\n", FunctGroupArray[sp_id].groupType, FunctGroupArray[sp_id].groupCode);
   89:         }
   90: 
```

**Suggested fix (from review):** Use `%d`, or print the group-type name string

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

## BIOL-26

**wrong parameter** · Conf L / Sev M  
**Lines (current code):** 190-192 vs 118  
**Lines at review (Oct 2026):** 190-192 vs 118

**Function:** `Grow_Macrophytes()`

**Why it's thought to be a bug**

Epiphyte nutrient limitation uses `KNepi_id` (l.118), but epiphyte NH/NO uptake partitioning uses `KN_id` (the leaf half-saturation). This is probably a copy from the leaf/sediment branch. Function: `Grow_Macrophytes()`.

**Current code** (`atecology/atmacrophytes.c` lines 188–194)

```c
  188: 	} else {
  189: 		/* Calculate nutrient uptake - water column nutrients */
  190: 		uptakeNH = sp_grow * (NH / (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * ((FunctGroupArray[sp_id].speciesParams[KN_id] + DIN) / DIN);
  191: 		uptakeNO = sp_grow * (NO / (FunctGroupArray[sp_id].speciesParams[KN_id] + DIN)) * (FunctGroupArray[sp_id].speciesParams[KN_id]
  192: 				/ (FunctGroupArray[sp_id].speciesParams[KN_id] + NH)) * (1.0 + FunctGroupArray[sp_id].speciesParams[KN_id] / DIN);
  193: 
  194: 		/* Cap nutrient uptake to what is available - this intentionally a max as it is rescaling the NH uptake based on which is the least limiting of the limiting nutients */
```

**Suggested fix (from review):** Use `KNepi_id` in the epiphyte uptake terms (if intended)

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

atmacrophytes.c l.148-152: root growth is `this_biom(roots) * growth[main] * ...`. `growth[main]` is already a flux (biomass × rate), so root growth has units of biomass² × rate. Check this against the Savina-Rolland formulation.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atmacrophytes.c l.174-215: P and C uptake (`Calculate_Phosphorus_Uptake/Carbon_Uptake`) is only computed for the epiphyte cohort. For leaves and roots `uptakeP = uptakeC = 0` is passed to `PP_uptake()`, although growth is P-limited (`Nutrient_Lim(...P...)`), so N:P:C ratios of leaves/roots drift. In the caller (atGroupProcesses.c:751-752) P and C come from *sediment* tracers even for epiphytes, which draw uptake from the WC.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

atmacrophytes.c l.178-179: `/ DINs` has no `small_num` guard (the same pattern as atprocess.c:1664), so it gives NaN if sediment DIN is exactly 0.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

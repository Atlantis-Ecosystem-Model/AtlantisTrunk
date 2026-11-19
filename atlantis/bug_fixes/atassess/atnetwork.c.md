# atassess/atnetwork.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NET-01](#net-01) ✔ ⭐ | 81, 84 | H/M | swapped | `pop_fraction[0/1]` = juvenile fraction (atattribute.c:974-978), but the juv branch uses `1 - pop_fraction`. A… |  | [ ] |
| [NET-02](#net-02) | 182 | L/M | index assumption | `detrit_import[stage*2 + detIndex]` assumes DL=0, DR=1 in group order. |  | [ ] |
| [NET-03](#net-03) | 246 | L/L | stale values | Only `[0][0][z]` is reset; skipped groups keep `minpool` from the previous year. |  | [ ] |

---

## NET-01

**swapped** · Conf H / Sev M ✔ re-checked ⭐ priority  
**Found at (current code):** `atnetwork.c` 81, `atnetwork.c` 84
**Lines at review (Oct 2026):** 80-86

**Why it's thought to be a bug**

`pop_fraction[0/1]` = juvenile fraction (atattribute.c:974-978), but the juv branch uses `1 - pop_fraction`. Also, birds never get pop_fraction.

**Code at issue** — `atnetwork.c` line 81 (the line the review pointed to, found by its text)  
Search for: `aprop = 1.0 - pop_fraction[0][sp][z];`

```c
   79: 
   80: 						if (stage == juv_id) {
   81→ 							aprop = 1.0 - pop_fraction[0][sp][z];
   82: 							afprop = 1.0 - pop_fraction[1][sp][z];
   83: 						} else {
```

**Code at issue** — `atnetwork.c` line 84 (the line the review pointed to, found by its text)  
Search for: `aprop = pop_fraction[0][sp][z];`

```c
   82: 							afprop = 1.0 - pop_fraction[1][sp][z];
   83: 						} else {
   84→ 							aprop = pop_fraction[0][sp][z];
   85: 							afprop = pop_fraction[1][sp][z];
   86: 						}
```

**Suggested fix (from review):** swap the branches

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

## NET-02

**index assumption** · Conf L / Sev M  
**Found at (current code):** `atnetwork.c` 182
**Lines at review (Oct 2026):** 182

**Why it's thought to be a bug**

`detrit_import[stage*2 + detIndex]` assumes DL=0, DR=1 in group order.

**Code at issue** — `atnetwork.c` line 182 (closest match to the quoted code; wording differs slightly)  
Search for: `importnetwk[sp+1][stage][z] = detrit_import[(stage * 2) + detIndex][z];`

```c
  180: 										DiscardFate[detIndex][stage] * sampledetbiom[FunctGroupArray[CarrionIndex].detIndex][0][z][id];
  181: 								exportnetwk[sp+1][stage][z] = 0;
  182→ 								importnetwk[sp+1][stage][z] = detrit_import[(stage * 2) + detIndex][z];
  183: 								respnetwk[sp+1][stage][z] = 0;
  184: 								break;
```

**Suggested fix (from review):** index by type

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

## NET-03

**stale values** · Conf L / Sev L  
**Found at (current code):** `atnetwork.c` 246
**Lines at review (Oct 2026):** 58-64, 102-198, 250-256

**Why it's thought to be a bug**

Only `[0][0][z]` is reset; skipped groups keep `minpool` from the previous year.

**Code at issue** — `atnetwork.c` line 246 (located by name only; check this is the right place)  
Search for: `biomass to minpool and the production etc estimates to the level`

```c
  244: 					/* For ECOPATH calculation purposes, make sure there is an entry
  245: 					 for each group in each zone. Where a group is missing set the
  246→ 					 biomass to minpool and the production etc estimates to the level
  247: 					 matching the average per biomass unit from the areas with the
  248: 					 group present. Assume no export or import for these "missing"
```

**Suggested fix (from review):** zero all

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

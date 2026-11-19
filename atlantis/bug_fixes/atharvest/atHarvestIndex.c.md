# atharvest/atHarvestIndex.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HRV2-15](#hrv2-15) | 225 | M/L | wrong index (year) | `Update_Harvest_Index_Values()` is called from the annual block right after `bm->thisyear++` (atlantismain.c:3… |  | [ ] |

---

## HRV2-15

**wrong index (year)** · Conf M / Sev L  
**Listed lines:** 225

**Why it's thought to be a bug**

`Update_Harvest_Index_Values()` is called from the annual block right after `bm->thisyear++` (atlantismain.c:385-402). At that point `TotCumCatch[sp][nf][bm->thisyear]` is the new, empty year, so `managementCost` is effectively always 0. The just-finished year is thisyear-1, which writeAnnCatchPerFishery (atHarvestIO.c:495) uses correctly.

**Current code** (`atharvest/atHarvestIndex.c` lines 223–227)

```c
  223: 
  224: 				/* Calculate management costs */
  225: 				if ((TotCumCatch[sp][nf][bm->thisyear] > 0) && (bm->SP_FISHERYprms[sp][nf][TAC_id] < no_quota))
  226: 					managementCost++;
  227: 			}
```

**Suggested fix (from review):** Use `bm->thisyear-1` (guarded for thisyear>0)

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

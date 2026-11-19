# ConvertAtlantis/atGroupsXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-19](#cnv2-19) ⭐ | 34-42 vs 261-264 | H/H | missing column count | `isCollisionRisk` is read as an extra column when `bm->flagindustry_on` is set, and the reader requires it (at… |  | [ ] |

---

## CNV2-19

**missing column count** · Conf H / Sev H ⭐ priority  
**Listed lines:** 34-42 vs 261-264

**Function:** `Convert_Groups_To_XML()`

**Why it's thought to be a bug**

`isCollisionRisk` is read as an extra column when `bm->flagindustry_on` is set, and the reader requires it (atUtilGroupIO.c:421-422). However, `numExpectedTokens` is only raised for `external_populations`, `terrestrial_on` and `flag_pollutant_impacts`. With industry on, a correct groups CSV has one more column than expected, and the converter quits with "does not have the correct number of columns". Function: `Convert_Groups_To_XML()`.

**Current code** (`ConvertAtlantis/atGroupsXML.c` lines 32–44)

```c
   32: 
   33: 
   34:     if(bm->external_populations){
   35:         numExpectedTokens = numExpectedTokens + 1;
   36:     }
   37:     if(bm->terrestrial_on) {
   38: 		numExpectedTokens = numExpectedTokens + 1;
   39:     }
   40:     if(bm->flag_pollutant_impacts){
   41:         numExpectedTokens = numExpectedTokens + 2;
   42:     }
   43: 
   44:     /* Init libxml */
```

**Suggested fix (from review):** Add `if (bm->flagindustry_on) numExpectedTokens++;`

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

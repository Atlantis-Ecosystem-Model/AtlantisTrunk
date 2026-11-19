# ConvertAtlantis/atGroupsXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-19](#cnv2-19) ⭐ | 261 | H/H | missing column count | `isCollisionRisk` is read as an extra column when `bm->flagindustry_on` is set, and the reader requires it (at… |  | [ ] |

---

## CNV2-19

**missing column count** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atGroupsXML.c` 261
**Lines at review (Oct 2026):** 34-42 vs 261-264

**Function:** `Convert_Groups_To_XML()`

**Why it's thought to be a bug**

`isCollisionRisk` is read as an extra column when `bm->flagindustry_on` is set, and the reader requires it (atUtilGroupIO.c:421-422). However, `numExpectedTokens` is only raised for `external_populations`, `terrestrial_on` and `flag_pollutant_impacts`. With industry on, a correct groups CSV has one more column than expected, and the converter quits with "does not have the correct number of columns". Function: `Convert_Groups_To_XML()`.

**Code at issue** — `atGroupsXML.c` line 261  
Search for: `flagindustry_on`

```c
  259:             
  260:             /* Mark if the group is a collision risk */
  261→             if(bm->flagindustry_on) {
  262:                 varStr = strtok(NULL, seps);
  263:                 Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupNode, "isCollisionRisk", "", "", varStr);
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

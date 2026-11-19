# ConvertAtlantis/atImplementationtoXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-24](#cnv2-24) | 29-33 | H/L | copy-paste metadata | The group `Fishing_Catchability_Change` has the descriptions "Changes in catchability through time" and "Numbe… |  | [ ] |

---

## CNV2-24

**copy-paste metadata** · Conf H / Sev L  
**Lines (current code):** 29-33  
**Lines at review (Oct 2026):** 29-33

**Function:** `ManagementImplementationChangeXML()`

**Why it's thought to be a bug**

The group `Fishing_Catchability_Change` has the descriptions "Changes in catchability through time" and "Number of changes of fishing catchability.", and passes `"catchability"` to `Create_Fished_Groups_ChangeXML`. All of this is copied from the Qchange block, but the content is TAC changes (`TAC_num_changes_id`, `TACchange`). The node name matches the reader (atImplementationParamIO.c:25), so this affects metadata only. Function: `ManagementImplementationChangeXML()`.

**Current code** (`ConvertAtlantis/atImplementationtoXML.c` lines 27–35)

```c
   27: 
   28: 	xmlNodePtr groupingNode;
   29: 	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Catchability_Change", "Changes in catchability through time", "", "");
   30: 
   31: 	Create_Harvest_Fishery_Group_ParamXML(bm, fp, fileName, groupingNode, TAC_num_changes_id, "Number of changes of fishing catchability.", "", XML_TYPE_INTEGERARRAY);
   32: 
   33: 	Create_Fished_Groups_ChangeXML(bm, fp, fileName, doc, groupingNode, "TACchange", "catchability");
   34: }
   35: 
```

**Suggested fix (from review):** Fix the description strings

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

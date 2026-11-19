# atharvest/atHarvestParamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [HARV-08](#harv-08) | 841-845, 860-866, 874-880, 887-893, 905-910 | M/H | buffer overrun | The change arrays are allocated with max size 1 unless the global flag (`flagchangesel`, `flagchangep`, `flagc… |  | [ ] |
| [HARV-09](#harv-09) | 696-699 | M/L | copy-paste duplicate | `Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);` is called twice (lines 696 and 698), so … |  | [ ] |
| [HARV-10](#harv-10) | 583-585, 831-835, 933-935 | M/L | NULL dereference | Unlike every other reader in this file, `attributeGroupNode` (`Fishing_Scenario_Flags`), `groupingNode`/`selGr… |  | [ ] |

---

## HARV-08

**buffer overrun** · Conf M / Sev H  
**Listed lines:** 841-845, 860-866, 874-880, 887-893, 905-910

**Function:** `readFisheryChangeXML()`

**Why it's thought to be a bug**

The change arrays are allocated with max size 1 unless the global flag (`flagchangesel`, `flagchangep`, `flagchangeswept`, `flagchangediscard`, `flagchangeq`) is set. The readers (`readSelChangeXML` line 394, `Util_XML_Read_Change_Values`, `readDiscardChangeXML` line 438, `Util_XML_Read_Change_Fished_Groups`) still write `num_changes` entries for each fishery/group. If any `*_num_changes > 1` while the global flag is 0, this is a heap overflow. The fix-up that turns on global flags from per-fishery flags (atHarvestSetup.c:111-129) runs after `Read_Harvest_Parameters` (atHarvestSetup.c:94), so it is too late. Function: `readFisheryChangeXML()`.

**Current code** (`atharvest/atHarvestParamIO.c` lines 839–878 (truncated))

```c
  839:     /* Get the max number of changes, allocate a big enough array and load the values */
  840: 
  841: 	SEL_max_num_changes = 1;
  842: 	if (bm->flagchangesel)
  843: 		SEL_max_num_changes = Manage_Get_Max_Fishery_Param(bm, SEL_num_changes_id) + 1;
  844: 
  845: 	SELchange = Util_Alloc_Init_3D_Double(4, SEL_max_num_changes, bm->K_num_fisheries, 0.0);
  846:     
  847:     /* not sure about the grouping of these nodes */
  848: 	//selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Selectivity_Change");
  849: 
  850: 	readSelChangeXML(bm, fileName, selGroupNode, "Start", start_id);
  851: 	readSelChangeXML(bm, fileName, selGroupNode, "NewCurve", selcurvechange_id);
  852: 	readSelChangeXML(bm, fileName, selGroupNode, "ConstantValueAddition", addlsm_id);
  853: 	readSelChangeXML(bm, fileName, selGroupNode, "SigmaValueAddition", addsigma_id);
  854:     
  855: 	/* */
  856: 	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Coverage_Change");
  857: 
  858: 	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, P_num_changes_id);
  859: 
  860: 	P_max_num_changes = 1;
  861: 	if (bm->flagchangep)
  862: 		P_max_num_changes = Manage_Get_Max_Fishery_Param(bm, P_num_changes_id) + 1;
  863: 
  864: 	Pchange = Util_Alloc_Init_3D_Double(4, P_max_num_changes, bm->K_num_fisheries, 0.0);
  865: 
  866: 	Util_XML_Read_Change_Values(bm, fileName, selGroupNode, "Pchange", P_num_changes_id, Pchange);
  867: 
  868: 	//Util_XML_Read_Change_Values(bm, selGroupNode, "Pchange", P_num_changes_id, Pchange);
  869: 
  870: 	selGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, groupingNode, "Swept_change");
  871: 
  872: 	Util_XML_Read_Fishery_Param(bm, fileName, selGroupNode, SWEPT_num_changes_id);
  873: 
  874: 	SWEPT_max_num_changes = 1;
  875: 	if (bm->flagchangeswept)
  876: 		SWEPT_max_num_changes = Manage_Get_Max_Fishery_Param(bm, SWEPT_num_changes_id) + 1;
  877: 
  878: 	SWEPTchange = Util_Alloc_Init_3D_Double(4, SWEPT_max_num_changes, bm->K_num_fisheries, 0.0);
```

**Suggested fix (from review):** Always size by `Manage_Get_Max_*_Param(...) + 1`, as is already done for mFC at line 917.

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

## HARV-09

**copy-paste duplicate** · Conf M / Sev L  
**Listed lines:** 696-699

**Function:** `readFishingMortalityXML()`

**Why it's thought to be a bug**

`Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);` is called twice (lines 696 and 698), so mFC is read twice. Harmless, but probably meant to read something else. Function: `readFishingMortalityXML()`.

**Current code** (`atharvest/atHarvestParamIO.c` lines 694–701)

```c
  694: 		quit("Fishing_Mortality attribute group not found in input file %s.\n", fileName);
  695: 
  696: 	Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);
  697: 	readAgeStructuredBiomassFisheryParamXML(bm, fileName, groupingNode, mFC_start_age_id);
  698:     Util_XML_Read_Impacted_Group_Param(bm, fileName, groupingNode, id);
  699:     readAgeStructuredBiomassFisheryParamXML(bm, fileName, groupingNode, mFC_end_age_id);
  700: 
  701: }
```

**Suggested fix (from review):** Remove the second call.

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

## HARV-10

**NULL dereference** · Conf M / Sev L  
**Listed lines:** 583-585, 831-835, 933-935

**Why it's thought to be a bug**

Unlike every other reader in this file, `attributeGroupNode` (`Fishing_Scenario_Flags`), `groupingNode`/`selGroupNode` (`Fishery_Change` and its children) and `inputDoc` (from `xmlReadFileDestFolder`) are not NULL-checked. A missing XML section or file crashes instead of giving a clear `quit`. Functions: `readFisheryScenarioFlags()`, `readFisheryChangeXML()`, `Read_Harvest_Parameters()`.

**Current code** (`atharvest/atHarvestParamIO.c` lines 581–587)

```c
  581: 	xmlNodePtr attributeGroupNode;
  582: 
  583: 	attributeGroupNode = Util_XML_Get_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "Fishing_Scenario_Flags");
  584: 
  585:     bm->flagchangediscard = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangediscrd"));
  586:     bm->flagchangesel = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangesel"));
  587:     bm->flagchangeF = (int) (Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE,  bm->ecotest, 1, attributeGroupNode, binary_check, "flagchangeF"));
```

**Suggested fix (from review):** Add `if (x == NULL) quit(...)`.

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

# atlantisUtil/atUtilFisheryIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-13](#utl2-13) | 106 | L/L | NULL dereference | `FisheryArray = malloc(...)` is never checked for NULL before use. Function: `Util_Read_Fisheries_XML()`. |  | [ ] |
| [UTL2-14](#utl2-14) | 92, 124-125 | L/M | logic | `bm->K_max_num_subfleet = 5; // -1;` sets a floor of 5 on the max subfleet count, so arrays sized by it are la… |  | [ ] |

---

## UTL2-13

**NULL dereference** · Conf L / Sev L  
**Listed lines:** 106

**Function:** `Util_Read_Fisheries_XML()`

**Why it's thought to be a bug**

`FisheryArray = malloc(...)` is never checked for NULL before use. Function: `Util_Read_Fisheries_XML()`.

**Current code** (`atlantisUtil/atUtilFisheryIO.c` lines 104–108)

```c
  104: 		}
  105: 
  106: 		FisheryArray = (FisheryStruct *) malloc(sizeof(FisheryStruct) * (size_t)(bm->K_num_fisheries + 1));
  107: 
  108: 
```

**Suggested fix (from review):** Check for NULL and quit.

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

## UTL2-14

**logic** · Conf L / Sev M  
**Listed lines:** 92, 124-125

**Function:** `Util_Read_Fisheries_XML()`

**Why it's thought to be a bug**

`bm->K_max_num_subfleet = 5; // -1;` sets a floor of 5 on the max subfleet count, so arrays sized by it are larger than needed. It is probably deliberate (the -1 is commented out), but any loop over 0..K_max_num_subfleet will visit subfleets that don't exist for fisheries with fewer. Function: `Util_Read_Fisheries_XML()`.

**Current code** (`atlantisUtil/atUtilFisheryIO.c` lines 90–127)

```c
   90: 
   91: 	}
   92: 	bm->K_max_num_subfleet= 5; // -1;
   93: 
   94: 	/* Read in each of the functional groups */
   95: 	fisheryList = Util_XML_Get_Node_List(ATLANTIS_ATTRIBUTE_SUB_GROUP, inputDoc->children, "Fishery");
   96: 
   97: 	if (fisheryList && fisheryList->nodesetval->nodeNr > 0) {
   98: 
   99: 		/* Check the number of groups match the values in the run input file*/
  100: 		numFisheries = fisheryList->nodesetval->nodeNr;
  101: 		if (numFisheries != bm->K_num_fisheries) {
  102: 			quit("Util_Read_Fisheries_XML: Number of fisheries specified (%d) in your fishery defintion file %s does not match the K_num_fisheries value (%d) in your run file\n",
  103: 					numFisheries, fileName, bm->K_num_fisheries);
  104: 		}
  105: 
  106: 		FisheryArray = (FisheryStruct *) malloc(sizeof(FisheryStruct) * (size_t)(bm->K_num_fisheries + 1));
  107: 
  108: 
  109: 		/* Now read in the groups. */
  110: 		for (fisheryIndex = 0; fisheryIndex < numFisheries; fisheryIndex++) {
  111: 
  112: 			fisheryNode = fisheryList->nodesetval->nodeTab[fisheryIndex];
  113: 
  114: 			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, fisheryNode, "Code", FisheryArray[fisheryIndex].fisheryCode);
  115:             
  116:             if (verbose > 0)
  117: 				printf("Reading in definition of %s\n",  FisheryArray[fisheryIndex].fisheryCode);
  118: 
  119:             Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, fisheryNode, integer_check, "Index", &FisheryArray[fisheryIndex].index);
  120: 			Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, fisheryNode, "Name", FisheryArray[fisheryIndex].name);
  121: 			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, fisheryNode, integer_check, "IsRec", &FisheryArray[fisheryIndex].isRec);
  122: 			Util_XML_Get_Value_Double(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, fisheryNode, integer_check, "NumSubFleets", &bm->FISHERYprms[fisheryIndex][nsubfleets_id]);
  123: 
  124: 			if( bm->FISHERYprms[fisheryIndex][nsubfleets_id] > bm->K_max_num_subfleet)
  125: 				bm->K_max_num_subfleet = (int)bm->FISHERYprms[fisheryIndex][nsubfleets_id];
  126: 		}
  127: 		xmlXPathFreeObject(fisheryList);
```

**Suggested fix (from review):** Confirm intent. Otherwise initialise to 0 (or -1).

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

atUtilFisheryIO.c:60: `strstr(convertedXMLFileName, ".csv")` matches the first ".csv" anywhere in the path, not the extension. The overlong-string strcpy into `fisheryCode[20]` (Util_XML_Get_Value_String, atUtilXML.c:1310) is unbounded.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`xmlCleanupParser()` at atUtilFisheryIO.c:134 is called mid-run, and libxml2 documents it as unsafe if the library is used afterwards. This is a common pattern in the codebase.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

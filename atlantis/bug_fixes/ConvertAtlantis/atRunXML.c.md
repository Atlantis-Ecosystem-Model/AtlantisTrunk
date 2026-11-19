# ConvertAtlantis/atRunXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-20](#cnv2-20) ⭐ | 56 | H/H | missing parameter | The reader requires `K_num_vessel_sizes` in `SubModelSetup` when `flagindustry_on == simple_industry_model (1)… |  | [ ] |
| [CNV2-21](#cnv2-21) | 126 | H/M | state only set by converter | `bm->track_contaminants` is set only as a side effect of conversion (line 127). In `Read_Run_Parameters()` the… |  | [ ] |
| [CNV2-22](#cnv2-22) | 133 | M/L | metadata clobber | `Parse_File(..., initFP=NULL, ...)` uses the `ScenarioOptions` sub-group node itself as `newnode`. It therefor… |  | [ ] |
| [CNV2-23](#cnv2-23) | 127 | H/L | wrong type metadata | `track_contaminants` is declared BOOLEAN but read back with `integer_check`. `debug` is declared `XML_TYPE_BOO… |  | [ ] |

---

## CNV2-20

**missing parameter** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atRunXML.c` 56
**Lines at review (Oct 2026):** 49-73

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

The reader requires `K_num_vessel_sizes` in `SubModelSetup` when `flagindustry_on == simple_industry_model (1)` (atparamIO.c:136-138, isRequired=1, which quits if absent at atUtilIO.c:71-72). The converter never writes it, so any `.prm` run file with industry on fails to load. Function: `Convert_Run_To_XML()`.

**Code at issue** — `atRunXML.c` line 56 (located by name only; check this is the right place)  
Search for: `Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagindustry_on", "Flag showing wh`

```c
   54: 			"", XML_TYPE_BOOLEAN,"1");
   55:     
   56→     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagindustry_on", "Flag showing whether want industries loaded and submodel run (1) or not (0)",
   57:             "", XML_TYPE_BOOLEAN,"1");
   58:     
```

**Suggested fix (from review):** After `flagindustry_on`, read it back and, if it is 1, call `Util_XML_Parse_Create_Node(..., "K_num_vessel_sizes", ..., XML_TYPE_INTEGER, "1")`

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

## CNV2-21

**state only set by converter** · Conf H / Sev M  
**Found at (current code):** `atRunXML.c` 126
**Lines at review (Oct 2026):** 126-129 (with atparamIO.c:234)

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

`bm->track_contaminants` is set only as a side effect of conversion (line 127). In `Read_Run_Parameters()` the read is commented out (atparamIO.c:234). When the run file is supplied directly as `.xml`, `track_contaminants` keeps its default, so contaminants (and the num_contaminants/names block) are silently skipped. Function: `Convert_Run_To_XML()` / `Read_Run_Parameters()`.

**Code at issue** — `atRunXML.c` line 126  
Search for: `track_contaminants`

```c
  124:  	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ContaminantSettings", "Contaminant Settings", "", "");
  125:     */
  126→     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_contaminants", "Flag to turn on tracking of contaminants", "", XML_TYPE_BOOLEAN,"0");
  127:     bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");
  128:   
```

**Suggested fix (from review):** Restore the `track_contaminants` read in atparamIO.c (ScenarioOptions group)

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

## CNV2-22

**metadata clobber** · Conf M / Sev L  
**Found at (current code):** `atRunXML.c` 133
**Lines at review (Oct 2026):** 133-139

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

`Parse_File(..., initFP=NULL, ...)` uses the `ScenarioOptions` sub-group node itself as `newnode`. It therefore overwrites that group's `AttributeType`/`ArrayLength` properties with StringArray/"" (atCreateXML.c:144-153). The values are still created correctly. Function: `Convert_Run_To_XML()`.

**Code at issue** — `atRunXML.c` line 133  
Search for: `Parse_File(`

```c
  131: 		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "num_contaminants", "The number of contaminants to track.", "", XML_TYPE_INTEGER,"0");
  132: 
  133→ 		Parse_File(bm, fp, fileName, groupingNode, "Contaminant_Names", "^contaminant_names",
  134: 					"The names of the contaminant tracers.",
  135: 					"", XML_TYPE_STRINGARRAY, -1, FALSE, NULL, NULL);
```

**Suggested fix (from review):** Pass an initFP that creates a child node, or use `Util_XML_Create_Node_Next_Line`

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

## CNV2-23

**wrong type metadata** · Conf H / Sev L  
**Found at (current code):** `atRunXML.c` 127
**Lines at review (Oct 2026):** 126-127, 199

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

`track_contaminants` is declared BOOLEAN but read back with `integer_check`. `debug` is declared `XML_TYPE_BOOLEAN` but has 25 lookup values (0-24). Function: `Convert_Run_To_XML()`.

**Code at issue** — `atRunXML.c` line 127 (the line the review pointed to, found by its text)  
Search for: `bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecote`

```c
  125:     */
  126:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_contaminants", "Flag to turn on tracking of contaminants", "", XML_TYPE_BOOLEAN,"0");
  127→     bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");
  128:   
  129: 	if(bm->track_contaminants == TRUE){
```

**Suggested fix (from review):** Use `XML_TYPE_LOOKUP`/`XML_TYPE_INTEGER`

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

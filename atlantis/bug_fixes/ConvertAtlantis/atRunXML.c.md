# ConvertAtlantis/atRunXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-20](#cnv2-20) ⭐ | 49-73 | H/H | missing parameter | The reader requires `K_num_vessel_sizes` in `SubModelSetup` when `flagindustry_on == simple_industry_model (1)… |  | [ ] |
| [CNV2-21](#cnv2-21) | 126-129 (with atparamIO.c:234) | H/M | state only set by converter | `bm->track_contaminants` is set only as a side effect of conversion (line 127). In `Read_Run_Parameters()` the… |  | [ ] |
| [CNV2-22](#cnv2-22) | 133-139 | M/L | metadata clobber | `Parse_File(..., initFP=NULL, ...)` uses the `ScenarioOptions` sub-group node itself as `newnode`. It therefor… |  | [ ] |
| [CNV2-23](#cnv2-23) | 126-127, 199 | H/L | wrong type metadata | `track_contaminants` is declared BOOLEAN but read back with `integer_check`. `debug` is declared `XML_TYPE_BOO… |  | [ ] |

---

## CNV2-20

**missing parameter** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 49-73  
**Lines at review (Oct 2026):** 49-73

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

The reader requires `K_num_vessel_sizes` in `SubModelSetup` when `flagindustry_on == simple_industry_model (1)` (atparamIO.c:136-138, isRequired=1, which quits if absent at atUtilIO.c:71-72). The converter never writes it, so any `.prm` run file with industry on fails to load. Function: `Convert_Run_To_XML()`.

**Current code** (`ConvertAtlantis/atRunXML.c` lines 47–75)

```c
   47: 	xmlDocSetRootElement(doc, rootnode);
   48: 
   49: 	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "SubModelSetup", "Determine which sub-models are active", "", "");
   50:     
   51:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagecon_on", "Flag showing whether want economics loaded and submodel run (1) or not (0)", "",
   52: 			XML_TYPE_BOOLEAN, "1");
   53: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_fisheries_on", "Flag showing whether want fisheries loaded and submodel run (1) or not (0)",
   54: 			"", XML_TYPE_BOOLEAN,"1");
   55:     
   56:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flagindustry_on", "Flag showing whether want industries loaded and submodel run (1) or not (0)",
   57:             "", XML_TYPE_BOOLEAN,"1");
   58:     
   59:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_OSP_mode", "Flag showing whether what OSP setting is any are being used", "", XML_TYPE_INTEGER,"0");
   60:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_OSP_FtargetSource", "Flag indicating source of F - whether OSP or other assessments in/called by Atlantis", "", XML_TYPE_INTEGER,"0");
   61:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_OSP_PriceSource", "Flag indicating source of prices - whether OSP or Atlantis native model ", "", XML_TYPE_INTEGER,"0");
   62:     
   63:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_skip_biol",
   64: 			"Flag showing whether want biological model run (1) or not (0 - only used when debugging fisheries)", "", XML_TYPE_BOOLEAN,"0");
   65: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_skip_phys",
   66: 			"Flag showing whether want physics loaded and submodel run (1) or not (0). Should only be used for debugging.", "", XML_TYPE_BOOLEAN,"1");
   67: 
   68: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "check_dups",
   69: 				"Turn on to check for duplicates in prm files. Turning on will significantly slow down prm file read in - only use from time to time.", "", XML_TYPE_BOOLEAN,"1");
   70:     
   71:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_migration_on", "Flag showing whether want migrations loaded and submodel run (1) or not (0)",
   72:                                "", XML_TYPE_BOOLEAN,"1");
   73:     bm->flag_migration_on = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, binary_check, "flag_migration_on");
   74: 
   75: 	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ScenarioOptions", "Run settings", "", "");
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
**Lines (current code):** 126-129 (with atparamIO.c:234)  
**Lines at review (Oct 2026):** 126-129 (with atparamIO.c:234)

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

`bm->track_contaminants` is set only as a side effect of conversion (line 127). In `Read_Run_Parameters()` the read is commented out (atparamIO.c:234). When the run file is supplied directly as `.xml`, `track_contaminants` keeps its default, so contaminants (and the num_contaminants/names block) are silently skipped. Function: `Convert_Run_To_XML()` / `Read_Run_Parameters()`.

**Current code** (`ConvertAtlantis/atRunXML.c` lines 124–131)

```c
  124:  	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ContaminantSettings", "Contaminant Settings", "", "");
  125:     */
  126:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_contaminants", "Flag to turn on tracking of contaminants", "", XML_TYPE_BOOLEAN,"0");
  127:     bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");
  128:   
  129: 	if(bm->track_contaminants == TRUE){
  130:         Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flag_contam_sanity_check", "Flag to turn on checking of contaminant fluxes.", "", XML_TYPE_BOOLEAN,"0");
  131: 		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "num_contaminants", "The number of contaminants to track.", "", XML_TYPE_INTEGER,"0");
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
**Lines (current code):** 133-139  
**Lines at review (Oct 2026):** 133-139

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

`Parse_File(..., initFP=NULL, ...)` uses the `ScenarioOptions` sub-group node itself as `newnode`. It therefore overwrites that group's `AttributeType`/`ArrayLength` properties with StringArray/"" (atCreateXML.c:144-153). The values are still created correctly. Function: `Convert_Run_To_XML()`.

**Current code** (`ConvertAtlantis/atRunXML.c` lines 131–141)

```c
  131: 		Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "num_contaminants", "The number of contaminants to track.", "", XML_TYPE_INTEGER,"0");
  132: 
  133: 		Parse_File(bm, fp, fileName, groupingNode, "Contaminant_Names", "^contaminant_names",
  134: 					"The names of the contaminant tracers.",
  135: 					"", XML_TYPE_STRINGARRAY, -1, FALSE, NULL, NULL);
  136: 
  137: 		Parse_File(bm, fp, fileName, groupingNode, "Contaminant_Units", "^contaminant_units",
  138: 					"The units of the contaminant tracers.",
  139: 					"", XML_TYPE_STRINGARRAY, -1, FALSE, NULL, NULL);
  140:         
  141: 	}
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
**Lines (current code):** 126-127, 199  
**Lines at review (Oct 2026):** 126-127, 199

**Function:** `Convert_Run_To_XML()`

**Why it's thought to be a bug**

`track_contaminants` is declared BOOLEAN but read back with `integer_check`. `debug` is declared `XML_TYPE_BOOLEAN` but has 25 lookup values (0-24). Function: `Convert_Run_To_XML()`.

**Current code** (`ConvertAtlantis/atRunXML.c` lines 124–129)

```c
  124:  	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE_SUB_GROUP, rootnode, "ContaminantSettings", "Contaminant Settings", "", "");
  125:     */
  126:     Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "track_contaminants", "Flag to turn on tracking of contaminants", "", XML_TYPE_BOOLEAN,"0");
  127:     bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");
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

# atphysics/atparamIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-01](#phy3-01) ⭐ | 105 | H/H | buffer size / wrong sizeof | `snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", biologyfile)`: `convertedXMLFileName` is a… |  | [ ] |
| [PHY3-02](#phy3-02) | 87-98 | H/M | buffer overrun / NULL deref | `char biologyfile[120]; strcpy(biologyfile, bm->runprmIfname)`, but `runprmIfname` is `char[BMSLEN]` (512, atl… |  | [ ] |
| [PHY3-03](#phy3-03) | 234-236 | M/M | uninitialised / missing read | `bm->track_contaminants` (read commented out at 234) and `bm->track_wind` (never read here) are only set insid… |  | [ ] |
| [PHY3-04](#phy3-04) | 253-279 | H/L | memory leak | The first `values` array from `Util_XML_Read_Array_String` (names) is never freed (`c_free2d` is commented out… |  | [ ] |

---

## PHY3-01

**buffer size / wrong sizeof** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 105  
**Lines at review (Oct 2026):** 105

**Function:** `Util_Read_Run_XML()`

**Why it's thought to be a bug**

`snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", biologyfile)`: `convertedXMLFileName` is a `char *` from malloc (line 89), so `sizeof` gives the pointer size (8). The name is cut to 7 characters, so a run file supplied directly as `.xml` cannot be opened. Function: `Util_Read_Run_XML()`.

**Current code** (`atphysics/atparamIO.c` lines 103–107)

```c
  103: 		Convert_Run_To_XML(bm, biologyfile, convertedXMLFileName);
  104: 	} else {
  105: 		snprintf(convertedXMLFileName, sizeof(convertedXMLFileName), "%s", biologyfile);
  106: 	}
  107: 
```

**Suggested fix (from review):** Use `fileSize` (as on line 96)

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

## PHY3-02

**buffer overrun / NULL deref** · Conf H / Sev M  
**Lines (current code):** 87-98  
**Lines at review (Oct 2026):** 87-98

**Function:** `Util_Read_Run_XML()`

**Why it's thought to be a bug**

`char biologyfile[120]; strcpy(biologyfile, bm->runprmIfname)`, but `runprmIfname` is `char[BMSLEN]` (512, atlantisboxmodel.h:4384), so a long path overruns the stack buffer. Line 97 `*(strstr(convertedXMLFileName, ".prm")) = '\0'` dereferences NULL if the name has neither ".xml" nor ".prm". `fileSize` comes from `fileName` while the copied string is `runprmIfname` (only equal because of how the caller calls it, atlantismain.c:1225). Function: `Util_Read_Run_XML()`.

**Current code** (`atphysics/atparamIO.c` lines 85–100)

```c
   85: int Util_Read_Run_XML(MSEBoxModel *bm, char *fileName) {
   86: 
   87: 	char biologyfile[120];
   88:     int fileSize = sizeof(char) * strlen(fileName) + 1;
   89: 	char *convertedXMLFileName = (char *)malloc(fileSize);
   90: 
   91: 	strcpy(biologyfile, bm->runprmIfname);
   92: 
   93: 	/* Check to see if we are reading in an XML file or a prm file */
   94: 	if (strstr(biologyfile, ".xml") == NULL) {
   95: 		/* Build the converted filename */
   96: 		snprintf(convertedXMLFileName, fileSize, "%s", biologyfile);
   97: 		*(strstr(convertedXMLFileName, ".prm")) = '\0';
   98: 		strcat(convertedXMLFileName, ".xml");
   99: 
  100: 		printf("convertedXMLFileName = %s\n", convertedXMLFileName);
```

**Suggested fix (from review):** Make the buffer BMSLEN, use `fileName`, check strstr for NULL

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

## PHY3-03

**uninitialised / missing read** · Conf M / Sev M  
**Lines (current code):** 234-236  
**Lines at review (Oct 2026):** 234-236

**Function:** `Read_Run_Parameters()`

**Why it's thought to be a bug**

`bm->track_contaminants` (read commented out at 234) and `bm->track_wind` (never read here) are only set inside the converter, ConvertAtlantis/atRunXML.c:102,127. If the run file is given as `.xml`, the converter is skipped, so contaminants and wind can never be switched on. Function: `Read_Run_Parameters()`.

**Current code** (`atphysics/atparamIO.c` lines 232–238)

```c
  232:         quit("ContaminantSettings attribute group not found in input file %s.\n", fileName);
  233:      */
  234:      //bm->track_contaminants = (int) Util_XML_Read_Value(fileName, ATLANTIS_ATTRIBUTE, bm->ecotest, 1, groupingNode, integer_check, "track_contaminants");
  235: 
  236:     if(bm->track_contaminants == TRUE){
  237:         
  238:         printf("Reading contaminant parameters\n");
```

**Suggested fix (from review):** Read `track_contaminants` and `trackWind` in Read_Run_Parameters

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

## PHY3-04

**memory leak** · Conf H / Sev L  
**Lines (current code):** 253-279  
**Lines at review (Oct 2026):** 253-279

**Function:** `Read_Run_Parameters()`

**Why it's thought to be a bug**

The first `values` array from `Util_XML_Read_Array_String` (names) is never freed (`c_free2d` is commented out at 266) before `values` is reused for the units. Function: `Read_Run_Parameters()`.

**Current code** (`atphysics/atparamIO.c` lines 251–281)

```c
  251:         
  252:         /* Read in the names */
  253:         if(Util_XML_Read_Array_String(ATLANTIS_ATTRIBUTE, fileName, "contaminant_names", groupingNode, "contaminant_names", &values, bm->num_contaminants) == FALSE){
  254:             quit("Error: Unable to read parameter 'Contaminant_Names' from input file %s\n", fileName);
  255:         }
  256:         
  257:         printf("String name set up\n");
  258: 
  259:         for(cIndex = 0; cIndex < bm->num_contaminants ; cIndex ++){
  260:             strcpy(bm->contaminantStructure[cIndex]->contaminant_name, values[cIndex]);
  261: 
  262:             printf("name = %s, value = %s\n", values[cIndex], bm->contaminantStructure[cIndex]->contaminant_name);
  263:         }
  264:         printf("Loaded values and about to free values\n");
  265:         
  266:         //c_free2d(values);
  267:         //printf("value = %s\n", bm->contaminantStructure[0]->contaminant_name);
  268: 
  269:         /* Read in the units */
  270:         if(Util_XML_Read_Array_String(ATLANTIS_ATTRIBUTE, fileName, "contaminant_units", groupingNode, "contaminant_units", &values,
  271:                                 bm->num_contaminants) == FALSE){
  272:                     quit("Error: Unable to read parameter 'Contaminant_Units' from input file %s\n", fileName);
  273:                 }
  274: 
  275:         for(cIndex = 0; cIndex < bm->num_contaminants ; cIndex ++){
  276:             strcpy(bm->contaminantStructure[cIndex]->contaminant_unit , values[cIndex]);
  277:         }
  278: 
  279:         c_free2d(values);
  280:     }
  281: 
```

**Suggested fix (from review):** Uncomment `c_free2d(values)` at line 266

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

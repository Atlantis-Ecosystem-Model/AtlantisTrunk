# ConvertAtlantis/atEconomicXML.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [CNV2-15](#cnv2-15) | 33-35 | H/M | parsing / wrong index | `*strchr(str, '_') = '\0'` treats everything before the first `_` as the fishery code. Fishery codes containin… |  | [ ] |
| [CNV2-16](#cnv2-16) | 257, 259 | H/L | duplicate parameter | `UseEconQuotaDefine` is parsed twice in a row, giving two same-named nodes in `Economic_Flags` (the first with… |  | [ ] |
| [CNV2-17](#cnv2-17) | 253, 823 | H/L | copy-paste text | The `UseConstPrice` description is copied from `MultiPlanEffort`. The quit message names the "Chemical_Limitat… |  | [ ] |
| [CNV2-18](#cnv2-18) | 69-71, 95-106, 117-118, 170-171, 603-604, 651-652 | M/L | parsing limitation | Market, subfleet and market-coefficient indices come from the last character only (`atoi(lastLetter)`). With 1… |  | [ ] |

---

## CNV2-15

**parsing / wrong index** · Conf H / Sev M  
**Listed lines:** 33-35

**Function:** `FisheryFirstXMLFunction()`

**Why it's thought to be a bug**

`*strchr(str, '_') = '\0'` treats everything before the first `_` as the fishery code. Fishery codes containing `_` (e.g. `Com_boat`, `Astoria_Generic`) give `Com`/`Astoria`, and `Util_Get_Fishery_Index()` quits. This affects all subfleet parameters (`_nboat`, `_varcost`, ...). Function: `FisheryFirstXMLFunction()`.

**Current code** (`ConvertAtlantis/atEconomicXML.c` lines 31–37)

```c
   31: 	int fisheryIndex;
   32: 
   33: 	*strchr(str, '_') = '\0';
   34: 
   35: 	fisheryIndex = Util_Get_Fishery_Index(bm, str);
   36: 	Util_XML_Set_Node_Value(ATLANTIS_FISHERY_ATTRIBUTE, parent, FisheryArray[fisheryIndex].fisheryCode, valueStr);
   37: }
```

**Suggested fix (from review):** Strip the known suffix (pass it in) instead of cutting at the first `_`

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

## CNV2-16

**duplicate parameter** · Conf H / Sev L  
**Listed lines:** 257, 259

**Function:** `economicFlagParamsXML()`

**Why it's thought to be a bug**

`UseEconQuotaDefine` is parsed twice in a row, giving two same-named nodes in `Economic_Flags` (the first without a description). The reader takes the first. Function: `economicFlagParamsXML()`.

**Current code** (`ConvertAtlantis/atEconomicXML.c` lines 255–261)

```c
  255: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "OrigEconCalc", "Whether to use the original economic driver calculation (1) or use one that is total price - total cost ",
  256: 			"", XML_TYPE_BOOLEAN,"0");
  257: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseEconQuotaDefine", "", "", XML_TYPE_BOOLEAN,"1");
  258: 
  259: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseEconQuotaDefine", "Use the TAC as sety in harvest.prm (0) as in QuotaAlloc in this file (1)", "", XML_TYPE_BOOLEAN,"0");
  260: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "flaglbs", "catch and cpue is in lbs (1=yes, 0=no)", "", XML_TYPE_BOOLEAN,"0");
  261:     
```

**Suggested fix (from review):** Delete line 257

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

## CNV2-17

**copy-paste text** · Conf H / Sev L  
**Listed lines:** 253, 823

**Why it's thought to be a bug**

The `UseConstPrice` description is copied from `MultiPlanEffort`. The quit message names the "Chemical_Limitation attribute group" instead of Economic_Flags. Functions: `economicFlagParamsXML()`, `Convert_Economic_To_XML()`.

**Current code** (`ConvertAtlantis/atEconomicXML.c` lines 251–255)

```c
  251: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "prop_boatbound", "Lower bound on proportion of large fleets that remain through length of simulated period", "", XML_TYPE_FLOAT, "0.3");
  252: 
  253: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseConstPrice", "Whether use base (Fulton) economic model (1) or Dan Holland model (0)", "", XML_TYPE_BOOLEAN,"0");
  254: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "UseMinValue", "Set price coefficients to zero in Newell model for species that have no landed price", "", XML_TYPE_FLOAT, "0");
  255: 	Util_XML_Parse_Create_Node(fp, fileName, groupingNode, "OrigEconCalc", "Whether to use the original economic driver calculation (1) or use one that is total price - total cost ",
```

**Suggested fix (from review):** Fix the strings

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

## CNV2-18

**parsing limitation** · Conf M / Sev L  
**Listed lines:** 69-71, 95-106, 117-118, 170-171, 603-604, 651-652

**Why it's thought to be a bug**

Market, subfleet and market-coefficient indices come from the last character only (`atoi(lastLetter)`). With 10 or more subfleets or markets, `sub10` maps to `sub0` / `Market_-1`. `createMonthlyMarketXMLNodes()` also hard-codes 2 markets, while the reader loops over `K_num_markets` (ateconParamIO.c:259). Functions: `MarketXML()`, `MonthlyMarketXML()`, `OwnQuotaXML()`, `TemporalBlackBookXML()`, `SpatialBlackBookXML()`, `createMonthlyMarketXMLNodes()`.

**Current code** (`ConvertAtlantis/atEconomicXML.c` lines 67–106 (truncated))

```c
   67: 	char *lastLetter;
   68: 
   69: 	lastLetter = str + strlen(str) - 1;
   70: 	index = atoi(lastLetter) - 1;
   71: 	*lastLetter = '\0';
   72: 
   73: 	/* find the parent node */
   74: 	if (strcmp(str, "MARKET_intercept") == 0) {
   75: 		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Intercept");
   76: 	} else if (strcmp(str, "MARKET_trend") == 0) {
   77: 		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Trend");
   78: 	} else if (strcmp(str, "MARKET_autocoefft") == 0) {
   79: 		groupingNode = Util_XML_Get_Or_Create_Node(ATLANTIS_ATTRIBUTE, parent, "Market_Autocoefft");
   80: 	} else {
   81: 		quit("MarketXML: Unrecognised option %s\n", str);
   82: 	}
   83: 
   84: 	snprintf(paramStr, sizeof(paramStr), "Market_%d", index);
   85: 	Util_XML_Set_Node_Value(ATLANTIS_ATTRIBUTE, groupingNode, paramStr, valueStr);
   86: 
   87: }
   88: 
   89: void createMonthlyMarketXMLNodes(MSEBoxModel *bm, xmlNodePtr parent) {
   90: 
   91: 	int index;
   92: 	char str[200];
   93: 	xmlNodePtr groupingNode;
   94: 
   95: 	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "MarketCoefft1",
   96: 			"Monthly market specific coefficients -  per fished species for the first market", "", "");
   97: 	for (index = 0; index < 12; index++) {
   98: 		snprintf(str, sizeof(str), "%s", monthStrings[index]);
   99: 		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
  100: 	}
  101: 	groupingNode = Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, parent, "MarketCoefft2",
  102: 			"Monthly market specific coefficients -  per fished species for the second market", "", "");
  103: 	for (index = 0; index < 12; index++) {
  104: 		snprintf(str, sizeof(str), "%s", monthStrings[index]);
  105: 		Util_XML_Create_Node(ATLANTIS_ATTRIBUTE, groupingNode, str, "", "", "");
  106: 	}
```

**Suggested fix (from review):** Parse the trailing digit run (`strspn` from the end) and loop over `K_num_markets`

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

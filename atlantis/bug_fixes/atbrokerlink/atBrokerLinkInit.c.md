# atbrokerlink/atBrokerLinkInit.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-45](#link-45) | 73-74 | M/H | missing bounds check | Polygon `id`s from the XML are stored in `polygonList` and later used directly as box indices `b` in Link_GetB… |  | [ ] |
| [LINK-46](#link-46) | 55 | M/L | unchecked malloc | `bm->linkageInterface = malloc(...)` is not checked; numPolygons and other fields are uninitialised until set.… |  | [ ] |
| [LINK-47](#link-47) | 93-98 | M/M | infinite loop / protocol | When `Linkage_DeserialiseCommand` returns NULL (unknown request type, see LINK-50) no reply is sent. A ZMQ_REP… |  | [ ] |

---

## LINK-45

**missing bounds check** · Conf M / Sev H  
**Listed lines:** 73-74

**Function:** `Linkage_Read_XML()`

**Why it's thought to be a bug**

Polygon `id`s from the XML are stored in `polygonList` and later used directly as box indices `b` in Link_GetBiomass, Link_SetBiomass, Link_SetMortality and Link_SetDetritus with no `0 <= id < bm->nbox` check. Out-of-range ids read and write out of bounds. Function: `Linkage_Read_XML()`.

**Current code** (`atbrokerlink/atBrokerLinkInit.c` lines 71–76)

```c
   71: 
   72: 			polygonNode = polygonList->nodesetval->nodeTab[polygonIndex];
   73: 			Util_XML_Get_Value_Integer(fileName, ATLANTIS_ATTRIBUTE, 0, TRUE, polygonNode, integer_check, "id",
   74: 					&bm->linkageInterface->polygonList[polygonIndex]);
   75: 
   76: 		}
```

**Suggested fix (from review):** Validate each id against bm->nbox.

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

## LINK-46

**unchecked malloc** · Conf M / Sev L  
**Listed lines:** 55

**Function:** `Linkage_Read_XML()`

**Why it's thought to be a bug**

`bm->linkageInterface = malloc(...)` is not checked; numPolygons and other fields are uninitialised until set. Function: `Linkage_Read_XML()`.

**Current code** (`atbrokerlink/atBrokerLinkInit.c` lines 53–57)

```c
   53: 
   54: 	Util_XML_Get_Value_String(fileName, ATLANTIS_ATTRIBUTE, TRUE, inputDoc->children, "URL", url);
   55: 	bm->linkageInterface = (LinkageInterface *) malloc(sizeof(LinkageInterface));
   56: 	Linkage_Initialise(bm, url);
   57: 
```

**Suggested fix (from review):** Check for NULL, or use calloc.

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

## LINK-47

**infinite loop / protocol** · Conf M / Sev M  
**Listed lines:** 93-98

**Function:** `Linkage_Start()`

**Why it's thought to be a bug**

When `Linkage_DeserialiseCommand` returns NULL (unknown request type, see LINK-50) no reply is sent. A ZMQ_REP socket must send before the next recv, so every following `zmq_recv` fails (EFSM). Network_Receive ignores the failure and returns an empty message, which makes `requests__request__unpack` fail and `exit(1)`. Function: `Linkage_Start()`.

**Current code** (`atbrokerlink/atBrokerLinkInit.c` lines 91–100)

```c
   91: 	Message *message = NULL;
   92: 
   93: 	while (bm->linkageInterface->shutdown == FALSE) {
   94: 		message = Linkage_ReceiveCommand(bm->linkageInterface);
   95: 		message = Linkage_DeserialiseCommand(bm, bm->linkageInterface, message);
   96: 		if (message != NULL)
   97: 			Linkage_SendCommand(bm->linkageInterface, message);
   98: 	}
   99: }
  100: 
```

**Suggested fix (from review):** Always send a (failure) response, and abort the loop on receive errors.

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

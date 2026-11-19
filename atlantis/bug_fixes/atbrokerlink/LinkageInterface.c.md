# atbrokerlink/LinkageInterface.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-48](#link-48) | 23-24, 82, 87 | M/L | unchecked malloc | `malloc` for the Network struct, polygonList and valueList is unchecked. Calling Linkage_SetNumPolygons a seco… |  | [ ] |
| [LINK-49](#link-49) | 25 | L/L | type mismatch | `group = -1` is assigned to an `unsigned int group` field (atBrokerLink.h:21), so a "-1 unset" test can never … |  | [ ] |

---

## LINK-48

**unchecked malloc** · Conf M / Sev L  
**Listed lines:** 23-24, 82, 87

**Why it's thought to be a bug**

`malloc` for the Network struct, polygonList and valueList is unchecked. Calling Linkage_SetNumPolygons a second time leaks the previous polygonList. Functions: `Linkage_Initialise()`, `Linkage_SetNumPolygons()`.

**Current code** (`atbrokerlink/LinkageInterface.c` lines 21–26)

```c
   21: int Linkage_Initialise(MSEBoxModel *bm, char *networkURL) {
   22: 
   23: 	bm->linkageInterface->networkInterface = (Network *) malloc(sizeof(Network));
   24: 	Network_Initialise(bm->linkageInterface->networkInterface, networkURL);
   25: 	bm->linkageInterface->group = -1;
   26: 	bm->linkageInterface->polygonList = NULL;
```

**Suggested fix (from review):** Check for NULL, and free the old polygonList.

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

## LINK-49

**type mismatch** · Conf L / Sev L  
**Listed lines:** 25

**Function:** `Linkage_Initialise()`

**Why it's thought to be a bug**

`group = -1` is assigned to an `unsigned int group` field (atBrokerLink.h:21), so a "-1 unset" test can never be true. Function: `Linkage_Initialise()`.

**Current code** (`atbrokerlink/LinkageInterface.c` lines 23–27)

```c
   23: 	bm->linkageInterface->networkInterface = (Network *) malloc(sizeof(Network));
   24: 	Network_Initialise(bm->linkageInterface->networkInterface, networkURL);
   25: 	bm->linkageInterface->group = -1;
   26: 	bm->linkageInterface->polygonList = NULL;
   27: 	bm->linkageInterface->shutdown = FALSE;
```

**Suggested fix (from review):** Make the field `int`.

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

# atbrokerlink/include (Deserialiser.h, ImportExportData.h, LinkageInterface.h, Message.h, NetworkError.h, atBrokerLink.h, network.h) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-68](#link-68) | Deserialiser.h:12 | M/L | prototype mismatch | `double Deserialiser_Init(void *bytes);` (external linkage, void*) conflicts with the `static double Deseriali… |  | [ ] |
| [LINK-69](#link-69) | ImportExportData.h | L/L | missing prototype | `Link_SetDetritus` and `Link_GetMortality` are defined in ImportExportData.c but not declared, so any future c… |  | [ ] |

---

## LINK-68

**prototype mismatch** · Conf M / Sev L  
**Listed lines:** Deserialiser.h:12

**Why it's thought to be a bug**

`double Deserialiser_Init(void *bytes);` (external linkage, void*) conflicts with the `static double Deserialiser_Init(Requests__Request *)` in Deserialiser.c:25. It only compiles because Deserialiser.c does not include this header. LinkageInterface.h also declares functions that do not exist (Linkage_SetPolygons, Linkage_SerialiseCommand), and network.h declares Close_Message.

**Current code** (`atbrokerlink/include/Deserialiser.h` lines 10–14)

```c
   10: 
   11: 
   12: double Deserialiser_Init(void *bytes);
   13: Message *Deserialise_Message(MSEBoxModel *bm, LinkageInterface *interface, Message *message);
   14: //int Deserialiser_InitResp(void *resp);
```

**Suggested fix (from review):** Remove the stale prototypes.

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

## LINK-69

**missing prototype** · Conf L / Sev L  
**Listed lines:** ImportExportData.h

**Why it's thought to be a bug**

`Link_SetDetritus` and `Link_GetMortality` are defined in ImportExportData.c but not declared, so any future caller gets an implicit declaration.

**Suggested fix (from review):** Add the prototypes.

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

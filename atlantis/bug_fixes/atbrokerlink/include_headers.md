# atbrokerlink/include (Deserialiser.h, ImportExportData.h, LinkageInterface.h, Message.h, NetworkError.h, atBrokerLink.h, network.h) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-68](#link-68) | 12 | M/L | prototype mismatch | `double Deserialiser_Init(void *bytes);` (external linkage, void*) conflicts with the `static double Deseriali… |  | [ ] |
| [LINK-69](#link-69) | 420 | L/L | missing prototype | `Link_SetDetritus` and `Link_GetMortality` are defined in ImportExportData.c but not declared, so any future c… |  | [ ] |

---

## LINK-68

**prototype mismatch** · Conf M / Sev L  
**Found at (current code):** `Deserialiser.h` 12
**Lines at review (Oct 2026):** Deserialiser.h:12

**Why it's thought to be a bug**

`double Deserialiser_Init(void *bytes);` (external linkage, void*) conflicts with the `static double Deserialiser_Init(Requests__Request *)` in Deserialiser.c:25. It only compiles because Deserialiser.c does not include this header. LinkageInterface.h also declares functions that do not exist (Linkage_SetPolygons, Linkage_SerialiseCommand), and network.h declares Close_Message.

**Code at issue** — `Deserialiser.h` line 12  
Search for: `double Deserialiser_Init(void *bytes)`

```c
   10: 
   11: 
   12→ double Deserialiser_Init(void *bytes);
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
**Found at (current code):** `ImportExportData.c` 420
**Lines at review (Oct 2026):** ImportExportData.h

**Why it's thought to be a bug**

`Link_SetDetritus` and `Link_GetMortality` are defined in ImportExportData.c but not declared, so any future caller gets an implicit declaration.

**Code at issue** — `ImportExportData.c` line 420 (located by name only; check this is the right place)  
Search for: `int Link_GetMortality(MSEBoxModel *bm, char *groupCode, int numBoxes, int *boxIndexValues,`

```c
  418: }
  419: 
  420→ int Link_GetMortality(MSEBoxModel *bm, char *groupCode, int numBoxes, int *boxIndexValues, double *boxValues) {
  421: 	quit("Linkage - Link_GetMortality - Function not yet supported\n");
  422: 
```

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

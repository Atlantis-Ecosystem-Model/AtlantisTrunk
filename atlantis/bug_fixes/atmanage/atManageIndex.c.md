# atmanage/atManageIndex.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNG-30](#mng-30) | 105-111 | M/L | uninitialised flag | `first_indx = (int) manageindx[fishery_id][frstindx_id]` — manageindx is allocated/initialised to 0.0 (atManag… |  | [ ] |

---

## MNG-30

**uninitialised flag** · Conf M / Sev L  
**Listed lines:** 105-111

**Function:** `Update_Management_Indx()`

**Why it's thought to be a bug**

`first_indx = (int) manageindx[fishery_id][frstindx_id]` — manageindx is allocated/initialised to 0.0 (atManageSetup.c:1210) and frstindx is never set to 1 anywhere in the repo, so the "first index" branch never runs and the first mgmtAccess value is averaged with 0 (halved). Function: `Update_Management_Indx()`.

**Current code** (`atmanage/atManageIndex.c` lines 103–113)

```c
  103: 		newindx = season_impact * TAC_impact * MPA_impact * TEP_impact;
  104: 
  105: 		first_indx = (int) (manageindx[fishery_id][frstindx_id]);
  106: 		if (!first_indx)
  107: 			oldindx = manageindx[fishery_id][mgmtaccess_id];
  108: 		else {
  109: 			oldindx = newindx;
  110: 			manageindx[fishery_id][frstindx_id] = 0;
  111: 		}
  112: 
  113: 		manageindx[fishery_id][mgmtaccess_id] = (oldindx + newindx) / 2.0;
```

**Suggested fix (from review):** Initialise `manageindx[nf][frstindx_id] = 1` after allocation.

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

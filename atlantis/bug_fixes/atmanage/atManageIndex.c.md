# atmanage/atManageIndex.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [MNG-30](#mng-30) | 105-111 | M/L | uninitialised flag | `first_indx = (int) manageindx[fishery_id][frstindx_id]` — manageindx is allocated/initialised to 0.0 (atManag… |  | [ ] |

---

## MNG-30

**uninitialised flag** · Conf M / Sev L  
**Lines (current code):** 105-111  
**Lines at review (Oct 2026):** 105-111

**Function:** `Update_Management_Indx()`

**Why it's thought to be a bug**

`first_indx = (int) manageindx[fishery_id][frstindx_id]` — manageindx is allocated/initialised to 0.0 (atManageSetup.c:1209) and frstindx is never set to 1 anywhere in the repo, so the "first index" branch never runs and the first mgmtAccess value is averaged with 0 (halved). Function: `Update_Management_Indx()`.

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

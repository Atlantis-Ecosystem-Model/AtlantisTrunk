# Outside atassess (found during atassess review) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **1 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [XTR-01](#xtr-01) ✔ ⭐ | 178 | H/M |  | `proportion_check`: `value < 0.0 && value > 1.0` can never be true, so out-of-range proportions are accepted s… |  | [ ] |

---

## XTR-01

**finding** · Conf H / Sev M ✔ re-checked ⭐ priority  
**Found at (current code):** `atUtilIO.c` 178
**Lines at review (Oct 2026):** atlantisUtil/atUtilIO.c:178

**Why it's thought to be a bug**

`proportion_check`: `value < 0.0 && value > 1.0` can never be true, so out-of-range proportions are accepted silently.

**Code at issue** — `atUtilIO.c` line 178  
Search for: `value < 0.0 && value > 1.0`

```c
  176: 	case proportion_check:
  177: 		/* value must be between 0 and 1 */
  178→ 		if (value < 0.0 && value > 1.0)
  179: 			quit("Error: flag %s - %s must be between 0 and 1 (currently %e)\n", typeName, paramString, value);
  180: 		break;
```

**Suggested fix (from review):** `||`

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

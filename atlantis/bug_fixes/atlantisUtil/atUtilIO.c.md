# atlantisUtil/atUtilIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-04](#utl2-04) | 272 | H/M | wrong operator | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)` in the `probability_check` case is always false, so out-of-range entries… |  | [ ] |
| [UTL2-05](#utl2-05) | 189-191 | H/L | format mismatch | `quit("... current set to %d for %s ...", value, paramString);`: `value` is a double passed for `%d`, so the o… |  | [ ] |
| [UTL2-06](#utl2-06) | 317-323 | M/L | logic / unreachable | `if (sized != verifsize) quit(...); else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) warn(...)`. … |  | [ ] |
| [UTL2-07](#utl2-07) | 92-103 | M/L | unchecked return / uninitialised | `Util_XML_Get_Value_String(...)`'s return is ignored. When `isRequired` is FALSE and the node is missing, `val… |  | [ ] |

---

## UTL2-04

**wrong operator** · Conf H / Sev M  
**Lines (current code):** 272  
**Lines at review (Oct 2026):** 272

**Function:** `readkeyprm_darray_checked()`

**Why it's thought to be a bug**

`if ((*p)[i] < 0.0 && (*p)[i] > 1.0)` in the `probability_check` case is always false, so out-of-range entries are never rejected (only the sum is checked). This is a separate instance from the already-logged line 178, and the `proportion_check` case at line 262 correctly uses `||`. Function: `readkeyprm_darray_checked()`.

**Current code** (`atlantisUtil/atUtilIO.c` lines 270–274) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
  270: 		sum = 0.0;
  271: 		for (i = 0; i < sized; i++) {
  272: 			if ((*p)[i] < 0.0 && (*p)[i] > 1.0)
  273: 				quit("Entries in %s are supposed to be proportions (i.e. between 0 and 1), but %d entry is %e\n", key, i, (*p)[i]);
  274: 			else {
```

**Suggested fix (from review):** Use `||`.

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

## UTL2-05

**format mismatch** · Conf H / Sev L  
**Lines (current code):** 189-191  
**Lines at review (Oct 2026):** 189-191

**Function:** `Check_Value()`

**Why it's thought to be a bug**

`quit("... current set to %d for %s ...", value, paramString);`: `value` is a double passed for `%d`, so the output is undefined (and `%s` may read a garbage pointer on some ABIs). The variable meant here is `checkType`. Function: `Check_Value()`.

**Current code** (`atlantisUtil/atUtilIO.c` lines 187–193)

```c
  187: 		break;
  188: 	default:
  189: 		quit(
  190: 				"Error: invalid entry_value must be set to 0, 1, 2, or 4, but current set to %d for %s (3 would indicate a sum which is of no use for single value entries)\n",
  191: 				value, paramString);
  192: 		break;
  193: 	}
```

**Suggested fix (from review):** Pass `checkType` instead of `value`.

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

## UTL2-06

**logic / unreachable** · Conf M / Sev L  
**Lines (current code):** 317-323  
**Lines at review (Oct 2026):** 317-323

**Function:** `readkeyprm_iarray_checked()`

**Why it's thought to be a bug**

`if (sized != verifsize) quit(...); else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) warn(...)`. Unlike the darray version (line 237: `verifsize > 0 &&`), passing verifsize <= 0 to mean "no verification" always quits. The "no verification" warning can only run in the degenerate case sized == verifsize <= 0. Current callers (atphysics/atphysics.c:151-172) pass `ts.nv - 1`, which quits if nv == 1. Function: `readkeyprm_iarray_checked()`.

**Current code** (`atlantisUtil/atUtilIO.c` lines 315–325)

```c
  315: 	sized = *size;
  316: 
  317: 	if (sized != verifsize) {
  318: 		quit("Array %s is wrong size (expecting %d and got %d as defined size from param file)\n", key, verifsize, sized);
  319: 		ans = 0;
  320: 	} else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) {
  321: 		fprintf(stderr, "\n");
  322: 		warn("No verification of size possible for %s. Expected size is %d but an array of size %d was found.\n", key, verifsize, sized);
  323: 	}
  324: 
  325: 	if (!ans)
```

**Suggested fix (from review):** Mirror the darray guard: `if (verifsize > 0 && sized != verifsize)`.

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

## UTL2-07

**unchecked return / uninitialised** · Conf M / Sev L  
**Lines (current code):** 92-103  
**Lines at review (Oct 2026):** 92-103

**Function:** `Util_XML_Get_Value_Time()`

**Why it's thought to be a bug**

`Util_XML_Get_Value_String(...)`'s return is ignored. When `isRequired` is FALSE and the node is missing, `value` stays uninitialised and is passed to `strtosecs()`. The `localecotest` and `entry_type` parameters are also ignored. All current callers pass isRequired = 1, so this is latent. The error text names `Util_XML_Read_Time` instead of the function. Function: `Util_XML_Get_Value_Time()`.

**Current code** (`atlantisUtil/atUtilIO.c` lines 90–105)

```c
   90: }
   91: 
   92: int Util_XML_Get_Value_Time(char *fileName, int type, int localecotest, int isRequired, xmlNodePtr parent, int entry_type, char *valueName, double *val) {
   93: 
   94: 	char value[STRLEN];
   95: 
   96: 	Util_XML_Get_Value_String(fileName, type, isRequired, parent, valueName, value);
   97: 
   98: 
   99: 	if(!strtosecs(value,val) )
  100: 		quit("Util_XML_Read_Time: Can't read %s\n",valueName);
  101: 
  102: 	return TRUE;
  103: }
  104: 
  105: /**
```

**Suggested fix (from review):** Check the return and handle the missing, non-required case (e.g. set `*val = 0` and return FALSE).

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

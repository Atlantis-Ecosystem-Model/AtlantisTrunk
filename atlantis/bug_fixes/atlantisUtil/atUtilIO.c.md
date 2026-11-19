# atlantisUtil/atUtilIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [UTL2-04](#utl2-04) | 266 | H/M | wrong operator | `if ((*p)[i] < 0.0 && (*p)[i] > 1.0)` in the `probability_check` case is always false, so out-of-range entries… |  | [ ] |
| [UTL2-05](#utl2-05) | 186, 190 | H/L | format mismatch | `quit("... current set to %d for %s ...", value, paramString);`: `value` is a double passed for `%d`, so the o… |  | [ ] |
| [UTL2-06](#utl2-06) | 320, 237 | M/L | logic / unreachable | `if (sized != verifsize) quit(...); else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) warn(...)`. … |  | [ ] |
| [UTL2-07](#utl2-07) | 96 | M/L | unchecked return / uninitialised | `Util_XML_Get_Value_String(...)`'s return is ignored. When `isRequired` is FALSE and the node is missing, `val… |  | [ ] |

---

## UTL2-04

**wrong operator** · Conf H / Sev M  
**Found at (current code):** `atUtilIO.c` 266
**Lines at review (Oct 2026):** 272

**Function:** `readkeyprm_darray_checked()`

**Why it's thought to be a bug**

`if ((*p)[i] < 0.0 && (*p)[i] > 1.0)` in the `probability_check` case is always false, so out-of-range entries are never rejected (only the sum is checked). This is a separate instance from the already-logged line 178, and the `proportion_check` case at line 262 correctly uses `||`. Function: `readkeyprm_darray_checked()`.

**Code at issue** — `atUtilIO.c` line 266 (located by name only; check this is the right place)  
Search for: `case probability_check:`

```c
  264: 		}
  265: 		break;
  266→ 	case probability_check:
  267: 		/* Values are between 0 and 1, and sum is 1.0 */
  268: 		if (verbose > 2)
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
**Found at (current code):** `atUtilIO.c` 186, `atUtilIO.c` 190
**Lines at review (Oct 2026):** 189-191

**Function:** `Check_Value()`

**Why it's thought to be a bug**

`quit("... current set to %d for %s ...", value, paramString);`: `value` is a double passed for `%d`, so the output is undefined (and `%s` may read a garbage pointer on some ABIs). The variable meant here is `checkType`. Function: `Check_Value()`.

**Code at issue** — `atUtilIO.c` line 186  
Search for: `quit("`

```c
  184: 		diff = fabs(base - value);
  185: 		if (diff > 0)
  186→ 			quit("Error: value of %s - %s must be an integer (currently %e)\n", typeName, paramString, value);
  187: 		break;
  188: 	default:
```

**Code at issue** — `atUtilIO.c` line 190  
Search for: `"Error: invalid entry_value must be set to 0, 1, 2, or 4, but current set to %d for %s (3 `

```c
  188: 	default:
  189: 		quit(
  190→ 				"Error: invalid entry_value must be set to 0, 1, 2, or 4, but current set to %d for %s (3 would indicate a sum which is of no use for single value entries)\n",
  191: 				value, paramString);
  192: 		break;
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
**Found at (current code):** `atUtilIO.c` 320, `atUtilIO.c` 237
**Lines at review (Oct 2026):** 317-323

**Function:** `readkeyprm_iarray_checked()`

**Why it's thought to be a bug**

`if (sized != verifsize) quit(...); else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) warn(...)`. Unlike the darray version (line 237: `verifsize > 0 &&`), passing verifsize <= 0 to mean "no verification" always quits. The "no verification" warning can only run in the degenerate case sized == verifsize <= 0. Current callers (atphysics/atphysics.c:151-172) pass `ts.nv - 1`, which quits if nv == 1. Function: `readkeyprm_iarray_checked()`.

**Code at issue** — `atUtilIO.c` line 320 (closest match to the quoted code; wording differs slightly)  
Search for: `} else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) {`

```c
  318: 		quit("Array %s is wrong size (expecting %d and got %d as defined size from param file)\n", key, verifsize, sized);
  319: 		ans = 0;
  320→ 	} else if ((sized != 1) && (verifsize > -2) && (verifsize < 1)) {
  321: 		fprintf(stderr, "\n");
  322: 		warn("No verification of size possible for %s. Expected size is %d but an array of size %d was found.\n", key, verifsize, sized);
```

**Code at issue** — `atUtilIO.c` line 237  
Search for: `verifsize > 0 &&`

```c
  235: 	sized = *size;
  236: 
  237→ 	if (verifsize > 0 && sized != verifsize) {
  238: 		quit("Array %s is wrong size (expecting %d and got %d as defined size from param file)\n", key, verifsize, sized);
  239: 		ans = 0;
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
**Found at (current code):** `atUtilIO.c` 96
**Lines at review (Oct 2026):** 92-103

**Function:** `Util_XML_Get_Value_Time()`

**Why it's thought to be a bug**

`Util_XML_Get_Value_String(...)`'s return is ignored. When `isRequired` is FALSE and the node is missing, `value` stays uninitialised and is passed to `strtosecs()`. The `localecotest` and `entry_type` parameters are also ignored. All current callers pass isRequired = 1, so this is latent. The error text names `Util_XML_Read_Time` instead of the function. Function: `Util_XML_Get_Value_Time()`.

**Code at issue** — `atUtilIO.c` line 96  
Search for: `Util_XML_Get_Value_String(`

```c
   94: 	char value[STRLEN];
   95: 
   96→ 	Util_XML_Get_Value_String(fileName, type, isRequired, parent, valueName, value);
   97: 
   98: 
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

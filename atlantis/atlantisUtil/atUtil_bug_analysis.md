# Bug Analysis for atUtil.c

## Bugs Found

### 1. Copy-paste error in function name within error message
**Location:** Line 228  
**Function:** `Util_Get_Fishery_Index()`  
**Issue:** The error message refers to the wrong function name
```c
quit("Util_Get_Fishery_Name - fishery code '%s' not recognised", strPtr);
```
**Problem:** Should be `Util_Get_Fishery_Index` not `Util_Get_Fishery_Name` (which is a different function)  
**Why it's a bug:** This is a copy-paste error that will confuse debugging efforts when this error is triggered, as it points to the wrong function name.

---

### 2. Inconsistent parameter string naming
**Location:** Line 358  
**Function:** `Util_Setup_Species_Param_Strings()`  
**Issue:** Parameter name mismatch between ID and string value
```c
snprintf(paramStrings[RSstarve_id], paramlen, "%s", "RNstarve_");
```
**Problem:** The parameter ID is `RSstarve_id` but the string is `"RNstarve_"` (RN instead of RS)  
**Why it's a bug:** This appears to be a typo/copy-paste error. Based on the pattern of other parameters like `SNcost_id`/`"SNcost_"` and `RNcost_id`/`"RNcost_"` on lines 356-357, this should likely be `"RSstarve_"` to match `RSstarve_id`. This will cause the wrong parameter name to be used when reading input files.

---

### 3. Unnecessary "_id" suffix in parameter strings
**Location:** Lines 456-457  
**Function:** `Util_Setup_Species_Param_Strings()`  
**Issue:** Parameter strings include "_id" suffix which shouldn't be in the actual parameter name
```c
snprintf(paramStrings[threshdepth_id], paramlen, "%s", "_threshdepth_id");
snprintf(paramStrings[depmum_scalar_id], paramlen, "%s", "_depmum_scalar_id");
```
**Problem:** All other parameter strings don't include the "_id" suffix. These should be:
- `"_threshdepth"` not `"_threshdepth_id"`
- `"_depmum_scalar"` not `"_depmum_scalar_id"`

**Why it's a bug:** Inconsistent with all other parameter naming patterns in this function. The `_id` suffix is part of the C enum/constant name, not part of the actual parameter name that would appear in input files. This will cause the parameters to be unreadable from input files or create inconsistent naming.

---

### 4. Missing format argument in quit() call
**Location:** Line 886  
**Function:** `Util_Get_Current_Stock_Index()`  
**Issue:** Format specifier without corresponding argument
```c
default:
    quit("No such stock structure case (%d) defined. Recode\n");
    break;
```
**Problem:** The format string contains `%d` but no argument is provided. Should be:
```c
quit("No such stock structure case (%d) defined. Recode\n", sp_stock_type);
```
**Why it's a bug:** This will either cause undefined behavior or print garbage values when the error is triggered. The intent is clearly to show which stock structure case caused the problem.

---

### 5. Missing format argument in quit() call (duplicate issue)
**Location:** Line 932  
**Function:** `Util_Calculate_StockID()`  
**Issue:** Format specifier without corresponding argument
```c
default:
    quit("No such stock structure case (%d) defined. Recode\n", stock_type);
    break;
```
**Wait, checking again...**  
Actually, this one IS correct - it has `stock_type` as the argument. Let me recheck line 886...

Actually, looking at line 886 again:
```c
default:
    quit("No such stock structure case (%d) defined. Recode\n");
    break;
```
This is missing the argument, while line 932 has it correctly.

---

## Summary

**Total bugs found: 4**

1. **Line 228:** Wrong function name in error message (`Util_Get_Fishery_Name` should be `Util_Get_Fishery_Index`)
2. **Line 358:** Parameter string mismatch (`"RNstarve_"` should be `"RSstarve_"`)
3. **Lines 456-457:** Incorrect "_id" suffixes in parameter strings
4. **Line 886:** Missing format argument for `%d` in quit() call

All of these are subtle bugs that could be copy-paste mistakes or typos, exactly the kind of issues you asked me to look for.

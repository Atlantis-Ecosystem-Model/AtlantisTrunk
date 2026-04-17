# Bug Analysis: Deserialiser.c

## Bugs Found

### 1. **Copy-Paste Error in GETDETRITUS case (Line 121)**
**Line:** 121  
**Severity:** HIGH - Functional bug  
**Issue:** In the `GETDETRITUS` case, the code calls `Link_GetBiomass()` instead of what should likely be `Link_GetDetritus()`.

```c
case REQUESTS__REQUEST__TYPE__GETDETRITUS:
    Deserialiser_GetDetritus(interface, request);
    
    /* Get the response*/
    data = Util_Alloc_Init_1D_Double(interface->numPolygons, 0.0);
    Link_GetBiomass(bm, interface->groupName, interface->numPolygons, interface->polygonList, data);  // WRONG FUNCTION!
```

**Why it's an error:** This is a clear copy-paste bug from the `GETBIOMASS` case (lines 89-103). The function name should match the request type. When handling a detritus request, it should call a detritus-specific function, not the biomass function.

---

### 2. **Copy-Paste Error in Error Message (Line 318)**
**Line:** 318  
**Severity:** LOW - Error message bug  
**Issue:** In `Deserialiser_GetMortality()`, the error message incorrectly says "Deserialiser_GetBiomass" instead of "Deserialiser_GetMortality".

```c
void Deserialiser_GetMortality(LinkageInterface *interface, Requests__Request *baseRequest) {
    Requests__GetMortality *request;
    
    request = baseRequest->getmortality;
    if (request == NULL) {
        fprintf(stderr, "Deserialiser_GetBiomass: getMortality message is null.\n");  // WRONG FUNCTION NAME!
    }
```

**Why it's an error:** This is a copy-paste error from `Deserialiser_GetBiomass()` (line 258). The error message should correctly identify which function is reporting the error for debugging purposes.

---

### 3. **Dead Code / Redundant Assignment (Line 86)**
**Line:** 86  
**Severity:** LOW - Code quality issue  
**Issue:** Response is set to NULL and then immediately overwritten on the next line.

```c
case REQUESTS__REQUEST__TYPE__SHUTDOWN:
    if (interface->verbose > 0)
        printf("ShutDown request\n");
    Deserialiser_ShutDown(request);
    interface->shutdown = TRUE;
    /* No response required - but broker sits there listening for one.*/
    response = NULL;                                    // Line 86 - Dead code
    response = Serialise_Boolean_Response(TRUE);        // Line 87 - Overwrites NULL
    break;
```

**Why it's an error:** The assignment `response = NULL` is pointless dead code. Either the comment is outdated and line 86 should be removed, or line 87 was added later and shouldn't be there.

---

### 4. **Incorrect Use of sizeof(void) (Lines 187, 209)**
**Lines:** 187, 209  
**Severity:** MEDIUM - Portability/correctness issue  
**Issue:** Using `sizeof(void)` which is non-standard (GNU extension) and semantically incorrect.

```c
// Line 187 in Serialise_Boolean_Response
message->bytes = malloc(sizeof(void) * (message->size));

// Line 209 in Serialise_Data_Response  
message->bytes = malloc(sizeof(void) * (message->size));
```

**Why it's an error:** 
- `sizeof(void)` is not standard C (it's a GNU extension that evaluates to 1)
- It's semantically incorrect - the intent is to allocate `message->size` bytes
- Should be: `malloc(message->size)` or `malloc(sizeof(uint8_t) * message->size)`
- While it may work in practice (sizeof(void) == 1), it's non-portable and confusing

---

### 5. **Suspected Missing/Wrong Function Call (Line 148)**
**Line:** 148  
**Severity:** MEDIUM - Suspicious commented code  
**Issue:** In the `SETDETRITUS` case, there's a commented-out call to `Link_SetBiomass()` with no replacement.

```c
case REQUESTS__REQUEST__TYPE__SETDETRITUS:
    Deserialiser_SetDetritus(interface, request);
    
    //Link_SetBiomass(bm, interface->groupName, interface->numPolygons, interface->polygonList, interface->valueList);
    
    /* Run model timestep */
    response = Serialise_Boolean_Response(TRUE);
    break;
```

**Why it's suspicious:** 
- This appears to be a copy-paste from the `SETBIOMASS` case (lines 130-138)
- The function call is commented out but there's no replacement
- If this is intentional (detritus is handled differently), there should be a comment explaining why
- If it's unintentional, it should call `Link_SetDetritus()` instead of the commented `Link_SetBiomass()`
- This makes SETDETRITUS a no-op aside from unpacking the request

---

## Summary

Total bugs found: **5**

**High Priority:**
1. Line 121: Wrong function call in GETDETRITUS case (calling Link_GetBiomass instead of likely Link_GetDetritus)

**Medium Priority:**
2. Lines 187, 209: Non-standard sizeof(void) usage
3. Line 148: Missing/commented-out function call in SETDETRITUS case

**Low Priority:**
4. Line 86: Dead code (redundant NULL assignment)
5. Line 318: Wrong function name in error message

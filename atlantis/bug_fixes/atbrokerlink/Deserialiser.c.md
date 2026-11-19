# atbrokerlink/Deserialiser.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-50](#link-50) | 57 | M/M | missing default / leak | The `switch (request->type)` has no `default`: an unrecognised type leaks the unpacked request (never `request… |  | [ ] |
| [LINK-51](#link-51) | 260, 286, 167 | H/M | NULL dereference | Every sub-message NULL check only prints, then dereferences anyway (`biomassRequest->groupid`, `request->n_val… |  | [ ] |
| [LINK-52](#link-52) ⭐ | 293 | H/H | buffer overrun (network input) | `strcpy(interface->groupName, request->groupid)` copies an unbounded network string into `char groupName[1024]… |  | [ ] |
| [LINK-53](#link-53) | 121 | M/M | copy-paste | The GETDETRITUS response calls `Link_GetBiomass(...)` rather than `Link_GetDetritus` (which happens to wrap Ge… |  | [ ] |
| [LINK-54](#link-54) | 133 | M/M | missing call | SETMORTALITY and SETDETRITUS unpack values into `interface->valueList` but never call `Link_SetMortality` / `L… |  | [ ] |
| [LINK-55](#link-55) | 74 | L/M | ignored return | `halt = Link_RunNextTimeStep(bm);` — it is printed only, and the broker always receives TRUE even if the model… |  | [ ] |
| [LINK-56](#link-56) | 277 | M/L | format specifier | `%ld` for a `size_t length` (and `%d` for an `unsigned int numPolygons`). This is wrong on LLP64 Windows. Func… |  | [ ] |
| [LINK-57](#link-57) | 187 | M/L | unchecked malloc | `malloc` results for message and bytes are not checked before `pack()` writes into them. `sizeof(void)` is a G… |  | [ ] |

---

## LINK-50

**missing default / leak** · Conf M / Sev M  
**Found at (current code):** `Deserialiser.c` 57
**Lines at review (Oct 2026):** 57-153

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

The `switch (request->type)` has no `default`: an unrecognised type leaks the unpacked request (never `requests__request__free_unpacked`) and returns NULL, which causes the protocol failure in LINK-47. Function: `Deserialise_Message()`.

**Code at issue** — `Deserialiser.c` line 57  
Search for: `switch (request->type)`

```c
   55: 	}
   56: 
   57→ 	switch (request->type) {
   58: 	case REQUESTS__REQUEST__TYPE__INIT:
   59: 
```

**Suggested fix (from review):** Add a default that frees the request and returns a FALSE boolean response.

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

## LINK-51

**NULL dereference** · Conf H / Sev M  
**Found at (current code):** `Deserialiser.c` 260, `Deserialiser.c` 286, `Deserialiser.c` 167
**Lines at review (Oct 2026):** 256-261, 280-293, 316-321, 339-351, 373-378, 396-408, 164-167

**Why it's thought to be a bug**

Every sub-message NULL check only prints, then dereferences anyway (`biomassRequest->groupid`, `request->n_values`, `requestInit->timestep`). `groupid` and `timestep` (char*) can also be NULL in protobuf-c. A malformed broker message crashes Atlantis. Functions: `Deserialiser_Init()`, `Deserialiser_GetBiomass()`, `Deserialiser_SetBiomass()`, `Deserialiser_GetMortality()`, `Deserialiser_SetMortality()`, `Deserialiser_GetDetritus()`, `Deserialiser_SetDetritus()`.

**Code at issue** — `Deserialiser.c` line 260  
Search for: `biomassRequest->groupid`

```c
  258: 		fprintf(stderr, "Deserialiser_GetBiomass: getbiomass message is null.\n");
  259: 	}
  260→ 	group = biomassRequest->groupid;
  261: 	strcpy(interface->groupName, group);
  262: 	if (interface->verbose) {
```

**Code at issue** — `Deserialiser.c` line 286  
Search for: `request->n_values`

```c
  284: 
  285: 	/* Check the length specified is what we expect */
  286→ 	length = request->n_values;
  287: 	if (length != interface->numPolygons) {
  288: 		fprintf(stderr, "Deserialiser_SetBiomass. Number of values sent from Broker %ld does not match number of expected polygon values %d\n", length,
```

**Code at issue** — `Deserialiser.c` line 167  
Search for: `requestInit->timestep`

```c
  165: 
  166: 	// display the message's fields.
  167→ 	returnValue = atof(requestInit->timestep);
  168: 
  169: 	// Free the unpacked message
```

**Suggested fix (from review):** Return or exit after the error message, and check groupid/timestep for NULL.

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

## LINK-52

**buffer overrun (network input)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `Deserialiser.c` 293
**Lines at review (Oct 2026):** 261, 293, 321, 351, 378, 408

**Why it's thought to be a bug**

`strcpy(interface->groupName, request->groupid)` copies an unbounded network string into `char groupName[1024]` (atBrokerLink.h:22): a remote input overflows a fixed buffer. Same six functions as LINK-51.

**Code at issue** — `Deserialiser.c` line 293  
Search for: `strcpy(interface->groupName, request->groupid)`

```c
  291: 	}
  292: 	/* Copy the groupName and valueList data across */
  293→ 	strcpy(interface->groupName, request->groupid);
  294: 	memcpy(interface->valueList, request->values, (sizeof(double) * length));
  295: 
```

**Suggested fix (from review):** `snprintf(interface->groupName, sizeof(interface->groupName), "%s", groupid)` and reject over-long names.

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

## LINK-53

**copy-paste** · Conf M / Sev M  
**Found at (current code):** `Deserialiser.c` 121
**Lines at review (Oct 2026):** 115-121

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

The GETDETRITUS response calls `Link_GetBiomass(...)` rather than `Link_GetDetritus` (which happens to wrap GetBiomass now). GETMORTALITY returns dummy `data[i] = i` (lines 109-111). The broker gets fake mortality data with a TRUE response. Function: `Deserialise_Message()`.

**Code at issue** — `Deserialiser.c` line 121  
Search for: `Link_GetBiomass(`

```c
  119: 
  120: 		data = Util_Alloc_Init_1D_Double(interface->numPolygons, 0.0);
  121→ 		Link_GetBiomass(bm, interface->groupName, interface->numPolygons, interface->polygonList, data);
  122: 
  123: 		for (i = 0; i < interface->numPolygons; i++) {
```

**Suggested fix (from review):** Call Link_GetDetritus, and implement mortality or return a failure response.

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

## LINK-54

**missing call** · Conf M / Sev M  
**Found at (current code):** `Deserialiser.c` 133
**Lines at review (Oct 2026):** 139-152

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

SETMORTALITY and SETDETRITUS unpack values into `interface->valueList` but never call `Link_SetMortality` / `Link_SetDetritus` (the line is commented out). The broker data is silently discarded while TRUE is returned. Function: `Deserialise_Message()`.

**Code at issue** — `Deserialiser.c` line 133  
Search for: `interface->valueList`

```c
  131: 		Deserialiser_SetBiomass(interface, request);
  132: 
  133→ 		Link_SetBiomass(bm, interface->groupName, interface->numPolygons, interface->polygonList, interface->valueList);
  134: 
  135: 		/* Run model timestep */
```

**Suggested fix (from review):** Call Link_SetMortality / Link_SetDetritus with valueList.

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

## LINK-55

**ignored return** · Conf L / Sev M  
**Found at (current code):** `Deserialiser.c` 74
**Lines at review (Oct 2026):** 74-78

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

`halt = Link_RunNextTimeStep(bm);` — it is printed only, and the broker always receives TRUE even if the model signalled halt or failure. Function: `Deserialise_Message()`.

**Code at issue** — `Deserialiser.c` line 74  
Search for: `halt = Link_RunNextTimeStep(bm)`

```c
   72: 		Deserialiser_RunNextTimestep(request);
   73: 
   74→ 		halt = Link_RunNextTimeStep(bm);
   75: 		printf("halt = %d\n", halt);
   76: 
```

**Suggested fix (from review):** Return `!halt` (or the appropriate status) in the boolean response.

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

## LINK-56

**format specifier** · Conf M / Sev L  
**Found at (current code):** `Deserialiser.c` 277
**Lines at review (Oct 2026):** 288

**Function:** `Deserialiser_SetBiomass()`

**Why it's thought to be a bug**

`%ld` for a `size_t length` (and `%d` for an `unsigned int numPolygons`). This is wrong on LLP64 Windows. Function: `Deserialiser_SetBiomass()`.

**Code at issue** — `Deserialiser.c` line 277 (located by name only; check this is the right place)  
Search for: `size_t length;`

```c
  275: 	Requests__SetBiomass *request;
  276: 
  277→ 	size_t length;
  278: 	unsigned int i;
  279: 
```

**Suggested fix (from review):** Use `%zu` and `%u`.

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

## LINK-57

**unchecked malloc** · Conf M / Sev L  
**Found at (current code):** `Deserialiser.c` 187
**Lines at review (Oct 2026):** 180-189, 199-211

**Why it's thought to be a bug**

`malloc` results for message and bytes are not checked before `pack()` writes into them. `sizeof(void)` is a GNU extension (1). Functions: `Serialise_Boolean_Response()`, `Serialise_Data_Response()`.

**Code at issue** — `Deserialiser.c` line 187 (located by name only; check this is the right place)  
Search for: `message->bytes = malloc(sizeof(void) * (message->size));`

```c
  185: 
  186: 	message->size = responses__response__get_packed_size(&response);
  187→ 	message->bytes = malloc(sizeof(void) * (message->size));
  188: 
  189: 	responses__response__pack(&response, message->bytes);
```

**Suggested fix (from review):** Check for NULL, and use `malloc(message->size)`.

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

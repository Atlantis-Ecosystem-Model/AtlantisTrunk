# atbrokerlink/Deserialiser.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **8 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-50](#link-50) | 57-153 | M/M | missing default / leak | The `switch (request->type)` has no `default`: an unrecognised type leaks the unpacked request (never `request… |  | [ ] |
| [LINK-51](#link-51) | 256-261, 280-293, 316-321, 339-351, 373-378, 396-408, 164-167 | H/M | NULL dereference | Every sub-message NULL check only prints, then dereferences anyway (`biomassRequest->groupid`, `request->n_val… |  | [ ] |
| [LINK-52](#link-52) ⭐ | 261, 293, 321, 351, 378, 408 | H/H | buffer overrun (network input) | `strcpy(interface->groupName, request->groupid)` copies an unbounded network string into `char groupName[1024]… |  | [ ] |
| [LINK-53](#link-53) | 115-121 | M/M | copy-paste | The GETDETRITUS response calls `Link_GetBiomass(...)` rather than `Link_GetDetritus` (which happens to wrap Ge… |  | [ ] |
| [LINK-54](#link-54) | 139-152 | M/M | missing call | SETMORTALITY and SETDETRITUS unpack values into `interface->valueList` but never call `Link_SetMortality` / `L… |  | [ ] |
| [LINK-55](#link-55) | 74-78 | L/M | ignored return | `halt = Link_RunNextTimeStep(bm);` — it is printed only, and the broker always receives TRUE even if the model… |  | [ ] |
| [LINK-56](#link-56) | 288 | M/L | format specifier | `%ld` for a `size_t length` (and `%d` for an `unsigned int numPolygons`). This is wrong on LLP64 Windows. Func… |  | [ ] |
| [LINK-57](#link-57) | 180-189, 199-211 | M/L | unchecked malloc | `malloc` results for message and bytes are not checked before `pack()` writes into them. `sizeof(void)` is a G… |  | [ ] |

---

## LINK-50

**missing default / leak** · Conf M / Sev M  
**Listed lines:** 57-153

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

The `switch (request->type)` has no `default`: an unrecognised type leaks the unpacked request (never `requests__request__free_unpacked`) and returns NULL, which causes the protocol failure in LINK-47. Function: `Deserialise_Message()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 55–89 (range truncated))

```c
   55: 	}
   56: 
   57: 	switch (request->type) {
   58: 	case REQUESTS__REQUEST__TYPE__INIT:
   59: 
   60: 		time = Deserialiser_Init(request);
   61: 		printf("time = %e\n", time);
   62: 		if (interface->verbose > 0)
   63: 			printf("Init request, time - %f\n", time);
   64: 		Link_SetTime(bm, time);
   65: 		response = Serialise_Boolean_Response(TRUE);
   66: 		break;
   67: 	case REQUESTS__REQUEST__TYPE__RUNNEXTTIMESTEP:
   68: 
   69: 		if (interface->verbose > 0)
   70: 			printf("RunNextTimeStep request\n");
   71: 		/* Model run next timestep */
   72: 		Deserialiser_RunNextTimestep(request);
   73: 
   74: 		halt = Link_RunNextTimeStep(bm);
   75: 		printf("halt = %d\n", halt);
   76: 
   77: 		/* Run model timestep */
   78: 		response = Serialise_Boolean_Response(TRUE);
   79: 		break;
   80: 	case REQUESTS__REQUEST__TYPE__SHUTDOWN:
   81: 		if (interface->verbose > 0)
   82: 			printf("ShutDown request\n");
   83: 		Deserialiser_ShutDown(request);
   84: 		interface->shutdown = TRUE;
   85: 		/* No response required - but broker sits there listening for one.*/
   86: 		response = NULL;
   87: 		response = Serialise_Boolean_Response(TRUE);
   88: 		break;
   89: 	case REQUESTS__REQUEST__TYPE__GETBIOMASS:
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
**Listed lines:** 256-261, 280-293, 316-321, 339-351, 373-378, 396-408, 164-167

**Why it's thought to be a bug**

Every sub-message NULL check only prints, then dereferences anyway (`biomassRequest->groupid`, `request->n_values`, `requestInit->timestep`). `groupid` and `timestep` (char*) can also be NULL in protobuf-c. A malformed broker message crashes Atlantis. Functions: `Deserialiser_Init()`, `Deserialiser_GetBiomass()`, `Deserialiser_SetBiomass()`, `Deserialiser_GetMortality()`, `Deserialiser_SetMortality()`, `Deserialiser_GetDetritus()`, `Deserialiser_SetDetritus()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 254–263)

```c
  254: 	char *group;
  255: 
  256: 	biomassRequest = request->getbiomass;
  257: 	if (biomassRequest == NULL) {
  258: 		fprintf(stderr, "Deserialiser_GetBiomass: getbiomass message is null.\n");
  259: 	}
  260: 	group = biomassRequest->groupid;
  261: 	strcpy(interface->groupName, group);
  262: 	if (interface->verbose) {
  263: 		printf("Deserialiser_GetBiomass. Group %s\n", interface->groupName);
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
**Listed lines:** 261, 293, 321, 351, 378, 408

**Why it's thought to be a bug**

`strcpy(interface->groupName, request->groupid)` copies an unbounded network string into `char groupName[1024]` (atBrokerLink.h:22): a remote input overflows a fixed buffer. Same six functions as LINK-51.

**Current code** (`atbrokerlink/Deserialiser.c` lines 259–298 (truncated))

```c
  259: 	}
  260: 	group = biomassRequest->groupid;
  261: 	strcpy(interface->groupName, group);
  262: 	if (interface->verbose) {
  263: 		printf("Deserialiser_GetBiomass. Group %s\n", interface->groupName);
  264: 	}
  265: 
  266: 	/* Free the unpacked message */
  267: 	requests__request__free_unpacked(request, NULL);
  268: }
  269: 
  270: /**
  271:  * Unpack the setbiomass request..
  272:  */
  273: int Deserialiser_SetBiomass(LinkageInterface *interface, Requests__Request *baseRequest) {
  274: 
  275: 	Requests__SetBiomass *request;
  276: 
  277: 	size_t length;
  278: 	unsigned int i;
  279: 
  280: 	request = baseRequest->setbiomass;
  281: 	if (request == NULL) {
  282: 		fprintf(stderr, "Deserialiser_SetBiomass: setbiomass message is null.\n");
  283: 	}
  284: 
  285: 	/* Check the length specified is what we expect */
  286: 	length = request->n_values;
  287: 	if (length != interface->numPolygons) {
  288: 		fprintf(stderr, "Deserialiser_SetBiomass. Number of values sent from Broker %ld does not match number of expected polygon values %d\n", length,
  289: 				interface->numPolygons);
  290: 		exit(-1);
  291: 	}
  292: 	/* Copy the groupName and valueList data across */
  293: 	strcpy(interface->groupName, request->groupid);
  294: 	memcpy(interface->valueList, request->values, (sizeof(double) * length));
  295: 
  296: 	if (interface->verbose) {
  297: 		printf("SetBiomass - %s, [", interface->groupName);
  298: 		for (i = 0; i < length; i++) {
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
**Listed lines:** 115-121

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

The GETDETRITUS response calls `Link_GetBiomass(...)` rather than `Link_GetDetritus` (which happens to wrap GetBiomass now). GETMORTALITY returns dummy `data[i] = i` (lines 109-111). The broker gets fake mortality data with a TRUE response. Function: `Deserialise_Message()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 111–121)

```c
  111: 		}
  112: 		response = Serialise_Data_Response(data, interface->numPolygons);
  113: 		free(data);
  114: 		break;
  115: 	case REQUESTS__REQUEST__TYPE__GETDETRITUS:
  116: 		Deserialiser_GetDetritus(interface, request);
  117: 
  118: 		/* Get the response*/
  119: 
  120: 		data = Util_Alloc_Init_1D_Double(interface->numPolygons, 0.0);
  121: 		Link_GetBiomass(bm, interface->groupName, interface->numPolygons, interface->polygonList, data);
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
**Listed lines:** 139-152

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

SETMORTALITY and SETDETRITUS unpack values into `interface->valueList` but never call `Link_SetMortality` / `Link_SetDetritus` (the line is commented out). The broker data is silently discarded while TRUE is returned. Function: `Deserialise_Message()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 137–154)

```c
  137: 
  138: 		break;
  139: 	case REQUESTS__REQUEST__TYPE__SETMORTALITY:
  140: 		Deserialiser_SetMortality(interface, request);
  141: 		/* Run model timestep */
  142: 		response = Serialise_Boolean_Response(TRUE);
  143: 
  144: 		break;
  145: 	case REQUESTS__REQUEST__TYPE__SETDETRITUS:
  146: 		Deserialiser_SetDetritus(interface, request);
  147: 
  148: 		//Link_SetBiomass(bm, interface->groupName, interface->numPolygons, interface->polygonList, interface->valueList);
  149: 
  150: 		/* Run model timestep */
  151: 		response = Serialise_Boolean_Response(TRUE);
  152: 		break;
  153: 	}
  154: 	return response;
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
**Listed lines:** 74-78

**Function:** `Deserialise_Message()`

**Why it's thought to be a bug**

`halt = Link_RunNextTimeStep(bm);` — it is printed only, and the broker always receives TRUE even if the model signalled halt or failure. Function: `Deserialise_Message()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 72–80)

```c
   72: 		Deserialiser_RunNextTimestep(request);
   73: 
   74: 		halt = Link_RunNextTimeStep(bm);
   75: 		printf("halt = %d\n", halt);
   76: 
   77: 		/* Run model timestep */
   78: 		response = Serialise_Boolean_Response(TRUE);
   79: 		break;
   80: 	case REQUESTS__REQUEST__TYPE__SHUTDOWN:
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
**Listed lines:** 288

**Function:** `Deserialiser_SetBiomass()`

**Why it's thought to be a bug**

`%ld` for a `size_t length` (and `%d` for an `unsigned int numPolygons`). This is wrong on LLP64 Windows. Function: `Deserialiser_SetBiomass()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 286–290)

```c
  286: 	length = request->n_values;
  287: 	if (length != interface->numPolygons) {
  288: 		fprintf(stderr, "Deserialiser_SetBiomass. Number of values sent from Broker %ld does not match number of expected polygon values %d\n", length,
  289: 				interface->numPolygons);
  290: 		exit(-1);
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
**Listed lines:** 180-189, 199-211

**Why it's thought to be a bug**

`malloc` results for message and bytes are not checked before `pack()` writes into them. `sizeof(void)` is a GNU extension (1). Functions: `Serialise_Boolean_Response()`, `Serialise_Data_Response()`.

**Current code** (`atbrokerlink/Deserialiser.c` lines 178–213)

```c
  178: 	Responses__BooleanResponse responseInit = RESPONSES__BOOLEAN_RESPONSE__INIT;
  179: 
  180: 	message = (Message *) malloc(sizeof(Message));
  181: 
  182: 	response.bool_ = &responseInit;
  183: 	response.type = RESPONSES__RESPONSE__TYPE__BOOL;
  184: 	responseInit.response = responseValue;
  185: 
  186: 	message->size = responses__response__get_packed_size(&response);
  187: 	message->bytes = malloc(sizeof(void) * (message->size));
  188: 
  189: 	responses__response__pack(&response, message->bytes);
  190: 
  191: 	return message;
  192: }
  193: 
  194: Message *Serialise_Data_Response(double *data, int length) {
  195: 	Message *message;
  196: 	Responses__Response response = RESPONSES__RESPONSE__INIT; // AMessage
  197: 	Responses__GetResponse responseGet = RESPONSES__GET_RESPONSE__INIT;
  198: 
  199: 	message = (Message *) malloc(sizeof(Message));
  200: 
  201: 	response.get = &responseGet;
  202: 	response.type = RESPONSES__RESPONSE__TYPE__GET;
  203: 
  204: 	/* Allocate memory and copy data across */
  205: 	responseGet.n_values = length;
  206: 	responseGet.values = data;
  207: 
  208: 	message->size = responses__response__get_packed_size(&response);
  209: 	message->bytes = malloc(sizeof(void) * (message->size));
  210: 
  211: 	responses__response__pack(&response, message->bytes);
  212: 
  213: 	return message;
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

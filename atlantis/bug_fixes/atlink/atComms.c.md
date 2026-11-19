# atlink/atComms.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-23](#link-23) ⭐ | 321-328 | H/H | buffer / sizeof(pointer) / overlap | `snprintf(str, sizeof(str), "%s,%e", str, ...)` — `str` is a `char *`, so `sizeof(str)` is 8 and the output is… |  | [ ] |
| [LINK-24](#link-24) | 345, 353 | H/M | copy-paste / ignored param | `soap_ctx_new_with_method(EwEUrn, EwEGetBiomassMethod, ...)` and `soap_env_add_item(..., "name", "PL")` — the … |  | [ ] |
| [LINK-25](#link-25) | 252 | M/M | copy-paste | `soap_client_invoke(..., EwEServerUrl, "http://tempuri.org/Init")` — the SOAPAction is hard-coded to Init for … |  | [ ] |
| [LINK-26](#link-26) | 265-268 | L/L | leak | On `checkResponse()==FALSE` it returns without `endClient()`, leaking request, response and the client. checkR… |  | [ ] |

---

## LINK-23

**buffer / sizeof(pointer) / overlap** · Conf H / Sev H ⭐ priority  
**Listed lines:** 321-328

**Function:** `buildDataString()`

**Why it's thought to be a bug**

`snprintf(str, sizeof(str), "%s,%e", str, ...)` — `str` is a `char *`, so `sizeof(str)` is 8 and the output is truncated to 7 characters. snprintf with the destination also used as a `%s` source is undefined behaviour (overlapping). The resulting data string sent to EwE is garbage. malloc is also unchecked. Function: `buildDataString()`.

**Current code** (`atlink/atComms.c` lines 319–330)

```c
  319: 	char *str;
  320: 
  321: 	str = (char *)malloc(sizeof(char) * numColumns * numRows * 50);
  322: 
  323:    	for(rows = 0; rows < numRows; rows++){
  324:    		for(columns = 0; columns < numColumns; columns++){
  325:    			if(rows == 0 && columns == 0)
  326:    				snprintf(str, sizeof(str), "%e", dataArray[rows][columns]);
  327:    			else
  328:    				snprintf(str, sizeof(str), "%s,%e", str, dataArray[rows][columns]);
  329:    		}
  330:    	}
```

**Suggested fix (from review):** Track the offset: `pos += snprintf(str+pos, cap-pos, pos?",%e":"%e", v)` with `cap = numRows*numColumns*50`.

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

## LINK-24

**copy-paste / ignored param** · Conf H / Sev M  
**Listed lines:** 345, 353

**Function:** `getParameter()`

**Why it's thought to be a bug**

`soap_ctx_new_with_method(EwEUrn, EwEGetBiomassMethod, ...)` and `soap_env_add_item(..., "name", "PL")` — the `method` and `species` parameters are ignored, so getProduction, getMortality, getNutrients and getProperties all request biomass for group "PL". The returned data is also never parsed into `dataArray`. Function: `getParameter()`.

**Current code** (`atlink/atComms.c` lines 343–355)

```c
  343: 
  344: 	/* Create the XML Envelope */
  345:    	if ((err = soap_ctx_new_with_method(EwEUrn, EwEGetBiomassMethod, &request)) != H_OK)
  346:    	{
  347:      	printf("getBiomass - %s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
  348:      	herror_release(err);
  349:      	soap_client_destroy();
  350:      	exit(1);
  351:    	}
  352: 
  353:    	soap_env_add_item(request->env, "xsd:string", "name", "PL");
  354: 
  355: 	/* Invoke the server function */
```

**Suggested fix (from review):** Use `method` and `species`, and parse the response into dataArray.

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

## LINK-25

**copy-paste** · Conf M / Sev M  
**Listed lines:** 252

**Function:** `callBooleanServerFunction()`

**Why it's thought to be a bug**

`soap_client_invoke(..., EwEServerUrl, "http://tempuri.org/Init")` — the SOAPAction is hard-coded to Init for every method (LoadModel, RunEcospace, RunNextEcospaceTS, runModel). Function: `callBooleanServerFunction()`.

**Current code** (`atlink/atComms.c` lines 250–254)

```c
  250: 
  251: 	/* Invoke the server function */
  252:    	if ((err = soap_client_invoke(request, &response, EwEServerUrl, "http://tempuri.org/Init")) != H_OK)
  253:    	{
  254:      	printf("[%d] %s(): %s\n", herror_code(err), herror_func(err), herror_message(err));
```

**Suggested fix (from review):** Build the SOAPAction from `method`.

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

## LINK-26

**leak** · Conf L / Sev L  
**Listed lines:** 265-268

**Function:** `callBooleanServerFunction()`

**Why it's thought to be a bug**

On `checkResponse()==FALSE` it returns without `endClient()`, leaking request, response and the client. checkResponse currently always returns TRUE. Function: `callBooleanServerFunction()`.

**Current code** (`atlink/atComms.c` lines 263–270)

```c
  263: 
  264: 	/* Check that the response was true */
  265: 	if(checkResponse(response) == FALSE){
  266: 		printf("EwE model setup failed \n");
  267: 		return FALSE;
  268: 	}
  269: 
  270: 	endClient(request, response);
```

**Suggested fix (from review):** Call endClient before returning.

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

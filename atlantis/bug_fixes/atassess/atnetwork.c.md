# atassess/atnetwork.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 1).

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [NET-01](#net-01) ✔ ⭐ | 80-86 | H/M | swapped | `pop_fraction[0/1]` = juvenile fraction (atattribute.c:974-978), but the juv branch uses `1 - pop_fraction`. A… |  | [ ] |
| [NET-02](#net-02) | 182 | L/M | index assumption | `detrit_import[stage*2 + detIndex]` assumes DL=0, DR=1 in group order. |  | [ ] |
| [NET-03](#net-03) | 58-64, 102-198, 250-256 | L/L | stale values | Only `[0][0][z]` is reset; skipped groups keep `minpool` from the previous year. |  | [ ] |

---

## NET-01

**swapped** · Conf H / Sev M ✔ re-checked ⭐ priority  
**Lines (current code):** 80-86  
**Lines at review (Oct 2026):** 80-86

**Why it's thought to be a bug**

`pop_fraction[0/1]` = juvenile fraction (atattribute.c:974-978), but the juv branch uses `1 - pop_fraction`. Also, birds never get pop_fraction.

**Current code** (`atassess/atnetwork.c` lines 78–88)

```c
   78: 					if (FunctGroupArray[sp].isVertebrate == TRUE) {
   79: 
   80: 						if (stage == juv_id) {
   81: 							aprop = 1.0 - pop_fraction[0][sp][z];
   82: 							afprop = 1.0 - pop_fraction[1][sp][z];
   83: 						} else {
   84: 							aprop = pop_fraction[0][sp][z];
   85: 							afprop = pop_fraction[1][sp][z];
   86: 						}
   87: 
   88: 						eatnetwk[sp + 1][stage][z] = stockinfo[seat_id][sp][z][id] * aprop;
```

**Suggested fix (from review):** swap the branches

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

## NET-02

**index assumption** · Conf L / Sev M  
**Lines (current code):** 182  
**Lines at review (Oct 2026):** 182

**Why it's thought to be a bug**

`detrit_import[stage*2 + detIndex]` assumes DL=0, DR=1 in group order.

**Current code** (`atassess/atnetwork.c` lines 180–184)

```c
  180: 										DiscardFate[detIndex][stage] * sampledetbiom[FunctGroupArray[CarrionIndex].detIndex][0][z][id];
  181: 								exportnetwk[sp+1][stage][z] = 0;
  182: 								importnetwk[sp+1][stage][z] = detrit_import[(stage * 2) + detIndex][z];
  183: 								respnetwk[sp+1][stage][z] = 0;
  184: 								break;
```

**Suggested fix (from review):** index by type

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

## NET-03

**stale values** · Conf L / Sev L  
**Lines (current code):** 58-64, 102-198, 250-256  
**Lines at review (Oct 2026):** 58-64, 102-198, 250-256

**Why it's thought to be a bug**

Only `[0][0][z]` is reset; skipped groups keep `minpool` from the previous year.

**Current code** (`atassess/atnetwork.c` lines 56–95 (truncated)) — ⚠ the code quoted in the review is not at these lines any more (changed, or already fixed?)

```c
   56: 		printf("Calculating network info\n");
   57: 
   58: 	eatnetwk[0][0][z] = 0;
   59: 	prodnetwk[0][0][z] = 0.0;
   60: 	biomnetwk[0][0][z] = 0.0;
   61: 	exportnetwk[0][0][z] = 0;
   62: 	respnetwk[0][0][z] = 0; // TODO: Fix - calculation of this currently not sensible
   63: 	importnetwk[0][0][z] = 0.0;
   64: 	mortnetwk[0][0][z] = 0.0;
   65: 
   66: 	for (sp = 0; sp < bm->K_num_tot_sp; sp++) {
   67: 		if (FunctGroupArray[sp].isAssessed == TRUE) {
   68: 			/* Work out the max stage (or location index) */
   69: 			if (FunctGroupArray[sp].isVertebrate == TRUE || FunctGroupArray[sp].isDetritus == TRUE) {
   70: 				maxStage = adult_id; /* Allow for wc and sed values for detritus */
   71: 			} else {
   72: 				maxStage = juv_id;
   73: 			}
   74: 
   75: 			for (stage = 0; stage <= maxStage; stage++) {
   76: 				if (z != bm->nfzones) {
   77: 
   78: 					if (FunctGroupArray[sp].isVertebrate == TRUE) {
   79: 
   80: 						if (stage == juv_id) {
   81: 							aprop = 1.0 - pop_fraction[0][sp][z];
   82: 							afprop = 1.0 - pop_fraction[1][sp][z];
   83: 						} else {
   84: 							aprop = pop_fraction[0][sp][z];
   85: 							afprop = pop_fraction[1][sp][z];
   86: 						}
   87: 
   88: 						eatnetwk[sp + 1][stage][z] = stockinfo[seat_id][sp][z][id] * aprop;
   89: 						prodnetwk[sp + 1][stage][z] = stockinfo[sprod_id][sp][z][id] * aprop;
   90: 						u = drandom(1.0, 3.0);
   91: 						ge = u / 10.0;
   92: 						pcalc = ge * eatnetwk[sp + 1][stage][z];
   93: 						if (pcalc < prodnetwk[sp + 1][stage][z])
   94: 							prodnetwk[sp + 1][stage][z] = pcalc;
   95: 
```

**Suggested fix (from review):** zero all

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

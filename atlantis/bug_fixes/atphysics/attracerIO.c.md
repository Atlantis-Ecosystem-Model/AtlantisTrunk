# atphysics/attracerIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-04](#phy2-04) | 235, 231, 481 | H/M | stale variable | `ncattget(fid, NC_GLOBAL, "icenz", &n); if(n!= 0)` runs with `ncopts=0`. When the attribute is missing, ncattg… |  | [ ] |
| [PHY2-05](#phy2-05) | 590, 721 | H/L | accumulator not reset | `found_partial_match_n` is set to 1 but never reset per tracer (only `found_partial_match` is reset at 658). O… |  | [ ] |
| [PHY2-06](#phy2-06) | 821 | H/L | format specifier | `quit("... %d", ..., sizeof(doubleINPUT))` passes a size_t to `%d`. The same pattern is at atagetracerIO.c:243… |  | [ ] |

---

## PHY2-04

**stale variable** · Conf H / Sev M  
**Found at (current code):** `attracerIO.c` 235, `attracerIO.c` 231, `attracerIO.c` 481
**Lines at review (Oct 2026):** 223-244

**Function:** `readBMTracerInfo()`

**Why it's thought to be a bug**

`ncattget(fid, NC_GLOBAL, "icenz", &n); if(n!= 0)` runs with `ncopts=0`. When the attribute is missing, ncattget fails without writing to `n`, so `n` keeps the `sednz` value read at line 227 (non-zero). So `ice_on` is set TRUE for every model that has no ice. Then every tracer goes through the `inice` read path, and `boxes[b].ice.ntr` is set to ntracer (nice++ for every tracer). In atlantismain.c:1267 this triggers `readBMIceInfo`, which only partly undoes it: it sets ice_on FALSE but `num_active_habitats = ICE_BASED` (4), not the `EPIFAUNA+1` (3) intended here. Function: `readBMTracerInfo()`.

**Code at issue** — `attracerIO.c` line 235  
Search for: `ncattget(fid, NC_GLOBAL, "icenz", &n);`

```c
  233: 
  234: 	/* Check numbers of layers from attributes against geometry */
  235→     ncattget(fid, NC_GLOBAL, "icenz", &n);
  236:     if(n!= 0){
  237: 	//if(ncattinq(fid, NC_GLOBAL, "icenz", &t_type, &t_len) != -1){  //icenz is not a variable its a global dimension
```

**Code at issue** — `attracerIO.c` line 231  
Search for: `ncopts = 0`

```c
  229: 		quit("readBMTracerInfo: Number of layers in sediment doesn't match geometry\n");
  230: 
  231→ 	ncopts = 0;	// turn off verbose as well otherwise we get a warning about attribute not found if icenz not found - quite misleading as its fine if its not found it
  232: 				// just means ice is not active in this model.
  233: 
```

**Code at issue** — `attracerIO.c` line 481  
Search for: `boxes[b].ice.ntr`

```c
  479: 		bm->boxes[b].ntr = (int)n;
  480: 		bm->boxes[b].sm.ntr = (int)n;
  481→ 		bm->boxes[b].ice.ntr = nice;
  482: 	}
  483: 
```

**Suggested fix (from review):** Set `n = 0;` before the icenz ncattget, or use `ncattinq(...)!=-1` as in aticeIO.c:117

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

## PHY2-05

**accumulator not reset** · Conf H / Sev L  
**Found at (current code):** `attracerIO.c` 590, `attracerIO.c` 721
**Lines at review (Oct 2026):** 590, 682, 721

**Function:** `readBMTracerData()`

**Why it's thought to be a bug**

`found_partial_match_n` is set to 1 but never reset per tracer (only `found_partial_match` is reset at 658). Once one tracer matches `_N`, every later unmatched tracer triggers the "no string matches found" warning. Function: `readBMTracerData()`.

**Code at issue** — `attracerIO.c` line 590 (the line the review pointed to, found by its text)  
Search for: `int i, sp, found_partial_match = 0, found_partial_match_n = 0, this_sp;`

```c
  588:  *********************************************************************/
  589: void readBMTracerData(int fid, int dump, MSEBoxModel *bm) {
  590→ 	int i, sp, found_partial_match = 0, found_partial_match_n = 0, this_sp;
  591:     int found_sn, found_rn;
  592: 	doubleINPUT **val;
```

**Code at issue** — `attracerIO.c` line 721 (the line the review pointed to, found by its text)  
Search for: `} else if ((found_partial_match || found_partial_match_n) && (!found_sn && !found_rn)){`

```c
  719:                     }
  720: 				}
  721→ 			} else if ((found_partial_match || found_partial_match_n) && (!found_sn && !found_rn)){
  722: 				warn("run.prm file says scale initial numbers or biomasses, but no string matches found, no scaling performed for %s\n", bm->tinfo[i].name);
  723: 			}
```

**Suggested fix (from review):** Add `found_partial_match_n = 0;` next to line 658

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

## PHY2-06

**format specifier** · Conf H / Sev L  
**Found at (current code):** `attracerIO.c` 821
**Lines at review (Oct 2026):** 821

**Function:** `writeBMTracerData()`

**Why it's thought to be a bug**

`quit("... %d", ..., sizeof(doubleINPUT))` passes a size_t to `%d`. The same pattern is at atagetracerIO.c:243 and 517. Function: `writeBMTracerData()`.

**Code at issue** — `attracerIO.c` line 821  
Search for: `quit("`

```c
  819: 			/* Check double compatibility */
  820: 			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
  821→ 				quit("writeBMTracerData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, n, sizeof(doubleINPUT));
  822: 
  823: 			/* Get water column data from model storage */
```

**Suggested fix (from review):** Use `%zu` or cast to `(int)`

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

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

attracerIO.c `readBMTracerData()` matches species to tracers with `strstr(tracer, FunctGroupArray[sp].name)`, and the first substring hit wins. If one group name is a substring of another (e.g. "Cod" vs "RedCod"), the wrong `init_scalar` could be applied.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

attracerIO.c `readBMTracerInfo()`: `int d[3]` is filled by `ncvarinq`. A "tracer" variable with more than 3 dimensions would overflow it.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

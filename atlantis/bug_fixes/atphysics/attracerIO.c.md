# atphysics/attracerIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-04](#phy2-04) | 223-244 | H/M | stale variable | `ncattget(fid, NC_GLOBAL, "icenz", &n); if(n!= 0)` runs with `ncopts=0`. When the attribute is missing, ncattg… |  | [ ] |
| [PHY2-05](#phy2-05) | 590, 682, 721 | H/L | accumulator not reset | `found_partial_match_n` is set to 1 but never reset per tracer (only `found_partial_match` is reset at 658). O… |  | [ ] |
| [PHY2-06](#phy2-06) | 821 | H/L | format specifier | `quit("... %d", ..., sizeof(doubleINPUT))` passes a size_t to `%d`. The same pattern is at atagetracerIO.c:243… |  | [ ] |

---

## PHY2-04

**stale variable** · Conf H / Sev M  
**Lines (current code):** 223-244  
**Lines at review (Oct 2026):** 223-244

**Function:** `readBMTracerInfo()`

**Why it's thought to be a bug**

`ncattget(fid, NC_GLOBAL, "icenz", &n); if(n!= 0)` runs with `ncopts=0`. When the attribute is missing, ncattget fails without writing to `n`, so `n` keeps the `sednz` value read at line 227 (non-zero). So `ice_on` is set TRUE for every model that has no ice. Then every tracer goes through the `inice` read path, and `boxes[b].ice.ntr` is set to ntracer (nice++ for every tracer). In atlantismain.c:1267 this triggers `readBMIceInfo`, which only partly undoes it: it sets ice_on FALSE but `num_active_habitats = ICE_BASED` (4), not the `EPIFAUNA+1` (3) intended here. Function: `readBMTracerInfo()`.

**Current code** (`atphysics/attracerIO.c` lines 221–246)

```c
  221: 
  222: 	/* Check numbers of layers from attributes against geometry */
  223: 	ncattget(fid, NC_GLOBAL, "wcnz", &n);
  224: 	if (n != bm->wcnz)
  225: 		quit("readBMTracerInfo: Number of layers in water doesn't match geometry\n");
  226: 
  227: 	ncattget(fid, NC_GLOBAL, "sednz", &n);
  228: 	if (n != bm->sednz)
  229: 		quit("readBMTracerInfo: Number of layers in sediment doesn't match geometry\n");
  230: 
  231: 	ncopts = 0;	// turn off verbose as well otherwise we get a warning about attribute not found if icenz not found - quite misleading as its fine if its not found it
  232: 				// just means ice is not active in this model.
  233: 
  234: 	/* Check numbers of layers from attributes against geometry */
  235:     ncattget(fid, NC_GLOBAL, "icenz", &n);
  236:     if(n!= 0){
  237: 	//if(ncattinq(fid, NC_GLOBAL, "icenz", &t_type, &t_len) != -1){  //icenz is not a variable its a global dimension
  238: 		bm->ice_on = TRUE;
  239: 		bm->num_active_habitats = ICE_BASED + 1;
  240: 	} else {
  241: 		bm->ice_on = FALSE;
  242:         bm->num_active_habitats = EPIFAUNA + 1;  // TODO: Should this be LAND_BASED + 1 if terrestrial on? Likely not as treat land animals as epifauna
  243:         bm->icenz = 0;
  244: 	}
  245: 
  246: 
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
**Lines (current code):** 590, 682, 721  
**Lines at review (Oct 2026):** 590, 682, 721

**Function:** `readBMTracerData()`

**Why it's thought to be a bug**

`found_partial_match_n` is set to 1 but never reset per tracer (only `found_partial_match` is reset at 658). Once one tracer matches `_N`, every later unmatched tracer triggers the "no string matches found" warning. Function: `readBMTracerData()`.

**Current code** (`atphysics/attracerIO.c` lines 588–592)

```c
  588:  *********************************************************************/
  589: void readBMTracerData(int fid, int dump, MSEBoxModel *bm) {
  590: 	int i, sp, found_partial_match = 0, found_partial_match_n = 0, this_sp;
  591:     int found_sn, found_rn;
  592: 	doubleINPUT **val;
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
**Lines (current code):** 821  
**Lines at review (Oct 2026):** 821

**Function:** `writeBMTracerData()`

**Why it's thought to be a bug**

`quit("... %d", ..., sizeof(doubleINPUT))` passes a size_t to `%d`. The same pattern is at atagetracerIO.c:243 and 517. Function: `writeBMTracerData()`.

**Current code** (`atphysics/attracerIO.c` lines 819–823)

```c
  819: 			/* Check double compatibility */
  820: 			if ((n = ncvarsize(fid, vid)) != sizeof(doubleINPUT))
  821: 				quit("writeBMTracerData: %s has %d bytes per value in file, code compiled with %d\n", bm->tinfo[i].name, n, sizeof(doubleINPUT));
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

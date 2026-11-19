# ateconomic/atFishMIP_OSP_PluginAdapter.cpp — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-23](#ecio-23) | 221 | M/M | exception across C ABI | `FishMIP_Adapter_CalcPrice()`, `FishMIP_Adapter_CalcEffort()` and `FishMIP_Adapter_RunAssessment()` are `exter… |  | [ ] |
| [ECIO-24](#ecio-24) | 231 | L/H | unchecked buffer copy | `std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out)` assumes `price_ctry.size() == nsp`, but `pr… |  | [ ] |

---

## ECIO-23

**exception across C ABI** · Conf M / Sev M  
**Found at (current code):** `atFishMIP_OSP_PluginAdapter.cpp` 221
**Lines at review (Oct 2026):** 221-286

**Why it's thought to be a bug**

`FishMIP_Adapter_CalcPrice()`, `FishMIP_Adapter_CalcEffort()` and `FishMIP_Adapter_RunAssessment()` are `extern "C"` and called from C, but unlike the two loaders (l.100-128, 174-191) they have no try/catch. Any std::exception from the plug-in (bad_alloc, runtime_error, NdArray bounds) unwinding into C frames ends in std::terminate.

**Code at issue** — `atFishMIP_OSP_PluginAdapter.cpp` line 221 (the line the review pointed to, found by its text)  
Search for: `void FishMIP_Adapter_CalcPrice(int nsp, const double *prod, const double *X, const double `

```c
  219:  * Price / effort / assessment calls.
  220:  * ========================================================================== */
  221→ void FishMIP_Adapter_CalcPrice(int nsp, const double *prod, const double *X, const double *W, double epsilon,
  222:                                 double *price_out) {
  223:     const std::size_t n = static_cast<std::size_t>(nsp);
```

**Suggested fix (from review):** Wrap each body in try/catch and return a failure flag / leave outputs unchanged.

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

## ECIO-24

**unchecked buffer copy** · Conf L / Sev H  
**Found at (current code):** `atFishMIP_OSP_PluginAdapter.cpp` 231
**Lines at review (Oct 2026):** 231

**Function:** `FishMIP_Adapter_CalcPrice()`

**Why it's thought to be a bug**

`std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out)` assumes `price_ctry.size() == nsp`, but `price_out` is caller-allocated (bridge: K_num_tot_sp). If the plug-in returns a longer vector it overflows; a shorter one leaves stale values. Function: `FishMIP_Adapter_CalcPrice()`.

**Code at issue** — `atFishMIP_OSP_PluginAdapter.cpp` line 231  
Search for: `std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out)`

```c
  229:     PriceCalcResult pr =
  230:         plugin_model_1_calc_price(geo_agreg, nsp, nsp, nsp, region_idx, subregion_idx, prodV, Xv, Wv, epsilon, aqua);
  231→     std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out);
  232: }
  233: 
```

**Suggested fix (from review):** Copy `min(size, nsp)` elements.

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

atFishMIP_OSP_PluginAdapter.cpp:140-142: if `sim.xsize == 0`, `t` becomes -1 and is cast to size_t.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

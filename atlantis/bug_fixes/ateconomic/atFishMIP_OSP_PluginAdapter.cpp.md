# ateconomic/atFishMIP_OSP_PluginAdapter.cpp — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [ECIO-23](#ecio-23) | 221-286 | M/M | exception across C ABI | `FishMIP_Adapter_CalcPrice()`, `FishMIP_Adapter_CalcEffort()` and `FishMIP_Adapter_RunAssessment()` are `exter… |  | [ ] |
| [ECIO-24](#ecio-24) | 231 | L/H | unchecked buffer copy | `std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out)` assumes `price_ctry.size() == nsp`, but `pr… |  | [ ] |

---

## ECIO-23

**exception across C ABI** · Conf M / Sev M  
**Lines (current code):** 221-286  
**Lines at review (Oct 2026):** 221-286

**Why it's thought to be a bug**

`FishMIP_Adapter_CalcPrice()`, `FishMIP_Adapter_CalcEffort()` and `FishMIP_Adapter_RunAssessment()` are `extern "C"` and called from C, but unlike the two loaders (l.100-128, 174-191) they have no try/catch. Any std::exception from the plug-in (bad_alloc, runtime_error, NdArray bounds) unwinding into C frames ends in std::terminate.

**Current code** (`ateconomic/atFishMIP_OSP_PluginAdapter.cpp` lines 219–258 (truncated))

```c
  219:  * Price / effort / assessment calls.
  220:  * ========================================================================== */
  221: void FishMIP_Adapter_CalcPrice(int nsp, const double *prod, const double *X, const double *W, double epsilon,
  222:                                 double *price_out) {
  223:     const std::size_t n = static_cast<std::size_t>(nsp);
  224:     std::vector<double> geo_agreg(n, 0.0);
  225:     std::vector<double> prodV(prod, prod + n), Xv(X, X + n), Wv(W, W + n), aqua(n, 0.0);
  226:     std::vector<int> region_idx(n), subregion_idx(n);
  227:     for (std::size_t k = 0; k < n; ++k) region_idx[k] = subregion_idx[k] = static_cast<int>(k);
  228: 
  229:     PriceCalcResult pr =
  230:         plugin_model_1_calc_price(geo_agreg, nsp, nsp, nsp, region_idx, subregion_idx, prodV, Xv, Wv, epsilon, aqua);
  231:     std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out);
  232: }
  233: 
  234: void FishMIP_Adapter_CalcEffort(int nfishery, const double *price, const double *Y, const double *effo,
  235:                                  const double *q, const double *Ftarget, const double *comply, double risk,
  236:                                  const double *d_U, const double *d_V, double M, double dt, int effort_growth_lim,
  237:                                  double f_growth_rate_max, double f_growth_rate_half_sat, double *f_out) {
  238:     const std::size_t nf = static_cast<std::size_t>(nfishery);
  239:     auto toRow = [nf](const double *src) {
  240:         NdArray2 a({1, nf}, 0.0);
  241:         for (std::size_t i = 0; i < nf; ++i) a(0, i) = src[i];
  242:         return a;
  243:     };
  244:     NdArray2 priceA = toRow(price), Ya = toRow(Y), effoA = toRow(effo), qA = toRow(q), complyA = toRow(comply),
  245:              dUA = toRow(d_U), dVA = toRow(d_V);
  246:     std::vector<double> FtargetV(Ftarget, Ftarget + nf);
  247:     std::vector<double> Mv{M};  // nfleet_type collapsed to 1 -- see the header doc comment
  248: 
  249:     EffortCalcResult ec = plugin_model_2_calc_effort(1, nfishery, priceA, Ya, effoA, qA, FtargetV, complyA, risk, dUA,
  250:                                                       dVA, Mv, dt, effort_growth_lim, f_growth_rate_max,
  251:                                                       f_growth_rate_half_sat);
  252:     for (std::size_t i = 0; i < nf; ++i) f_out[i] = ec.f(0, i);
  253: }
  254: 
  255: void FishMIP_Adapter_RunAssessment(int speciesId, int nyears, int nfishery, const double *catchFlat,
  256:                                     const double *effortFlat, double prior_mean_r, double prior_sd_logr,
  257:                                     double prior_mean_z, double prior_sd_logz, int optimizer, int verbose,
  258:                                     FishMIP_AssessmentResult *out) {
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
**Lines (current code):** 231  
**Lines at review (Oct 2026):** 231

**Function:** `FishMIP_Adapter_CalcPrice()`

**Why it's thought to be a bug**

`std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out)` assumes `price_ctry.size() == nsp`, but `price_out` is caller-allocated (bridge: K_num_tot_sp). If the plug-in returns a longer vector it overflows; a shorter one leaves stale values. Function: `FishMIP_Adapter_CalcPrice()`.

**Current code** (`ateconomic/atFishMIP_OSP_PluginAdapter.cpp` lines 229–233)

```c
  229:     PriceCalcResult pr =
  230:         plugin_model_1_calc_price(geo_agreg, nsp, nsp, nsp, region_idx, subregion_idx, prodV, Xv, Wv, epsilon, aqua);
  231:     std::copy(pr.price_ctry.begin(), pr.price_ctry.end(), price_out);
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

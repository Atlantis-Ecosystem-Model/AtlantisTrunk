# atphysics/atbioturb.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-17](#phy3-17) ⭐ | 116 | H/H | operation repeated per tracer | `transfer_sed(sm,k,dst,thick)` moves the whole sediment slab (dz and *all* tracers, atvertgeom.c:398-417), but… |  | [ ] |
| [PHY3-18](#phy3-18) ⭐ | 98 | H/H | unit mismatch (s vs days) | `thick[k] = ... * bm->dt * 86400.0`: the comment assumes dt is in days, but `bm->dt` is in seconds (read via U… |  | [ ] |
| [PHY3-19](#phy3-19) | 116 | H/M | changes lost (wrong storage) | `transfer_sed` changes `sm->tr` (= `bm->sedtr[b]`, atlantismain.c:1191), not the `newsed` working copy passed … |  | [ ] |
| [PHY3-20](#phy3-20) | 98, 90, 115 | M/M | off-by-one / missing init | `thick[k]` is only set for `k = tk+1..nz-1`. The loop at 114 uses `thick[tk]` (always 0) for the `tk -> tk+1` … |  | [ ] |

---

## PHY3-17

**operation repeated per tracer** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atbioturb.c` 116
**Lines at review (Oct 2026):** 102-119,150-166,196-213

**Why it's thought to be a bug**

`transfer_sed(sm,k,dst,thick)` moves the whole sediment slab (dz and *all* tracers, atvertgeom.c:398-417), but it is called inside `for(n...)` over tracers. The bulk transfer therefore runs once for every qualifying particulate tracer, multiplying bioturbation by roughly the number of such tracers. Functions: `partic_diff()`, `expul()`, `mix_in()`.

**Code at issue** — `atbioturb.c` line 116 (closest match to the quoted code; wording differs slightly)  
Search for: `transfer_sed(sm,k,k+1,thick[k]);`

```c
  114: 		for(k=tk; k<sm->nz-1; k++){
  115: 			if( k<sm->nz )
  116→ 				transfer_sed(sm,k,k+1,thick[k]);
  117: 			if( k>tk )
  118: 				transfer_sed(sm,k,k-1,thick[k]);
```

**Suggested fix (from review):** Drop the tracer loop and call transfer_sed once per layer

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

## PHY3-18

**unit mismatch (s vs days)** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atbioturb.c` 98
**Lines at review (Oct 2026):** 98,143,189

**Why it's thought to be a bug**

`thick[k] = ... * bm->dt * 86400.0`: the comment assumes dt is in days, but `bm->dt` is in seconds (read via Util_XML_Get_Value_Time, atparamIO.c:178; bioirrig uses `bm->dt` directly). Transfer thicknesses are 86400x too large. Functions: `partic_diff()`, `expul()`, `mix_in()`.

**Code at issue** — `atbioturb.c` line 98  
Search for: `thick[k] =`

```c
   96: 	thus to get it to m must multiply by s m-1.
   97: 	*/
   98→ 		thick[k] = sm->partic_kz[k] * bm->dt * 86400.0 / sm->dz[k];
   99: 	}
  100: 
```

**Suggested fix (from review):** Remove `* 86400.0`

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

## PHY3-19

**changes lost (wrong storage)** · Conf H / Sev M  
**Found at (current code):** `atbioturb.c` 116
**Lines at review (Oct 2026):** 59-65 (all routines)

**Function:** `bioturb()`

**Why it's thought to be a bug**

`transfer_sed` changes `sm->tr` (= `bm->sedtr[b]`, atlantismain.c:1333), not the `newsed` working copy passed in. atlantismain.c:697 then copies `newsedtr` back over `sedtr`, so bioturbation's tracer redistribution is thrown away while the layer-thickness changes persist. Function: `bioturb()`.

**Code at issue** — `atbioturb.c` line 116 (located by name only; check this is the right place)  
Search for: `transfer_sed(sm,k,k+1,thick[k]);`

```c
  114: 		for(k=tk; k<sm->nz-1; k++){
  115: 			if( k<sm->nz )
  116→ 				transfer_sed(sm,k,k+1,thick[k]);
  117: 			if( k>tk )
  118: 				transfer_sed(sm,k,k-1,thick[k]);
```

**Suggested fix (from review):** Work on newsed (or sync newsed into sm->tr before and back after)

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

## PHY3-20

**off-by-one / missing init** · Conf M / Sev M  
**Found at (current code):** `atbioturb.c` 98, `atbioturb.c` 90, `atbioturb.c` 115
**Lines at review (Oct 2026):** 90-118

**Function:** `partic_diff()`

**Why it's thought to be a bug**

`thick[k]` is only set for `k = tk+1..nz-1`. The loop at 114 uses `thick[tk]` (always 0) for the `tk -> tk+1` transfer, so the top layer never diffuses downward but does receive from below, giving net upward pumping. `if(k<sm->nz)` at 115 is always true. Function: `partic_diff()`.

**Code at issue** — `atbioturb.c` line 98  
Search for: `thick[k]`

```c
   96: 	thus to get it to m must multiply by s m-1.
   97: 	*/
   98→ 		thick[k] = sm->partic_kz[k] * bm->dt * 86400.0 / sm->dz[k];
   99: 	}
  100: 
```

**Code at issue** — `atbioturb.c` line 90 (closest match to the quoted code; wording differs slightly)  
Search for: `for(k=tk+1; k<sm->nz; k++) {`

```c
   88: 
   89: 	/* Calculate the amount to 'diffuse' (move) between layers */
   90→ 	for(k=tk+1; k<sm->nz; k++) {
   91: 		sm->partic_kz[k] = bm->bt_partic_kz * sm->biodens * sm->turbenh
   92: 			* bioprofile(bm,sm->gridz[k],sm->biodepth);
```

**Code at issue** — `atbioturb.c` line 115  
Search for: `if( k<sm->nz )`

```c
  113: 	small amounts being moved between layers in multi-layered cases. */
  114: 		for(k=tk; k<sm->nz-1; k++){
  115→ 			if( k<sm->nz )
  116: 				transfer_sed(sm,k,k+1,thick[k]);
  117: 			if( k>tk )
```

**Suggested fix (from review):** Compute thick[tk] too, or use the interface value thick[k+1] for k->k+1

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

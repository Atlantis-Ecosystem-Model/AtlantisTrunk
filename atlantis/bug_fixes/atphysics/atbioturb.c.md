# atphysics/atbioturb.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 2; on the priority list: 2).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-17](#phy3-17) ⭐ | 102-119,150-166,196-213 | H/H | operation repeated per tracer | `transfer_sed(sm,k,dst,thick)` moves the whole sediment slab (dz and *all* tracers, atvertgeom.c:398-417), but… |  | [ ] |
| [PHY3-18](#phy3-18) ⭐ | 98,143,189 | H/H | unit mismatch (s vs days) | `thick[k] = ... * bm->dt * 86400.0`: the comment assumes dt is in days, but `bm->dt` is in seconds (read via U… |  | [ ] |
| [PHY3-19](#phy3-19) | 59-65 (all routines) | H/M | changes lost (wrong storage) | `transfer_sed` changes `sm->tr` (= `bm->sedtr[b]`, atlantismain.c:1191), not the `newsed` working copy passed … |  | [ ] |
| [PHY3-20](#phy3-20) | 90-118 | M/M | off-by-one / missing init | `thick[k]` is only set for `k = tk+1..nz-1`. The loop at 114 uses `thick[tk]` (always 0) for the `tk -> tk+1` … |  | [ ] |

---

## PHY3-17

**operation repeated per tracer** · Conf H / Sev H ⭐ priority  
**Listed lines:** 102-119,150-166,196-213

**Why it's thought to be a bug**

`transfer_sed(sm,k,dst,thick)` moves the whole sediment slab (dz and *all* tracers, atvertgeom.c:398-417), but it is called inside `for(n...)` over tracers. The bulk transfer therefore runs once for every qualifying particulate tracer, multiplying bioturbation by roughly the number of such tracers. Functions: `partic_diff()`, `expul()`, `mix_in()`.

**Current code** (`atphysics/atbioturb.c` lines 114–135) — code appears to have moved +14 lines since the review

```c
  114: 		for(k=tk; k<sm->nz-1; k++){
  115: 			if( k<sm->nz )
  116: 				transfer_sed(sm,k,k+1,thick[k]);
  117: 			if( k>tk )
  118: 				transfer_sed(sm,k,k-1,thick[k]);
  119: 		}
  120: 	}
  121: 
  122:     free1d(thick);
  123: 
  124: }
  125: 
  126: void expul(MSEBoxModel *bm, Box *bp, SedModel *sm, double **newwc, double **newsed)
  127: {
  128: 	int k = 0;
  129: 	int n = 0;
  130: 	int tk = sm->topk;			        /* Index of top sediment layer */
  131: 	double *thick = alloc1d(sm->nz);		/* Temporary storage - thickness (amount) of
  132: 											sed to transfer*/
  133: 
  134: 	/* Nothing to do if less than 2 layers */
  135: 	if( tk >= sm->nz-1){
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
**Listed lines:** 98,143,189

**Why it's thought to be a bug**

`thick[k] = ... * bm->dt * 86400.0`: the comment assumes dt is in days, but `bm->dt` is in seconds (read via Util_XML_Get_Value_Time, atparamIO.c:178; bioirrig uses `bm->dt` directly). Transfer thicknesses are 86400x too large. Functions: `partic_diff()`, `expul()`, `mix_in()`.

**Current code** (`atphysics/atbioturb.c` lines 96–100)

```c
   96: 	thus to get it to m must multiply by s m-1.
   97: 	*/
   98: 		thick[k] = sm->partic_kz[k] * bm->dt * 86400.0 / sm->dz[k];
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
**Listed lines:** 59-65 (all routines)

**Function:** `bioturb()`

**Why it's thought to be a bug**

`transfer_sed` changes `sm->tr` (= `bm->sedtr[b]`, atlantismain.c:1191), not the `newsed` working copy passed in. atlantismain.c:575 then copies `newsedtr` back over `sedtr`, so bioturbation's tracer redistribution is thrown away while the layer-thickness changes persist. Function: `bioturb()`.

**Current code** (`atphysics/atbioturb.c` lines 57–67)

```c
   57: 	if( bp->type != BOUNDARY && bp->type != LAND) {
   58: 	    /* Diffusion - based on diffusion in bioirrigation */
   59: 	    partic_diff(bm,bp,sm,newwc[b],newsed[b]);
   60: 
   61: 	    /* Expulsion */
   62:         expul(bm,bp,sm,newwc[b],newsed[b]);
   63: 
   64: 	    /* Expulsion and intake */
   65:         mix_in(bm,bp,sm,newwc[b],newsed[b]);
   66: 	}
   67:     }
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
**Listed lines:** 90-118

**Function:** `partic_diff()`

**Why it's thought to be a bug**

`thick[k]` is only set for `k = tk+1..nz-1`. The loop at 114 uses `thick[tk]` (always 0) for the `tk -> tk+1` transfer, so the top layer never diffuses downward but does receive from below, giving net upward pumping. `if(k<sm->nz)` at 115 is always true. Function: `partic_diff()`.

**Current code** (`atphysics/atbioturb.c` lines 91–123) — code appears to have moved +3 lines since the review

```c
   91: 		sm->partic_kz[k] = bm->bt_partic_kz * sm->biodens * sm->turbenh
   92: 			* bioprofile(bm,sm->gridz[k],sm->biodepth);
   93: 	/* To convert this rate to a thickness multiply by timestep (as then
   94: 	converted to seconds ensure that timestep is always in days) and divide
   95: 	by cell thickness. This is because the diffusion coefficient is in m2 s-1
   96: 	thus to get it to m must multiply by s m-1.
   97: 	*/
   98: 		thick[k] = sm->partic_kz[k] * bm->dt * 86400.0 / sm->dz[k];
   99: 	}
  100: 
  101: 	/* Loop over tracers */
  102: 	for(n=0; n<bm->ntracer; n++) {
  103: 
  104: 	/* Skip tracers which are dissolved or not allowed in the sediment or
  105: 	macrobenthos (indicated by particle size).
  106: 	*/
  107: 		if( !(bm->tinfo[n].partic) || !(bm->tinfo[n].insed) || (bm->tinfo[n].psize >= 1.0) || !bm->tinfo[n].can_be_moved)
  108: 			continue;
  109: 
  110: 	/* Do diffusion - using simplification of simply moving amounts in
  111: 	the following way: x-1 <-> x <-> x+1 (where <-> means movement goes both ways
  112: 	to and from x etc.). This is a simplification, but its OK wrt to the
  113: 	small amounts being moved between layers in multi-layered cases. */
  114: 		for(k=tk; k<sm->nz-1; k++){
  115: 			if( k<sm->nz )
  116: 				transfer_sed(sm,k,k+1,thick[k]);
  117: 			if( k>tk )
  118: 				transfer_sed(sm,k,k-1,thick[k]);
  119: 		}
  120: 	}
  121: 
  122:     free1d(thick);
  123: 
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

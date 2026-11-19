# atphysics/atsuspension.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-21](#phy3-21) ⭐ | 153-156 | H/H | missing braces | `else if (insed && dissol && can_be_moved)` has no braces, so line 156 `newsed[b][tk][n] -= newsed[b][tk][n]*p… |  | [ ] |
| [PHY3-22](#phy3-22) | 146,156,164-165 | M/H | erosion counted twice | Sediment tracers are per volume (summary output multiplies by sm.dz, atsummaryIO.c:346). After moving `c*ervol… |  | [ ] |
| [PHY3-23](#phy3-23) | 66,100 | M/M | division by zero | `Shipping_stress_ratio = stress / init_stress`, and `init_stress` is set to `nxsbs` each step. If the stress w… |  | [ ] |
| [PHY3-24](#phy3-24) | 46,108-120 | M/M | logic: loop variable not set | `nxsbs` is only set when `dynamic_stress` is on, but `erosion_rate` uses the stored `boxes[b].stress`. With `d… |  | [ ] |

---

## PHY3-21

**missing braces** · Conf H / Sev H ⭐ priority  
**Lines (current code):** 153-156  
**Lines at review (Oct 2026):** 153-156

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

`else if (insed && dissol && can_be_moved)` has no braces, so line 156 `newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(...)` runs for **every** tracer. Particulates are depleted twice, and non-sediment and non-movable tracers lose mass that goes nowhere. Function: `resuspendBM()`.

**Current code** (`atphysics/atsuspension.c` lines 151–158)

```c
  151:             */
  152: 
  153: 		} else if( bm->tinfo[n].insed && bm->tinfo[n].dissol && bm->tinfo[n].can_be_moved)
  154: 		    /* Pore water */
  155: 		    newwc[b][0][n] = (newwc[b][0][n]*bp->volume[0] + newsed[b][tk][n]*pwv)/newwcvol;
  156:             newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(sm->dz[tk]*bp->area);  // Adjust the sediment values accordingly
  157: 	    }
  158: 
```

**Suggested fix (from review):** Wrap lines 155-156 in braces

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

## PHY3-22

**erosion counted twice** · Conf M / Sev H  
**Lines (current code):** 146,156,164-165  
**Lines at review (Oct 2026):** 146,156,164-165

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

Sediment tracers are per volume (summary output multiplies by sm.dz, atsummaryIO.c:346). After moving `c*ervol` to the water column, the code lowers the concentration (`c *= 1-erdz/dz`) **and** cuts the layer thickness by `erdz`. The remaining mass is `c(1-erdz/dz)(dz-erdz)A`, not `c(dz-erdz)A`, so mass is lost. Function: `resuspendBM()`.

**Current code** (`atphysics/atsuspension.c` lines 144–167)

```c
  144:             //oldval = newwc[b][0][n];
  145: 		    newwc[b][0][n] = (newwc[b][0][n]*bp->volume[0] + newsed[b][tk][n]*ervol)/newwcvol;
  146:             newsed[b][tk][n] -= newsed[b][tk][n]*ervol/(sm->dz[tk]*bp->area);  // Adjust the sediment values accordingly
  147:             
  148:             /*
  149:             if (bp->n == bm->checkbox)
  150:                 fprintf(llogfp,"Time: %e box%d newwc: %e oldval: %.20e volwc: %e, newsed: %e ervol: %e newwcvol: %e\n", bm->dayt, n, newwc[b][0][n], oldval, bp->volume[0], newsed[b][tk][n], ervol, newwcvol);
  151:             */
  152: 
  153: 		} else if( bm->tinfo[n].insed && bm->tinfo[n].dissol && bm->tinfo[n].can_be_moved)
  154: 		    /* Pore water */
  155: 		    newwc[b][0][n] = (newwc[b][0][n]*bp->volume[0] + newsed[b][tk][n]*pwv)/newwcvol;
  156:             newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(sm->dz[tk]*bp->area);  // Adjust the sediment values accordingly
  157: 	    }
  158: 
  159: 	    /* Adjust water column volume */
  160: 	    bp->volume[0] += pwv;
  161: 	    bp->dz[0] = bp->volume[0]/bp->area;
  162: 
  163: 	    /* Adjust sediment thickness */
  164: 	    sm->dz[tk] -= erdz;
  165: 	    sm->volume[tk] = sm->dz[tk]*bp->area;
  166: 
  167:         if(bm->maintain_sedlayer) {
```

**Suggested fix (from review):** Drop the concentration adjustment (thickness change already accounts for it)

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

## PHY3-23

**division by zero** · Conf M / Sev M  
**Lines (current code):** 66,100  
**Lines at review (Oct 2026):** 66,100

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

`Shipping_stress_ratio = stress / init_stress`, and `init_stress` is set to `nxsbs` each step. If the stress was 0 in the previous step (stress file value 0, or `soft==1`) this gives inf/NaN. Function: `resuspendBM()`.

**Current code** (`atphysics/atsuspension.c` lines 64–102)

```c
   64:         Shipping_stress_ratio = 1.0;
   65:         if(bm->containsShipping) {
   66:             Shipping_stress_ratio = bm->boxes[b].stress / bm->boxes[b].init_stress;
   67:         }
   68:         
   69: 		/* Read the normalised excess bottom stress value.
   70: 		 * Here we assume that this represents the value
   71: 		 * appropriately integrated over the time step, and including
   72: 		 * wave effects. These calculations do not depend on the transport
   73: 		 * model as such (except for time varying critical shear stresses,
   74: 		 * making the integral over long time steps problematic, and,
   75: 		 * perhaps, for time varying bottom roughness?).
   76: 		 * For efficiency, they should be done by the Interface
   77: 		 * between the Hydrodynamic model and the Transport model.
   78: 		 * The calculations are described in Grant and Madsen 1986 
   79: 		 * Ann. Rev. Fluid Mech. 18 265-305. An implementation
   80: 		 * for a simple 1 column model was part of the work that
   81: 		 * Reg Uncles did during his visit to 
   82: 		 * CSIRO Division of Oceanography in 1994.
   83: 		 */
   84:         
   85:         if (bm->ts_on_hydro_time)
   86:             ask_t = bm->hd.t;
   87:         else
   88:             ask_t = bm->t;
   89: 
   90:         if(bm->use_stressfiles)
   91:             nxsbs = tsEvalXY(bm->stress,bm->stress_id,ask_t,bp->inside.x,bp->inside.y);
   92:         else {
   93:             /* Using from Mitchener and Torfs, 1996 relationship */
   94:             step1 = (1.0 - bm->boxes[b].soft) * 300.0;
   95:             nxsbs = 0.0012 * pow(step1, 1.2);
   96:         }
   97:         
   98: 		/*Store new stress values*/
   99: 		bm->boxes[b].stress = nxsbs * Shipping_stress_ratio;  // So don't loose any recent shipping movement effects
  100:         bm->boxes[b].init_stress = nxsbs;
  101: 
  102:         /*
```

**Suggested fix (from review):** Guard: ratio=1 if init_stress<=0

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

## PHY3-24

**logic: loop variable not set** · Conf M / Sev M  
**Lines (current code):** 46,108-120  
**Lines at review (Oct 2026):** 46,108-120

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

`nxsbs` is only set when `dynamic_stress` is on, but `erosion_rate` uses the stored `boxes[b].stress`. With `dynamic_stress==0` the loop `while(nxsbs > 0.0 ...)` never runs, so there is no resuspension at all, even with valid stress values. Function: `resuspendBM()`.

**Current code** (`atphysics/atsuspension.c` lines 44–48)

```c
   44: 	SedModel *sm = &bm->boxes[b].sm;
   45: 	double tleft = bm->dt;
   46: 	double nxsbs = 0;
   47: 	double er_limited;
   48: 	//oldval;
```

**Suggested fix (from review):** Loop on `er_limited > 0` (or set nxsbs = stress)

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

atdeposition.c:175-180 and atsuspension.c:167-172: the `maintain_sedlayer` clamp changes `sm->dz[tk]` without updating `sm->volume[tk]`, so dz and volume disagree, and material above maxdz is dropped from the thickness.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

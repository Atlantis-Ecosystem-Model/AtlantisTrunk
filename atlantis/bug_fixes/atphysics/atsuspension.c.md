# atphysics/atsuspension.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **4 findings** (severity H: 2; confidence H and severity H: 1; on the priority list: 1), plus 1 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY3-21](#phy3-21) ⭐ | 153, 156 | H/H | missing braces | `else if (insed && dissol && can_be_moved)` has no braces, so line 156 `newsed[b][tk][n] -= newsed[b][tk][n]*p… |  | [ ] |
| [PHY3-22](#phy3-22) | 128 | M/H | erosion counted twice | Sediment tracers are per volume (summary output multiplies by sm.dz, atsummaryIO.c:346). After moving `c*ervol… |  | [ ] |
| [PHY3-23](#phy3-23) | 66 | M/M | division by zero | `Shipping_stress_ratio = stress / init_stress`, and `init_stress` is set to `nxsbs` each step. If the stress w… |  | [ ] |
| [PHY3-24](#phy3-24) | 66, 120 | M/M | logic: loop variable not set | `nxsbs` is only set when `dynamic_stress` is on, but `erosion_rate` uses the stored `boxes[b].stress`. With `d… |  | [ ] |

---

## PHY3-21

**missing braces** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atsuspension.c` 153, `atsuspension.c` 156
**Lines at review (Oct 2026):** 153-156

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

`else if (insed && dissol && can_be_moved)` has no braces, so line 156 `newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(...)` runs for **every** tracer. Particulates are depleted twice, and non-sediment and non-movable tracers lose mass that goes nowhere. Function: `resuspendBM()`.

**Code at issue** — `atsuspension.c` line 153 (closest match to the quoted code; wording differs slightly)  
Search for: `} else if( bm->tinfo[n].insed && bm->tinfo[n].dissol && bm->tinfo[n].can_be_moved)`

```c
  151:             */
  152: 
  153→ 		} else if( bm->tinfo[n].insed && bm->tinfo[n].dissol && bm->tinfo[n].can_be_moved)
  154: 		    /* Pore water */
  155: 		    newwc[b][0][n] = (newwc[b][0][n]*bp->volume[0] + newsed[b][tk][n]*pwv)/newwcvol;
```

**Code at issue** — `atsuspension.c` line 156  
Search for: `newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(`

```c
  154: 		    /* Pore water */
  155: 		    newwc[b][0][n] = (newwc[b][0][n]*bp->volume[0] + newsed[b][tk][n]*pwv)/newwcvol;
  156→             newsed[b][tk][n] -= newsed[b][tk][n]*pwv/(sm->dz[tk]*bp->area);  // Adjust the sediment values accordingly
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
**Found at (current code):** `atsuspension.c` 128
**Lines at review (Oct 2026):** 146,156,164-165

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

Sediment tracers are per volume (summary output multiplies by sm.dz, atsummaryIO.c:346). After moving `c*ervol` to the water column, the code lowers the concentration (`c *= 1-erdz/dz`) **and** cuts the layer thickness by `erdz`. The remaining mass is `c(1-erdz/dz)(dz-erdz)A`, not `c(dz-erdz)A`, so mass is lost. Function: `resuspendBM()`.

**Code at issue** — `atsuspension.c` line 128 (closest match to the quoted code; wording differs slightly)  
Search for: `double erdz = min(er*tleft, sm->dz[tk]);`

```c
  126: 	    double er = er_limited;
  127: 	    /* Amount to erode from this layer */
  128→ 	    double erdz = min(er*tleft, sm->dz[tk]);
  129: 	    /* Volume eroded */
  130: 	    double ervol = erdz*bp->area;
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
**Found at (current code):** `atsuspension.c` 66
**Lines at review (Oct 2026):** 66,100

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

`Shipping_stress_ratio = stress / init_stress`, and `init_stress` is set to `nxsbs` each step. If the stress was 0 in the previous step (stress file value 0, or `soft==1`) this gives inf/NaN. Function: `resuspendBM()`.

**Code at issue** — `atsuspension.c` line 66 (closest match to the quoted code; wording differs slightly)  
Search for: `Shipping_stress_ratio = bm->boxes[b].stress / bm->boxes[b].init_stress;`

```c
   64:         Shipping_stress_ratio = 1.0;
   65:         if(bm->containsShipping) {
   66→             Shipping_stress_ratio = bm->boxes[b].stress / bm->boxes[b].init_stress;
   67:         }
   68:         
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
**Found at (current code):** `atsuspension.c` 66, `atsuspension.c` 120
**Lines at review (Oct 2026):** 46,108-120

**Function:** `resuspendBM()`

**Why it's thought to be a bug**

`nxsbs` is only set when `dynamic_stress` is on, but `erosion_rate` uses the stored `boxes[b].stress`. With `dynamic_stress==0` the loop `while(nxsbs > 0.0 ...)` never runs, so there is no resuspension at all, even with valid stress values. Function: `resuspendBM()`.

**Code at issue** — `atsuspension.c` line 66  
Search for: `boxes[b].stress`

```c
   64:         Shipping_stress_ratio = 1.0;
   65:         if(bm->containsShipping) {
   66→             Shipping_stress_ratio = bm->boxes[b].stress / bm->boxes[b].init_stress;
   67:         }
   68:         
```

**Code at issue** — `atsuspension.c` line 120  
Search for: `while( nxsbs > 0.0`

```c
  118: 
  119: 	/* Loop while erosion will occur and more time remains for this step */
  120→ 	while( nxsbs > 0.0 && tleft > 0.0 ) {
  121: 	    int n;
  122: 	    /* Get sediment surface index */
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

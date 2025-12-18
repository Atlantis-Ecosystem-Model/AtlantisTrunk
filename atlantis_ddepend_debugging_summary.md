# Atlantis Density-Dependent Movement (ddepend_move) Debugging Summary

## Overview

This document summarizes the debugging process for the density-dependent movement system in Atlantis, specifically when `ddepend_move=2` is enabled for cod (FCD). The investigation revealed multiple bugs causing fish to accumulate inappropriately in shallow single-layer boxes (Box 20, Box 31) and experience excessive mortality.

## Initial Problem

When `ddepend_move` was enabled:
- Adult cod accumulated massively in shallow single-layer boxes (50m depth)
- Total cod abundance declined ~6x faster than with `ddepend_move` off
- By day 6, Box 31 contained 234M fish (64% of entire population) vs 14.5M with ddepend off
- Fish were compressed into single layers in multi-layer boxes

## Bug #1: Incorrect Box Width in Movement Rate Limiter

### Location
`atmovement.c` - movement rate limiter calculation

### Original Code
```c
newden[sp][n][k][ij] = min(1.0,(spSpeed * dt / bm->width)) * (newden[sp][n][k][ij] - currentden[sp][n][k][ij])
                        + currentden[sp][n][k][ij];
```

### Problem
`bm->width` is the **entire model domain width** (calculated in `atgeomIO.c`), not the width of individual boxes. This meant the movement rate limiter was using the wrong spatial scale, allowing unrealistic movement rates.

### Fix
Created a new function `Compute_Box_Widths()` to calculate individual box widths and updated the calculation:

```c
/* JMK: width-from-center = max distance from inside point to any polygon vertex (radius).
   Stores the radius in bm->box_width[i] and prints both radius and diameter. */
static void Compute_Box_Widths(MSEBoxModel *bm)
{
    if (!bm || bm->nbox <= 0) return;
    if (bm->box_width) { free(bm->box_width); bm->box_width = NULL; }
    bm->box_width = (double*)malloc((size_t)bm->nbox * sizeof(double));
    if (!bm->box_width) {
        quit("Compute_Box_Widths: no memory for box_width (%d)\n", bm->nbox);
    }
    for (int i = 0; i < bm->nbox; i++) {
        polyline *poly = bm->boxes[i].bnd;
        if (!poly || poly->np <= 0 || !poly->start) {
            bm->box_width[i] = 0.0;  /* no geometry */
            continue;
        }
        /* center (inside point) */
        const double cx = bm->boxes[i].inside.x;
        const double cy = bm->boxes[i].inside.y;
        /* find max distance from center to any vertex */
        double max_r = 0.0;
        int    max_v = -1;
        int    v     = 0;
        for (linepoint *lp = poly->start; lp != NULL; lp = lp->next, v++) {
            const double dx = lp->p.x - cx;
            const double dy = lp->p.y - cy;
            const double r  = sqrt(dx*dx + dy*dy);
            if (r > max_r) { max_r = r; max_v = v; }
        }
        /* store radius in box_width (distance from center to farthest boundary point) */
        bm->box_width[i] = max_r;
    }
}
```

Updated movement calculation:
```c
newden[sp][n][k][ij] = min(1.0,(spSpeed * dt / bm->box_width[ij])) * (newden[sp][n][k][ij] - currentden[sp][n][k][ij])
                        + currentden[sp][n][k][ij]; /* JMK box_width[ij] */
```

---

## Bug #2: Hardcoded `current_box` in Depth Check

### Location
`atmovement.c` - ROC (rate of change) calculation for habitat suitability

### Original Code
```c
if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) && (int) (FunctGroupArray[sp].speciesParams[active_id]) &&
        ( bm->boxes[ij].botz <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
         bm->boxes[ij].botz >= (-1.0 * FunctGroupArray[sp].speciesParams[maxdepth_id]) &&
        ((-1.0 * bm->boxes[bm->current_box].botz) <= FunctGroupArray[sp].speciesParams[maxtotdepth_id]))) {
    roc[ij][n] = E1_sp * clear;
} else {
    roc[ij][n] = 0.0;
}
```

### Problem
`bm->current_box` was initialized to 0 in `atlantismain.c` and **never updated**. This meant the third depth check always evaluated based on Box 0's depth (1250m) instead of the box being evaluated (`ij`).

For cod with `maxtotdepth=600m`:
- Check: `1250 <= 600` → **FALSE**
- Result: `roc = 0` for ALL boxes, even those within cod's depth range

### Diagnostic Output (Before Fix)
```
JMK roc ZERO sp=0 ij=14 n=0 - depth check failed: botz=-300 mindepth=0 maxdepth=600 current_box=0 current_botz=-1250 maxtotdepth=600
JMK roc ZERO sp=0 ij=20 n=0 - depth check failed: botz=-50 mindepth=0 maxdepth=600 current_box=0 current_botz=-1250 maxtotdepth=600
```

### Fix
Changed `bm->current_box` to `ij` in the depth check:

```c
if ((int) (FunctGroupArray[sp].speciesParams[flag_id]) && (int) (FunctGroupArray[sp].speciesParams[active_id]) &&
        ( bm->boxes[ij].botz <= (-1.0 * FunctGroupArray[sp].speciesParams[mindepth_id]) &&
         bm->boxes[ij].botz >= (-1.0 * FunctGroupArray[sp].speciesParams[maxdepth_id]) &&
        ((-1.0 * bm->boxes[ij].botz) <= FunctGroupArray[sp].speciesParams[maxtotdepth_id]))) { /* JMK bug fix: changed current_box to ij */
    roc[ij][n] = E1_sp * clear;
} else {
    roc[ij][n] = 0.0;
}
```

### Diagnostic Output (After Fix)
```
JMK roc SET sp=0 ij=14 n=0 roc=5.3 (E1_sp=0.8 * clear=6.625)
JMK roc SET sp=0 ij=20 n=0 roc=5.3 (E1_sp=0.8 * clear=6.625)
JMK roc ZERO sp=0 ij=4 n=0 - depth check failed: botz=-1250 mindepth=0 maxdepth=600 ...
JMK roc ZERO sp=0 ij=9 n=0 - depth check failed: botz=-800 mindepth=0 maxdepth=600 ...
```

Now boxes within cod's depth range (0-600m) get `roc > 0`, while deep boxes correctly get `roc = 0`.

---

## Ongoing Investigation: spawnmove × roc/totroc Formula

### Current Behavior

After fixing bugs #1 and #2, fish are no longer accumulating in shallow boxes, but total abundance is still declining faster with `ddepend_move` on (~2.7M/day) vs off (~0.4M/day).

### The newden Calculation

The density-dependent movement calculates new density as:

```c
newden[sp][n][k][ij] = spawnmove * roc[ij][n] / (totroc[n] + small_num);
if(!totroc[n]) {
    newden[sp][n][k][ij] = currentden[sp][n][k][ij];
}
```

Where:
- `spawnmove` = interpolated prescribed seasonal distribution (sums to 1.0 across all boxes)
- `roc[ij][n]` = habitat suitability for box ij
- `totroc[n]` = sum of roc across all boxes
- `roc/totroc` = fraction of total habitat suitability (sums to 1.0 across all boxes)

### The Problem

Multiplying `spawnmove × roc/totroc` produces values that **do not sum to 1.0**:

**Example for Box 14:**
- `spawnmove = 0.0564` (5.64% of fish prescribed here)
- `roc/totroc = 0.0435` (4.35% of habitat suitability)
- `newden = 0.0564 × 0.0435 = 0.00245`

This is much smaller than `currentden = 0.0564`, causing fish to "disappear".

### Diagnostic Evidence

```
JMK MOVE_BEFORE sp=0 n=5 ij=14 k=0 newden=2.453478e-03 currentden=5.643000e-02 ...
```

The target `newden` (0.00245) is only 4.3% of `currentden` (0.0564).

With `move_frac = 1.0` (fish can move 100% toward target each timestep), fish rapidly drain away:

```
currentden progression: 0.0564 → 0.0212 → 0.0160 → declining...
```

### What spawnmove Represents

```c
spawnmove = this_HowFar * (FunctGroupArray[sp].distrib[ij][next_qrt][stage] - FunctGroupArray[sp].distrib[ij][qrt][stage]) + FunctGroupArray[sp].distrib[ij][qrt][stage];
```

This is **linear interpolation** between seasonal prescribed distributions:
- `distrib[qrt]` = prescribed distribution for current quarter
- `distrib[next_qrt]` = prescribed distribution for next quarter
- `this_HowFar` = fraction of progress through quarter (0 to 1)

Sum of `spawnmove` across all boxes = 1.0 (it's a probability distribution).

### Potential Issue

The formula `spawnmove × roc/totroc` combines two independent probability distributions by multiplication, which:
1. Does not preserve total fish (sum ≠ 1.0)
2. Double-weights boxes (once by prescribed distribution, once by habitat suitability)

### Possible Fixes to Investigate

1. **Use only roc/totroc** (pure density-dependent):
   ```c
   newden = roc[ij][n] / (totroc[n] + small_num);
   ```

2. **Use only spawnmove** (pure prescribed):
   ```c
   newden = spawnmove;
   ```

3. **Weighted average** (blend both):
   ```c
   newden = weight * spawnmove + (1-weight) * roc[ij][n] / (totroc[n] + small_num);
   ```

4. **Review original intent** of the formula - may need to check Atlantis documentation or consult with original developers.

---

## Key Variables Reference

| Variable | Description |
|----------|-------------|
| `sp` | Species index (0 = FCD/cod) |
| `n` | Age class (0-15 for cod) |
| `k` | Vertical layer within box |
| `ij` | Box index |
| `stage` | Life stage (0=juvenile, 1=adult) |
| `sp_ddepend_move` | Density-dependent movement type (2 = only_ddepend) |
| `roc[ij][n]` | Rate of change / habitat suitability for box |
| `totroc[n]` | Sum of roc across all boxes |
| `spawnmove` | Interpolated prescribed seasonal distribution |
| `vertdistrib` | Vertical distribution preference for layer |
| `currentden` | Current density (where fish are) |
| `newden` | Target density (where fish should move to) |

## Switch Statement Values

| Constant | Value | Description |
|----------|-------|-------------|
| `sedentary_move` | 0 | No movement |
| `perscribed_move` | 1 | Follow prescribed distribution only |
| `only_ddepend` | 2 | Density-dependent only |
| `weight_ddepend` | 3 | Weighted density-dependent |
| `switch_ddepend` | 5 | Switch between prescribed and density-dependent |

---

## Files Modified

1. **atmovement.c**
   - Added `Compute_Box_Widths()` function
   - Fixed `bm->width` → `bm->box_width[ij]` in movement limiter
   - Fixed `bm->current_box` → `ij` in depth check

2. **atlantisboxmodel.h** (potentially)
   - Added `box_width` array to MSEBoxModel structure

---

## Diagnostic Commands Used

```bash
# Check roc values
grep "JMK roc SET" log.txt | head -20
grep "JMK roc ZERO" log.txt | head -20

# Check newden progression
grep "MOVE_BEFORE" log.txt | grep "ij=14" | grep "n=5" | head -10
grep "MOVE_AFTER" log.txt | grep "ij=14" | grep "n=5" | head -10
grep "VERT_FINAL" log.txt | grep "ij=14" | grep "n=5" | head -10

# Check spawnmove values
grep "SPAWNMOVE_AFTER" log.txt | grep "sp=0" | grep "n=5" | grep "k=0" | head -40

# Compare total abundance
# In R:
ddepon$erla_plot$"Cod Adult" |> group_by(Time) |> summarize(total = sum(number))
ddepoff$erla_plot$"Cod Adult" |> group_by(Time) |> summarize(total = sum(number))
```

---

## Next Steps

1. Review Atlantis documentation for intended behavior of `spawnmove × roc/totroc`
2. Consult with original developers or Atlantis community
3. Test alternative formulas (roc/totroc only, weighted average)
4. Validate that total fish is conserved across boxes after any formula changes

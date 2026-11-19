# atphysics/atvertgeom.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-10](#phy2-10) ⭐ | 699-704, 596-597 | H/H | loop direction / wrong layer | `for(kk=k; kk<ice->currentnz; kk++) copy_ice_layer(ice,kk,kk+1);` shifts forward in ascending order, so every … |  | [ ] |
| [PHY2-11](#phy2-11) | 728 | H/M | off-by-one | `clear_ice_layer(ice, ice->currentnz); ice->currentnz--;` clears one slot past the last active layer. When `cu… |  | [ ] |

---

## PHY2-10

**loop direction / wrong layer** · Conf H / Sev H ⭐ priority  
**Listed lines:** 699-704, 596-597

**Function:** `insert_ice_layer()`

**Why it's thought to be a bug**

`for(kk=k; kk<ice->currentnz; kk++) copy_ice_layer(ice,kk,kk+1);` shifts forward in ascending order, so every layer k..currentnz ends up as a copy of layer k and the deeper ice layers are lost. Then `clear_ice_layer(ice,k)` empties layer k, and the caller `check_ice_dz` does `transfer_ice(ice,k,k+1,maxdz-mindz,...)` from the now-empty layer k. That trips `a > ice->dz[src_k]` → quit("Too much material"). As written, the thick-ice splitting path cannot work. Function: `insert_ice_layer()`, `check_ice_dz()`.

**Current code** (`atphysics/atvertgeom.c` lines 697–706)

```c
  697: 
  698:     /* Move deeper layers down a slot */
  699:     for(kk=k; kk<ice->currentnz; kk++)
  700:     	copy_ice_layer(ice,kk,kk+1);
  701:     ice->currentnz++;
  702: 
  703:     /* Clear new layer */
  704:     clear_ice_layer(ice,k);
  705: }
  706: 
```

**Suggested fix (from review):** Loop `for(kk=ice->currentnz-1; kk>=k; kk--)`, then transfer from k+1 into k (or clear k+1 and transfer k→k+1)

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

## PHY2-11

**off-by-one** · Conf H / Sev M  
**Listed lines:** 728

**Function:** `delete_ice_layer()`

**Why it's thought to be a bug**

`clear_ice_layer(ice, ice->currentnz); ice->currentnz--;` clears one slot past the last active layer. When `currentnz == nz`, clear_ice_layer quits ("Invalid index"). Otherwise the stale duplicate in slot `currentnz-1` is left behind. Function: `delete_ice_layer()`.

**Current code** (`atphysics/atvertgeom.c` lines 726–730)

```c
  726: 
  727:     /* Clear old deepest layer, and point to new one */
  728:     clear_ice_layer(ice,ice->currentnz);
  729:     ice->currentnz--;
  730: }
```

**Suggested fix (from review):** `clear_ice_layer(ice, ice->currentnz - 1);`

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

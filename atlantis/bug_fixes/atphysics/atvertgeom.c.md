# atphysics/atvertgeom.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **2 findings** (severity H: 1; confidence H and severity H: 1; on the priority list: 1).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-10](#phy2-10) ⭐ | 699, 704, 597 | H/H | loop direction / wrong layer | `for(kk=k; kk<ice->currentnz; kk++) copy_ice_layer(ice,kk,kk+1);` shifts forward in ascending order, so every … |  | [ ] |
| [PHY2-11](#phy2-11) | 728, 695 | H/M | off-by-one | `clear_ice_layer(ice, ice->currentnz); ice->currentnz--;` clears one slot past the last active layer. When `cu… |  | [ ] |

---

## PHY2-10

**loop direction / wrong layer** · Conf H / Sev H ⭐ priority  
**Found at (current code):** `atvertgeom.c` 699, `atvertgeom.c` 704, `atvertgeom.c` 597
**Lines at review (Oct 2026):** 699-704, 596-597

**Function:** `insert_ice_layer()`

**Why it's thought to be a bug**

`for(kk=k; kk<ice->currentnz; kk++) copy_ice_layer(ice,kk,kk+1);` shifts forward in ascending order, so every layer k..currentnz ends up as a copy of layer k and the deeper ice layers are lost. Then `clear_ice_layer(ice,k)` empties layer k, and the caller `check_ice_dz` does `transfer_ice(ice,k,k+1,maxdz-mindz,...)` from the now-empty layer k. That trips `a > ice->dz[src_k]` → quit("Too much material"). As written, the thick-ice splitting path cannot work. Function: `insert_ice_layer()`, `check_ice_dz()`.

**Code at issue** — `atvertgeom.c` line 699  
Search for: `for(kk=k; kk<ice->currentnz; kk++)`

```c
  697: 
  698:     /* Move deeper layers down a slot */
  699→     for(kk=k; kk<ice->currentnz; kk++)
  700:     	copy_ice_layer(ice,kk,kk+1);
  701:     ice->currentnz++;
```

**Code at issue** — `atvertgeom.c` line 704  
Search for: `clear_ice_layer(ice,k)`

```c
  702: 
  703:     /* Clear new layer */
  704→     clear_ice_layer(ice,k);
  705: }
  706: 
```

**Code at issue** — `atvertgeom.c` line 597 (closest match to the quoted code; wording differs slightly)  
Search for: `transfer_ice(ice,k,k+1,ice->maxdz-ice->mindz,expanding);`

```c
  595: 						fprintf(bm->logFile,"check_ice_dz: Add ice layer - splitting thick layer\n");
  596: 						insert_ice_layer(ice,k);
  597→ 						transfer_ice(ice,k,k+1,ice->maxdz-ice->mindz,expanding);
  598: 						k++; // As don't want to check the next layer immediately as only just created it;
  599: 					}
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
**Found at (current code):** `atvertgeom.c` 728, `atvertgeom.c` 695
**Lines at review (Oct 2026):** 728

**Function:** `delete_ice_layer()`

**Why it's thought to be a bug**

`clear_ice_layer(ice, ice->currentnz); ice->currentnz--;` clears one slot past the last active layer. When `currentnz == nz`, clear_ice_layer quits ("Invalid index"). Otherwise the stale duplicate in slot `currentnz-1` is left behind. Function: `delete_ice_layer()`.

**Code at issue** — `atvertgeom.c` line 728  
Search for: `clear_ice_layer(ice,ice->currentnz);`

```c
  726: 
  727:     /* Clear old deepest layer, and point to new one */
  728→     clear_ice_layer(ice,ice->currentnz);
  729:     ice->currentnz--;
  730: }
```

**Code at issue** — `atvertgeom.c` line 695 (closest match to the quoted code; wording differs slightly)  
Search for: `if( ice->currentnz == ice->nz)`

```c
  693:     if( k < 0 || k >= ice->nz )
  694:     	quit("insert_ice_layer: invalid layer number (%d)\n",k);
  695→     if( ice->currentnz == ice->nz)
  696:     	quit("insert_ice_layer: no room for insertion (%d)\n",k);
  697: 
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

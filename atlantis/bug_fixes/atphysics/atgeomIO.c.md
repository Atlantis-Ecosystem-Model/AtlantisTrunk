# atphysics/atgeomIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers were re-mapped on 2026-11-19 to the current code (after the merge of main). Each was traced through git history from the version the review used, then checked against the text of the line. **Lines (current code)** and line numbers in the text are now current; ≈ means the code there has changed so the position is approximate. The review's own numbers are kept as **Lines at review**; a number ending in ? could not be traced and is the review's number. ⚠ means the quoted code is no longer at those lines: it may have changed or already been fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Lines (current) | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-01](#phy2-01) | 190-223 | H/M | wrong counting | `bm->nboundary++` runs in three separate `if` blocks that are not mutually exclusive. A box that fails `pointi… |  | [ ] |
| [PHY2-02](#phy2-02) | 769-774 | M/L | wrong pointer | `valueStr = strstr(str, "="); return atof(valueStr);` passes a string that starts with `=` to atof, so it alwa… |  | [ ] |
| [PHY2-03](#phy2-03) | 795-796 | M/L | unchecked return | `fgets(buf, buflen, fp)`'s return value is not checked. If fgets fails, `buf` is uninitialised and `strcmp(buf… |  | [ ] |

---

## PHY2-01

**wrong counting** · Conf H / Sev M  
**Lines (current code):** 190-223  
**Lines at review (Oct 2026):** 190-223

**Function:** `readMSEBoxModelGeom()`

**Why it's thought to be a bug**

`bm->nboundary++` runs in three separate `if` blocks that are not mutually exclusive. A box that fails `pointinpoly` (counted once) and also has `botz == 0.0` (or `botz > 0` with land off) gets counted two times. `nbox - nboundary - nland` is then used as a divisor for ecological indices (atecology/atbiolmessage.c:982-994) and as `ncells` (atmanage/atManage.c:1394), so those averages come out wrong and the divisor could even reach 0 or go negative. Also, a box outside the polygon with `botz > 0` and terrestrial on is counted in both `nboundary` and `nland`. Function: `readMSEBoxModelGeom()`.

**Current code** (`atphysics/atgeomIO.c` lines 188–225)

```c
  188: 		* box is also inside the model boundary
  189: 		*/
  190:         bm->is_boundary[i] = 0;
  191: 		if( pointinpoly(bm->boxes[i].inside, *bm->bnd) ) {
  192: 			bm->boxes[i].type = INTERNALBOX;
  193: 		}
  194: 		else {
  195: 			bm->boxes[i].type = BOUNDARY;
  196: 			bm->nboundary++;
  197: 			if( verbose > 1)
  198: 				fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box\n",i,bm->boxes[i].label);
  199:             bm->is_boundary[i] = 1;
  200: 		}
  201: 
  202: 		if( bm->boxes[i].botz == 0.0 ){
  203: 			bm->boxes[i].type = BOUNDARY;
  204: 			bm->nboundary++;
  205: 			if( verbose > 1)
  206: 				fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box as it has 0.0 depth\n",i,bm->boxes[i].label);
  207:             bm->is_boundary[i] = 1;
  208: 		}
  209: 
  210: 		if( bm->boxes[i].botz > 0.0){
  211: 			if(bm->terrestrial_on == TRUE){
  212: 				bm->boxes[i].type = LAND;
  213:                 bm->nland++;
  214: 				if( verbose > 1)
  215: 					fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a land box as it has %f depth\n",i,bm->boxes[i].label, bm->boxes[i].botz);
  216: 			} else {
  217: 				bm->boxes[i].type = BOUNDARY;
  218: 				bm->nboundary++;
  219: 				if( verbose > 1)
  220: 					fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box as it has 0.0 depth\n",i,bm->boxes[i].label);
  221:                 bm->is_boundary[i] = 1;
  222: 			}
  223: 		}
  224: 	}
  225: 
```

**Suggested fix (from review):** Count once after classification, e.g. only increment when `is_boundary[i]` changes from 0 to 1, and decrement nboundary if the box is later set to LAND

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

## PHY2-02

**wrong pointer** · Conf M / Sev L  
**Lines (current code):** 769-774  
**Lines at review (Oct 2026):** 769-774

**Function:** `getProjectionValue()`

**Why it's thought to be a bug**

`valueStr = strstr(str, "="); return atof(valueStr);` passes a string that starts with `=` to atof, so it always returns 0.0. Currently has no callers (latent). Function: `getProjectionValue()`.

**Current code** (`atphysics/atgeomIO.c` lines 767–776)

```c
  767: 	char *valueStr;
  768: 
  769: 	valueStr = strstr(str, "=");
  770: 
  771: 	if(valueStr == NULL){
  772: 		quit("Box Model Projection format is incorrect.\n The format must be like: \nprojection proj=alb, lon_0=134. ,lat_1=-36. ,lat_2=-18. x_0=3000000., y_0=6000000\n");
  773: 	}
  774: 	return atof(valueStr);
  775: 
  776: }
```

**Suggested fix (from review):** `return atof(valueStr + 1);`

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

## PHY2-03

**unchecked return** · Conf M / Sev L  
**Lines (current code):** 795-796  
**Lines at review (Oct 2026):** 795-796

**Function:** `parseProjectionString()`

**Why it's thought to be a bug**

`fgets(buf, buflen, fp)`'s return value is not checked. If fgets fails, `buf` is uninitialised and `strcmp(buf,"")` reads garbage. Function: `parseProjectionString()`.

**Current code** (`atphysics/atgeomIO.c` lines 793–798)

```c
  793: 
  794: 	 /* read in the line from the input file */
  795: 	 fgets(buf, buflen, fp);
  796:      if (strcmp(buf, "") == 0)
  797: //	 if(buf == NULL )  // This was tripping Wtautological-pointer-compare error
  798: 		 quit("Box Model Projection format is incorrect.\n The format must be like: \nprojection proj=aec lon_0=134. lat_1=-36. lat_2=-18. x_0=3000000. y_0=6000000\n");
```

**Suggested fix (from review):** `if (fgets(buf, buflen, fp) == NULL || buf[0]=='\0') quit(...)`

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

atgeomIO.c `parseProjectionString()` (791-807): `char str[100][10]` is cast to `char **` and parseline stores up to 20 pointers in it. That works only because 20*8 bytes fits in the 1000-byte block. It should be `char *str[20]`. `parseline` can also return `n+1` when more than 20 tokens are present.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atgeomIO.c `freeMSEBoxModelGeom()` does not free `uniqueAdjBoxes`, `distID` or `is_boundary`. This leak happens only at shutdown.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

# atphysics/atgeomIO.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [PHY2-01](#phy2-01) | 196, 202, 184 | H/M | wrong counting | `bm->nboundary++` runs in three separate `if` blocks that are not mutually exclusive. A box that fails `pointi… |  | [ ] |
| [PHY2-02](#phy2-02) | 769 | M/L | wrong pointer | `valueStr = strstr(str, "="); return atof(valueStr);` passes a string that starts with `=` to atof, so it alwa… |  | [ ] |
| [PHY2-03](#phy2-03) | 795 | M/L | unchecked return | `fgets(buf, buflen, fp)`'s return value is not checked. If fgets fails, `buf` is uninitialised and `strcmp(buf… |  | [ ] |

---

## PHY2-01

**wrong counting** · Conf H / Sev M  
**Found at (current code):** `atgeomIO.c` 196, `atgeomIO.c` 202, `atgeomIO.c` 184
**Lines at review (Oct 2026):** 190-223

**Function:** `readMSEBoxModelGeom()`

**Why it's thought to be a bug**

`bm->nboundary++` runs in three separate `if` blocks that are not mutually exclusive. A box that fails `pointinpoly` (counted once) and also has `botz == 0.0` (or `botz > 0` with land off) gets counted two times. `nbox - nboundary - nland` is then used as a divisor for ecological indices (atecology/atbiolmessage.c:982-994) and as `ncells` (atmanage/atManage.c:1394), so those averages come out wrong and the divisor could even reach 0 or go negative. Also, a box outside the polygon with `botz > 0` and terrestrial on is counted in both `nboundary` and `nland`. Function: `readMSEBoxModelGeom()`.

**Code at issue** — `atgeomIO.c` line 196  
Search for: `nboundary++`

```c
  194: 		else {
  195: 			bm->boxes[i].type = BOUNDARY;
  196→ 			bm->nboundary++;
  197: 			if( verbose > 1)
  198: 				fprintf(stderr,"readMSEBoxModelGeom: box %d (%s) is a boundary box\n",i,bm->boxes[i].label);
```

**Code at issue** — `atgeomIO.c` line 202  
Search for: `botz == 0.0`

```c
  200: 		}
  201: 
  202→ 		if( bm->boxes[i].botz == 0.0 ){
  203: 			bm->boxes[i].type = BOUNDARY;
  204: 			bm->nboundary++;
```

**Code at issue** — `atgeomIO.c` line 184  
Search for: `botz > 0`

```c
  182: 
  183: 		/* Check to see if we have land */
  184→ 		// if(!bm->terrestrial_on && bm->boxes[i].botz > 0.0 )
  185: 	//	    	quit("readBoxGeom: Box %d has a botz of greater than 0 which indicates land but you do not have the land mode turned on (flagAllowLand = 0 in yout run.prm file)\n", i);
  186: 
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
**Found at (current code):** `atgeomIO.c` 769
**Lines at review (Oct 2026):** 769-774

**Function:** `getProjectionValue()`

**Why it's thought to be a bug**

`valueStr = strstr(str, "="); return atof(valueStr);` passes a string that starts with `=` to atof, so it always returns 0.0. Currently has no callers (latent). Function: `getProjectionValue()`.

**Code at issue** — `atgeomIO.c` line 769 (the line the review pointed to, found by its text)  
Search for: `valueStr = strstr(str, "=");`

```c
  767: 	char *valueStr;
  768: 
  769→ 	valueStr = strstr(str, "=");
  770: 
  771: 	if(valueStr == NULL){
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
**Found at (current code):** `atgeomIO.c` 795
**Lines at review (Oct 2026):** 795-796

**Function:** `parseProjectionString()`

**Why it's thought to be a bug**

`fgets(buf, buflen, fp)`'s return value is not checked. If fgets fails, `buf` is uninitialised and `strcmp(buf,"")` reads garbage. Function: `parseProjectionString()`.

**Code at issue** — `atgeomIO.c` line 795  
Search for: `fgets(buf, buflen, fp)`

```c
  793: 
  794: 	 /* read in the line from the input file */
  795→ 	 fgets(buf, buflen, fp);
  796:      if (strcmp(buf, "") == 0)
  797: //	 if(buf == NULL )  // This was tripping Wtautological-pointer-compare error
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

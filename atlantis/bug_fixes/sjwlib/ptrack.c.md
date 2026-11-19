# sjwlib/ptrack.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0).

Each finding's **Code at issue** is located by searching the current code for the code quoted in the review, not by line number. Use the **Search for** text with your editor's Find, because line numbers will keep changing as fixes go in. → marks the matching line. If only a close match or a name could be found, the block says so. ⚠ means the quoted code is no longer in the file: it may have been changed or already fixed. **Lines at review** are the October numbers. Line numbers in the description text are approximate.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Found at | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [SJW3-05](#sjw3-05) | 87 | H/L | type mismatch / stack corruption | `int n; ... nc_inq_dimlen(fid,p_did,(size_t *)&n);` writes an 8-byte `size_t` into a 4-byte `int` on 64-bit pl… |  | [ ] |
| [SJW3-06](#sjw3-06) | 154 | H/L | wrong logic / buffer overrun / NULL deref | `if( t ) nc_get_vara_double(..., count(=1), b); for(n = 0; n < rec; n++){ t[n] = (double)b[n]; }` — only one t… |  | [ ] |
| [SJW3-07](#sjw3-07) | 100 | M/L | format specifier mismatch | `quit("... (only %d records)\n",rec,name,nrec)` — `nrec` is `size_t` passed to `%d`. Function: `readparticles(… |  | [ ] |

## Reviewer notes for this file

(Not called anywhere in the Atlantis tree — all latent.)

---

## SJW3-05

**type mismatch / stack corruption** · Conf H / Sev L  
**Found at (current code):** `ptrack.c` 87
**Lines at review (Oct 2026):** 57, 87

**Function:** `readparticles()`

**Why it's thought to be a bug**

`int n; ... nc_inq_dimlen(fid,p_did,(size_t *)&n);` writes an 8-byte `size_t` into a 4-byte `int` on 64-bit platforms, clobbering adjacent stack. Function: `readparticles()`.

**Code at issue** — `ptrack.c` line 87  
Search for: `nc_inq_dimlen(fid,p_did,(size_t *)&n)`

```c
   85:     	quit("readparticles: no n dimension");
   86:     /* Get number of particles */
   87→     nc_inq_dimlen(fid,p_did,(size_t *)&n);
   88: 
   89:     /* Check that there is a variable called "t" */
```

**Suggested fix (from review):** Use a `size_t` temporary and cast to int afterwards.

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

## SJW3-06

**wrong logic / buffer overrun / NULL deref** · Conf H / Sev L  
**Found at (current code):** `ptrack.c` 154
**Lines at review (Oct 2026):** 154-158

**Function:** `readparticles()`

**Why it's thought to be a bug**

`if( t ) nc_get_vara_double(..., count(=1), b); for(n = 0; n < rec; n++){ t[n] = (double)b[n]; }` — only one time value is read, but the loop writes `rec` values into `t` (a pointer to a single double per the API doc), reading uninitialised `b[1..]`; the loop is also outside the `if(t)`, so `t==NULL` with `rec>0` dereferences NULL, and for `rec==0` `*t` is never set. Function: `readparticles()`.

**Code at issue** — `ptrack.c` line 154  
Search for: `if( t )`

```c
  152:     start[0] = (size_t)rec;
  153:     count[0] = 1;
  154→     if( t )
  155: 	nc_get_vara_double(fid,t_vid,start,count, (doubleINPUT *)b);
  156:     for(n = 0; n < rec; n++){
```

**Suggested fix (from review):** Replace loop with `if (t) *t = b[0];` inside the `if(t)` block.

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

## SJW3-07

**format specifier mismatch** · Conf M / Sev L  
**Found at (current code):** `ptrack.c` 100
**Lines at review (Oct 2026):** 100

**Function:** `readparticles()`

**Why it's thought to be a bug**

`quit("... (only %d records)\n",rec,name,nrec)` — `nrec` is `size_t` passed to `%d`. Function: `readparticles()`.

**Code at issue** — `ptrack.c` line 100  
Search for: `quit("`

```c
   98:     nc_inq_dimlen(fid,t_did,&nrec);
   99:     if( rec >= (int)nrec )
  100→     	quit("readparticles: Record %d not in %s (only %d records)\n",rec,name,nrec);
  101:     if( ndump )
  102: 	*ndump = (int)nrec;
```

**Suggested fix (from review):** Cast `(int)nrec`.

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

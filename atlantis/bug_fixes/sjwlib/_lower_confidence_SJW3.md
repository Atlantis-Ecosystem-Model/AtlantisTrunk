# sjwlib — lower-confidence notes from reviewer group SJW3 (not tied to one file) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **0 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 4 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

(Outside my file list, for the datafile.c reviewer) datafile.c:317-320 `dfFindTime()`: `leftover = floor(r / lengthdf); *newt = r - leftover * df->records[ilow];` — multiplies by the first record time instead of the cycle length `lengthdf`, so forcing-file recycling (`rwind`) produces a wrong time unless `records[0]` happens to equal the period; also does not account for a non-zero start time. Likely should be `*newt = df->records[ilow] + fmod(r - df->records[ilow], lengthdf)`.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

text_input.c `parseline()` returns `max+1` when more fields exist than `max`; callers that use the return value as a count into `str[]` (e.g. atgeomIO.c:805 with max 20) must not index `str[max]`. Appears to be intended as an "overflow" signal.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

decay.c `decay_exact()` caches `exp(-k*dt)` in statics — not thread-safe, but correct for single-threaded use.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-4

pointsourcesink.c: `pssN.rewind` is read with `readkeyprm_i` and its return value ignored; if missing, `rewindid` is left uninitialised (malloc'd struct) unless `readkeyprm_i` quits on missing keys (depends on keyprm_errfn setting).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

# sjwlib — lower-confidence notes from reviewer group SJW2 (not tied to one file) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **0 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 3 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

Outside the assigned files but found while checking callers: atmanage/atManageSetup.c:1552 computes the "top" skipper CDF with `speciesParams[cpue_cdf_poor_p_id]` instead of `cpue_cdf_top_p_id`, which is read in atManageParamIO.c:682 but never used. This looks like a high-confidence copy-paste bug.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

atphysics/atgeomIO.c:789-807 passes `char str[100][10]` cast to `char **` into `parseline`/`mpInitialise`. It works only by accident (the storage is reused as a pointer array), and the strings live in the stack `buf[200]`, which is the reason SJW2-01 crashes.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-3

In `mpInitialise` (amg/mga), the nested utm call reallocs the same `proj=` string again. The sizes are equal so realloc normally returns the same pointer, but if it moved, the outer free loop would double-free.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

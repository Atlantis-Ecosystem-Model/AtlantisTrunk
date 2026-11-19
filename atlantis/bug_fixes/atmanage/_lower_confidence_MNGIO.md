# atmanage — lower-confidence notes from reviewer group MNGIO (not tied to one file) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **0 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atManageMPATS.c:263 `this_t = (nyr*365.0 + ij)*86400.0` ignores `bm->tstart`, so the MPA time series would be offset if tstart≠0. With `flagSimpleStartStopMPAs`, `startyr = bm->MPAstartyr` is not checked for <0 (line 107). The rolling loader (`Load_MPAlist_From_File`) also overwrites fixed-MPA fisheries' lists and ignores the start/stop window. All of this may be by design.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

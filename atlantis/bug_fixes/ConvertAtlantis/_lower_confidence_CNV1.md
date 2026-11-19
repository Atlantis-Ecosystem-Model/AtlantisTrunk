# ConvertAtlantis — lower-confidence notes from reviewer group CNV1 (not tied to one file) — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **0 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 2 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

Several regexes assume 2-3 uppercase-letter group codes: `^C_[A-Z]{2,3}$`, `^mum_[A-Z]{2,3}$`, `^KMIGa_[A-Z]{2,3}$`, `^trait_active_[A-Z]{2,3}$`, `^q10_([A-Z]{2,3})$`, `[A-Z]{2,3}_stock_struct$`. Groups with 4-character or digit-containing codes would be silently skipped (or quit as "not found" when required).

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

### LC-2

`commentString[200]` in `Create_Species_Cohort_ParamXML`/`Create_Species_Spawn_ParamXML` is built with strcpy/strcat from comment+units but never used. Current comments are short, so this is a latent overrun only.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 

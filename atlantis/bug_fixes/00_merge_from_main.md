# Step 0 — Merge `origin/main` into `atlantis_update`

Prepared 2026-11-19. Do this **before** starting on the bug files. Several of main's changes touch code covered by the to-do list. Merging first means the bug review files are built from the code we will actually fix.

## Where things stand

| | |
|---|---|
| Branch | `atlantis_update` (HEAD `4dd3106` "Committing ahead of big push on bugs") |
| Common ancestor with main | `834c6a8` (6 Nov 2025, PR #11) |
| Commits on main not on this branch | 137 (including merges), up to PR #95 |
| `origin/dev` vs `origin/main` | dev is 2 commits ahead, and they only change a docs link, so merging main covers everything that matters |
| Files main changed | 155 in total: 39 source/build files, 95 under `example/`, the rest docs/CI (`.github/`, CHANGELOG, README, LICENSE, CONTRIBUTING, data-raw) |

**Main brings in:**
- **Contaminants**: maternal transfer and lactation timing fixes, a segfault fix in `Get_Settler_Contaminants`, transfer of contaminants to carrion on mortality, a new `min_pool_cont` parameter, and separate prey/predator tracer arrays in `Group_Transfer_Contaminant`.
- **Effort displacement with MPAs** (the "JMK fixes", closes #22) and the new `flagMFCdisplace_id`.
- **atdemography fixes**: migration queue index, `boxPopRatio` loop, recruits' `tempPopRatio`.
- **Smaller fixes**: `BiTAC_sp` is now zero-initialised; `calloc` for `forceMoveEntryInput`.
- **Version reporting**: the version is now taken from git tags via `configure.ac` (`RELEASE_VERSION`) instead of SVN.
- **Build**: OS-conditional compiler warnings (`OS_MACOS`/`OS_LINUX`) in PreRules.am.
- **Other**: the `example/` folder updates and the ReactiveAtlantis example.

I ran a dry-run merge in a temporary separate checkout; your working tree was not touched. Using `-X ignore-space-change`, which lets whitespace-only differences merge automatically, the conflicts are:
- 8 source files with 1 hunk each, below;
- the 15 generated `Makefile.in` files, `PreRules.am` and `.gitignore`;
- 7 `.DS_Store` files.

Without that option, atdemography.c (18 hunks) and atlantisboxmodel.h (4 hunks) also conflict, but only because of reformatting.

---

## Part 1 — Conflicts that need a decision

### M-01 `atecology/atmovement.c` — returning migrants and `ContaminantMigrationIn` (relates to CONT-13)

Both sides fixed the same original bug: `mignum_actual` was used before it was computed. They did it differently.

```c
/* main */
mignum = MIGRATION[sp].DEN[n][qid] * migtime2;
mignum_actual = MIGRATION[sp].survival[qid] * mignum;
if(bm->track_contaminants)
    ContaminantMigrationIn(bm, sp, n, qid, MIGRATION[sp].DEN[n][qid], mignum_actual);   // DEN = pool BEFORE removal
if(!mignum) continue;
MIGRATION[sp].DEN[n][qid] -= mignum;
...
/* this branch */
MIGRATION[sp].DEN[n][qid] -= mignum;
mignum_actual = MIGRATION[sp].survival[qid] * mignum;
totden[sp][n] += mignum_actual;
if(bm->track_contaminants)
    ContaminantMigrationIn(bm, sp, n, qid, MIGRATION[sp].DEN[n][qid], mignum_actual);   // DEN = pool AFTER removal
```

`ContaminantMigrationIn` computes `contam_return = final_contam * num_returning / (tot_num_mig + small_num)`. That share only makes sense if `tot_num_mig` is the pool *before* the returners are removed. With the branch's order the share is too big, and when the whole pool returns (DEN → 0) it goes to ~1e8×. A plain merge would also keep **both** calls.

- [Yes] A. **Recommended:** use main's order, and delete the branch's second call after `totden += mignum_actual`
- [ ] B. Other (comment)

> 

### M-02 `atecology/atContaminants.c` — `Group_Transfer_Contaminant` declarations

The branch declares `double *tracerArray;`. Main declares `double *preyTracerArray, *predatorTracerArray;`, which its rewritten function body uses. The prey and predator can now sit in different habitats (EPIFAUNA vs the habitat). The only remaining use of `tracerArray` is a comment.

- [Yes] A. **Recommended:** take main's declarations
- [ ] B. Other

> 

### M-03 `atharvest/atHarvestIO.c` — name of the displaced-effort output file

The branch kept `DisplaceEffort.txt` and switched to `snprintf`. Main renamed the output to `DetailedEffort.txt`, as part of the effort-displacement fix (commit 1483ee4). Any R or post-processing scripts that read this file will need the new name.

- [Yes] A. Use main's name with your `snprintf`: `snprintf(fname, sizeof(fname), "%sDetailedEffort.txt", ...)`
- [ ] B. Keep `DisplaceEffort.txt` (differs from trunk)

> 

### M-04 Trivial conflicts (whitespace, comments, a doubled `;`)
- `atlantisUtil/atUtilIO.c`, `atlantisUtil/atUtilXML.c`: main only adds commented-out debug `printf`s. Take the branch's version.
- `atlantismain/atlantismain.c`: a blank line. Take the branch's version. Note that `num_scaling_indices` uses the branch's spelling; main's `num_scaling_indicies` merges away automatically.
- `atmanage/atManage.c`: the same declaration line, wrapped differently. Take the branch's version; `flagMFCdisplace` is declared in both.
- `atmanage/atManageSetup.c`: `BiTAC_sp` allocation. Take the branch's version (main has a stray `;;`).

- [Yed] A. **Recommended:** as above
- [ ] B. Other

> 

### M-05 `atlantis/PreRules.am` — compiler flags

Both sides made the same Mac warning change differently. Main wraps the flags in `if OS_MACOS` / `if OS_LINUX`, using new conditionals in `configure.ac`, and fixes `$(INCLUDE)` → `$(INCLUDES)`. The branch has two additions that main doesn't:
- the `FISHMIP_OSP_DIR` block, with a default under **your** `$(HOME)/Work/...` path;
- `OPT = -O2 -mtune=generic` (main: `-mtune=native`).

- [ ] A. **Recommended:** main's OS-conditional structure and `$(INCLUDES)`, plus the branch's `FISHMIP_OSP_DIR` block and `OPT` line
- [Yes] B. Other - as recommended but should we bundle the FISHMIP directory into Atlantis? I was not going to as it is its own git REPO and we could get out of date

> 

Note: if this branch is ever pushed to trunk, the `FISHMIP_OSP_DIR` default will point to a path that only exists on your machine.

### M-06 The 15 generated `Makefile.in` files, plus `config.h.in` and `configure`

Both sides regenerated these with automake 1.16.5 (installed at `/opt/homebrew/bin`), so hand-merging them is pointless.

- [Yes] A. **Recommended:** merge `Makefile.am`, `PreRules.am` and `configure.ac`, then run `autoreconf -fi` in `atlantis/` to regenerate them, then do a test `./configure && make`
- [ ] B. Take main's versions as they are

> 

### M-07 `.gitignore` and `.DS_Store`
Main deleted the tracked `.DS_Store` files. The branch had modified them.

- [Yes] A. **Recommended:** take the union of both `.gitignore` versions (keep `.DS_Store` and the xcuserstate line; add `*.Rproj` and `example/outputFolder/`), and accept main's removal of the `.DS_Store` files from git. Your local copies stay on disk, just untracked.
- [ ] B. Other

> 

---

## Part 2 — Changes from main that merge automatically and need checking

These merge without conflicts, but they change behaviour or overlap to-do items. The to-do IDs come from searching the list for the same code, so they are "likely related", not confirmed.

### A-01 `atdemography.c` `Get_Latest_Additions()` — main has introduced a wrong index ⚠
```c
bm->tempPopRatio[stock_id][species][ngene][0] += coming_SPden[ngene];   // main: was [cohort]
if((!_finite(bm->tempPopRatio[stock_id][ngene][cohort][0])) || ...       // main: ngene in the SPECIES slot
```
The first line is a reasonable fix (recruits go into cohort `ngene`). The `_finite` check now uses `ngene` as the species index, so it checks the wrong array element and can read out of bounds. Related to-do items: DEMO-16/17/18/25/26, BSET-12.

- [yes] A. **Recommended:** after the merge, correct the check to `[stock_id][species][ngene][0]` (included in the merge fix-up)
- [ ] B. Leave it for the DEMO review

> 

### A-02 `atdemography.c` `Update_Age_Distrib()` — `boxPopRatio[...][b-1]` loop (likely DEMO-16)
Main now starts the loop at `b = 1` (it started at 0, so `b-1` read index −1). It also changes the target index from `[k]` to `[b]` in `tempPopRatio` and `recVERTpopratio`. The first part is clearly right. The index change needs checking against what the last dimension of `tempPopRatio` means (elsewhere it's indexed with layer `k`).

- [Yes but we really will need to check!!] A. Accept it now, and check it properly when we do DEMO-16
- [ ] B. Keep `b = 1` but revert `[b]` → `[k]` for now

> 

### A-03 Fixes that look like they close to-do items (we'll confirm when we reach each file)
| Main change | Likely to-do ID |
|---|---|
| `Find_Final_Invert_Recruit_Distribtuion`: `Leave_Now[qid]` → `Leave_Now[mid]` | EINIT-11 |
| atManage.c: `MPA[bm->current_box]` → `MPA[ij]`; `orig_FCpressure` set before the branch; `MPA_check` no longer inverted; `CumDisplaceEffort` accumulated after effort is final; MPA-driven displacement | MNG-04, MNG-06, MNG-07, MNG-12 |
| `BiTAC_sp` zero-initialised | MNGA-25, MNGIO-04 |
| `forceMoveEntryInput` `malloc` → `calloc` | EMISC-01 |
| `Get_Settler_Contaminants` segfault with `flagmaternaltransfer` | CONT-16 |
| `ContaminantMigrationIn` `time_step = day_end - day_start` (both sides made this change) | CONT-13 |

### A-04 Behaviour changes to be aware of (results will differ from runs before the merge)
- **Contaminants**:
  - mortality now passes contaminant to CARRION (atGroupProcesses.c);
  - the `Group_Transfer_Contaminant` global habitat argument changes from `habitat` to `level_id` (atprocess.c);
  - maternal transfer is now done at birth and the neonate is flagged (`suckler_contam_today_id`, which replaces the unused `flag_contam_distrib_id` enum slot);
  - external-spawner maternal transfer is commented out.
- **`diagTol`** (atUtilGroupIO.c; related to PROC-25): CARRION goes from 3 to 1, and PWN moves from 2 to 1. Both now feed into the adaptive timestep, which can change run time and results even with contaminants off.
- **New biology XML parameter `min_pool_cont`**: it is read only when contaminants are on, and it is **required** (`isRequired = 1`, so `Util_XML_Read_Value` calls `quit()` if it's missing). Contaminant runs whose biology XML lacks it will stop with an error until it's added (main's example value is `0.00000001`).
- **Version output**: the log and `-v` now show `git describe --tags` instead of the SVN revision.

- [Yes] Noted / OK to proceed

> 

---

## Part 3 — After the merge

1. Make the merge in your working tree with `git merge --no-commit --no-ff -X ignore-space-change origin/main`, resolve the conflicts as decided above, and run `autoreconf` and a test build.
2. **Regenerate the bug review files** against the merged code. No one has filled any in yet, so nothing is lost. To-do items that main's changes touch will get a note such as "main changed this code in the merge — check whether it's fixed".
3. Write the merge commit message to `commit_messages/000_merge_origin_main.txt`. You run `git commit`; `MERGE_HEAD` will be in place, so that makes the merge commit.

**Your overall go-ahead:**
- [Yes] Proceed with the merge as decided above
- [ ] Hold

> 

---

## Outcome (2026-11-19, Claude)

Merged with `git merge --no-commit --no-ff -X ignore-space-change origin/main`. Not committed: `MERGE_HEAD` is in place.

- **M-01 to M-07**: resolved as ticked. For M-05 the FISHMIP block stays unchanged and is not bundled, because it is its own repo. If you want it pinned to a version later, a git submodule is the usual way to do that.
- **A-01**: the `_finite` check in `Get_Latest_Additions` is now `tempPopRatio[stock_id][species][ngene][0]`.
- **A-02**: accepted as is. It is flagged 🔀 on DEMO-16 for a proper check.
- `autoreconf -i` regenerated all `Makefile.in` files (automake 1.16.5). `configure` and `config.h.in` came out identical to main's.
- **Build check**: `./configure && make` succeeded in a scratch copy of the tree, with no compiler warnings on any line the merge changed. It could not be built in place because the existing `.o` files in the tree are owned by **root** (from a `sudo` or Docker build on 15 Nov). Run `sudo find atlantis -name '*.o' -user root -delete` (or `make clean` as root) before your next local build.
- **Also arrived from main**: `atlantis/outamps`, a stray 29-byte file containing "bash: cd: too many arguments", committed by accident upstream. I left it in so this merge matches main. Delete it whenever you like.
- **Bug review files** were regenerated against the merged code. 21 findings are flagged 🔀 because main changed their code: CONT-01/05/06/07/08/13/16/18, DEMO-16/25, EINIT-11, EMISC-01, MNG-03/04/06/07/12, MNGA-25, MNGIO-04, MOVE-19, PROC-25.
- **Commit message**: [commit_messages/000_merge_origin_main.txt](commit_messages/000_merge_origin_main.txt)

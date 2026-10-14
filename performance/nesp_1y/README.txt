NESP one-year Atlantis benchmark
================================

This directory holds the model used for performance tests of atlantisMerged.
Inputs and run outputs are gitignored. This README is tracked.

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ What was copied ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

Copied on 2026-10-05 as real files (symlinks followed) from:

  /home/por07g/Documents/Projects/NESP_ParksAustralia/Atlantis/whacamole_local/results/runs/local_1y/runs/0000_NESP_bio

That run directory itself is left untouched. forcing_files/ comes from
Running_model/forcing_files. NESP_bio.prm is the 1.4 MB file used by the
live local_1y run, not a symlink.

Cadence in NESP_run.prm: dt 12 hour, tstop 365 day, toutinc 7 day.
Fisheries and economics are off (flag_fisheries_on 0, flagecon_on 0).

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Run command ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

From performance/nesp_1y/model, with this repository's executable:

  LD_LIBRARY_PATH=/usr/local/proj4-4.9.3/lib \
  /usr/bin/time -v \
  ../../../atlantis/atlantismain/atlantisMerged \
    -i NESP_init.nc 0 \
    -o output.nc \
    -r NESP_run.prm \
    -m NESP_migration.csv \
    -f NESP_force_noevap.prm \
    -p NESP_physics.prm \
    -b NESP_bio.prm \
    -s NESP_groups.csv \
    -d ../runs/<run_name>

Do not change tstop, groups, forcing, or output frequency for a regression run.

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Layout ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

  model/       gitignored inputs
  baselines/   gitignored archived outputs, never overwritten by a later run
  runs/        gitignored timed and profiled runs

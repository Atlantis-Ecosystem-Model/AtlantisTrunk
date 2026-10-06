#!/bin/bash

find /app/model -type f | xargs dos2unix

#valgrind --log-file=valgrind%p.log --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes atlantisMerged -i INIT_VMPA_Jan2015.nc 0 -o outputSETAS.nc -r VMPA_setas_run_fishing_F_Trunk.prm -f VMPA_setas_force_fish_Trunk.prm -p VMPA_setas_physics.prm -b VMPA_setas_biol_fishing_Trunk.prm -h VMPA_setas_harvest_F_Trunk.prm  -s SETasGroupsDem.csv -q SETasFisheries.csv -d outputFolder
#atlantisMerged -i INIT_VMPA_Jan2015.nc 0 -o outputSETAS.nc -r VMPA_setas_run_fishing_F_Trunk.prm -f VMPA_setas_force_fish_Trunk.prm -p VMPA_setas_physics.prm -b VMPA_setas_biol_fishing_Trunk.prm -m SETas_Migrations.csv -h VMPA_setas_harvest_F_Trunk.prm  -s SETasGroupsDem.csv -q SETasFisheries.csv -d outputFolder

# Using lldb
#file atlantisMerged
#process launch -- -i INIT_VMPA_Jan2015.nc 0 -o outputSETAS.nc -r VMPA_setas_run_fishing_F_Trunk.prm -f VMPA_setas_force_fish_Trunk.prm -p VMPA_setas_physics.prm -b VMPA_setas_biol_fishing_Trunk.prm -m SETas_Migrations.csv -h VMPA_setas_harvest_F_Trunk.prm  -s SETasGroupsDem.csv -q SETasFisheries.csv -d outputFolder

# used with assessment prm
atlantisMerged -i INIT_VMPA_Jan2015.nc 0 -o outputSETAS.nc -r VMPA_setas_run_fishing_F_Trunk.prm -f VMPA_setas_force_fish_Trunk.prm -a VMPA_setas_assess.prm -p VMPA_setas_physics.prm -b VMPA_setas_biol_fishing_Trunk.prm -m SETas_Migrations.csv -h VMPA_setas_harvest_F_Trunk.prm  -s SETasGroupsDem.csv -q SETasFisheries.csv -d output 2>out.txt 
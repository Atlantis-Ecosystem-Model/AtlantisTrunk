rm(list = ls()) # clear memory

library("devtools")
library("ReactiveAtlantis")
library("proj4")
library("here")

####### Compare outputs and Biomass visualization #######
nc.current  <- here::here("example","outputFolder","outputSETAS.nc")
nc.old      <- here::here("example","outputFolderPristine","outputSETAS.nc")
grp.csv     <- here::here("example","SETasGroupsDem.csv")
bgm.file    <- here::here("example","VMPA_setas.bgm")
cum.depths  <- c(0,20,50,100,250,700,2000) ## This should be the cummulative depth of your model
## individual file
compare(nc.current, nc.out.old = NULL, grp.csv, bgm.file, cum.depths)
## compare to previous run
compare(nc.current, nc.old, grp.csv, bgm.file, cum.depths)

####### Predation analysis from the Atlantis output #######
biom        <- here::here("example","outputFolderPristine","outputSETASBiomIndx.txt")
diet.file   <- here::here("example","outputFolderPristine","outputSETASDietCheck.txt")
bio.age     <- here::here("example","outputFolderPristine","outputSETASAgeBiomIndx.txt") ## optional file. just if you want to check the predation by age
grp.csv     <- here::here("example","SETasGroupsDem.csv")
## Predation by Age
predation(biom, grp.csv, diet.file, bio.age)
## No predation by Age
#predation(biom, grp.csv, diet.file, bio.age = NULL)

####### Exploring predator-prey interactions from the initial conditions #######
prm.file    <-  here::here("example","VMPA_setas_biol_fishing_Trunk.prm")
nc.initial  <-  here::here("example","INIT_VMPA_Jan2015.nc")
grp.csv     <- here::here("example","SETasGroupsDem.csv")
bgm.file    <- here::here("example","VMPA_setas.bgm")
cum.depths  <- c(0,20,50,100,250,700,2000) ## This should be the cummulative depth of your model
feeding.mat(prm.file, grp.csv, nc.initial, bgm.file, cum.depths)

####### Atlantis food web and trophic level composition #######
grp.csv     <- here::here("example","SETasGroupsDem.csv")
prm.file    <-  here::here("example","VMPA_setas_biol_fishing_Trunk.prm")
diet.file   <- here::here("example","outputFolder_original","outputSETASDietCheck.txt")
food.web(diet.file, grp.csv)
## optional you can explore the food web by polygon
#food.web(diet.file, grp.file, diet.file.bypol)
## diet.file.bypol Detailed diet check file, this can be obtained as an extra output from Atlantis "DetailedDietCheck.txt". To get this file from Atlantis turn on the option "flagdietcheck" on the Run.prm file.

####### Growth of primary producers and limiting factors #######
nc.initial  <-  here::here("example","INIT_VMPA_Jan2015.nc")
nc.current  <- here::here("example","outputFolderPristine","outputSETAS.nc")
grp.csv     <- here::here("example","SETasGroupsDem.csv")
prm.file    <-  here::here("example","VMPA_setas_biol_fishing_Trunk.prm")
growth.pp(nc.initial, grp.csv, prm.file, nc.current)

####### Analysis of recruitment and primary production #######
nc.initial  <-  here::here("example","INIT_VMPA_Jan2015.nc")
nc.current  <- here::here("example","outputFolderPristine","outputSETAS.nc")
yoy.file    <- here::here("example","outputFolderPristine","outputSETASYOY.txt")
grp.csv     <- here::here("example","SETasGroupsDem.csv")
prm.file    <-  here::here("example","VMPA_setas_biol_fishing_Trunk.prm")
recruitment.cal(nc.initial, nc.current, yoy.file, grp.csv, prm.file)

####### Harvest outputs and model skill assessment #######
catch.nc    <- here::here("example","outputSETASCATCH.nc")
ext.catch   <- 'external_catch_time_series.csv'
cum.depths  <- c(0,20,50,100,250,700,2000) ## This should be the cummulative depth of your model
fsh.csv     <- 'SETasFisheries.csv'
bgm.file    <- 'VMPA_setas.bgm'
grp.csv     <- 'SETasGroupsDem.csv'
catch(grp.csv, fsh.csv, catch.nc, ext.catch)

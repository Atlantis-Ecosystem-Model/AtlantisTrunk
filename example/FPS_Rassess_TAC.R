setwd("/app/model/output/")
#setwd("M:/Atlantis/Runs/atlantis_v6702_r/test_TAC_FHA_survey_8")
##cat(as.character(Sys.time()), "After setwd:", getwd(), "\n", file = "FDG_Rassess_called.log", append = TRUE)

infilename <- "FPS_RAssessinput.csv"
outfilename <- "FPS_Rassess.out"
catchdata <- "outputSETASCatch.txt"




dfTAC <- data.frame(fishery = 0:32, TAC = c(0,0,50,rep(0,30))*1000)
write.table(dfTAC, file = outfilename, row.names = FALSE, quote = FALSE)

cat("R done\n")


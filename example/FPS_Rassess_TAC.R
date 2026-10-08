setwd("/app/model/output/")

# Created by atlantis and populated with survey data etc.
# The output to this file will need to be reviewed and adapted
infilename <- "FPS_RAssessinput.csv"
# output of this R scrpt is read in by atlanis (required)
outfilename <- "FPS_Rassess.out"
# You could read in Catch and change TAC based on previous year(s) (Optional)
catchdata <- "outputSETASCatch.txt"

# This is where you can write code to estimate new TAC.
# ....

# For this example, we are fixing the TAC for the 3rd fishery (33 in total)
# (The 3rd fishery is set in the harvest file (xxx_flagmagage = 2))

# TAC by fishery
dfTAC <- data.frame(fishery = 0:32, TAC = c(0, 0, 50, rep(0, 30)) * 1000)
# Write to a file that atlantis will read in and overwrite parameter TAC_FPS
write.table(dfTAC, file = outfilename, row.names = FALSE, quote = FALSE)

# write to console as Atlantis is running
cat("R done\n")

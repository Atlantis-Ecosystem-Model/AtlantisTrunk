# runDynTier4onRTMB.R
#
# Ratpack dynamic Tier 4 assessment (Pella-Tomlinson production model fitted with RTMB) called by
# Atlantis for tier 10 with tiertype 1 (RDynamicTier4Assessment in atmanage/atManageTier.c).
#
# Adapted Nov 2026 from runDynTier4onRTMB.R in PuntRatpackR/MISC HCR (A. Punt). Changes from that version:
#  - reads dynTier4.dat and dynTier4.ctl, and writes dynTier4.rep, in the working directory
#    (Atlantis changes into the DynTier4_sim_X_year_Y directory before calling Rscript)
#  - CPUE likelihood loop fixed (loop variable was Icue but the body used Icpue, so only one series was fitted)
#  - map typo fixed (logr -> logR) for EstR = 0 and EstZ = 1
#  - B0(t) -> B0[t] in the dynamic B0 option
#  - MSYL penalty only applied if CV(MSYL) > 0 and the r prior only if its sd > 0 (as in the Atlantis C version)
#  - hockey stick break points read from the control file ("# Hockey stick alpha beta", default 0.2 and 0.35)
#  - no plots
#  - eval.max and iter.max passed to nlminb (they were given to MakeADFun, which doesn't use them, so nlminb
#    stopped at its default limit of 200 function evaluations, e.g. with process error option 1)
#
# Year columns written by Atlantis are data year indices (0 = first data year), so the BMSY and MSY year
# ranges in the control file are data year indices too.

suppressMessages(library(RTMB))

MatchTable<-function(Table,Char1=NULL,Char2=NULL,Char3=NULL,Char4=NULL,Char5=NULL,Char6=NULL,Optional=FALSE)
{
  ii <- rep(T,length(Table[,1]))
  if (!is.null(Char1)) ii <- ii & (Table[,1]==Char1)
  if (!is.null(Char2)) ii <- ii & (Table[,2]==Char2)
  if (!is.null(Char3)) ii <- ii & (Table[,3]==Char3)
  if (!is.null(Char4)) ii <- ii & (Table[,4]==Char4)
  if (!is.null(Char5)) ii <- ii & (Table[,5]==Char5)
  if (!is.null(Char6)) ii <- ii & (Table[,6]==Char6)
  ii <- seq(1:length(Table[,1]))[ii]
  if (length(ii) == 0 & Optional == FALSE) stop(paste("Failed to find:",Char1,Char2,Char3,Char4,Char5,Char6))
  return(ii)
}

FindZ <- function(z,msyl)
{
  val <- 1-(z+1)*msyl^z
  return(val)
}

# ====================================================================================================

f2 <- function(parms) {
  getAll(data, parms, warn=FALSE)

  n <- length(C); Yr1 <- Year[1]

  # Extract the parameters
  k <- exp(logK);
  z <- exp(logz);
  q <- exp(logQ);
  sigma <- exp(logSigma);
  SigmaR <- exp(logSigmaR);
  if (ProcessError==0) SigmaR = 0;

  # Is MSYL (BMSY/B0) calculated from z (1) or pre-specified (0)
  if (EstZ==1) MSYLOut <- (1.0/(z+1.0))^(1.0/z) else MSYLOut <- MSYL;

  # Is R estimated (1) or computed from MSYINPUT (0)
  BMSY <- MSYLOut*k;
  if (EstR==1) {  r <- exp(logR); } else {  r <- MSYINPUT / (BMSY*(1.0-MSYLOut^z));   }
  MSY <- r*BMSY*(1.0-MSYLOut^z);

  # End of years
  n1 <- n + 1;
  f <- 0

  # Declare variables
  B <- rep(0,n1);
  Dep <- rep(0,n1);
  Ihat <- matrix(0,nrow=n,ncol=Ncpue)
  Chat <- rep(0,n);
  ExpOut <-rep (0,n);
  B0<- rep(0,n1+50);

  # project model forward
  B[1] <- k;
  B0[1] <- k;
  for (t in 1:n)
   {
    Expl <- 1.0/(1.0+exp(-FF[t]));
    B[t+1] <- B[t] + r*B[t]*(1-(B[t]/k)^z) - Expl*B[t];
    B[t+1] <- B[t+1]*exp(Rec_dev[t]*SigmaR-SigmaR*SigmaR/2.0);
    if (IsDymB0==1)
     {
      B0[t+1] <- B0[t] + r*B0[t]*(1-(B0[t]/k)^z);
      B0[t+1] <- B0[t+1]*exp(Rec_dev[t]*SigmaR-SigmaR*SigmaR/2.0);
     }
    else
      B0[t+1] <- k;
    Chat[t] <- Expl*B[t];
    ExpOut[t] <- Expl;
    # Predicted CPUE
    for (Icpue in 1:Ncpue) Ihat[t,Icpue] <- q[Icpue]*(B[t]+B[t+1])/2.0;
   }
  f <- f - sum(dnorm(log(C), log(Chat), 0.05, log=T));

  # project model under B0
  for(t in (n+1):(n+50)) B0[t+1] <- B0[t] + r*B0[t]*(1-(B0[t]/k)^z);

  # Negative log-likelihood for the cpue indices (only where the index is positive)
  for (Icpue in 1:Ncpue)
   for (t in 1:n)
    if (I[t,Icpue] > 0)
        f <- f - dnorm(log(I[t,Icpue]), log(Ihat[t,Icpue]), sigma, log=T);

  # Can fit to mean biomass relative to MSYL
  BioAve = 0;
  for(t in (MSYRY1-Yr1+1):(MSYRY2-Yr1+1)) BioAve <- BioAve + B[t];
  BioAve <- BioAve / (MSYRY2-MSYRY1+1);

  # Penalty on that mean biomass matches MSYL
  CompPrior1 <- 0;
  if (CVMSYL > 0) CompPrior1 <- 1.0/(CVMSYL*CVMSYL)*(log(BioAve) - log(BMSY))*(log(BioAve) - log(BMSY));

  # Prior on r
  CompPrior2 <- 0;
  if (PriorSDr > 0) CompPrior2 <- dnorm(r,PriorMeanR,PriorSDr,log=T);

  # Prior on rec_devs
  CompPrior3 <- 0;
  for (t in 1:n) CompPrior3 <- CompPrior3 + dnorm(Rec_dev[t],0.0,1.0,log=T);

  # Calc dep and report
  Dep <- B/B[1];

  # Add penalties
  f <- f + CompPrior1;  f <- f - CompPrior2; f <- f - CompPrior3;

  ADREPORT(r);
  ADREPORT(k);
  ADREPORT(z);
  ADREPORT(q);
  ADREPORT(MSY);
  ADREPORT(MSYLOut);
  ADREPORT(MSYLOut*k);
  ADREPORT(BioAve);
  ADREPORT(sigma);
  ADREPORT(B);          # uncertainty
  ADREPORT(Dep);        # stock status
  REPORT(f);
  REPORT(B);
  REPORT(B0);
  REPORT(Chat);
  REPORT(ExpOut);
  REPORT(Ihat);

  return(f);

}

# ====================================================================================================

DatFile <- read.table("dynTier4.dat",comment.char = "?",fill=T,blank.lines.skip=T,stringsAsFactors=F,col.names=1:100)
CtlFile <- read.table("dynTier4.ctl",comment.char = "?",fill=T,blank.lines.skip=T,stringsAsFactors=F,col.names=1:100)

# Read the data file
Index <- MatchTable(DatFile,Char1="#",Char2="Number",Char4="fleets"); Nfleet <- as.numeric(DatFile[Index+1,1])
Index <- MatchTable(DatFile,Char1="#",Char2="Range",Char4="years"); Yr1 <- as.numeric(DatFile[Index+1,1]); Yr2 <- as.numeric(DatFile[Index+1,2]);
Nyears <- Yr2-Yr1+1
cat("Nfleet and year range",Nfleet,Yr1,Yr2,Nyears,"\n")
Index <- MatchTable(DatFile,Char1="#",Char2="Year,",Char3="Catch");
TheData <- matrix(0,nrow=Nyears,ncol=(1+2*Nfleet))
for (Iyear in 1:Nyears)
 for (II in 1:(1+2*Nfleet)) TheData[Iyear,II] <- as.numeric(DatFile[Index+Iyear,II])

# Extract the data
Catch <- apply(as.matrix(TheData[,2:(1+Nfleet)],ncol=Nfleet),1,sum)
IndexData <- as.matrix(TheData[,(2+Nfleet):(1+2*Nfleet)])
Years <- TheData[,1]

# how many Cpue series
UseCPUE <- apply(IndexData,2,max) > 0
Ncpue <- sum(UseCPUE)
IndexData <- matrix(IndexData[,UseCPUE],ncol=Ncpue)
cat("Number of CPUE series: ",Ncpue,"\n")
if (Ncpue < 1) stop("No CPUE series with positive values")

# Read the CTL file
Index <- MatchTable(CtlFile,Char1="#",Char2="MSYL"); MSYL <- as.numeric(CtlFile[Index+1,1])
Index <- MatchTable(CtlFile,Char1="#",Char2="EstR"); EstR <- as.numeric(CtlFile[Index+1,1])
Index <- MatchTable(CtlFile,Char1="#",Char2="EstZ"); EstZ <- as.numeric(CtlFile[Index+1,1])
Index <- MatchTable(CtlFile,Char1="#",Char2="BMSY"); MSYRY1 <- as.numeric(CtlFile[Index+1,1]); MSYRY2 <- as.numeric(CtlFile[Index+1,2])
Index <- MatchTable(CtlFile,Char1="#",Char2="Target"); Btarg <- as.numeric(CtlFile[Index+1,1])
Index <- MatchTable(CtlFile,Char1="#",Char2="MSY"); MSYPriorY1 <- as.numeric(CtlFile[Index+1,1]); MSYPriorY2 <- as.numeric(CtlFile[Index+1,2])
Index <- MatchTable(CtlFile,Char1="#",Char2="CV(MSYL)"); CVMSYL <- as.numeric(CtlFile[Index+1,1])
Index <- MatchTable(CtlFile,Char1="#",Char2="R(prior)"); PriorMeanR <- as.numeric(CtlFile[Index+1,1]);PriorSDr <- as.numeric(CtlFile[Index+1,2])
Index <- MatchTable(CtlFile,Char1="#",Char2="Process"); ProcessError <- as.numeric(CtlFile[Index+1,1])
HCRalpha <- 0.2; HCRbeta <- 0.35
Index <- MatchTable(CtlFile,Char1="#",Char2="Hockey",Optional=TRUE)
if (length(Index) > 0) { HCRalpha <- as.numeric(CtlFile[Index+1,1]); HCRbeta <- as.numeric(CtlFile[Index+1,2]) }

# Default ranges (as in the Atlantis C version): MSY years 0 = all years, BMSY period only used if CV(MSYL) > 0
if (MSYPriorY2 < 1 | MSYPriorY2 > Yr2) MSYPriorY2 <- Yr2
if (MSYPriorY1 < Yr1 | MSYPriorY1 > MSYPriorY2) MSYPriorY1 <- Yr1
if (MSYRY2 < MSYRY1) MSYRY2 <- MSYRY1

cat(MSYL,EstR,EstZ,MSYRY1,MSYRY2,Btarg,MSYPriorY1,MSYPriorY2,CVMSYL,PriorMeanR,PriorSDr,ProcessError,HCRalpha,HCRbeta,"\n")

# Compute median catch as a proxy for MSY
CIndex <- which(TheData[,1] >= MSYPriorY1 & TheData[,1] <= MSYPriorY2)
MSYINPUT= median(Catch[CIndex])

# find the  z corresponding to BMSY/B0
fitz <- uniroot(FindZ,lower=0.0001,upper=10,msyl=MSYL)
z <- fitz$root
MSYL <- (1.0/(z+1.0))^(1.0/z)

data <- list(Year=Years,C=Catch,I=IndexData,MSYL=MSYL,EstR=EstR,EstZ=EstZ,MSYINPUT=MSYINPUT,MSYRY1=MSYRY1,MSYRY2=MSYRY2,
             Ncpue=Ncpue,CVMSYL=CVMSYL,MSYPriorY1=MSYPriorY1,MSYPriorY2=MSYPriorY2,
             PriorMeanR=PriorMeanR,PriorSDr=PriorSDr,ProcessError=ProcessError,IsDymB0=0)
Nyear <- length(Catch)

# Initial values for the parameters
rhat <- exp(-1.1)
Khat <- MSYINPUT/(rhat*MSYL*(1-MSYL^z))
Q <- rep(0,Ncpue)
for (Ifleet in 1:Ncpue) Q[Ifleet] <- mean(IndexData[IndexData[,Ifleet]>0,Ifleet])/(Khat/2)
logSigma = -2.3

parameters <- list(logR=log(rhat), logK=log(Khat), logQ=log(Q), logSigma=logSigma,FF=rep(-2,Nyear),logz=log(z),logSigmaR=log(0.1),Rec_dev=rep(0,Nyear))
if (data$EstR==0 & data$EstZ==0) map <- list(logz=factor(NA),logR=factor(NA))
if (data$EstR==1 & data$EstZ==0) map <- list(logz=factor(NA))
if (data$EstR==0 & data$EstZ==1) map <- list(logR=factor(NA))
if (data$EstR==1 & data$EstZ==1) map <- NULL
if (ProcessError==0) map <- c(map,list(logSigmaR=factor(NA),Rec_dev=rep(factor(NA),length(parameters$Rec_dev))))
if (ProcessError==1) map <- c(map,list(logSigmaR=factor(NA)))

################################################################################

 # fit the model
 if (ProcessError %in% c(0,1))
  model <- MakeADFun(f2, parameters,map=map,control=list(eval.max=10000,iter.max=1000,rel.tol=1e-15),silent=T)
 if (ProcessError %in% c(2))
  model <- MakeADFun(f2, parameters,map=map,random=c("Rec_dev"),control=list(eval.max=10000,iter.max=1000,rel.tol=1e-15),silent=T)
 fit <- nlminb(model$par, model$fn, model$gr, control=list(eval.max=10000,iter.max=1000))
 if (ProcessError != 2)
  for (i in 1:8)
   {
    BestP <- model$env$last.par.best
    fit <- nlminb(BestP, model$fn, model$gr, control=list(eval.max=10000,iter.max=1000))
   }

 # extract results
 rep <- sdreport(model)
 rep2 <- summary(rep)
 print(rep2)

 Results <- NULL
 Results$B <- model$report()$B[1:Nyear]
 Results$C <- Catch
 Results$t <- Years
 Results$I <- IndexData
 Results$Ihat <- model$report()$Ihat
 Results$Chat <- model$report()$Chat
 r <- rep2[row.names(rep2) == "r",1]
 k <- rep2[row.names(rep2) == "k",1]
 z <- rep2[row.names(rep2) == "z",1]
 MSYL <- rep2[row.names(rep2) == "MSYLOut",1]
 Bcurr <- model$report()$B[Nyear+1]

# ################################################################################

 # Project using the hockey stick control rule
 Ftarg <- r*(1.0-Btarg^z)
 Nproj <- 50
 Bproj <- rep(0,Nproj+1)
 Cproj <- rep(0,Nproj)
 Bproj[1] <- Bcurr
 for (Iyear in 1:Nproj)
  {
   if (Bproj[Iyear]/k <= HCRalpha)
    Fval <- 0
   else
    if (Bproj[Iyear]/k < HCRbeta)
     Fval <- Ftarg*(Bproj[Iyear]/k-HCRalpha)/(HCRbeta-HCRalpha)
    else
     Fval <- Ftarg
   Cproj[Iyear] <- Bproj[Iyear]*Fval
   Bproj[Iyear+1] <- Bproj[Iyear] + r*Bproj[Iyear]*(1.0-(Bproj[Iyear]/k)^z)-Cproj[Iyear]
   if (Bproj[Iyear+1] < 1) Bproj[Iyear+1] <- 1
  }
 Results$Cproj <- Cproj

# ################################################################################
# Output read by setRBC in atManageTier.c (rows "Maximum gradient component:", "MSY", "k", "Binit", "Bcurr", "RBC")

 write.table(rep2, 'dynTier4.rep')
 write.table(data.frame(max(abs(rep$gradient.fixed)),row.names="Maximum gradient component:"), 'dynTier4.rep', append = TRUE,col.names=F)
 write.table(data.frame(Results$B[1],row.names="Binit"), 'dynTier4.rep', append = TRUE,col.names=F)
 write.table(data.frame(Bcurr,row.names="Bcurr"), 'dynTier4.rep', append = TRUE,col.names=F)
 write.table(data.frame(Results$Cproj[1],row.names="RBC"), 'dynTier4.rep', append = TRUE,col.names=F)
 write("Catch projection", 'dynTier4.rep', append = TRUE,ncol=1)
 write(t(Cproj), 'dynTier4.rep', append = TRUE,ncol=1)
 write("Biomass projection", 'dynTier4.rep', append = TRUE,ncol=1)
 write(t(Bproj), 'dynTier4.rep', append = TRUE,ncol=1)
 for (Index in 1:Ncpue)
  {
   write("Index", 'dynTier4.rep',append = TRUE,ncol=2)
   write(Index, 'dynTier4.rep',append = TRUE,ncol=2)
   write(t(cbind(IndexData[,Index],Results$Ihat[,Index])), 'dynTier4.rep', append = TRUE,ncol=2)
  }
 write("Biomass", 'dynTier4.rep', append = TRUE,ncol=1)
 write(t(cbind(1:Nyear,Results$B)), 'dynTier4.rep', append = TRUE,ncol=2)
 write("Catch", 'dynTier4.rep', append = TRUE,ncol=1)
 write(t(cbind(1:Nyear,Catch,Results$Chat)), 'dynTier4.rep', append = TRUE,ncol=3)

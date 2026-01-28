# Runs a series of tests on Atlantis output to signify "passing"
#
#

tests <- function() {
  # check the console output for error messages to manually fail the workflow
  # read in the out.txt file
  lines <- readLines("example/out.txt")
  print(tail(lines, 10))
  # find line with missing parameter error
  error_line <- grepl("Could not find parameter", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis Failed to run due to a missing parameter ")
    stop(error_message)
  }

  # find line with fopen error (often file not found)
  error_line <- grepl("fopen: Can't open", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to a missing forcing file ")
    stop(error_message)
  }

  # find line with cannot open error (often file not found)
  error_line <- grepl("Cannot open", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to a missing parameter file ")
    stop(error_message)
  }

  # find line with poorly defined shell script
  error_line <- grepl("Util_Usage: atlantis", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to a poorly defined shell script")
    stop(error_message)
  }

  # find line with poorly defined geometry
  error_line <- grepl("readBMphysInfo: Number of boxes", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to a poorly geometry")
    stop(error_message)
  }

  # find line with poorly defined geometry
  ## NEED TO LOOK INTO THIS. FALSELY REPORTS FAILED RUN
  error_line <- grepl("skipToKeyEnd:", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message(
      "Atlantis failed to run due to issues with box faces and other errors"
    )
    #    stop(error_message)
  }
  # find line with group.csv issues
  error_line <- grepl("Set_Tracer_Index", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to issues with the groups.csv file")
    message(
      "Check groups.csv age class group definition. This could also be in the parameter file"
    )
    stop(error_message)
  }

  # find line with ERROR. Colmumn numbers
  error_line <- grepl("ERROR", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to issues with the groups.csv file")
    message("Looks like you may the wrong number of columns specified")
    stop(error_message)
  }

  # find line with translation error
  error_line <- grepl("Util_Read_Functional_Group_XML", lines)
  if (any(TRUE %in% error_line)) {
    # if any line contains the error then fail the workflow
    error_message <- lines[error_line]
    message("Atlantis failed to run due to issues with the groups.csv file")
    message("Not sure why! Please check the groups.csv file")
    stop(error_message)
  }

  ## Now check the log file and make sure it finished running correctly
  # Maybe not all errors are captured in the above section

  # Read in log.file
  lines <- readLines("example/testFolder/log.txt")
  # select last line
  last_line <- tail(lines, n = 1)
  # trim white space
  last_line <- trimws(last_line)
  # check if last line is "Atlantis Done"
  if (last_line != "Atlantis Done") {
    print(lines)
    stop(
      "Atlantis did not finish running: Don't know why, please check the workflow log"
    )
  } else {
    message("Atlantis Ran Successfully")
    message("Now performing diagnostics on the output files")
    ## Test: Run previous version and compare output
    # via a suite of diagnostics.
    # If fail then create an issue and attach table of diagnostics
    # and assign to the developer
    # If pass then create a new version and push to the repo
  }
}

# https://stackoverflow.com/questions/48886065/rcpp-does-not-find-rtools
library(installr)
install.Rtools(choose_version = TRUE) # C:\RTools\3.4 (3.5 not working)

# https://github.com/r-lib/devtools/issues/1772
library(devtools)
assignInNamespace("version_info", c(devtools:::version_info, list("3.5" = list(version_min = "3.3.0", version_max = "99.99.99", path = "bin"))), "devtools")
find_rtools() # is TRUE now

library(Rcpp)
evalCpp("1+1")

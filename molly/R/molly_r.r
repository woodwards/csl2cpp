# run molly.cpp via Rcpp and check against molly_cpp_output.tsv

library(Rcpp)
library(tidyverse)

# cpp <- read_tsv("molly_cpp_output.tsv",
# 				col_names=c("nsteps", "system_time", "time", "xd", "x"))

cat("sourceCpp(main_r.cpp)\n")
sourceCpp("main_r.cpp")

start_time <- 0.0
time_step <- 0.1
end_time <- 4.0

# retrieve initial conditions
initialise_model( start_time ) # does not reset rate and aux variables
pull_variables_from_model()
x00 <- get_molly_variables()
unique(x00)
x00[is.nan(x00)]
x00[is.infinite(x00)]

# loop through time
cat("start simulation loop\n")
start_timer <- Sys.time()
out_times <- seq( start_time , end_time , time_step )
xx <- vector("list", length(out_times))
i <- 1
for ( i in 1:length(out_times) ){
  	advance_model( out_times[i] , time_step / 10.0 )
	pull_variables_from_model()
	x <- get_molly_variables()
	xx[[i]] <- as.list(x)
}
print(Sys.time() - start_timer)
xx <- bind_rows(xx) # collect output
# keep columns that don't change
# colvals <- sapply(xx, function(x) length(unique(x)), simplify=TRUE)
# cols <- which(colvals!=1)
# xx <- xx[,cols]

# get zero state
x0 <- as.numeric(xx[1,])
names(x0) <- names(xx)
x0[!is.finite(x0)]
x0["RQEQ"]

# get final state
x1 <- as.numeric(xx[nrow(xx),])
names(x1) <- names(xx)
names(xx)[!is.finite(x1)]
x1["dCd"]

# p1 <- ggplot() +
# 	geom_point(data=cpp, mapping=aes(x=x, y=xd), colour="blue") +
# 	geom_path(data=xx, mapping=aes(x=x, y=xd), colour="red")
# print(p1)


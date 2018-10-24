# run molly.cpp via Rcpp and check against molly_cpp_output.tsv

library(Rcpp)
library(tidyverse)

cpp <- read_tsv("molly_cpp_output.tsv",
				col_names=c("nsteps", "t", "dEating", "WtPUter", "LhorAdip")) %>%
	gather(key, value, -t, -nsteps)

cat("sourceCpp(main_r.cpp)\n")
sourceCpp("main_r.cpp")
# sourceCpp("main_r.cpp", rebuild=TRUE)

start_time <- 0.0
time_step <- 1
end_time <- 300.0

# initialise
# does not reset constants, only reruns initialise_model() and initialises state variables
# FIXME maybe we should also run discrete and derivative etc at this point
initialise_model( start_time )

# # retrieve initial conditions
# pull_variables_from_model()
# x00 <- get_molly_variables()
# x00[!is.finite(x00)]
#
# # advance to start_time
# advance_model( start_time , time_step / 100.0 )
# pull_variables_from_model()
# x11 <- get_molly_variables()
# x11[!is.finite(x11)]
#
# # advance to 1 day
# advance_model( start_time + 1.0 , time_step / 100.0 )
# pull_variables_from_model()
# x22 <- get_molly_variables()
# x22[!is.finite(x22)]

# loop through time
cat("start simulation loop\n")
start_timer <- Sys.time()
out_times <- seq( start_time , end_time , time_step )
xx <- vector("list", length(out_times))
i <- 1
for ( i in 1:length(out_times) ){
  	advance_model( out_times[i] , time_step / 100.0 )
	pull_variables_from_model()
	x <- get_molly_variables()
	xx[[i]] <- as.list(x)
}
print(Sys.time() - start_timer)
xx <- bind_rows(xx) # collect output

# keep some columns that change
colvals <- sapply(xx, function(x) length(unique(x)), simplify=TRUE)
cols <- which(colvals!=1)
cols <- sample(cols, 6)
cols <- which(names(colvals) %in% c("t", "dEating", "WtPUter", "LhorAdip", "dLowMfDecay", "dNonUterEBW"))
xx2 <- xx[,cols] %>%
	gather(key, value, -t)

# get zero state
x0 <- as.numeric(xx[1,])
names(x0) <- names(xx)
x0[!is.finite(x0)]

# get final state
x1 <- as.numeric(xx[nrow(xx),])
names(x1) <- names(xx)
names(x1)[!is.finite(x1)]

p1 <- ggplot() +
	labs(title="Some outputs from Molly.cpp!") +
	geom_point(data=cpp, mapping=aes(x=t, y=value, colour=key), pch=1, size=2, alpha=0.3) +
	geom_path(data=xx2, mapping=aes(x=t, y=value, colour=key), size=1) +
	# coord_cartesian(ylim=c(0.001, 1000)) +
	scale_y_sqrt()

print(p1)


# run molly.cpp via Rcpp and check against molly_cpp_output.tsv

library(Rcpp)
library(tidyverse)
library(scales)

# get old results for comparison
cpp <- read_tsv("molly_cpp_output.tsv",
				col_names=c("nsteps", "t", "dEating", "WtPUter", "LhorAdip")) %>%
	gather(key, value, -t, -nsteps)

# compile model
cat("sourceCpp(main_r.cpp)\n")
Sys.setenv("PKG_CXXFLAGS"="-Wno-reorder") # suppress array initialisation warning
# or put PKG_CXXFLAGS = -Wno-reorder in C:\Users\WoodwardS\Documents\.R\Makevars
sourceCpp("main_r.cpp")
# sourceCpp("main_r.cpp", rebuild=TRUE)

# initialise model
# does not reset constants, only reruns initialise_model() and initialises state variables
# FIXME maybe we should also run discrete and derivative etc at this point
start_time <- 0.0
output_step <- 1
time_step <- output_step / 10.0
end_time <- 300
cat("initialise model\n")
initialise_model( start_time , TRUE )
x00 <- get_molly_variables()
x00[!is.finite(x00)]
# loop through time
cat("start simulation loop\n")
start_timer <- Sys.time()
out_times <- seq( start_time , end_time , output_step )
xx <- vector("list", length(out_times))
xx[[1]] <- as.list(x00)
i <- 2
nsteps <- 0
for ( i in 2:length(out_times) ){
  	nsteps <- nsteps + advance_model( out_times[i] , time_step )
	x <- get_molly_variables()
	xx[[i]] <- as.list(x)
}
print(Sys.time() - start_timer)
xx <- bind_rows(xx) # collect output

# keep some columns
# colvals <- sapply(xx, function(x) length(unique(x)), simplify=TRUE)
# cols <- which(colvals!=1)
# cols <- sample(cols, 6)
getvars <- c("t", "dEating", "WtPUter", "LhorAdip", "ResidMamMilk") # to compare to external results
# getvars <- c("t", "NonUterEBW", "dNonUterEBW", "iBW", "iFdRat", "WtGrvUter", "EBW1", "WtOth","WtAdip","WtVis","WaPool","WaPoolTarget")
# getvars <- c("t", "LowMfDecay", "dLowMfDecay", "kMamCellsUsMfDecay", "CumulativeLowMfDays")
cols <- which(names(xx) %in% getvars)
xx1 <- xx[,cols]
View(xx1)
xx2 <- xx1 %>% gather(key, value, -t)

# check initial state
x0 <- as.numeric(xx[1,])
names(x0) <- names(xx)
x0[!is.finite(x0)]

# check final state
x1 <- as.numeric(xx[nrow(xx),])
names(x1) <- names(xx)
names(x1)[!is.finite(x1)]

# plot traces
halfway <- 2
trans_atan <- trans_new(name="atan",
						transform=function(x) atan(x/halfway),
						inverse=function(y) halfway*tan(y))
p1 <- ggplot() +
	labs(title="Some outputs from Molly.cpp!", y="Combined scale") +
	geom_point(data=cpp, mapping=aes(x=t, y=value, colour=key), pch=1, size=2, alpha=0.3) +
	geom_path(data=xx2, mapping=aes(x=t, y=value, colour=key), size=1) +
	# coord_cartesian(ylim=c(0.0, 50)) +
	coord_trans(y=trans_atan) +
	scale_y_continuous(breaks=seq(-100,100,10))
print(p1)


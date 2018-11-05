# run molly.cpp via Rcpp and check against molly_cpp_output.tsv

library(Rcpp)
library(tidyverse)
library(scales)

# change to UTF-8 encoding!!!
csl0 <- read_tsv("molly_csl_output.tsv",
				skip=1,
				col_names=c("t", "BCS",
							"t2", "BW",
							"t3", "dMilkProd",
							"t4", "IntakeDay",
							"t5", "WtAdip",
							"t6", "WtGrvUter")) %>%
	select(-t2, -t3, -t4, -t5, -t6)
csl <- csl0 %>%
	gather(key, value, -t)

# compile model
cat("sourceCpp(main_r.cpp)\n")
Sys.setenv("PKG_CXXFLAGS"="-Wno-reorder") # suppress array initialisation warning
# or put PKG_CXXFLAGS = -Wno-reorder in C:\Users\WoodwardS\Documents\.R\Makevars
sourceCpp("main_r.cpp")
# sourceCpp("main_r.cpp", rebuild=TRUE)

# initialise model
cat("initialise model\n")
start_time <- 0.0
output_step <- 1
time_step <- 0.005
end_time <- 305
out_times <- seq( start_time , end_time , output_step )
xx <- vector("list", length(out_times))
debug = FALSE
initialise_model( start_time , debug )
x00 <- get_molly_variables()
# x00[!is.finite(x00)]
cat("start simulation loop\n")
start_timer <- Sys.time()
i <- 1
nsteps <- 0
for ( i in 1:length(out_times) ){
  	nsteps <- nsteps + advance_model( out_times[i] , time_step )
	x <- get_molly_variables()
	# x[!is.finite(x)]
	# stopifnot(all(is.finite(x)))
	xx[[i]] <- as.list(x)
}
print(Sys.time() - start_timer)
xx <- bind_rows(xx) # collect output

# keep some columns
getvars <- c("t", "BCS", "BW", "dMilkProd", "IntakeDay", "WtAdip", "WtGrvUter")
cols <- which(names(xx) %in% getvars)
xx1 <- xx[,cols]
# View(xx1)
xx2 <- xx1 %>% gather(key, value, -t) %>%
	arrange(key)

# plot traces
halfway <- 20
trans_atan <- trans_new(name="atan",
						transform=function(x) atan(x/halfway),
						inverse=function(y) halfway*tan(y))
p1 <- ggplot() +
	labs(title="Circles = Molly.csl, Lines = Molly.cpp", y="Combined scale") +
	geom_point(data=csl, mapping=aes(x=t, y=value, colour=key), pch=1, size=2, alpha=0.1) +
	geom_path(data=xx2, mapping=aes(x=t, y=value, colour=key), size=1) +
	# coord_cartesian(ylim=c(0.0, 50)) +
	coord_trans(y=trans_atan) +
	scale_y_continuous(breaks=seq(-100,100,10))
print(p1)


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

initialise_model( start_time )

pull_variables_from_model()
x <- get_molly_variables()
View(x)

out_times <- seq( start_time , end_time , time_step )

cat("start simulation loop\n")
start_timer <- Sys.time()
xx <- vector("list", length(out_times))
initialise_model( start_time )
i <- 1
for ( i in 1:length(out_times) ){
  	advance_model( out_times[i] , time_step / 10.0 )
	pull_variables_from_model()
	x <- get_molly_variables()
	xx[[i]] <- as.list(x)
}
print(Sys.time() - start_timer)

xx <- bind_rows(xx)

pull_variables_from_model()
x <- get_molly_variables()

# p1 <- ggplot() +
# 	geom_point(data=cpp, mapping=aes(x=x, y=xd), colour="blue") +
# 	geom_path(data=xx, mapping=aes(x=x, y=xd), colour="red")
# print(p1)


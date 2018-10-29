# run spring.cpp via Rcpp and check against spring_cpp_output.tsv

library(Rcpp)
library(tidyverse)

cpp <- read_tsv("spring_cpp_output.tsv",
				col_names=c("nsteps", "system_time", "time", "xd", "x"))

cat("sourceCpp(main_r.cpp)\n")
sourceCpp("main_r.cpp")

start_time <- 0.00
time_step <- 0.02
end_time <- 3.99

initialise_model( start_time , TRUE )

pull_variables_from_model()
x <- get_spring_variables()
print(x)

out_times <- seq( start_time , end_time , time_step )

cat("start simulation loop\n")
start_timer <- Sys.time()
xx <- vector("list", length(out_times))
initialise_model( start_time , TRUE )
pull_variables_from_model()
x <- get_spring_variables()
xx[[1]] <- as.list(x)
i <- 2
for ( i in 2:length(out_times) ){
  	advance_model( out_times[i] , time_step / 10.0 )
	pull_variables_from_model()
	x <- get_spring_variables()
	xx[[i]] <- as.list(x)
}
print(Sys.time() - start_timer)

xx <- bind_rows(xx)

p1 <- ggplot() +
	geom_point(data=cpp, mapping=aes(x=x, y=xd), colour="blue") +
	geom_path(data=xx, mapping=aes(x=x, y=xd), colour="red")
print(p1)


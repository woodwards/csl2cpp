# csl2cpp translator script
# Simon Woodward, DairyNZ Ltd, 2018
# please read the documentation
# note: the CSL file might require minor changes
# note: the resulting CPP file also might require minor changes

# libraries
library(tidyverse)

# options
log_file <- FALSE # log to console or log.txt file?
m_files <- c()

# specify project
input_dir <- "spring"
csl_file <- "Spring.csl"
output_dir <- "spring"
model_name <- "spring"

# input_dir <- "molly"
# input_dir <- "I:/Molly/Molly 910/Molly DNZ"
# # input_dir <- "I:/Simon/Projects/Molly_Translation/Molly 900s/Molly DNZ"
# csl_file <- "../Molly.csl"
# # csl_file <- "Molly3.csl"
# # m_files <- c("Molly6a.m") # optional
# # output_dir <- "molly"
# output_dir <- "molly910"
# model_name <- "molly"

# input_dir <- "mindy"
# csl_file <- "../Molly.csl"
# output_dir <- "mindy"
# model_name <- "molly"

# input_dir <- "mdh"
# csl_file <- "Molly5yTest.csl"
# m_files <- c("Molly5z.m") # needed
# output_dir <- "mdh"
# model_name <- "molly"

# log file on
while (sink.number()>0) {
  sink()
}
if (log_file){
  log_file_name <- paste(output_dir, "/log.txt", sep="")
  cat("sinking console output to", log_file_name, "\n")
  sink(file=log_file_name)
}

# get code
cat("reading code", "\n")
source("csl2cpp_read.r") # load functions
csl <- read_csl(input_dir, csl_file, m_files) # read lines

# write aggregated raw csls
file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
if (file_name == "Molly.csl"){
  source("csl2cpp_write.r") # load functions
  csl2 <- paste(csl$code, "\n", sep="")
  write_cpp(csl2, output_dir, paste(model_name, "2", sep=""), "csl") # write aggregated raw csl
  csl3 <- str_replace_all(csl2, "(?<=([0-9]\\.0{0,5}[1-9]))[0-9]+", "555") # jitter
  write_cpp(csl3, output_dir, paste(model_name, "3", sep=""), "csl") # write jittered aggregated raw csl
}

# checkpoint
temp_file <- paste(output_dir, "checkpoint_after_read.RData", sep="/")
save.image(temp_file) # save progress

# parse and translate
cat("parsing code", "\n")

# separate code into tokens
source("csl2cpp_do_parse_one.r")
# plot code for fun!
 y <- 1:nrow(csl)
 plot1 <- ggplot() +
   labs(title=csl_file, x="Width", y="Line Number") +
   geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
   scale_y_reverse()
 print(plot1)

# translate to C++
source("csl2cpp_do_parse_two.r")

# sort lines and analyse variable dependence
source("csl2cpp_do_parse_three.r")

# make C++ file
temp_file <- paste(output_dir, "checkpoint_after_parse_three.RData", sep="/")
load(temp_file)
cat("making cpp code", "\n")
source("csl2cpp_make.r") # load functions
cpp <- make_cpp(csl, tokens, model_name, delay_post=FALSE)
cpp_df <- enframe(cpp, name=NULL)
cat("writing cpp code", "\n")
source("csl2cpp_write.r") # load functions
write_cpp(cpp, output_dir, model_name, "h")

# log file off
while (sink.number()>0) {
  sink()
}
cat("finished\n")


# csl2cpp translator script
# Simon Woodward, DairyNZ Ltd, 2018

# libraries
library(tidyverse)

# source
csl_file <- "spring/Spring.csl"
csl_file <- "molly/Molly.csl"
csl_file <- "molly/Molly2.csl"
m_files <- c("Params 2014.m")

# split file names
file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation
model_name <- path_name

# get code
cat(file=stderr(), "reading code", "\n")
source("csl2cpp_read.r") # load functions
csl <- read_csl(csl_file, m_files) # read lines

# write aggregated raw csl
if (csl_file == "molly/Molly.csl"){
  csl2 <- paste(csl$code, "\n", sep="")
  csl2 <- str_replace_all(csl2, "(?<=([0-9]\\.0{0,5}[1-9]))[0-9]+", "555") # jitter
  source("csl2cpp_write.r") # load functions
  write_cpp(csl2, path_name, paste(model_name, "2", sep=""), "csl") # write aggregated raw csl
}

# checkpoint
temp_file <- paste(path_name, "checkpoint_after_read.RData", sep="/")
save.image(temp_file) # save progress

# parse and translate
cat(file=stderr(), "parsing code", "\n")

# separate code into tokens
source("csl2cpp_do_parse_one.r")
# plot code for fun!
# y <- 1:nrow(csl)
# plot1 <- ggplot() +
#   labs(title=csl_file, x="Width", y="Line Number") +
#   geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
#   scale_y_reverse()
# print(plot1)

# translate to C++
source("csl2cpp_do_parse_two.r")

# sort lines and analyse variable dependence
source("csl2cpp_do_parse_three.r")

# make C++ code
temp_file <- paste(path_name, "checkpoint_after_parse_three.RData", sep="/")
load(temp_file)
cat(file=stderr(), "making cpp code", "\n")
source("csl2cpp_make.r") # load functions
cpp <- make_cpp(csl, tokens, model_name, delay_post=FALSE)
cpp_df <- as_data_frame(cpp)
cat(file=stderr(), "writing cpp code", "\n")
source("csl2cpp_write.r") # load functions
write_cpp(cpp, path_name, model_name, "cpp")



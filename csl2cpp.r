# csl2cpp translator script
# Simon Woodward, DairyNZ Ltd, 2018

# libraries
library(tidyverse)

# source
csl_file <- "spring/Spring.csl"
csl_file <- "molly/Molly.csl"

# split file names
file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation
model_name <- path_name

# get code
cat(file=stderr(), "reading code", "\n")
source("csl2cpp_read.r") # load functions
csl <- read_csl(csl_file) # read lines
if (csl_file == "molly/Molly.csl"){ # comment out first INITIAL section, this is header
  csl$code[9] <- paste("!", csl$code[9])
  csl$code[594] <- paste("!", csl$code[594])
}
source("csl2cpp_write.r") # load functions
csl2 <- paste(csl$code, "\n", sep="")
write_cpp(csl2, path_name, model_name, "csl2") # write aggregated raw csl
temp_file <- paste(path_name, "checkpoint_after_read.RData", sep="/")
save.image(temp_file) # save progress

# parse code
cat(file=stderr(), "parsing code", "\n")
source("csl2cpp_parse.r") # load functions
source("csl2cpp_do_parse_one.r")
source("csl2cpp_do_parse_two.r")

# plot code for fun!
y <- 1:nrow(csl)
plot1 <- ggplot() +
  labs(title=csl_file, x="Width", y="Line Number") +
  geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
  scale_y_reverse()
print(plot1)

cat(file=stderr(), "making cpp code", "\n")
source("csl2cpp_make.r") # load functions
cpp <- make_cpp(csl, model_name)
cpp_df <- as_data_frame(cpp)

cat(file=stderr(), "writing cpp code", "\n")
write_cpp(cpp, path_name, model_name, "cpp")



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

cat(file=stderr(), "reading code", "\n")
source("csl2cpp_read.r") # load functions
csl <- read_csl(csl_file) # read lines

if (csl_file == "molly/Molly.csl"){ # comment out first INITIAL section, this is header
  csl$code[9] <- paste("!", csl$code[9])
  csl$code[594] <- paste("!", csl$code[594])
}

cat(file=stderr(), "parsing code", "\n")
source("csl2cpp_parse.r") # load functions
temp_file <- paste(path_name, "parse_checkpoint1.RData", sep="/")
save.image(temp_file) # save progress

source("csl2cpp_do_parse_one.r")
temp_file <- paste(path_name, "parse_checkpoint2.RData", sep="/")
save.image(temp_file) # save progress

source("csl2cpp_do_parse_two.r")
temp_file <- paste(path_name, "parse_checkpoint3.RData", sep="/")
save.image(temp_file) # save progress

# plot code for fun!
y <- 1:nrow(csl)
plot1 <- ggplot() +
  labs(title=csl_file, x="Width", y="Line Number") +
  geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
  scale_y_reverse()
print(plot1)

model_name <- path_name
cat(file=stderr(), "making cpp code", "\n")
source("csl2cpp_make.r") # load functions
cpp <- make_cpp(csl, model_name)
cpp_df <- as_data_frame(cpp)

cat(file=stderr(), "writing cpp code", "\n")
source("csl2cpp_write.r") # load functions
write_cpp(cpp, path_name, model_name)


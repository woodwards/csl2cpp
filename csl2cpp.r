# csl2cpp translator
# Simon Woodward, DairyNZ Ltd, 2018

# libraries
library(tidyverse)

# functions
source("csl2cpp_read.r")
source("csl2cpp_parse.r")
source("csl2cpp_make.r")
source("csl2cpp_write.r")

# options
options(warn=2) # raise warnings for testing

# options
csl_file <- "spring/Spring.csl"
# csl_file <- "molly/Molly.csl"
already_preprocessed <- FALSE

# split file names
file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation

# read or load csl
if (!already_preprocessed){ # read from source

  cat(file=stderr(), "reading code", "\n")
  csl <- read_csl(csl_file)   # read lines

  cat(file=stderr(), "parsing code", "\n")
  temp <- parse_csl(csl) # parse lines, returns list(csl, tokens)
  csl <- temp$csl
  tokens <- temp$tokens

  temp_file <- paste(path_name, "parse_final.rds", sep="/")
  cat(file=stderr(), paste("saving", temp_file), "\n")
  saveRDS(temp, file=temp_file) # save data

} else { # read from preprocessed

  temp_file <- paste(path_name, "parse_final.rds", sep="/")
  cat(file=stderr(), paste("loading", temp_file), "\n")
  temp <- readRDS(file=temp_file) # read prepocessed data
  csl <- temp$csl
  tokens <- temp$tokens

}

# plot code for fun!
y <- 1:nrow(csl)
plot1 <- ggplot() +
  labs(title=csl_file, x="Width", y="Line Number") +
  geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
  scale_y_reverse()
print(plot1)

# look at subset
# temp <- csl[match_code(csl, ";"), ]

model_name <- path_name
cat(file=stderr(), "making cpp code", "\n")
cpp <- make_cpp(csl, model_name)
cpp_df <- as_data_frame(cpp)

cat(file=stderr(), "writing cpp code", "\n")
write_cpp(cpp, path_name, model_name)


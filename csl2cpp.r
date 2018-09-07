# csl2cpp translator
# Simon Woodward, DairyNZ Ltd, 2018

# libraries
library(tidyverse)

# functions
source("csl2cpp_functions.r")
source("csl2cpp_read.r")

# options
options(warn=2) # raise warnings for testing

# file names
csl_file <- "spring/Spring.csl"
# csl_file <- "molly/Molly.csl"
file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation
temp_file <- paste(path_name, "parse_results.rds", sep="/")
cpp_file <- paste(path_name, "output.cpp", sep="/")

# read or load csl
if (FALSE){ # read from source
  cat(file=stderr(), "reading code", "\n")
  csl <- read_csl(csl_file, silent=FALSE, drop_comments=FALSE)   # read lines
  cat(file=stderr(), "parsing code", "\n")
  temp <- parse_csl(csl, silent=FALSE, split_lines=FALSE) # parse lines, returns list(csl, tokens)
  cat(file=stderr(), paste("saving", temp_file), "\n")
  saveRDS(temp, file=temp_file) # save data
} else { # read from preprocessed
  cat(file=stderr(), paste("loading", temp_file), "\n")
  temp <- readRDS(file=temp_file) # read prepocessed data
}

# split parse_csl output
csl <- temp$csl
tokens <- temp$tokens

# look at subset
temp <- csl[match_code(csl, ";"), ]

# analyse assignments

# find code patterns (each line is a pattern?)
# do I need to analyse them or just translate them to C++?

# plot code!
y <- 1:nrow(csl)
plot1 <- ggplot() +
  labs(title=csl_file, x="Width", y="Line Number") +
  geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
  scale_y_reverse()
print(plot1)

# multiple statements on a line, continuations
# https://stackoverflow.com/questions/11561856/add-new-row-to-dataframe-at-specific-row-index-not-appended
# or just convert to the C++ syntax

# output declarations, equations, comment, blank lines, identation

# then convert spring.csl and run from R fully

# then add functionality to spring, e.g. event queue, start stop query

# Spring in rcpp/cpp
# Script in cpp? But it's a bit low level. Could also have used python? And convert to python.
# Add features to spring eg events
# Molly token sorting analysis


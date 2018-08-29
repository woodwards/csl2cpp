# csl2cpp translator
# Simon Woodward, DairyNZ Ltd, 2018

library(tidyverse)
source("csl2cpp_functions.r")
source("csl2cpp_read.r")

options(warn=2) # raise warnings for testing

# point to files
# csl_file <- "spring/Spring.csl"
csl_file <- "molly/Molly.csl"
cpp_file <- "molly/output.cpp"

# read lines
csl <- read_csl(csl_file, silent=TRUE, drop_comments=TRUE)

# parse
temp <- parse_csl(csl, silent=FALSE) # returns list(csl, tokens)
tokens <- temp$tokens
csl <- temp$csl
rm(temp)

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


# csl2cpp translator
# Simon Woodward, DairyNZ Ltd, 2018

library(tidyverse)
source("csl2cpp_functions.r")
source("csl2cpp_read.r")

# point to files
# csl_file <- "spring/Spring.csl"
csl_file <- "molly/Molly.csl"
cpp_file <- "molly/output.cpp"

# read lines
csl <- read_csl(csl_file, silent=TRUE, drop_comments=TRUE)

# analyse line types
# determine indent
# gather tokens
nlines <- nrow(csl)
csl$parse_list <- as.character(NA)
csl$line_type <- as.character(NA)
csl$indent <- as.integer(NA)
declaration <- c("constant", "parameter", "cinterval", "integer", "logical")
control <- c("program", "derivative", "termt", "end", "initial", "discrete", "dynamic", "procedural")
indent_plus <- c("program", "derivative", "if", "initial", "discrete", "dynamic", "procedural")
indent_minus <- c("end")
indent <- 0
tokens <- vector("character", 10000)
tokeni <- 1
linei <- 1
cat(file=stderr(), "analysing line type, indent, collecting tokens", "\n")
for (linei in 1:nlines){ # loop through lines

  # parse next line
  if (linei %% 500 == 500 %% 500)
    cat(file=stderr(), linei, "\n")
  parse_list <- code_split(csl$code[linei])
  csl$parse_list[linei] <- obj_to_str(parse_list)[[1]] # save parse list as string

  # identify line type
  type1 <- word(str_to_lower(parse_list[[1]]), 1) # get first item
  value1 <- word(str_to_lower(parse_list[[1]]), 2)
  if (length(parse_list) > 1){
    type2 <- word(str_to_lower(parse_list[[2]]), 1) # get second item
    value2 <- word(str_to_lower(parse_list[[2]]), 2)
  } else {
    type2 <- ""
    value2 <- ""
  }
  csl$line_type[linei] <- case_when(
    type1 == "token" && value1 %in% control ~ value1, # control word
    type1 == "comment" ~  "!", # comment
    type1 == "blank" ~  "", # blank line
    type1 == "token" && type2 == "equals" ~  "assign", # assignment
    type1 == "token" && value1 %in% declaration ~  "declare", # declaration
    TRUE ~ "unknown" # other
    )

  # add indents
  if (type1 == "token" && value1 %in% indent_plus){
    csl$indent[linei] <- indent
    indent <- indent + 1
  } else if (type1 == "token" && value1 %in% indent_minus){
    indent <- indent - 1
    csl$indent[linei] <- indent
  } else {
    csl$indent[linei] <- indent
  }

  # gather tokens
  for (itemi in 1:length(parse_list)){
    typei <- word(str_to_lower(parse_list[[itemi]]), 1)
    valuei <- word(parse_list[[itemi]], 2)
    if (typei == "token"){
      if (FALSE){ # report unique tokens as we go
        if (all(!str_detect(tokens, valuei))){
          cat(file=stderr(), linei, typei, valuei, "\n")
          tokens[tokeni] <- valuei
          tokeni <- tokeni + 1
          stopifnot(tokeni <= length(tokens))
        }
      } else { # fast version
        tokens[tokeni] <- valuei
        tokeni <- tokeni + 1
        stopifnot(tokeni <= length(tokens)) # need more storage?
      }
    }
  }


}

# avoid case sensitivity
# unique_tokens <- sort(unique(tokens), decreasing=TRUE) # prefer upper case
# unique_tokens <- unique_tokens[unique_tokens != ""] # drop ""
# names(unique_tokens) <- str_to_lower(unique_tokens)
# unique_tokens['constant'] # return case sensitive version of token like this

# token table (use rownames)
token_df <- data.frame(name = sort(unique(tokens), decreasing=TRUE), stringsAsFactors = FALSE) %>%
  filter(name > "") %>% # drop blank
  mutate(
    lower = str_to_lower(name),
    type = NA,
    set_line = NA, # first time set
    integ_line = NA, # if integrated
    rate = NA, # if integrated
    initial_value = NA, # if integrated
    const_value = NA # if constant
  ) %>%
  distinct(lower, .keep_all = TRUE) %>% # drop duplicates
  arrange(name) %>%
  column_to_rownames("lower") # index using lower case names

# drop comments and blanks for debugging
if (FALSE){
  csl <- csl %>%
    filter(line_type != "blank", line_type != "comment")
}

# split multiple statements on a line

# output declarations, equations, comment, blank lines, identation

# then convert spring.csl and run from R

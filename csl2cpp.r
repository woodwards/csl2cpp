# csl2cpp translator
# Simon Woodward, DairyNZ Ltd, 2018

library(tidyverse)
source("csl2cpp_functions.r")

# point to files
csl_file <- "Spring.csl"
cpp_file <- "temp.csl"

# read lines
csl <- read_lines(csl_file) %>%
  iconv(to="ASCII//TRANSLIT") %>% # remove accents
  as_tibble() %>%
  rename(code=value) %>%
  mutate(
    line_number=1:n()
  )

# analyse line types
# determine indent
# gather tokens
nlines <- nrow(csl)
csl$parse_list <- as.character(NA)
csl$line_type <- as.character(NA)
csl$indent <- as.integer(NA)
linei <- 1
indent <- 0
reserved <- c("program", "derivative", "cinterval", "constant", "termt", "end")
indent_plus <- c("program", "derivative")
indent_minus <- c("end")
tokens <- vector("character", 50)
tokeni <- 1
for (linei in 1:nlines){ # loop through lines

  # parse next line
  # cat(file=stderr(), linei, "\n")
  parse_list <- code_split(csl$code[linei])
  csl$parse_list[linei] <- capture.output(dput(parse_list))[[1]] # save parse list as string

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
    type1 == "token" && value1 %in% reserved ~ value1, # reserved word
    type1 == "comment" ~  "comment", # comment
    type1 == "blank" ~  "blank", # blank line
    type1 == "token" && type2 == "assign" ~  "assign", # assignment
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
    valuei <- word(str_to_lower(parse_list[[itemi]]), 2)
    if (typei == "token"){
      if (all(!str_detect(tokens, valuei))){
        cat(file=stderr(), linei, typei, valuei, "\n")
        tokens[tokeni] <- valuei
        tokeni <- tokeni + 1
      }
    }
  }


}
token_df <- tibble(token=unique(tokens))

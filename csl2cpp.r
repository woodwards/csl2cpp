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
csl <- csl %>%
  mutate(
    parse_list = NA,
    line_type = NA,
    indent = NA,
    length = NA
  )

# analyse line types
# determine indent
# gather tokens
cat(file=stderr(), "parsing code for line type, tokens, indent", "\n")
tokens      <- vector("character", 10000)
tokens_line <- vector("integer", 10000)
tokeni <- 1
indent <- 0
declaration <- c("constant", "parameter", "cinterval", "integer", "logical", "doubleprecision")
control1 <- c("program", "derivative", "initial", "discrete", "dynamic", "procedural", "terminal", "else") # increase indent
control2 <- c("end", "endif") # decrease indent
control3 <- c("termt", "schedule", "interval", "if") # no change to indent
control <- c(declaration, control1, control2, control3)
continuation <- FALSE
linei <- 1
for (linei in 1:nrow(csl)){ # loop through lines

  # parse next line
  if (linei %% 500 == 500 %% 500)
    cat(file=stderr(), linei, "\n")
  code <- csl$code[linei]
  parse_list <- code_split(code)
  parse_str <- obj_to_str(parse_list)

  # handle semicolons not followed by comment
  semii <- str_detect(parse_list, "^semicolon\\s;$") & !lead(str_detect(parse_list, "^comment\\s!"), 1)
  stopifnot(sum(semii, na.rm=TRUE) <= 1)
  if (sum(semii) == 1){
    cat(file=stderr(), code, "\n")
  }

  # convert parse list to string
  parse_str <- obj_to_str(parse_list)
  csl$parse_list[linei] <- parse_str
  csl$length[linei] <- length(parse_list)

  # multi token controls
  if_then <- str_detect(str_to_lower(parse_str), "token\\sif.+token\\sthen")
  else_if_then <- str_detect(str_to_lower(parse_str), "token\\selse.+token\\sif.+token\\sthen")
  end_if <- str_detect(str_to_lower(parse_str), "token\\send.*token\\sif|token\\sendif")

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
    continuation ~ "continuation",
    if_then ~ "ifthen", # drop _
    else_if_then ~ "elseifthen", # drop _
    end_if ~ "endif", # drop _ to match token endif
    type1 == "token" && value1 %in% control ~ value1, # control word
    type1 == "comment" ~  "!", # comment
    type1 == "blank" ~  "", # blank line
    type1 == "token" && type2 == "equals" ~  "assignment", # assignment
    str_detect(str_to_lower(parse_str), "token integ") ~ "integ", # integ
    TRUE ~ "unknown" # other
    )

  # indent
  if ((type1 == "token" && value1 %in% control1) || if_then || else_if_then){ # increase indent
    csl$indent[linei] <- indent
    indent <- indent + 1
  } else if ((type1 == "token" && value1 %in% control2) || end_if){ # decrease indent
    indent <- indent - 1
    csl$indent[linei] <- indent
  } else { # no change to indent
    csl$indent[linei] <- indent
  }

  # gather tokens
  for (itemi in 1:length(parse_list)){
    if (itemi == 1){
      prevtype1 <- ""
      prevvalue1 <- ""
      type1 <- word(str_to_lower(parse_list[[itemi]]), 1) # get this item
      value1 <- word(parse_list[[itemi]], 2)
    } else {
      prevtype1 <- type1
      prevvalue1 <- value1
      type1 <- type2
      value1 <- value2
    }
    if (itemi < length(parse_list)){
      type2 <- word(str_to_lower(parse_list[[itemi+1]]), 1) # get next item
      value2 <- word(parse_list[[itemi+1]], 2)
    } else {
      type2 <- ""
      value2 <- ""
    }
    if (type1 == "token"){
      tokens[tokeni] <- value1
      tokens_line[tokeni] <- ifelse(type2 == "equals", -linei, linei)
      tokeni <- tokeni + 1
      stopifnot(tokeni <= length(tokens)) # need more storage?
    }
  }

  # continuation?
  if ((type1 == "ampersand") || (prevtype1 == "ampersand" && type1 == "comment")) {
    continuation <- TRUE
  } else {
    continuation <- FALSE
  }


}

# plot code!
y <- 1:nrow(csl)
plot1 <- ggplot() +
  labs(title=csl_file, x="Width", y="Line Number") +
  geom_segment(data=csl, mapping=aes(x=indent, xend=indent+length, y=y, yend=y, colour=file_name)) +
  scale_y_reverse()
print(plot1)

stop("user stop")

# avoid case sensitivity
# unique_tokens <- sort(unique(tokens), decreasing=TRUE) # prefer upper case
# unique_tokens <- unique_tokens[unique_tokens != ""] # drop ""
# names(unique_tokens) <- str_to_lower(unique_tokens)
# unique_tokens['constant'] # return case sensitive version of token like this

# token table (use rownames)
token_df <- data.frame(name = tokens, line = tokens_line, stringsAsFactors = FALSE) %>%
  filter(name > "") %>% # drop blank
  mutate(lower = str_to_lower(name)) %>%
  arrange(name)


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

# split multiple statements on a line

# output declarations, equations, comment, blank lines, identation

# then convert spring.csl and run from R fully

# then add functionality to spring, e.g. event queue, start stop query

# Spring in rcpp/cpp
# Script in cpp? But it's a bit low level. Could also have used python? And convert to python.
# Add features to spring eg events
# Molly token sorting analysis


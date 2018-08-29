# csl2cpp_functions.r

# convert object to string and back
obj_to_str <- function(obj){
  paste(capture.output(dput(obj)), collapse=" ") # paste avoids line breaks
}
str_to_obj <- function(str){
  eval(parse(text=str))
}

# split a line of code into tokens, strings, comments, etc
code_split <- function(code){
  remaining <- code
  outlist <- vector("list", 50)
  outi <- 1
  patterns <- c(
    space="^[:blank:]+",
    comment="^!.*",
    string=paste(c('^\\".*\\"', "^\\'.*\\'"), sep="", collapse="|"),
    token="^[:alpha:]+[[:alnum:]_]*",
    equals="^\\=",
    openbracket="^\\(",
    closebracket="^\\)",
    colon="^:",
    semicolon="^;",
    dollar="^\\$",
    ampersand="^&",
    comma="^,",
    at="^\\.at\\.",
    truefalse=paste("^\\.", c("true", "false"),
                  "\\.", sep="", collapse="|"),
    logical=paste("^\\.", c("or", "and", "not", "eqv", "neqv", "ge", "gt", "le", "lt", "eq", "ne"),
                  "\\.", sep="", collapse="|"),
    # number="^[\\-\\+]?[0-9]*\\.?[0-9]+([ed][\\-\\+]?[0-9]+)?", # https://www.regular-expressions.info/floatingpoint.html
    number="^[0-9]*\\.?[0-9]+([ed][\\-\\+]?[0-9]+)?", # https://www.regular-expressions.info/floatingpoint.html
    math="^[\\/\\*\\^\\-\\+]|^\\*\\*" # how to distinguish +/- from number? do this logic later
  )
  # whole line analysis
  if (str_detect(remaining, "^[:blank:]*$")){ # blank line
    outlist[[outi]] <- "blank"
    outi <- outi + 1
    outlist[[outi]] <- ""
    outi <- outi + 1
    remaining <- ""
  }
  # next item analysis
  while (str_length(remaining) > 0){ # loop through elements
    next_match <- str_match(remaining, regex(patterns, ignore_case=TRUE))[,1]
    matchj <- which(!is.na(next_match)) # all matching patterns
    stopifnot(length(matchj) == 1) # only match one pattern
    matchj <- matchj[[1]] # take first pattern if stopifnot() disabled
    if (names(patterns)[matchj] != "space"){ # don't save space
      outlist[[outi]] <- names(patterns)[matchj]
      outi <- outi + 1
      outlist[[outi]] <- next_match[matchj]
      outi <- outi + 1
    }
    remaining <- str_replace(remaining, regex(patterns[matchj], ignore_case=TRUE), "")
  }
  return(compact(outlist)) # drop NULLs from list
}
# test code_split()
if (FALSE){
  code <- "sev55 = -1 + .TRUE."
  obj_to_str(code_split(code))
  code <- "CONSTANT       xic = 0.0    , xdic = 0.0"
  obj_to_str(code_split(code))
  code = "xdd     =(-mass*-g - a*x)/mass"
  obj_to_str(code_split(code))
  code = ""
  obj_to_str(code_split(code))
}

# parse csl$code for tokens and line types and indent
parse_csl <- function(csl, silent=FALSE){

  if (!silent) cat(file=stderr(), "parsing code for line type, tokens, indent", "\n")

  # add columns to csl
  csl <- csl %>%
    mutate(
      parse_list = NA,
      line_type = NA,
      indent = NA,
      length = NA
    )

  # create regex strings for detection of multi token controls (crude)
  if_then_str <- "token.+if.+token.+then"
  else_if_then_str <- "token.+else.+token.+if.+token.+then"
  end_if_str <- "token.+end.+token.+if"

  # control tokens
  declaration <- c("constant", "parameter", "cinterval", "integer", "logical", "doubleprecision")
  control1 <- c("program", "derivative", "initial", "discrete", "dynamic", "procedural", "terminal", "else") # increase indent
  control2 <- c("end", "endif") # decrease indent
  control3 <- c("termt", "schedule", "interval", "if") # no change to indent
  control <- c(declaration, control1, control2, control3)

  # prepare loop
  tokens      <- vector("character", 10000)
  tokens_line <- vector("integer", 10000)
  tokeni <- 1
  indent <- 0
  continuation <- FALSE
  linei <- 1
  for (linei in 1:nrow(csl)){ # loop through lines

    if (linei %% 500 == 500 %% 500 && !silent) cat(file=stderr(), linei, "\n")

    # parse next line
    code <- csl$code[linei]
    parse_list <- code_split(code)

    # handle semicolons not followed by comment
    # semii <- str_detect(parse_list, "^semicolon\\s;$") & !lead(str_detect(parse_list, "^comment\\s!"), 1)
    # stopifnot(sum(semii, na.rm=TRUE) <= 1)
    # if (sum(semii) == 1){
    #   cat(file=stderr(), code, "\n")
    # }

    # convert parse list to string
    parse_str <- obj_to_str(parse_list)
    csl$parse_list[linei] <- parse_str
    csl$length[linei] <- length(parse_list) / 2

    # detect multi token controls
    if_then <- str_detect(str_to_lower(parse_str), if_then_str)
    else_if_then <- str_detect(str_to_lower(parse_str), else_if_then_str)
    end_if <- str_detect(str_to_lower(parse_str), end_if_str)

    # identify line type
    type1 <- parse_list[[1]] # get first item
    value1 <- str_to_lower(parse_list[[2]])
    if (length(parse_list) > 2){
      type2 <- parse_list[[3]] # get second item
      value2 <- str_to_lower(parse_list[[4]])
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
      type1 == "token" && type2 == "equals" ~  "assignment",
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
    for (itemi in seq(1, length(parse_list)-1, 2)){
      if (itemi == 1){
        prevtype1 <- ""
        prevvalue1 <- ""
        type1 <- parse_list[[itemi]] # get this item
        value1 <- parse_list[[itemi+1]]
      } else {
        prevtype1 <- type1
        prevvalue1 <- value1
        type1 <- type2
        value1 <- value2
      }
      if (itemi + 2 < length(parse_list)){
        type2 <- parse_list[[itemi+2]] # get next item
        value2 <- parse_list[[itemi+3]]
      } else {
        type2 <- ""
        value2 <- ""
      }
      if (type1 == "token"){
        tokens[tokeni] <- value1
        tokens_line[tokeni] <- linei
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

  # token table (use rownames)
  token_df <- data.frame(name = tokens, line = tokens_line, stringsAsFactors = FALSE) %>%
    filter(name > "") %>%
    arrange(name)

  return(list(csl=csl, tokens=token_df))
}

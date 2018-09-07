# csl2cpp_functions.r

# convert object to string
obj_to_str <- function(obj){
  paste(capture.output(dput(obj)), collapse=" ") # paste avoids line breaks
}

# convert string to object
str_to_obj <- function(str){
  eval(parse(text=str))
}

# return lines of csl with code matching pattern
match_code <- function(csl, pattern, ignore_case=FALSE){
  found <- str_detect(
    str_replace(csl$code, ";?[:blank:]*(!.*)?$", ""), # ignore trailing ; and comments
    regex(pattern, ignore_case=ignore_case)
  )
  return(which(found))
}

# insert row into dataframe
# https://stackoverflow.com/questions/11561856/add-new-row-to-dataframe-at-specific-row-index-not-appended
insert_row <- function(existingDF, newrow, r){
  existingDF[seq(r+1,nrow(existingDF)+1),] <- existingDF[seq(r,nrow(existingDF)),]
  existingDF[r,] <- newrow
  existingDF
}

# split a line of code into tokens, strings, comments, etc
code_split <- function(code){
  remaining <- code
  outlist <- vector("list", 50)
  outi <- 1
  patterns <- c(
    space="^[:blank:]+",
    comment="^!.*",
    # string=paste(c('^\\".*\\"', "^\\'.*\\'"), sep="", collapse="|"),
    string="^\\'.*\\'", # ACSL allows single quotes only
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
    bool=paste("^\\.", c("true", "false"), "\\.", sep="", collapse="|"),
    logical=paste("^\\.", c("or", "and", "not", "eqv", "neqv", "ge", "gt", "le", "lt", "eq", "ne"),
                  "\\.", sep="", collapse="|"),
    # number="^[\\-\\+]?[0-9]*\\.?[0-9]+([ed][\\-\\+]?[0-9]+)?", # https://www.regular-expressions.info/floatingpoint.html
    number="^[0-9]*\\.?[0-9]+([ed][\\-\\+]?[0-9]+)?", # https://www.regular-expressions.info/floatingpoint.html
    math="^[\\/\\*\\^\\-\\+]|^\\*\\*"
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
  code = "650.0 65 (65)"
  obj_to_str(code_split(code))
}

# parse csl$code for tokens and line types and indent
parse_csl <- function(csl, silent=FALSE, split_lines=FALSE){

  if (!silent){
    cat(file=stderr(), "parsing code for tokens, line type, indent", "\n")
  }

  # add columns to csl
  csl <- csl %>%
    mutate(
      label = NA,
      head = NA,
      body = NA,
      cont = NA,
      tail = NA,
      split = NA,
      parse_list = NA,
      line_type = NA,
      indent = NA,
      length = NA
    )

  # start of line control tokens (e.g. not including INTEG, GO TO, etc)
  declaration <- c("constant", "algorithm", "nsteps", "maxterval",
                   "parameter", "cinterval", "integer", "logical", "doubleprecision")
  control1 <- c("program", "derivative", "initial", "discrete", "dynamic", "procedural", "terminal", "do") # increase indent
  control2 <- c("end", "endif") # decrease indent
  control3 <- c("termt", "schedule", "interval", "if", "goto") # no change to indent
  control4 <- c("else") # decrease and increase indent
  control <- c(declaration, control1, control2, control3, control4)

  # create regex strings for detection
  integ_str <- "token.+integ"
  if_then_str <- "token.+if.+token.+then"
  else_if_then_str <- "token.+else.+token.+if.+token.+then"
  label_str <- "^[:blank:]*[0-9]+[:blank:]*\\:|^[:blank:]*[:alpha:]+[[:alnum:]_]*[:blank:]*\\:"
  control_str <- paste("^[:blank:]*", control, "(?![[:alnum:]_])", sep="", collapse="|")

  # these statements can be split by comma
  # splitwords <- c("constant", "integer", "logical")
  # splittable <- FALSE

  # prepare loop
  tokens      <- vector("character", 10000)
  tokens_line <- vector("integer", 10000)
  tokeni <- 1
  indent <- 0
  is_continuation <- FALSE
  linei <- 1
  while (linei <= nrow(csl)){ # loop through lines (this allows inserting rows into csl)

    # report progress
    if (linei %% 200 == 1 && !silent){
      cat(file=stderr(), linei, "of", nrow(csl), "\n")
    }

    # get line
    this_line <- csl[linei, ]
    this_line_body <- this_line$code

    # use max() as a compact way to convert NA to ""

    # label
    this_line_label <- max(str_extract(this_line_body, label_str), "", na.rm=TRUE)
    if (this_line_label > ""){
      has_label <- TRUE
      this_line_body <- str_replace(this_line_body, this_line_label, "") # strip label
    } else {
      has_label <- FALSE
    }

    # collapse various items
    this_line_body <- str_replace(this_line_body, regex("end[:blank:]*if", ignore_case=TRUE), "ENDIF")
    this_line_body <- str_replace(this_line_body, regex( "go[:blank:]*to", ignore_case=TRUE), "GOTO")

    # head
    this_line_head <- max(str_extract(this_line_body, regex(control_str, ignore_case=TRUE)), "", na.rm=TRUE)
    if (this_line_head > ""){
      this_line_body <- str_replace(this_line_body, this_line_head, "") # strip head
    }

    # tail
    this_line_tail <- max(str_extract(this_line_body, "!.*$"), "", na.rm=TRUE)
    if (this_line_tail > ""){
      this_line_body <- str_replace(this_line_body, this_line_tail, "") # strip trailing comment
    }

    # continuation
    this_line_cont <- max(str_extract(this_line_body, "&[:blank:]*$"), "", na.rm=TRUE)
    if (this_line_cont > ""){
      this_line_body <- str_replace(this_line_body, this_line_cont, "") # strip trailing &
    }

    # trailing ; or $
    this_line_body <- str_replace(this_line_body, "\\$[:blank:]*$", "")
    this_line_body <- str_replace(this_line_body,   ";[:blank:]*$", "")

    # store
    csl$label[linei] <- this_line_label
    csl$head[linei] <- this_line_head
    csl$body[linei] <- this_line_body
    csl$cont[linei] <- this_line_cont
    csl$tail[linei] <- this_line_tail

    # expand semicolon (easy in Molly)
    pattern <- ";"
    split_semicolon <- split_lines && (length(match_code(csl[linei, ], pattern)) == 1)
    if (split_semicolon){
      this_line <- csl[linei, ]
      csl <- insert_row(csl, this_line, linei + 1) # duplicate this_line
      temp <- str_split(this_line$code, pattern)[[1]]
      csl$split[linei] <- temp[1]
      csl$code[linei+1] <- "! semicolon split"
      csl$split[linei+1] <- temp[2]
    }

    # # expand comma (more complicated)
    # pattern <- ","
    # if (!is_continuation){
    #   str_detect_splitwords <- str_extract(csl$code[linei],
    #                                        regex(paste("^[:blank:]*", splitwords, sep=""),
    #                                              ignore_case=TRUE)
    #                                        )
    #   this_line_head <- str_detect_splitwords[!is.na(str_detect_splitwords)]
    #   splittable <- any(!is.na(str_detect_splitwords))
    #   split_comma <- split_lines && splittable
    # } else{ # continuation
    #   split_comma <- split_lines && splittable
    # }
    # if (split_comma){
    #   # browser()
    #   this_line <- csl[linei, ]
    #   this_line_tail <- str_extract(this_line$code, ";?[:blank:]*&?[:blank:]*(!.*)?$")
    #   this_line_body <- str_replace(this_line$code, ";?[:blank:]*&?[:blank:]*(!.*)?$", "")
    #   this_line_body <- str_replace(this_line_body, regex(paste("^[:blank:]*", this_line_head, sep=""), ignore_case=TRUE), "")
    #   # temp <- str_extract_all(this_line_body, "[[:alnum:][:space:]\\.\\-\\+]+\\=?[[:alnum:][:space:]\\.\\-\\+]*"  )[[1]] # look for assignments
    #   temp <- str_split(this_line_body, pattern)[[1]] # look for commas (beware arrays)
    #   # browser()
    #   if (length(temp) > 1){
    #     for (i in 1:length(temp)){
    #       if (i == 1){
    #         # csl$code[linei+(i-1)] <- # keep this for reference
    #         csl$split[linei]       <- paste(this_line_head, temp[1], this_line_tail)
    #       } else {
    #         csl <- insert_row(csl, this_line, linei+(i-1)) # duplicate this_line
    #         csl$code[linei+(i-1)] <- this_line_tail # carry &
    #         csl$split[linei+(i-1)] <- paste(this_line_head, temp[i])
    #       }
    #     }
    #   }
    # }

    # parse next line
    parse_list <- code_split(str_c(this_line_head, this_line_body))

    # convert parse list to string
    parse_str <- obj_to_str(parse_list)
    csl$parse_list[linei] <- parse_str
    csl$length[linei] <- length(parse_list) / 2

    # detect multi token controls
    has_integ <- str_detect(str_to_lower(parse_str), integ_str)
    else_if_then <- str_detect(str_to_lower(parse_str), else_if_then_str)
    if_then <- str_detect(str_to_lower(parse_str), if_then_str) && !else_if_then

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
      has_integ ~ "integ",
      is_continuation ~ "continuation",
      if_then ~ "ifthen", # drop _
      else_if_then ~ "elseifthen", # drop _
      type1 == "token" && value1 %in% control ~ value1, # control word
      type1 == "comment" ~  "!", # comment
      type1 == "blank" ~  "", # blank line
      type1 == "token" && type2 == "equals" ~  "assignment",
      TRUE ~ "unknown" # other
    )

    # indent
    if ((type1 == "token" && value1 %in% control1) || if_then){ # increase indent
      csl$indent[linei] <- indent
      indent <- indent + 1
    } else if ((type1 == "token" && value1 %in% control4) || else_if_then){ # decrease and increase indent
      indent <- indent - 1
      csl$indent[linei] <- indent
      indent <- indent + 1
    } else if ((type1 == "token" && value1 %in% control2) || has_label){ # decrease indent
      indent <- indent - 1
      csl$indent[linei] <- indent
    } else { # no change to indent
      csl$indent[linei] <- indent + ifelse(is_continuation, 1, 0) # indent continuation
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
    is_continuation <- this_line_cont != ""

    # next line
    linei <- linei + 1

  } # end while

  # token table (use rownames)
  token_df <- data.frame(name = tokens, line = tokens_line, stringsAsFactors = FALSE) %>%
    filter(name > "") %>%
    mutate(lower = str_to_lower(name)) %>%
    group_by(lower) %>%
    summarise(
      name = max(name),
      lines = obj_to_str(line)
      # lines = paste(line, collapse=",")
    )

  return(list(csl=csl, tokens=token_df))
}



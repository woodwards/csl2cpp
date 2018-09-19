# parse csl code and initial analysis

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

  # first pass - split and gather tokens
  csl <- csl %>%
    mutate(
      label = NA,
      head = NA,
      body = NA,
      cont = NA,
      tail = NA,
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
      cat(file=stderr(), "line", linei, "of", nrow(csl), "\n")
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
      this_line_head <- str_to_lower(this_line_head)
    }

    # tail
    this_line_tail <- max(str_extract(this_line_body, "!.*$"), "", na.rm=TRUE)
    if (this_line_tail > ""){
      this_line_body <- str_replace(this_line_body, "!.*$", "") # strip trailing comment
      has_tail = TRUE
    } else {
	  has_tail = FALSE
    }

    # continuation
    this_line_cont <- max(str_extract(this_line_body, "&[:blank:]*$"), "", na.rm=TRUE)
    if (this_line_cont > ""){
      this_line_body <- str_replace(this_line_body, "&[:blank:]*$", "") # strip trailing &
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
    split_semicolon <- split_lines && str_detect(this_line_body, ";")
    if (split_semicolon){
      cat("line", linei, "dropping semicolon", "\n")
      this_line <- csl[linei, ]
      csl <- insert_row(csl, this_line, linei + 1) # duplicate this_line
      temp <- str_split(this_line_body, pattern)[[1]]
      this_line_body <- temp[1]
      csl$body[linei] <- this_line_body
      # create a false line containing  the remaining code
      csl$code[linei+1] <- paste(this_line_label,
                                 this_line_head,
                                 paste(temp[-1], collapse=" ; "),
                                 this_line_cont,
                                 "! *** dropped semicolon")
    }

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
      type1 == "token" && type2 == "equals" ~  "assignment",
      type1 == "blank" & has_tail ~  "comment", # comment only
      type1 == "blank" ~  "blank", # blank line
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

  # token table
  token_df <- data.frame(name = tokens, line = tokens_line, stringsAsFactors = FALSE) %>%
    filter(name > "") %>%
    mutate(lower = str_to_lower(name)) %>%
    group_by(lower) %>%
    summarise(
      name = max(name),
      lines = obj_to_str(line)
      # lines = paste(line, collapse=",")
    )

  # token lists
  token_list <- setNames(token_df$name, token_df$lower)
  reserved <- c("integ") # other reserved words
  reserved <- c(control, reserved) # all reserved words
  token_list[reserved] <- reserved # force reserved words to be lower case
  token_decl_line <- setNames(rep(0L, length(token_list)), names(token_list))
  token_set_line <- setNames(rep(0L, length(token_list)), names(token_list))
  token_set_line[reserved] <- 1 # assume reserved words are set
  token_used_line <- setNames(rep(0L, length(token_list)), names(token_list))

  # second pass -
  csl <- csl %>%
  	mutate(
  	  section = NA,
  	  declare = NA,
  	  init = NA,
  	  calc = NA,
  	  calc = NA,
  	  integ = NA
  	)

  # loop through rows again
  major_sections <- c("initial", "dynamic", "derivative", "discrete", "terminal")
  major_section <- ""
  i <- 1
  for (i in 1:nrow(csl)){

    # identify major section
    if (csl$head[i] %in% major_sections){
      major_section <- csl$head[i]
    }
    csl$section[i] <- major_section

    # get tokens for this line, ensure case sensitive
    parse_list <- unlist(str_to_obj(csl$parse_list[i]))
    j <- which(str_detect(unlist(parse_list), "token")) + 1
    if (length(j)>0){
      parse_list[j] <- token_list[str_to_lower(parse_list[j])] # convert tokens to standard case
    }
    nitems <- length(parse_list) / 2

    # handle various line_types, this is a bit complex!
    if (csl$line_type[i] %in% c("constant", "integer", "doubleprecision", "logical", "parameter")){

      # declaration
      parse_list[1:2] <- c("comma", ",") # insert fake comma in place of keyword
      k <- which( parse_list == "token" & lag(parse_list, 2) == "comma" ) + 1L
      bad <- token_decl_line[parse_list[k]] != 0 # already declared
      if (any(bad)){
        cat("line", i, "redeclaration:", paste(parse_list[k][bad], collapse=" "), "\n")
      }
      csl$declare[i] <- paste(parse_list[k][!bad], collapse=" , ")
      token_decl_line[parse_list[k][!bad]] <- i
      # initialisation (no check for duplication)
      k <- seq(4, nitems * 2, 2)
      csl$init[i] <- str_replace(paste(parse_list[k], collapse=" "), ",", ";")
      # which tokens set
      k <- which(parse_list == "token" & lead(parse_list, 2) == "equals") + 1L
      token_set_line[parse_list[k]] <- i
      # which tokens used
      k <- which( parse_list == "token" & lag(parse_list, 2) != "comma" ) + 1L
      bad <- token_set_line[parse_list[k]] == 0 # not set
      if (any(bad)){
        cat("line", i, "uses unset token:", paste(parse_list[k][bad], collapse=" "), "\n")
      }
      token_used_line[parse_list[k]] <- i

    } else if (csl$line_type[i] %in% c("integ")){

      # warning only handles statements of the form, a = integ( b , c ), no expressions allowed
      k <- c(4, 6, 8, 12, 16, 18)
      if (str_to_lower(paste(parse_list[k], collapse="")) != "=integ(,)na"){
        k <- seq(2, nitems * 2, 2)
        cat("line", i, "unhandled form of integ:", paste(parse_list[k], collapse=" "), "\n")
      }
      j <- which(parse_list == "integ")
      # declare state variable
      if (token_decl_line[parse_list[j-4]] != 0){
        cat("line", i, "use of previously declared variable for integ:", parse_list[j-4], "\n")
      }
      csl$declare[i] <- parse_list[j-4]
      token_decl_line[parse_list[j-4]] <- i
      # initial condition of state variable
      csl$init[i] <- paste(parse_list[c(j-4, j-2, j+8)], collapse=" ")
      token_set_line[parse_list[j-4]] <- i
      if (parse_list[j+7] == "token"){ # could be a number
        if (token_set_line[parse_list[j+8]] == 0){
          cat("line", i, "undefined initial condition in integ:", parse_list[j+8], "\n")
        }
        token_used_line[parse_list[j+8]] <- i
      }
      # integration rate of state variable
      csl$integ[i] <- paste(parse_list[c(j-4, j-2, j+4)], collapse=" ")
      if (parse_list[j+3] == "token"){ # could be a number
        token_used_line[parse_list[j+4]] <- i
      }

    } else if (csl$line_type[i] %in% c("assignment")){

      # implicit declaration of lhs
      k <- which( parse_list == "token" & lead(parse_list, 2) == "equals" ) + 1L
      bad <- token_decl_line[parse_list[k]] != 0 # already declared (actually not bad)
      if (!bad){
        csl$declare[i] <- parse_list[k]
        token_decl_line[parse_list[k]] <- i
      }
      # assignment
      token_set_line[parse_list[k]] <- i
      # initial section should not need sorting
      k <- seq(2, nitems * 2, 2)
      if (major_section == "initial"){
        csl$init[i] <- paste(parse_list[k], collapse=" ")
      } else {
        # dynamic section might need sorting
        csl$calc[i] <- paste(parse_list[k], collapse=" ")
        k <- which(parse_list == "token" & lead(parse_list, 2) != "equals") + 1L
        bad <- token_set_line[parse_list[k]] == 0
        if (any(bad)){
          cat("line", i, "uses unset token:", paste(parse_list[k][bad], collapse=" "), "\n")
        }
        token_used_line[parse_list[k][bad]] <- i
      }

    }

    i <- i + 1

  } # end loop



  # return result
  return(list(csl=csl, tokens=token_df))
}


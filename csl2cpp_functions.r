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
  outlist <- vector("list", 30)
  outi <- 1
  patterns <- c( # special characters .!?\(){}
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
    logical=paste("^\\.", c("or", "and", "true", "false", "not", "eqv", "neqv", "ge", "gt", "le", "lt", "eq", "ne"),
                  "\\.", sep="", collapse="|"),
    number="^[\\-\\+]?[0-9]*\\.?[0-9]+([ed][\\-\\+]?[0-9]+)?", # https://www.regular-expressions.info/floatingpoint.html
    math="^[\\/\\+\\-\\*\\^]|\\*\\*"
  )
  # whole line analysis
  if (str_detect(remaining, "^[:blank:]*$")){ # blank line
    outlist[[outi]] <- "blank"
    outi <- outi + 1
    remaining <- ""
  }
  # next item analysis
  while (str_length(remaining) > 0){ # loop through elements
    next_match <- str_match(remaining, regex(patterns, ignore_case=TRUE))[,1]
    matchj <- which(!is.na(next_match))[[1]] # first matching pattern
    # if (length(matchj) != 1){ # matches 0 or more than 1 pattern
    #   outlist[[outi]] <- paste("unknown", remaining)
    #   outi <- outi + 1
    #   remaining <- ""
    # } else { # matches 1 pattern
      if (names(patterns)[matchj] != "space"){ # don't save space
        outlist[[outi]] <- paste(names(patterns)[matchj], next_match[matchj])
        outi <- outi + 1
      }
      remaining <- str_replace(remaining, regex(patterns[matchj], ignore_case=TRUE), "")
    # }
  }
  # return(capture.output(dput(compact(outlist)))) # return list as string
  return(compact(outlist)) # return list
}
# testing
if (FALSE){
  code <- "sev55 = -1 + .TRUE."
  x <- code_split(code)
  code <- "CONSTANT       xic = 0.0    , xdic = 0.0"
  x <- code_split(code)
  code = "xdd     =(mass*g - k*xd - a*x)/mass"
  x <- code_split(code)
  code = ""
  x <- code_split(code)
}


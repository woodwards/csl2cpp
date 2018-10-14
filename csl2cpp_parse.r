# parse functions

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
  # regex metacharacters are . \ | ( ) [ { ^ $ * + ?
  # regex inside character classes also ^ - \ ]
  # we must ensure only one pattern is matched (not easy!)
  patterns <- c(
    blank="^[:blank:]+",
    comment="^!.*",
    string="^\\'.*\\'", # ACSL allows single quotes only, C++ double quotes for strings
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
    bool=paste("^\\.", c("true", "false",
                         "or", "and", "not", "eqv", "neqv",
                         "ge", "gt", "le", "lt", "eq", "ne"),
                  "\\.", sep="", collapse="|"),
    int="^[\\-\\+]?[0-9]+(?![[0-9]\\.edED])", # https://www.regular-expressions.info/floatingpoint.html
    double="^[\\-\\+]?([0-9]+\\.[0-9]+|[0-9]+\\.|\\.[0-9]+)([edED][\\-\\+]?[0-9]+)?|^[\\-\\+]?[0-9]+([edED][\\-\\+]?[0-9]+)", # https://www.regular-expressions.info/floatingpoint.html
    # number="^[\\-\\+]?[0-9]*\\.?[0-9]+([edED][\\-\\+]?[0-9]+)?", # https://www.regular-expressions.info/floatingpoint.html
    mathop  ="^[\\-\\+](?![[0-9]\\.])|^\\/|^\\*(?![:blank:]*\\*)",
    power="^\\*[:blank:]*\\*|^\\^"
  )
  # whole line analysis
  if (str_detect(remaining, "^$")){ # blank line
    outlist[[outi]] <- "blank"
    outi <- outi + 1
    outlist[[outi]] <- ""
    outi <- outi + 1
  }
  # next item analysis
  while (str_length(remaining) > 0){ # loop through elements
    next_match <- str_match(remaining, regex(patterns, ignore_case=TRUE))[,1] # compare to all patterns
    matchj <- which(!is.na(next_match))
    stopifnot(length(matchj) == 1) # exactly one pattern should match
    pattern <- names(patterns)[matchj]
    if (pattern!="blank"){ # discard blanks
      outlist[[outi]] <- pattern
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
  code <- "sev55 = -1 + .TRUE. -(6+6)"
  obj_to_str(code_split(code))
  code <- "CONSTANT       xic = 0.0    , xdic = 0.0"
  obj_to_str(code_split(code))
  code = "xdd     =(-mass*-g - a*x)/mass"
  obj_to_str(code_split(code))
  code = "650.0 65 (65)"
  obj_to_str(code_split(code))
}


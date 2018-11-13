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
    # getting this right is very difficult!
    int="^[\\-\\+]?[0-9]+(?![0-9])(?!\\.[[0-9][:blank:][$]])(?![\\.]?[edED][\\-\\+]?[0-9]+)", # https://www.regular-expressions.info/floatingpoint.html
    double="^[\\-\\+]?([0-9]+\\.[0-9]+|[0-9]+\\.|\\.[0-9]+)([edED][\\-\\+]?[0-9]+)?(?![:alpha:])|^[\\-\\+]?[0-9]+([edED][\\-\\+]?[0-9]+(?![:alpha:]))", # https://www.regular-expressions.info/floatingpoint.html
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
    if (length(matchj) != 1){ # exactly one pattern should match
      stop(paste("function code_split() can't parse :", code))
    }
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

# avoid as.numeric coercion warnings
as_numeric <- function(x, default=555){
  suppressWarnings(if_else(is.na(as.numeric(x)), default, as.numeric(x)))
}

# ensnakify
ensnakeify <- function(x) {
  x %>%
    iconv(to="ASCII//TRANSLIT") %>% # remove accents
    str_replace_na() %>% # convert NA to string
    str_to_lower() %>% # convert to lower case
    str_replace_all(pattern="[^[:alnum:]]", replacement=" ") %>% # convert non-alphanumeric to space
    str_trim() %>% # trim leading and trailing spaces
    str_replace_all(pattern="\\s+", replacement="_") # convert remaining spaces to underscore
}

# handle min max copysign functions
handle_minmaxsign <- function(parse_list, file_name, line_number){
  odds <- seq(1, length(parse_list)-1, 2)
  kk <- which( parse_list[odds] %in% "token" & parse_list[odds+1] %in% c("min", "max", "copysign")) * 2 - 1
  for (k in kk){
    # search right
    nest <- 0
    j <- k
    repeat {
      j <- j + 2
      if (is.na(parse_list[j])){
        break # just do as much as we can
        # cat("min or max or copysign function across multiple lines not handled\n")
      } else if (parse_list[j] == "openbracket"){
        nest <- nest + 1
      } else if (parse_list[j] == "closebracket"){
        nest <- nest - 1
      } else if (nest == 1 & parse_list[j] == "int"){ # convert nest 1 integers to double
        original <- parse_list[j+1]
        dotted <- paste(original, ".", sep="")
        parse_list[j] <- "double"
        parse_list[j+1] <- dotted
        cat(file_name, line_number, "converted", original, "to", dotted, "in min() or max() or copysign()\n")
      }
      if (nest==0){
        break
      }
    }
  }
  return(parse_list)
}

# handle power operators
handle_pow <- function(parse_list){
  odds <- seq(1, length(parse_list)-1, 2)
  k <- which( parse_list[odds] == "power" ) * 2 - 1
  while (length(k)>0){
    k <- k[[1]]
    # convert operator to comma
    parse_list[k] <- "comma"
    parse_list[k+1] <- ","
    # search right
    nest <- 0
    j <- k
    repeat {
      j <- j + 2
      if (is.na(parse_list[j])){
        stop("power operator across multiple lines not handled")
      } else if (parse_list[j] %in% c("openbracket", "openarray")){
        nest <- nest + 1
      } else if (parse_list[j] %in% c("closebracket", "closearray")){
        nest <- nest - 1
      }
      if ((nest==0) && !(parse_list[j+2] %in% c("openbracket", "openarray")) ){ # not function or array
        break
      }
    }
    parse_list <- append(parse_list, c("closebracket", ")"), j+1)
    # search left
    nest <- 0
    j <- k
    repeat {
      j <- j - 2
      if (is.na(parse_list[j])){
        stop("power operator across multiple lines not handled")
      } else if (parse_list[j] %in% c("closebracket", "closearray")){
        nest <- nest + 1
      } else if (parse_list[j] %in% c("openbracket", "openarray")){
        nest <- nest - 1
      }
      if ((nest==0) && !(parse_list[j-2] %in% c("token"))){ # not function or array
        break
      }
    }
    parse_list <- append(parse_list, c("openbracket", "pow ("), j-1)
    # any more power operators?
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "power" ) * 2 - 1
  }
  return(parse_list)
}

# handle mfile tokens
handle_mfile<- function(parse_list, token_decl_columns, token_decl_rows){
  odds <- seq(1, length(parse_list)-1, 2)
  kk <- which( parse_list[odds] == "token" & parse_list[odds+1] %in% c(pullable, pullable_array) ) * 2 - 1
  for (k in kk){
    if (token_decl_columns[parse_list[k+1]]==""){
      parse_list[k+1] <- paste(model_name, ".variable[\"", parse_list[k+1], "\"]", sep="")
    } else if (token_decl_rows[parse_list[k+1]]==""){
      parse_list[k+1] <- paste("(*", model_name, ".vector[\"", parse_list[k+1], "\"])", sep="")
    } else {
      parse_list[k+1] <- paste("(*", model_name, ".array[\"", parse_list[k+1], "\"])", sep="")
    }
  }
  return(parse_list)
}

# get array dimensions
get_array_size <- function(array_name, arrays, token_decl_value){
  detected_array <- arrays[str_detect(arrays, paste(array_name, "\\(", sep="[:blank:]+"))]
  temp <- str_split(detected_array, "\\s")[[1]]
  this_array_name <- temp[2]
  this_array_dims <- 1
  this_array_row <- 1
  columns <- temp[4]
  if (str_detect(temp[4], "^[0-9]+$")){
    this_array_dim1 <- as.integer(temp[4])
  } else {
    this_array_dim1 <- token_decl_value[temp[4]]
  }
  this_array_dim2 <- 1
  rows <- ""
  if (temp[5] == ","){
    this_array_dims <- 2
    rows <- temp[6]
    if (str_detect(temp[6], "^[0-9]+$")){
      this_array_dim2 <- as.integer(temp[6])
    } else {
      this_array_dim2 <- token_decl_value[temp[6]]
    }
  }
  return(list(name=this_array_name,
              dims=this_array_dims,
              columns=columns,
              dim1=this_array_dim1,
              rows=rows,
              dim2=this_array_dim2))
}

#### second pass - standardise tokens, identify decl/init/calc/integ ####
# thoughts:
# separate information into header, declaration, initialisation, calculation for C++
# have to duplicate information since CSL doesn't have separate declaration section
# combine separate declaration and initialisation of variables in CSL, has to be one line in C++
# CSL constants are not actually constant, just initialised to the value
# convert some CSL tokens into C++ equivalents
# identify lines where tokens are set and used, but only check declarations
# http://www.neurophys.wisc.edu/comp/docs/not017/
# are we going to completely automate translation, or only mostly?
# some things are hard to automate but fairly easy to translate manually (goto, pow)

temp_file <- paste(path_name, "checkpoint_after_parse_one.RData", sep="/")
load(file=temp_file) # recover progress

cat(file=stderr(), "translating declarations, initialisation, calculation", "\n")

csl <- csl %>%
  mutate(
    i = 1:nrow(csl),
    section = NA,
    handled = FALSE,
    static = "", # C++ code pieces
    type = "",
    decl = "",
    ccl = "", # C++11 class constructor list
    init = "",
    disc = "",
    calc = "",
    integ = "",
    dend = "", # declaration section end of line delimiter
    delim = "", # other section end of line delimiter
    set = "", # track variable usage as csv (decl already has variable and constant/parameter declarations)
    used = "",
    mfile = ""
  )

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

# handle min max functions
handle_minmax <- function(parse_list, file_name, line_number){
  odds <- seq(1, length(parse_list)-1, 2)
  kk <- which( parse_list[odds] %in% "token" & parse_list[odds+1] %in% c("min", "max")) * 2 - 1
  for (k in kk){
    # search right
    nest <- 0
    j <- k
    repeat {
      j <- j + 2
      if (is.na(parse_list[j])){
        break # just do as much as we can
        # cat("min or max function across multiple lines not handled\n")
      } else if (parse_list[j] == "openbracket"){
        nest <- nest + 1
      } else if (parse_list[j] == "closebracket"){
        nest <- nest - 1
      } else if (nest == 1 & parse_list[j] == "int"){ # convert nest 1 integers to double
        original <- parse_list[j+1]
        dotted <- paste(original, ".", sep="")
        parse_list[j] <- "double"
        parse_list[j+1] <- dotted
        cat(file_name, line_number, "converted", original, "to", dotted, "in min() or max()\n")
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
      } else if (parse_list[j] == "openbracket"){
        nest <- nest + 1
      } else if (parse_list[j] == "closebracket"){
        nest <- nest - 1
      }
      if (nest==0){
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
      } else if (parse_list[j] == "closebracket"){
        nest <- nest + 1
      } else if (parse_list[j] == "openbracket"){
        nest <- nest - 1
      }
      if (nest==0){
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
  kk <- which( parse_list[odds] == "token" & parse_list[odds+1] %in% pullable ) * 2 - 1
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
  this_array <- arrays[str_detect(arrays, paste("^double", array_name, "\\(", sep="[:blank:]+"))]
  temp <- str_split(this_array, "\\s")[[1]]
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

# change all comments to C++ style
csl$tail <- str_trim(str_replace(csl$tail, "^! ?", "// "))

# seems to be easier to access named lists than dataframe rows
# token_list is used to standardise case of user tokens
# token_list is also used to translate some language keywords
token_list <- setNames(tokens$name, tokens$lower) # indexed by lower case
declaration <- c("constant", "parameter",
                 "algorithm", "nsteps", "maxterval", "minterval", "cinterval",
                 "character", "integer", "logical", "doubleprecision", "real", "dimension")
keyword1 <- c("program", "derivative", "initial", "discrete", "dynamic", "procedural", "terminal", "do") # +if_then, increase indent
keyword2 <- c("end", "endif", "enddo") # decrease indent
keyword3 <- c("termt", "schedule", "interval", "if", "goto", "continue", "sort") # + has_label + if_goto, no change to indent
keyword4 <- c("else") # +else_if_then, decrease and increase indent
keyword <- c(declaration, keyword1, keyword2, keyword3, keyword4)
reserved <- c(keyword, "t", "integ", "derivt",
              "max", "exp", "min", "log", "sqrt",
              "cos", "sin", "acos", "asin", "tan", "atan") # convert these keywords to lower case
reserved <- c(reserved, "fmod", "floor", "int", "bool", "double", "double", "string") # reserve these too
token_list[names(token_list) %in% reserved] <- str_to_lower(token_list[names(token_list) %in% reserved])
token_list[c("then", "end", "endif")] <- c("{", "}", "}") # translate these
token_list[c("mod", "aint")] <- c("fmod", "floor") # translate these
token_list[c("integer", "logical", "doubleprecision", "real", "character")] <- c("int", "bool", "double", "double", "string") # translate these

# keeping track of tokens
# declared, set, used
# need to identify inputs and outputs from statements/blocks of statements
token_decl_line <- setNames(rep(0L, length(token_list)), token_list) # avoid double decl
token_decl_type <- setNames(rep("", length(token_list)), token_list) # record type
token_decl_rows <- setNames(rep("", length(token_list)), token_list) # record array rows
token_decl_columns <- setNames(rep("", length(token_list)), token_list) # record array columns
token_decl_static <- setNames(rep(FALSE, length(token_list)), token_list) # record type
token_decl_value <- setNames(rep(NA, length(token_list)), token_list) # record parameter values

# token role analysis
token_role <- setNames(rep("", length(token_list)), token_list) # record role
token_role[names(token_list) %in% c(reserved, "mod", "aint", "then")] <- "reserved"
labels <- setdiff(str_to_lower(unique(str_replace(csl$label, ":", ""))), "")
token_role[names(token_list) %in% labels] <- "label"
unassignable <- c("reserved", "discrete", "label", "state", "constant", "parameter", "slope", "loop_counter", "constant_array", "dummy", "t")

# boolean and a few other conversions (WARNING: new values come first, old names come second!)
bool_list <- setNames(c("true", "false", "||", "&&", "!", ">=", ">", "<=", "<", "==", "!="),
                      c(".true.", ".false.", ".or.", ".and.", ".not.", ".ge.", ".gt.", ".le.", ".lt.", ".eq.", ".ne."))

# collect arrays
arrays <- ""

# array bound checking (use .at() instead of [])
check_bounds <- TRUE
cat("array bound checking :", check_bounds, "\n")

# loop through rows again
major_sections <- c("initial", "dynamic", "derivative", "discrete", "terminal", "mfile")
major_section_stack <- c("header") # current section is at position 1
i <- 1
for (i in 1:nrow(csl)){

  # report progress
  if ((i %% 200 == 1 || i == nrow(csl)) && !silent){
    cat(file=stderr(), "line", i, "of", nrow(csl), "\n")
  }

  ##### identify major section ####
  if (csl$line_type[i] %in% major_sections){
    # new major section
    major_section_stack <- c(csl$line_type[i], major_section_stack)
    major_section <- major_section_stack[1]
    cat(csl$file_name[i], csl$line_number[i], "major section starts:", major_section, "\n")
    if (major_section == "mfile"){
      # find pullable and pushable variables
      pullable <- tokens$name[token_decl_type %in% c("double", "int", "bool", "auto")]
      pushable <- tokens$name[token_decl_type %in% c("double", "int", "bool", "auto") & token_decl_static==FALSE]
    }
  } else if (csl$line_type[i] == "end" ){
    # check for end of major section
    if (csl$line_type[csl$stack[i]] %in% major_sections){
      major_section <- major_section_stack[1]
      cat(csl$file_name[i], csl$line_number[i], "major section ends:", major_section, "\n")
      major_section_stack <- tail(major_section_stack, -1)
    } else {
      major_section <- major_section_stack[1]
    }
  } else {
    major_section <- major_section_stack[1]
  }
  csl$section[i] <- major_section

  # get tokens for this line
  parse_list <- unlist(str_to_obj(csl$parse_list[i]))

  #### general code translation ####
  # note can these slow the loop down a lot

  # convert user tokens to case sensitive, and translate CSL to C++ keywords
  odds <- seq(1, length(parse_list)-1, 2)
  j <- which(parse_list[odds] == "token") * 2 - 1
  if (length(j)>0){
    parse_list[j+1] <- token_list[str_to_lower(parse_list[j+1])] # convert tokens to standard case
  }

  # convert CSL bool (e.g. .GE.) to C++ (e.g. >=)
  odds <- seq(1, length(parse_list)-1, 2)
  j <- which(parse_list[odds] == "bool") * 2 - 1
  if (length(j)>0){
    parse_list[j+1] <- bool_list[str_to_lower(parse_list[j+1])] # convert bool
  }

  # make concatenated version of original line for use in messages
  odds <- seq(1, length(parse_list)-1, 2)
  parse_str <- paste(parse_list[odds+1], collapse=" ")

  #### change array brackets from (,) to [][], reverse indices, and subtract 1 ####
  # warning CSL has 1-indexing and C++ has 0-indexing
  # also index order is reversed
  # CSL
  # DoublePrecision InitCond(MaxInitValues,MaxAnimals) ! (5,2)
  # Constant InitCond=	1.0,550,3.5,1,4, &
  #                     2.0,625,3.5,5,4
  # FirstEvent=InitCond(4,Animal)
  # C++
  #	std::array< std::array< double , MaxInitValues > , MaxAnimals > InitCond ;
  #	InitCond { { { 1.0 , 550 , 3.5 , 1 , 4 } ,
  #              { 2.0 , 625 , 3.5 , 5 , 4 } } }
  # FirstEvent = InitCond [ Animal - 1 ][ 4 - 1 ] ;
  # FIXME only works if array access all on one line, no brackets or commas inside access
  # C++
  #	std::vector< std::vector< double > > InitCond ; // declare
  #	: InitCond( MaxAnimals , std::vector< double > ( MaxInitValues , 0.0 ) ) , // size
  #	InitCond = { { 1.0 , 550 , 3.5 , 1 , 4 } ,
  #              { 2.0 , 625 , 3.5 , 5 , 4 } } ; // initialise (resizes)
  # note: using %in% works even when lead() or lag() returns NA
  odds <- seq(1, length(parse_list)-1, 2)
  k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "openbracket" ) * 2 - 1
  for (j in k){
    # detect array
    this_array <- max(arrays[str_detect(arrays, paste("^double", parse_list[j+1], "\\(", sep="[:blank:]+"))], "")
    if (this_array > ""){
      temp <- str_split(this_array, "\\s")[[1]]
      this_array_dims <- if_else(temp[5] == ",", 2, 1)
      jj <- j + 2
      parse_list[jj] <- "openarray"
      parse_list[jj+1] <- if_else(check_bounds, ".at(", "[")
      if (this_array_dims == 1){
        odds <- seq(jj+2, length(parse_list)-1, 2)
        jj1 <- odds[which( parse_list[odds] == "closebracket" )[1]]
        parse_list[jj1] <- "closearray"
        parse_list[jj1+1] <- if_else(check_bounds, "- 1 )", "- 1 ]")
      } else if (this_array_dims == 2){
        odds <- seq(jj+2, length(parse_list)-1, 2)
        jj1 <- odds[which( parse_list[odds] == "comma" )[1]]
        parse_list[jj1] <- "arraycomma"
        parse_list[jj1+1] <- if_else(check_bounds, "- 1 ) .at(", "- 1 ][")
        odds <- seq(jj1+2, length(parse_list)-1, 2)
        jj2 <- odds[which( parse_list[odds] == "closebracket" )[1]]
        parse_list[jj2] <- "closearray"
        parse_list[jj2+1] <- if_else(check_bounds, "- 1 )", "- 1 ]")
        # check for unhandled cases
        odds <- seq(jj+2, jj2-2, 2)
        k <- which( parse_list[odds] == "openbracket" )
        if (length(k)>0){
          stop("can't handle brackets in array reference")
        }
        # have to reverse indices
        temp <- parse_list # old parse_list
        # stopifnot(i<2160)
        jj3 <- jj+(jj2-jj1) # new comma position
        from <- seq(jj1+2, jj2-1) # second index becomes the first
        to <- seq(jj+2, jj+2+length(from)-1)
        parse_list[to] <- temp[from]
        from <- seq(jj1, jj1+1) # move comma
        to <- seq(jj3, jj3+1)
        parse_list[to] <- temp[from]
        from <- seq(jj+2, jj1-1) # first index becomes the second
        to <- seq(jj3+2, jj3+2+length(from)-1)
        parse_list[to] <- temp[from]
      }
    }
  }

  # fix string delimiter FIXME only handles simple cases
  odds <- seq(1, length(parse_list)-1, 2)
  k <- which( parse_list[odds] == "string" ) * 2 - 1
  if (length(k)>0){
    parse_list[k+1] <- str_replace_all(parse_list[k+1], "\'", "\"")
  }

  #### identify continuation ####
  is_continuation <- csl$line_type[i] %in% c("continuation")
  will_continue <- csl$cont[i] > ""
  if (is_continuation){
    if (csl$line_type[i-1] %in% c("assign", "arrayassign", "constant", "procedural")){ # might need to handle them differently
      csl$line_type[i] <- csl$line_type[i-1]
    }
  }

  #### mark power operator ####
  odds <- seq(1, length(parse_list)-1, 2)
  k <- which( parse_list[odds] == "power" ) * 2 - 1
  has_power <- length(k)>0
  if (length(k)>0 & !(csl$line_type[i] %in% c("assign", "arrayassign"))){
    message <- paste(csl$file_name[i], csl$line_number[i], ": power operators ** ^ not handled except in assignment lines")
    stop(message)
  }

  #### mark min and max functions ####
  odds <- seq(1, length(parse_list)-1, 2)
  k <- which( parse_list[odds] == "token" & parse_list[odds+1] %in% c("min", "max") ) * 2 - 1
  has_minmax <- length(k)>0
  if (length(k)>0 & !(csl$line_type[i] %in% c("assign", "arrayassign", "integ"))){
    message <- paste(csl$file_name[i], csl$line_number[i], ": min() max() not corrected except in assignment and integ lines")
  }

  #### these tokens must be on their own lines ###
  odds <- seq(1, length(parse_list)-1, 2)
  k <- which( parse_list[odds] == "token" &  parse_list[odds+1] %in% c("integ", "derivt", "schedule", "interval")) * 2 - 1
  if (length(k)>0 & any(parse_list[k+1] != csl$line_type[i])){
    message <- paste(csl$file_name[i], csl$line_number[i], ": these operators must be on their own line : integ derivt schedule interval")
    stop(message)
  }


  #### pause at specific line ####
  # stopifnot(i<4447)

  #### begin handling line types ####

  #### handle line_type = parameter ####
  if (csl$line_type[i] %in% c("parameter")){

    # delete first and last brackets
    k <- setdiff( seq(1, length(parse_list)), c(3, 4, length(parse_list)-1, length(parse_list)))
    parse_list <- parse_list[k]

    # note CSL PARAMETER is fixed, not array, may have type declared previously, a bit like C++ constexpr
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "equals") * 2 - 1 # find variables

    # declare variables
    csl$static[i] <- "static constexpr"
    csl$type[i] <- "auto"
    csl$decl[i] <- paste(parse_list[setdiff(odds+1, 2)], collapse=" ")
    csl$dend[i] <- ";"
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

    # deal with previous declaration of variables
    bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
    if (any(bad)){
      message <- paste("parameter on existing variable:", paste(parse_list[k+1][bad], collapse=" "))
      cat(csl$file_name[i], csl$line_number[i], message, "\n")
      # delete previous declarations
      for (j in parse_list[k+1][bad]){
        # remove j from previous declaration
        temp <- str_trim(str_split(csl$decl[token_decl_line[j]], ",")[[1]]) # previous decl
        temp <- max(str_c(temp[temp != j], collapse=" , "), "") # remove j
        if (temp > ""){ # edit previous decl
          csl$decl[token_decl_line[j]] <- temp # remove j from decl
          csl$type[i] <- csl$type[token_decl_line[j]] # get type (for parameter)
        } else { # delete previous decl
          csl$static[token_decl_line[j]] <- ""
          csl$type[i] <- csl$type[token_decl_line[j]] # get type (for parameter)
          csl$type[token_decl_line[j]] <- ""
          csl$decl[token_decl_line[j]] <- "" # delete decl
          csl$dend[token_decl_line[j]] <- ""
        }
        cat("removed declaration of", j, "on code line", csl$line_number[token_decl_line[j]], "\n")
      }
    }
    token_decl_line[parse_list[k+1]] <- i
    token_decl_type[parse_list[k+1]] <- csl$type[i]
    token_decl_static[parse_list[k+1]] <- TRUE
    token_decl_value[parse_list[k+1]] <- as_numeric(parse_list[k+5])
    token_role[parse_list[k+1]] <- "parameter"
    csl$set[i] <- paste(parse_list[k+1], collapse=",")
    csl$handled[i] <- TRUE

  }
  #### handle line_type = algorithm, nsteps, maxterval, cinterval, minterval ####
  if (csl$line_type[i] %in% c("algorithm", "nsteps", "maxterval", "cinterval", "minterval")){

    # these are control variables that affect dynamic and derivative blocks in ACSL
    # probably not used in C++, treat them the same as parameter
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "equals") * 2 - 1 # find variables

    # declare variables
    csl$static[i] <- "static constexpr"
    csl$type[i] <- "auto"
    csl$decl[i] <- paste(parse_list[setdiff(odds+1, 2)], collapse=" ")
    csl$dend[i] <- ";"
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

    # deal with previous declaration of variables
    bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
    if (any(bad)){
      stop()
      message <- paste("control using existing variable:", paste(parse_list[k+1][bad], collapse=" "))
      cat(csl$file_name[i], csl$line_number[i], message, "\n")
      # delete previous declarations
      for (j in parse_list[k+1][bad]){
        # remove j from previous declaration
        temp <- str_trim(str_split(csl$decl[token_decl_line[j]], ",")[[1]]) # previous decl
        temp <- max(str_c(temp[temp != j], collapse=" , "), "") # remove j
        if (temp > ""){ # edit previous decl
          csl$decl[token_decl_line[j]] <- temp # remove j from decl
          csl$type[i] <- csl$type[token_decl_line[j]] # get type (for parameter)
        } else { # delete previous decl
          csl$static[token_decl_line[j]] <- ""
          csl$type[i] <- csl$type[token_decl_line[j]] # get type (for parameter)
          csl$type[token_decl_line[j]] <- ""
          csl$decl[token_decl_line[j]] <- "" # delete decl
          csl$dend[token_decl_line[j]] <- ""
        }
        cat("removed declaration of", j, "on code line", csl$line_number[token_decl_line[j]], "\n")
      }
    }
    token_decl_line[parse_list[k+1]] <- i
    token_decl_type[parse_list[k+1]] <- csl$type[i]
    token_decl_static[parse_list[k+1]] <- TRUE
    token_decl_value[parse_list[k+1]] <- as_numeric(parse_list[k+5])
    token_role[parse_list[k+1]] <- "parameter"
    csl$set[i] <- paste(parse_list[k+1], collapse=",")
    csl$handled[i] <- TRUE

  }
  #### handle line_type = integer, character, doubleprecision, logical, real, dimension (including arrays) ####
  if (csl$line_type[i] %in% c("integer", "character", "doubleprecision", "logical", "real", "dimension")){

    # these are explicit type declarations
    # other variables are declared implictly by being assigned or initialised using "constant"

    # check for array declaration? FIXME rather simplistic logic
    odds <- seq(1, length(parse_list)-1, 2)
    j <- which( parse_list[odds] == "token" & lead(parse_list[odds],1) %in% "openbracket" ) * 2 - 1
    is_array <- length(j) > 0

    #### (a) array declaration ####
    if (is_array){ # array declaration only

      # commas indicate number of dimensions (FIXME only handles simple cases)
      odds <- seq(1, length(parse_list)-1, 2)
      j <- which( parse_list[odds] == "comma" ) * 2 - 1
      if (length(j) == 0){

        # https://www.learncpp.com/cpp-tutorial/6-15-an-introduction-to-stdarray/
        # std::arrays can only be initialized in the C++11 class constructor list
        # // https://stackoverflow.com/questions/10694689/how-to-initialize-an-array-in-c-objects
        # // https://stackoverflow.com/questions/33714866/initialize-stdarray-of-classes-in-a-class-constructor
        # std::array<int,2> a ; // 2 elements
        # std::array<int,2> a = {{ 13, 18 }} ; // note std::array requires one pair of extra outer braces
        # a[0] // base zero
        # single dimension fixed size array
        message <- paste("detected 1d array:", parse_str)
        arrays <- c(arrays, parse_str)
        cat(csl$file_name[i], csl$line_number[i], message, "\n")
        csl$static[i] <- ""
        # csl$type[i] <- paste("std::array< double ,", parse_list[8], "> ")
        csl$type[i] <- "std::vector< double >" # new version uses std::vector
        csl$decl[i] <- parse_list[4]
        csl$dend[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_decl_line[parse_list[4]] <- i
        token_decl_type[parse_list[4]] <- csl$type[i]
        token_decl_columns[parse_list[4]] <- parse_list[8]
        token_decl_rows[parse_list[4]] <- ""
        csl$handled[i] <- TRUE

      } else if (length(j) == 1){

        # https://stackoverflow.com/questions/17759757/multidimensional-stdarray
        # https://en.cppreference.com/w/cpp/container/array/operator_at
        # std::array<std::array<int,3>,2> a ; // 2 rows, 3 columns
        # std::array<std::array<int,3>,2> a = {{{1,2,3}, {4,5,6}}} ; // note std::array requires one pair of extra outer braces
        # a[2-1][3-1] // note axis order
        # two dimensional fixed size array
        # https://stackoverflow.com/questions/11610338/how-to-initialise-a-member-array-of-class-in-the-constructor
        message <- paste("detected 2d array:", parse_str)
        arrays <- c(arrays, parse_str)
        cat(csl$file_name[i], csl$line_number[i], message, "\n")
        csl$static[i] <- ""
        # csl$type[i] <- paste("std::array< std::array< double ,", parse_list[8], "> ,", parse_list[12], "> ") # need to reverse indices
        csl$type[i] <- "std::vector< std::vector< double > >" # new version uses std::vector
        csl$decl[i] <- parse_list[4]
        csl$dend[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_decl_line[parse_list[4]] <- i
        token_decl_type[parse_list[4]] <- csl$type[i]
        token_decl_columns[parse_list[4]] <- parse_list[8]
        token_decl_rows[parse_list[4]] <- parse_list[12]
        csl$handled[i] <- TRUE

      } else {

        # unhandled
        stop("only handles 1,2 dimensional arrays")

      }

    } # end array declaration

    #### (b) non-array declaration ####
    if (!is_array){

      #
      stopifnot(!is_continuation) # FIXME does not work for is_continuation
      this_type <- parse_list[2] # C++ type
      parse_list[1:2] <- c("comma" , ",") # put fake comma at start of list to make it easier to parse

      # update previous declarations
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lag(parse_list[odds], 1) %in% "comma") * 2 - 1 # find variables
      bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
      if (any(bad)){
        message <- paste("redeclaration:", paste(parse_list[k+1][bad], collapse=" "))
        cat(csl$file_name[i], csl$line_number[i], message, "\n")
        # update previous declaration
        for (j in parse_list[k+1][bad]){
          csl$type[token_decl_line[j]] <- this_type
          cat("updated type of", j, "on code line", csl$line_number[token_decl_line[j]], "\n")
        }
      }
      # declare variables
      if (any(!bad)){
        csl$static[i] <- ""
        csl$type[i] <- this_type
        csl$decl[i] <- paste(parse_list[k+1][!bad], collapse=" , ") # variable list
        csl$dend[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_decl_line[parse_list[k+1][!bad]] <- i
        token_decl_type[parse_list[k+1][!bad]] <- csl$type[i]
      }

    } # end non-array declaration

  }
  #### handle line_type = constant ####
  if (csl$line_type[i] %in% c("constant")){

    # note: a CSL CONSTANT is just an initialiser, values can be changed later (although this will raise a warning)
    # the value is set before the INITIAL section
    # CONSTANT is used to
    # (a) declare and initialise a list of variables (lots of = signs)
    # (b) initialise a declared array (single = on first line) like a C++ list initialiser
    # both of these can have continuation
    # FIXME can't handle simple initialisations that break across lines

    # get array size (array will/must be already declared)
    if (!is_continuation){
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "equals" ) * 2 - 1
      is_array <- length(k) == 1 && any(str_detect(arrays, paste("^double", parse_list[k+1], "\\(", sep="[:blank:]+")))
      if (is_array){ # set array details (carry over to continuation)
        temp <- get_array_size(parse_list[k+1], arrays, token_decl_value)
        this_array_name <- temp$name
        this_array_dims <- temp$dims
        this_array_dim1 <- temp$dim1 # columns!
        token_decl_columns[this_array_name] <- temp$columns
        this_array_dim2 <- temp$dim2 # rows!
        token_decl_rows[this_array_name] <- temp$rows
        this_array_row <- 1
      } else {
        this_array <- ""
      }
    }

    # declare non-array variables
    if (!is_array){ # just copy line and convert , to ; need to handle continuations

      # variables to declare
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "equals") * 2 - 1 # find variables

      # declare variables, breaj continuations onto separate lines
      csl$static[i] <- ""
      csl$type[i] <- "double" # assume double
      j1 <- if_else(is_continuation, 1, 3) # skip "constant"
      j2 <- if_else(will_continue, length(parse_list)-3, length(parse_list)-1) # skip final comma
      csl$decl[i] <- paste(parse_list[seq(j1+1, j2+1, 2)], collapse=" ")
      csl$dend[i] <- ";"
      csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

      # deal with previous declaration of variables
      bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
      if (any(bad)){
        message <- paste("constant on existing variable:", paste(parse_list[k+1][bad], collapse=" "))
        cat(csl$file_name[i], csl$line_number[i], message, "\n")
        # delete previous declarations
        for (j in parse_list[k+1][bad]){
          # remove j from previous declaration
          temp <- str_trim(str_split(csl$decl[token_decl_line[j]], ",")[[1]]) # previous decl
          temp <- max(str_c(temp[temp != j], collapse=" , "), "") # remove j
          if (temp > ""){ # edit previous decl
            csl$decl[token_decl_line[j]] <- temp # remove j from decl
            csl$type[i] <- csl$type[token_decl_line[j]] # get existing type
          } else { # delete previous decl
            csl$static[token_decl_line[j]] <- ""
            csl$type[i] <- csl$type[token_decl_line[j]] # get existing type
            csl$type[token_decl_line[j]] <- ""
            csl$decl[token_decl_line[j]] <- "" # delete decl
            csl$dend[token_decl_line[j]] <- ""
          }
          cat("removed declaration of", j, "on code line", csl$line_number[token_decl_line[j]], "\n")
        }
      }
      token_decl_line[parse_list[k+1]] <- i
      token_decl_type[parse_list[k+1]] <- csl$type[i]
      token_decl_value[parse_list[k+1]] <- as_numeric(parse_list[k+5])
      csl$set[i] <- paste(parse_list[k+1], collapse=",")
      token_role[parse_list[k+1]] <- "constant"
      csl$handled[i] <- TRUE

    } else if (!is_continuation){ # set array first line

      # FIXME assumes each line is an exact row of data
      # // https://stackoverflow.com/questions/10694689/how-to-initialize-an-array-in-c-objects
      # // https://stackoverflow.com/questions/33714866/initialize-stdarray-of-classes-in-a-class-constructor
      # a = {{1,2,3}} ; // note std::array requires one pair of extra outer braces
      # a = {{{1,2,3}, {4,5,6}}} ; // note std::array requires one pair of extra outer braces
      if (max(parse_list[10], "", na.rm=TRUE) == "*"){
        # replicator format for 1d array e.g. MamCellsF = MaxHerds * 0.0
        temp <- paste(rep(parse_list[12], this_array_dim1), collapse=" , ")
        # temp <- paste(parse_list[4], " { { ", temp, " } } ", sep="")
        temp <- paste(parse_list[4], " = { ", temp, " } ", sep="") # new version assigns to std::vector
        # print(temp) # print array parsing
        csl$ccl[i] <- temp
        csl$dend[i] <- ";"
        if (this_array_dim2 > 1){
          stop("illegal array initialisation")
        }
        csl$set[i] <- this_array_name
        token_role[this_array_name] <- "constant_array"
        csl$handled[i] <- TRUE
      } else {
        # explicit array initialisation
        if (length(parse_list) == 6 & will_continue){ # handle empty first lines e.g. CONSTANT Event = &
          # temp <- paste(parse_list[4], " { { ", sep="")
          temp <- paste(parse_list[4], " = { ", sep="") # new version assigns to std::vector
          this_array_row <- this_array_row - 1 # this line doesn't count
        } else { # first line contains a whole row
          # outer <- if_else(this_array_dims == 2, "{ {", "{")
          # ender <- if_else(will_continue, ",", if_else(this_array_dims == 2, "} }", "}"))
          outer <- if_else(this_array_dims == 2, "= {", "=") # new version assigns to std::vector
          ender <- if_else(will_continue, ",", if_else(this_array_dims == 2, "}", "")) # new version assigns to std::vector
          k <- seq(8, length(parse_list), 2)
          if (parse_list[tail(k,1)] == ","){
            k <- seq(8, length(parse_list)-2, 2)
          }
          if ((length(k)+1)/2 != this_array_dim1){
            stop("wrong number of elements in row")
          }
          temp <- paste(parse_list[k], collapse=" ")
          temp <- paste(parse_list[4], " ", outer, " { ", temp, " } ", ender, sep="")
        }
        # print(temp) # print array parsing
        csl$ccl[i] <- temp
        csl$dend[i] <- ifelse(will_continue, "", ";")
        csl$set[i] <- ifelse(will_continue, "", this_array_name)
        token_role[this_array_name] <- "constant_array"
        if (this_array_row == this_array_dim2 & will_continue){
          stop("too many rows")
        } else {
          this_array_row <- this_array_row + 1
        }
        csl$handled[i] <- TRUE
      }

    } else {

      # set array continuation FIXME assumes each line is an exact row of data
      ender <- if_else(will_continue, ",", "}")
      k <- seq(2, length(parse_list), 2)
      if (parse_list[tail(k,1)] == ","){
        # remove trailing comma
        k <- seq(2, length(parse_list)-2, 2)
      }
      if ((length(k)+1)/2 != this_array_dim1){
        stop("wrong number of elements in row")
      }
      temp <- paste(parse_list[k], collapse=" ")
      temp <- paste(" { ", temp, " } ", ender, sep="")
      # print(temp) # print array parsing
      csl$ccl[i] <- temp
      csl$dend[i] <- ifelse(will_continue, "", ";")
      csl$set[i] <- ifelse(will_continue, "", this_array_name)
      if (this_array_row == this_array_dim2 & will_continue){
        stop("too many rows")
      } else {
        this_array_row <- this_array_row + 1
      }
      csl$handled[i] <- TRUE

    }

  }
  #### handle line_type = integ, derivt ####
  if (csl$line_type[i] %in% c("integ", "derivt")){

    # only handles integ and derivt statements of particular simple forms
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

    # check form
    if (str_to_lower(paste(parse_list[c(4, 6, 8, 12, 16, 18)], collapse="")) == "=integ(,)na"){

      jj <- parse_list[2]
      # declare state variable
      if (token_decl_line[jj] != 0){
        message <- paste("previously declared integ:", jj)
        cat(csl$file_name[i], csl$line_number[i], message, "\n")
        # don't declare again
      } else {
        csl$static[i] <- ""
        csl$type[i] <- "double" # state vars are double
        csl$decl[i] <- jj
        csl$dend[i] <- ";"
        token_decl_line[jj] <- i
        token_decl_type[jj] <- csl$type[i]
        token_role[jj] <- "state"
      }
      # initialise even if already initialised
      csl$init[i] <- paste(parse_list[c(2, 4, 14)], collapse=" ")
      csl$delim[i] <- ";"
      csl$set[i] <- jj
      if (parse_list[13] == "token"){ # because could be a numeric constant
        csl$used[i] <- parse_list[14]
      }
      # integration rate of state variable
      csl$integ[i] <- paste(parse_list[c(2, 4, 10)], collapse=" ")
      if (parse_list[9] == "token"){ # could be a number
        token_role[parse_list[10]] <- "rate"
        csl$used[i] <- paste(csl$used[i], parse_list[10], sep=",")
      }
      csl$handled[i] <- TRUE

    } else if (str_to_lower(paste(parse_list[c(4, 6, 8, 12, 16, 18)], collapse="")) == "=derivt(,)na"){

      jj <- parse_list[2]
      # declare variable
      if (token_decl_line[jj] == 0){
        csl$static[i] <- ""
        csl$type[i] <- "double"
        csl$decl[i] <- jj
        csl$dend[i] <- ";"
        token_decl_line[jj] <- i
        token_decl_type[jj] <- csl$type[i]
        token_role[jj] <- "slope"
      }
      # initialise even if already initialised
      csl$init[i] <- paste(parse_list[c(2, 4, 10)], collapse=" ")
      csl$delim[i] <- ";"
      csl$set[i] <- jj
      if (parse_list[9] == "token"){ # because could be a numeric constant
        csl$used[i] <- parse_list[10]
      }
      # calculate numerical derivative
      # temp <- paste("if ( t > t_previous )", parse_list[2], "= (", parse_list[14], "-", paste(parse_list[14], "_previous", sep=""), ") / ( t - t_previous )", collapse=" ")
      # csl$calc[i] <- temp # done in observer not in derivative
      # derivative variable
      csl$integ[i] <- paste(parse_list[c(2, 4, 14)], collapse=" ")
      if (parse_list[13] == "token"){ # could be a number
        csl$used[i] <- paste(csl$used[i], parse_list[14], sep=",")
      }
      csl$handled[i] <- TRUE

    } else {

      # can't handle this type
      odds <- seq(1, length(parse_list)-1, 2)
      message <- paste("unhandled form of integ or derivt:", paste(parse_list[odds+1], collapse=" "))
      cat(csl$file_name[i], csl$line_number[i], message, "\n")

    }

  }
  #### handle line_type = assign, arrayassign (csl) ####
  if (csl$line_type[i] %in% c("assign", "arrayassign") & csl$section[i] != "mfile"){

    # implicit declaration of left hand side (non-array)
    if (!is_continuation){
      if (csl$line_type[i] %in% c("assign")){
        odds <- seq(1, length(parse_list)-1, 2)
        k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "equals" ) * 2 - 1
        if (length(k)>1){
          stop("multiple assignments on a line should have been separated")
        }
        csl$set[i] <- parse_list[k+1]
        if (token_role[parse_list[k+1]] %in% unassignable){
          message <- paste(csl$file_name[i], csl$line_number[i], ": illegal assignment to", token_role[parse_list[k+1]])
          stop(message)
        } else if (token_role[parse_list[k+1]] == ""){
          token_role[parse_list[k+1]] <- "assigned"
        }
        bad <- token_decl_line[parse_list[k+1]] != 0 # already declared (actually not bad)
        if (any(!bad)){ # declare these ones
          for (jj in parse_list[k+1][!bad]){
            csl$static[i] <- ""
            csl$type[i] <- "double"
            csl$decl[i] <- jj
            csl$dend[i] <- ";"
            token_decl_line[jj] <- i
            token_decl_type[jj] <- csl$type[i]
          }
        }
      } else { # arrayassign
        k <- 1
        csl$set[i] <- parse_list[k+1]
        if (token_role[parse_list[k+1]] %in% unassignable){
          message <- paste(csl$file_name[i], csl$line_number[i], ": illegal assignment to", token_role[parse_list[k+1]])
          stop(message)
        } else if (token_role[parse_list[k+1]] == ""){
          token_role[parse_list[k+1]] <- "assigned_array"
        }
      }
    }
    # some assignments have labels
    if (csl$label[i]>""){
      new_label <- paste("label_", csl$label[i], " ", sep="")
    } else {
      new_label=""
    }
    # assignment
    if (has_power){
      parse_list <- handle_pow(parse_list)
    }
    if (has_minmax){
      parse_list <- handle_minmax(parse_list, csl$file_name[i], csl$line_number[i])
    }
    odds <- seq(1, length(parse_list)-1, 2)
    temp <- paste(new_label, paste(parse_list[odds+1], collapse=" "), sep="")
    if (major_section == "header"){
      stop("can't assign in header section")
    } else if (major_section == "initial"){
      csl$init[i] <- temp
      csl$delim[i] <- ifelse(will_continue, "", ";")
    } else if (major_section == "discrete"){ # discrete section
      csl$disc[i] <- temp
      csl$delim[i] <- ifelse(will_continue, "", ";")
    } else { # dynamic section
      csl$calc[i] <- temp
      csl$delim[i] <- ifelse(will_continue, "", ";")
    }
    if (is_continuation | csl$line_type[i] %in% c("assign")){
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which(parse_list[odds] == "token" & !(lead(parse_list[odds], 1) %in% c("equals"))) * 2 - 1 # also catches keywords
      csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
      csl$handled[i] <- TRUE
    } else { # array assign first line
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which(parse_list[odds] == "token" & odds > 1 & !(lead(parse_list[odds], 1) %in% c("equals"))) * 2 - 1 # also catches keywords
      csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
      csl$handled[i] <- TRUE
    }

  }
  #### handle line_type = assign, arrayassign (mfile) ####
  if (csl$line_type[i] %in% c("assign", "arrayassign") & csl$section[i] == "mfile"){

    # implicit declaration of left hand side (non-array)
    if (csl$line_type[i] %in% c("assign")){
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) %in% "equals" ) * 2 - 1
      if (length(k)>1){
        stop("multiple assignments on a line should have been separated")
      }
    } else { # arrayassign
      odds <- seq(1, length(parse_list)-1, 2)
      k <- 1
    }
    # assignment to class property
    if (has_power){
      parse_list <- handle_pow(parse_list)
    }
    if (has_minmax){
      parse_list <- handle_minmax(parse_list, csl$file_name[i], csl$line_number[i])
    }
    parse_list <- handle_mfile(parse_list, token_decl_columns, token_decl_rows)
    odds <- seq(1, length(parse_list)-1, 2)
    temp <- paste(new_label, paste(parse_list[odds+1], collapse=" "), sep="")
    csl$mfile[i] <- temp
    csl$delim[i] <- ";"

  }
  #### handle line_type = if ####
  if (csl$line_type[i] %in% c("if")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    } else if (major_section == "discrete"){
      csl$disc[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    } else {
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    }
    k <- which( parse_list[odds] == "token" & lead(parse_list[odds],1) %in% "equals" ) * 2 - 1
    this_array_name <- str_extract(paste(parse_list, collapse=" "), "(?<=closebracket...token\\s).+(?=\\sopenarray.{1,100}equals)")
    if (length(k)>0){
      csl$set[i] <- paste(parse_list[k+1], collapse=",")
      if (token_role[parse_list[k+1]] %in% unassignable){
        message <- paste(csl$file_name[i], csl$line_number[i], ": illegal assignment to", token_role[parse_list[k+1]])
        stop(message)
      } else if (token_role[parse_list[k+1]] == ""){
        token_role[parse_list[k+1]] <- "assigned"
      }
      k <- which( parse_list[odds] == "token" & !(lead(parse_list[odds],1) %in% c("equals")) ) * 2 - 1
      csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
      csl$handled[i] <- TRUE
    } else if (this_array_name>"") { # array assignment
      csl$set[i] <- this_array_name
      if (token_role[this_array_name] %in% unassignable){
        message <- paste(csl$file_name[i], csl$line_number[i], ": illegal assignment to", token_role[this_array_name])
        stop(message)
      } else if (token_role[this_array_name] == ""){
        token_role[this_array_name] <- "assigned_array"
      }
      k <- which( parse_list[odds] == "token" & !(lag(parse_list[odds],1) %in% "closebracket") & !(lead(parse_list[odds],1) %in% c("equals")) ) * 2 - 1
      csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
      csl$handled[i] <- TRUE
    } else {
      stop("unhandled if statement")
    }

  }
  #### handle line_type = ifthen, endif, discrete ####
  if (csl$line_type[i] %in% c("ifthen", "endif", "discrete")){

    if (csl$line_type[i] == "discrete"){
      # if ( next_event == "event1" ){
      # schedule.erase(schedule.begin()) // do this elsewhere to avoid accidental trigger
      temp <- paste("if ( next_event == \"", parse_list[4], "\" ) { ", sep="")
      discrete_block <- parse_list[4] # need to know this for INTERVAL command
      token_role[discrete_block] <- "discrete"
    } else {
      odds <- seq(1, length(parse_list)-1, 2)
      temp <- paste(parse_list[odds+1], collapse=" ")
    }
    if (major_section == "initial"){
      csl$init[i] <- temp
      csl$delim[i] <- ""
    } else if (major_section == "discrete"){
      csl$disc[i] <- temp
      csl$delim[i] <- ""
    } else {
      csl$calc[i] <- temp
      csl$delim[i] <- ""
    }
    if (csl$line_type[i] == "ifthen" ) {
      k <- which( parse_list[odds] == "token" & !(parse_list[odds+1] %in% c("if", "{")) ) * 2 - 1
      csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = else ####
  if (csl$line_type[i] %in% c("else")){

    if (major_section == "initial"){
      csl$init[i] <- "} else {"
      csl$delim[i] <- ""
    } else if (major_section == "discrete"){
      csl$disc[i] <- "} else {"
      csl$delim[i] <- ""
    } else {
      csl$calc[i] <- "} else {"
      csl$delim[i] <- ""
    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = elseifthen ####
  if (csl$line_type[i] %in% c("elseifthen")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      csl$delim[i] <- ""
    } else if (major_section == "discrete"){
      csl$disc[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      csl$delim[i] <- ""
    } else {
      csl$calc[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      csl$delim[i] <- ""
    }
    k <- which( parse_list[odds] == "token" & !(parse_list[odds+1] %in% c("else", "if", "{")) ) * 2 - 1
    csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
    csl$handled[i] <- TRUE

  }
  #### handle line_type = do ####
  if (csl$line_type[i] %in% c("do")){

    # for (int i = 1; i <= n; ++i) {
    # }
    # handle label line
    labeli <- which(str_detect(csl$label, paste("^", parse_list[4], "\\:$", sep=""))) # label line
    if (labeli <= i){
      stop("illegal reference to earlier label in do loop")
    }
    if (csl$line_type[labeli]  != "enddo"){
      stop("illegal operation after label of do loop")
    }
    csl$label[labeli] <- "" # remove label
    csl$indent[(i+1):(labeli-1)] <- csl$indent[(i+1):(labeli-1)] + 1 # increase indent
    # declaration
    bad <- token_decl_line[parse_list[6]] != 0 # already declared (actually not bad)
    if (!bad){ # declare
      csl$static[i] <- ""
      csl$type[i] <- "double"
      csl$decl[i] <- parse_list[6]
      csl$dend[i] <- ";"
      token_decl_line[parse_list[6]] <- i
      token_decl_type[parse_list[6]] <- csl$type[i]
    }
    # construct loop
    temp <- paste("for ( int", parse_list[6], "=", parse_list[10], ";", parse_list[6], "<=", parse_list[14], "; ++", parse_list[6], ") {", collapse=" ")
    if (major_section == "initial"){
      csl$init[i] <- temp
      csl$delim[i] <- ""
      csl$init[labeli] <- "}"
      csl$delim[labeli] <- ""
    } else if (major_section == "discrete"){
      csl$disc[i] <- temp
      csl$delim[i] <- ""
      csl$disc[labeli] <- "}"
      csl$delim[labeli] <- ""
    } else {
      csl$calc[i] <- temp
      csl$delim[i] <- ""
      csl$calc[labeli] <- "}"
      csl$delim[labeli] <- ""
    }
    csl$tail[i] <- paste("//", csl$code[i])
    csl$tail[labeli] <- paste("//", csl$code[labeli])
    csl$set[i] <- parse_list[6]
    token_role[parse_list[6]] <- "loop_counter"
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "token" & odds > 7 ) * 2 - 1
    csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
    csl$handled[i] <- TRUE
    csl$handled[labeli] <- TRUE

  }
  #### handle line_type = goto, ifgoto ####
  if (csl$line_type[i] %in% c("goto", "ifgoto")){

    # unique(csl$label)
    k <- which( parse_list == "goto" )
    label <- parse_list[k+2]
    labeli <- which(str_detect(csl$label, paste("^", label, "\\:$", sep=""))) # label line
    if (csl$line_type[labeli] %in% c("assign", "continue") == FALSE){
      stop("goto target is not assign or continue") # catch unhandled cases
    }
    new_label <- paste("label_", label, sep="")
    parse_list[k+2] <- new_label
    odds <- seq(1, length(parse_list)-1, 2)
    temp <- paste(parse_list[odds+1], collapse=" ")
    if (major_section == "initial"){
      csl$init[i] <- temp
      csl$delim[i] <- ";"
      csl$init[labeli] <- paste(new_label, ":", sep="") # replaces continue statement
      csl$delim[labeli] <- ""
    } else if (major_section == "discrete"){
      csl$disc[i] <- temp
      csl$delim[i] <- ";"
      csl$disc[labeli] <- paste(new_label, ":", sep="") # replaces continue statement
      csl$delim[labeli] <- ""
    } else {
      csl$calc[i] <- temp
      csl$delim[i] <- ";"
      csl$calc[labeli] <- paste(new_label, ":", sep="") # replaces continue statement
      csl$delim[labeli] <- ""
    }
    if (csl$line_type[i] == "ifgoto" ) {
      k <- which( parse_list[odds] == "token" & !(parse_list[odds+1] %in% c("if", "goto")) & !str_detect(parse_list[odds+1], "^label_") ) * 2 - 1
      csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")
    }
    csl$handled[i] <- TRUE
    csl$handled[labeli] <- TRUE

  }
  #### handle line_type = program, derivative, initial, discrete, dynamic, terminal, procedural, mfile ####
  if (csl$line_type[i] %in% c("program", "initial", "derivative", "discrete", "dynamic", "terminal", "procedural", "mfile")){

    # these blocks are not needed in C++, remove indent
    if (!is_continuation && csl$line_type[i] %in% c("program", "initial", "derivative", "dynamic", "terminal", "procedural")){
      endi <- csl$stack[i]
      csl$indent[(i+1):(endi-1)] <- csl$indent[(i+1):(endi-1)] - 1 # remove indent
    }
    # program
    if (csl$line_type[i] %in% c("program")){
      token_role[parse_list[4]] <- "program_name"
    }
    # mfile
    if (csl$line_type[i] %in% c("mfile")){
      mfile_name <- ensnakeify(csl$file_name[i])
      class_name <- paste(model_name, "_class&", sep="")
      temp <- paste("static void", mfile_name, "(", class_name, model_name, ") {")
      csl$mfile[i] <- temp
      csl$delim[i] <- ""
    }
    # procedural input=output list is used for sorting
    if (csl$line_type[i] %in% c("procedural")){
      # equals sign
      if (!is_continuation){
        found_procedural_equals <- FALSE
      }
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "equals" ) * 2 - 1
      if (length(k)>0){
        found_procedural_equals <- TRUE
      } else if (!found_procedural_equals){
        k <- max(odds) + 1 # still on set list
      } else if (found_procedural_equals){
        k <- 1 # already on used list
      }
      # set
      j <- which( parse_list[odds] == "token" & parse_list[odds+1] != "procedural" ) * 2 - 1
      jj1 <- j[j<k]
      csl$set[i] <- paste(c("procedural", parse_list[jj1+1]), collapse=",") # add "procedural" to help with null returns
      # used
      jj2 <- j[j>k]
      csl$used[i] <- paste(parse_list[jj2+1], collapse=",")
    }
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
    csl$handled[i] <- TRUE

  }
  #### handle line_type = termt ####
  if (csl$line_type[i] %in% c("termt", "sort")){

    # these lines are not needed in C++
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
    csl$handled[i] <- TRUE

  }
  #### handle line_type = schedule, interval ####
  if (csl$line_type[i] %in% c("schedule", "interval")){

    # schedule[t+x] <- name
    if (csl$line_type[i] == "interval"){
      k <- seq(7, length(parse_list)-1, 2)
      temp <- paste(parse_list[k+1], collapse=" " )
      temp <- paste("schedule ( t + ", temp, " , \"", discrete_block, "\" )", sep="")
    } else {
      if (parse_list[5] != "at"){
        stop(paste("SCHEDULE", parse_list[6], "is not implemented"))
      }
      k <- seq(7, length(parse_list)-1, 2)
      temp <- paste(parse_list[k+1], collapse=" " )
      temp <- paste("schedule ( ", temp, " , \"", parse_list[4],  "\" )", sep="")
    }
    if (major_section == "initial"){
      csl$init[i] <- temp
      csl$delim[i] <- ";"
      csl$handled[i] <- TRUE
    } else if (major_section == "discrete"){
      if (csl$line_type[i] == "interval"){
        csl$init[i] <- temp # interval has to be initialised also
      }
      csl$disc[i] <- temp
      csl$delim[i] <- ";"
      csl$handled[i] <- TRUE
    } else {
      stop("schedule not allowed in derivative section")
    }
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "token" & odds > 5 ) * 2 - 1
    csl$used[i] <- paste(setdiff(unique(parse_list[k+1]), reserved), collapse=",")

  }
  #### handle line_type = end ####
  if (csl$line_type[i] %in% c("end")){

    # see what it matches
    j <- csl$stack[i]
    if (csl$line_type[j] %in% c("program", "initial", "derivative", "dynamic", "terminal", "procedural")){

      # } not needed
      csl$tail[i] <- paste("// end of", csl$line_type[j], csl$tail[i])
      csl$handled[i] <- TRUE

    } else { # closes an ifthen or discrete or mfile

      if (major_section == "initial"){
        csl$init[i] <- "}"
        csl$delim[i] <- ""
      } else if (major_section == "discrete"){
        csl$disc[i] <- "}"
        csl$delim[i] <- ""
      } else if (major_section == "mfile"){
        csl$mfile[i] <- "}"
        csl$delim[i] <- ""
      } else {
        csl$calc[i] <- "}"
        csl$delim[i] <- ""
      }
      csl$tail[i] <- paste("// end of", csl$line_type[j], csl$tail[i])
      csl$handled[i] <- TRUE

    }

  }
  #### handle line_type = comment, blank ####
  if (csl$line_type[i] %in% c("comment", "blank")){

    csl$handled[i] <- TRUE

  }

  # next
  i <- i + 1

} # end loop

# role report
message <- paste("unused tokens :", paste(token_list[token_role==""], collapse=" "))
cat(message, "\n")
token_role[token_role==""] <- "unused"

# add collected info back into tokens
token_decl_line["t"] <- 1
if (!is.na(token_list["procedural"])){
  token_decl_line["procedural"] <- 1
}
tokens <- data_frame(
  name=token_list,
  lower=names(token_list),
  decl_line=token_decl_line,
  decl_type=token_decl_type,
  decl_columns=token_decl_columns,
  decl_rows=token_decl_rows,
  decl_value=token_decl_value,
  decl_static=token_decl_static,
  role=token_role
  )

# save progress
rm(list=setdiff(ls(), c("csl", "tokens", "path_name", "model_name", "silent", lsf.str())))
temp_file <- paste(path_name, "checkpoint_after_parse_two.RData", sep="/")
save.image(temp_file)

#### list of line types ####
# sort(unique(csl$line_type))
# [1] "algorithm"       "arrayassign"     "assign"          "blank"           "character"
# [6] "comment"         "constant"        "continuation"    "continue"        "derivative"
# [11] "discrete"        "do"              "doubleprecision" "dynamic"         "else"
# [16] "elseifthen"      "end"             "endif"           "goto"            "if"
# [21] "ifgoto"          "ifthen"          "initial"         "integ"           "integer"
# [26] "interval"        "logical"         "maxterval"       "nsteps"          "parameter"
# [31] "procedural"      "program"         "schedule"        "termt"
if (FALSE){
  View(filter(csl, line_type %in% c("integ", "derivt")))
  View(filter(csl, handled==FALSE))
  View(filter(csl, section=="initial"))
  View(filter(csl, used>""|set>""))
  View(filter(tokens, used_line<set_line & used_line>0))
}


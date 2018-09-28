#### second pass - standardise tokens, identify decl/init/calc/integ ####
# thoughts:
# separate information into header, declaration, initialisation, calculation for C++
# have to duplicate information since CSL doesn't have separate declaration section
# combine separate declaration and initialisation of variables in CSL, has to be one line in C++
# CSL constants are not actually constant, just initialised to the value
# convert some CSL tokens into C++ equivalents
# keep track of lines
# http://www.neurophys.wisc.edu/comp/docs/not017/
# are we going to completely automate, or only mostly.
# some things are hard to automate but fairly easy to translate manually (goto, mod, pow)

temp_file <- paste(path_name, "checkpoint_after_parse_one.RData", sep="/")
load(file=temp_file) # recover progress

cat(file=stderr(), "organising declarations, initialisation, calculation", "\n")

csl <- csl %>%
  mutate(
    section = NA,
    handled = FALSE,
    static = "",
    type = "",
    decl = "",
    init = "",
    calc = "",
    integ = "",
    delim = ""
  )

# change all comments to C++ style
csl$tail <- str_trim(str_replace(csl$tail, "^! ?", "// "))

# seems to be easier to access named lists than tibble rows
# token_list is used to standardise case of user token
# token_list is also used to translate some language keywords
token_list <- setNames(tokens$name, tokens$lower) # indexed by lower case
reserved <- c(keyword, "aint", "t", "cos",
              "integ", "sin", "max", "exp", "min", "acos", "asin", "atan", "log") # convert these keywords to lower case
token_list[names(token_list) %in% reserved] <- str_to_lower(token_list[names(token_list) %in% reserved])
token_list[c("then", "end", "endif")] <- c("{", "}", "}") # translate these
token_list[c("integer", "logical", "doubleprecision", "real", "character")] <- c("int", "bool", "double", "double", "char") # translate these

# keeping track of tokens
token_decl_line <- setNames(rep(0L, length(token_list)), token_list)
token_decl_type <- setNames(rep("", length(token_list)), token_list)
token_set_line <- setNames(rep(0L, length(token_list)), token_list) # only in calc
token_set_line[names(token_list) %in% reserved] <- 1 # assume reserved words are set?
token_used_line <- setNames(rep(0L, length(token_list)), token_list) # only in calc
token_constant <- setNames(rep("", length(token_list)), token_list) # collect consts (i.e., PARAMETER)
token_constant_value <- setNames(rep(0, length(token_list)), token_list) # and values
token_variable <- setNames(rep("", length(token_list)), token_list) # collect variables

# boolean and a few other conversions (WARNING: new values come first, old names come second!)
bool_list <- setNames(c("true", "false", "||", "&&", "~", ">=", ">", "<=", "<", "==", "!="),
                      c(".true.", ".false.", ".or.", ".and.", ".not.", ".ge.", ".gt.", ".le.", ".lt.", ".eq.", ".ne."))

# collect arrays
arrays <- ""

# loop through rows again
major_section <- "header"
major_sections <- c("initial", "dynamic", "derivative", "discrete", "terminal")
i <- 1
for (i in 1:nrow(csl)){

  # report progress
  if ((i %% 200 == 1 || i == nrow(csl)) && !silent){
    cat(file=stderr(), "line", i, "of", nrow(csl), "\n")
  }

  # identify major section
  if (csl$head[i] %in% major_sections){
    major_section <- csl$head[i]
  }
  csl$section[i] <- major_section

  # get tokens for this line
  parse_list <- unlist(str_to_obj(csl$parse_list[i]))

  # general tidy up
  # convert user tokens to case sensitive, and translate CSL to C++ keywords
  odds <- seq(1, length(parse_list)-1, 2)
  j <- which(str_detect(parse_list[odds], "token")) * 2 - 1
  if (length(j)>0){
    parse_list[j+1] <- token_list[str_to_lower(parse_list[j+1])] # convert tokens to standard case
  }
  # convert CSL bool (e.g. .GE.) to C++
  odds <- seq(1, length(parse_list)-1, 2)
  j <- which(str_detect(parse_list[odds], "bool")) * 2 - 1
  if (length(j)>0){
    parse_list[j+1] <- bool_list[str_to_lower(parse_list[j+1])] # convert bool
  }
  # "parameter", delete first and last brackets
  if (csl$line_type[i] == "parameter"){
    k <- setdiff( seq(1, length(parse_list)), c(3, 4, length(parse_list)-1, length(parse_list)))
    parse_list <- parse_list[k]
  }

  #### identify continuation ####
  is_continuation <- csl$line_type[i] %in% c("continuation")
  if (is_continuation){
    if (csl$line_type[i-1] %in% c("assign", "constant")){ # might need to handle them differently
      csl$line_type[i] <- csl$line_type[i-1]
      csl$delim[i-1] <- ""
    }
  }

  # make concatenated version of original
  odds <- seq(1, length(parse_list)-1, 2)
  parse_str <- paste(parse_list[odds+1], collapse=" ")

  #### find specific line ####
  # stopifnot(!str_detect(parse_str, "OxTpCd"))

  #### begin handling line types ####

  #### handle parameter ####
  if (csl$line_type[i] %in% c("parameter")){

    # note CSL PARAMETER is fixed, not array (may have type declared previously) like C++ constexpr
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) == "equals") * 2 - 1 # find variable

    # declare variable
    csl$static[i] <- "static constexpr"
    csl$type[i] <- "auto"
    csl$decl[i] <- paste(parse_list[setdiff(odds+1, 2)], collapse=" ")
    csl$delim[i] <- ";"
    csl$tail[i] <- paste("//", paste(parse_list[odds+1], collapse=" "), csl$tail[i]) # put original in tail

    # deal with previous declaration of variable
    bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
    if (any(bad)){
      message <- paste("redeclaration:", paste(parse_list[k+1][bad], collapse=" "))
      # cat("line", i, message, "\n")
      # delete previous declaration
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
          csl$delim[i] <- ""
        }
        # cat("removed declaration of", j, "on line", token_decl_line[j], "\n")
      }
    }
    token_decl_line[parse_list[k+1]] <- i
    token_decl_type[parse_list[k+1]] <- csl$type[i]
    token_constant[parse_list[k+1]] <- parse_list[k+1]
    token_constant_value[parse_list[k+1]] <- as.numeric(parse_list[k+5])
    csl$handled[i] <- TRUE

  }
  #### handle integer, character, doubleprecision, logical, dimension (including arrays) ####
  if (csl$line_type[i] %in% c("integer", "character", "doubleprecision", "logical", "dimension")){

    # these are explicit declarations
    # other variables are declared implictly by being assigned or initialised using "constant"
    # check for array declaration? FIXME rather simplistic logic
    odds <- seq(1, length(parse_list)-1, 2)
    j <- which( parse_list[odds] == "token" & lead(parse_list[odds],1) == "openbracket" ) * 2 - 1
    is_array <- length(j) > 0

    #### (a) array declaration ####
    if (is_array){ # array declaration only

      # commas indicate number of dimensions (FIXME only handles simple cases)
      odds <- seq(1, length(parse_list)-1, 2)
      j <- which( parse_list[odds] == "comma" ) * 2 - 1
      if (length(j) == 0){

        # https://www.learncpp.com/cpp-tutorial/6-15-an-introduction-to-stdarray/
        # std::array<int,2> a ; // 2 elements
        # a = {{ 13, 18 }} ; // note std::array requires one pair of extra outer braces
        # a[0] // base zero
        # single dimension fixed size array
        message <- paste("array:", parse_str)
        arrays <- c(arrays, parse_str)
        cat("line", i, message, "\n")
        csl$static[i] <- ""
        csl$type[i] <- paste("std::array< double ,", parse_list[8], "> ")
        csl$decl[i] <- parse_list[4]
        csl$delim[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_variable[parse_list[4]] <- parse_list[4]
        token_decl_line[parse_list[4]] <- i
        token_decl_type[parse_list[4]] <- csl$type[i]

      } else if (length(j) == 1){

        # https://stackoverflow.com/questions/17759757/multidimensional-stdarray
        # https://en.cppreference.com/w/cpp/container/array/operator_at
        # std::array<std::array<int,3>,2> a ; // 2 rows, 3 columns
        # a = {{{1,2,3}, {4,5,6}}} ; // note std::array requires one pair of extra outer braces
        # a[2-1][3-1] // note axis order
        # two dimensional fixed size array
        message <- paste("2darray:", parse_str)
        arrays <- c(arrays, parse_str)
        cat("line", i, message, "\n")
        csl$static[i] <- ""
        csl$type[i] <- paste("std::array< std::array< double ,", parse_list[12], "> ,", parse_list[8], "> ") # need to reverse indices
        csl$decl[i] <- parse_list[4]
        csl$delim[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_variable[parse_list[4]] <- parse_list[4]
        token_decl_line[parse_list[4]] <- i
        token_decl_type[parse_list[4]] <- csl$type[i]

      } else {

        # unhandled
        stop()

      }

    } # end array declaration

    #### (b) non-array declaration and intialisation ####
    if (is_array == FALSE){

      #
      this_type <- parse_list[2] # C++ type
      parse_list[1:2] <- c("comma" , ",") # put fake comma at start of list to make it easier to parse

      # declare variables
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lag(parse_list[odds], 1) == "comma") * 2 - 1 # find variables
      csl$static[i] <- ""
      csl$type[i] <- this_type
      csl$decl[i] <- paste(parse_list[k+1], collapse=" , ") # variable list
      csl$delim[i] <- ";"
      # deal with previous declarations of these variables
      bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
      if (any(bad)){
        message <- paste("redeclaration:", paste(parse_list[k+1][bad], collapse=" "))
        # cat("line", i, message, "\n")
        # delete previous declaration
        for (j in parse_list[k+1][bad]){
          # remove j from previous declaration
          temp <- str_trim(str_split(csl$decl[token_decl_line[j]], ",")[[1]]) # previous decl
          temp <- max(str_c(temp[temp != j], collapse=" , "), "") # remove j
          if (temp > ""){ # edit previous decl
            csl$decl[token_decl_line[j]] <- temp # remove j from decl
            csl$type[i] <- csl$type[token_decl_line[j]] # get type FIXME ?
          } else { # delete previous decl
            csl$static[token_decl_line[j]] <- ""
            csl$type[i] <- csl$type[token_decl_line[j]] # get type FIXME ?
            csl$type[token_decl_line[j]] <- ""
            csl$decl[token_decl_line[j]] <- "" # delete decl
            csl$delim[i] <- ""
          }
          # cat("removed declaration of", j, "on line", token_decl_line[j], "\n")
        }
      }
      token_variable[parse_list[k+1]] <- parse_list[k+1]
      token_decl_line[parse_list[k+1]] <- i
      token_decl_type[parse_list[k+1]] <- csl$type[i]

      # initialise variables
      # which tokens get set (any followed by equals)
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which(parse_list[odds] == "token" & lead(parse_list[odds], 1) == "equals") * 2 - 1
      token_set_line[parse_list[k+1]] <- i
      # which tokens get used (any not preceeded by comma)
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lag(parse_list[odds], 1) != "comma" ) * 2 - 1
      bad <- token_set_line[parse_list[k+1]] == 0 # not set
      if (any(bad)){
        message <- paste("uses unset token:", paste(parse_list[k+1][bad], collapse=" "))
        cat("line", i, message, "\n")
      }
      token_used_line[parse_list[k+1][!bad]] <- i
      # initialisation (FIXME no check for duplicate initialisation)
      # find the bits of the parse_list that are not declaration-only
      k <- seq(4, length(parse_list), 2) # all code
      j <- which( parse_list == "token" & lag(parse_list, 2) == "comma" &
                    ( lead(parse_list, 2) != "equals" | is.na(lead(parse_list, 2)) ) ) + 1L # decl code
      k <- setdiff(k, union(j, j-2)) # remove decl code, keep rest
      csl$init[i] <- str_replace_all( paste( parse_list[k], collapse=" "), ",", ";") # initial assignment
      csl$delim[i] <- ";"

    } # end non-array declaration and initialisation
    csl$handled[i] <- TRUE

  }
  #### handle integ ####
  if (csl$line_type[i] %in% c("integ")){

    # FIXME only handles integ statements of particular simple forms
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

    # a = integ( b , c ), no expressions allowed
    if (str_to_lower(paste(parse_list[c(4, 6, 8, 12, 16, 18)], collapse="")) == "=integ(,)na"){

      j <- which(str_to_lower(parse_list) == "integ") # 6 actually
      jj <- parse_list[j-4]
      # declare state variable
      if (token_decl_line[jj] != 0){
        message <- paste("previously declared integ:", jj)
        # cat("line", i, message, "\n")
        # don't declare again
      } else {
        csl$static[i] <- ""
        csl$type[i] <- "double" # state vars are double
        csl$decl[i] <- jj
        csl$delim[i] <- ";"
        token_variable[jj] <- jj
        token_decl_line[jj] <- i
        token_decl_type[jj] <- csl$type[i]
      }
      # initial condition of state variable
      if (token_set_line[jj] != 0){
        message <- paste("previously initialised integ:", jj)
        # cat("line", i, message, "\n")
      }
      # initialise even if already initialised
      csl$init[i] <- paste(parse_list[c(j-4, j-2, j+8)], collapse=" ")
      token_set_line[jj] <- i
      if (parse_list[j+7] == "token"){ # could be a number
        token_used_line[parse_list[j+8]] <- i
      }
      # integration rate of state variable
      csl$integ[i] <- paste(parse_list[c(j-4, j-2, j+4)], collapse=" ")
      if (parse_list[j+3] == "token"){ # could be a number
        token_used_line[parse_list[j+4]] <- i
      }

    # } else if (str_to_lower(paste(parse_list[c(4, 6, 8, 12, 14, 16, 20, 24, 26, 28)], collapse="")) == "=max(,integ(,))na"){


    } else {

      # can't handle this type
      odds <- seq(1, length(parse_list)-1, 2)
      message <- paste("unhandled form of integ:", paste(parse_list[odds+1], collapse=" "))
      cat("line", i, message, "\n")

    }
    csl$handled[i] <- TRUE

  }
  #### handle assign ####
  if (csl$line_type[i] %in% c("assign")){

    if (is_continuation == FALSE){
      #  declaration of lhs
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) == "equals" ) * 2 - 1
      token_set_line[parse_list[k+1]] <- i
      bad <- token_decl_line[parse_list[k+1]] != 0 # already declared (actually not bad)
      if (any(!bad)){ # declare these ones
        for (jj in parse_list[k+1][!bad]){
          csl$static[i] <- ""
          csl$type[i] <- "double"
          csl$decl[i] <- jj
          csl$delim[i] <- ";"
          token_variable[jj] <- jj
          token_decl_line[jj] <- i
          token_decl_type[jj] <- csl$type[i]
        }
      }
    }
    # assignment
    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "header"){
      stop()
    } else if (major_section == "initial"){
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    } else { # dynamic section
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which(parse_list[odds] == "token" & lead(parse_list[odds], 1) != "equals") * 2 - 1
      token_used_line[parse_list[k+1]] <- i
    }
    csl$handled[i] <- TRUE

  }
  #### handle constant ####
  if (csl$line_type[i] %in% c("constant")){

    # note: a CSL CONSTANT is just an initialiser, values can be changed later!!!
    # CONSTANT is used to
    # (a) declare and initialise a list of variables (lots of = signs)
    # (b) initialise a declared array (single = on first line)
    # both of these can have continuation
    # FIXME can't handle simple initialisations that break across lines
    # declare
    if (!is_continuation){
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" &  lead(parse_list[odds], 1) == "equals" ) * 2 - 1
      is_array <- length(k) == 1 && any(str_detect(arrays, paste("^double", parse_list[k+1], "\\(", sep="[:blank:]+")))
      if (is_array){ # set array details
        this_array <- arrays[str_detect(arrays, paste("^double", parse_list[k+1], "\\(", sep="[:blank:]+"))]
        temp <- str_split(this_array, "\\s")[[1]]
        this_array_name <- temp[2]
        this_array_dims <- 1
        this_array_row <- 1
        if (str_detect(temp[4], "^[0-9]+$")){
          this_array_dim1 <- as.integer(temp[4])
        } else {
          this_array_dim1 <- token_constant_value[temp[4]]
        }
        this_array_dim2 <- 1
        if (temp[5] == ","){
          this_array_dims <- 2
          if (str_detect(temp[6], "^[0-9]+$")){
            this_array_dim2 <- as.integer(temp[6])
          } else {
            this_array_dim2 <- token_constant_value[temp[6]]
          }
        }
      } else {
        this_array <- ""
      }
    } else { # continuation of previous line(s)
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" &  lead(parse_list[odds], 1) == "equals" ) * 2 - 1
    }
    token_set_line[parse_list[k+1]] <- i
    bad <- token_decl_line[parse_list[k+1]] != 0 # already declared
    if (any(!bad)){ # declare these ones
      csl$static[i] <- ""
      csl$type[i] <- "double"
      csl$decl[i] <- paste(parse_list[k+1][!bad], collapse=" , ")
      csl$delim[i] <- ";"
      token_variable[parse_list[k+1][!bad]] <- parse_list[k+1][!bad]
      token_decl_line[parse_list[k+1][!bad]] <- i
      token_decl_type[parse_list[k+1][!bad]] <- csl$type[i]
    }
    # initialise
    if (!is_array){ # easy, just copy line and convert , to ;
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds+1] != "constant" ) * 2 - 1
      csl$init[i] <- str_replace(str_replace_all( paste( parse_list[k+1], collapse=" "), ",", ";"), ";$", "")
      csl$delim[i] <- ";"
      if (major_section != "initial"){
        csl$tail[i] <- paste("//", paste(parse_list[odds+1], collapse=" "), csl$tail[i]) # put original in tail
      }
      csl$handled[i] <- TRUE
    } else if (!is_continuation){ # array first line FIXME assumes each line is a row of data
      # a = {{1,2,3}} ; // note std::array requires one pair of extra outer braces
      # a = {{{1,2,3}, {4,5,6}}} ; // note std::array requires one pair of extra outer braces
      if (max(parse_list[10], "", na.rm=TRUE) == "*"){ # replicator format for 1d array e.g. MamCellsF = MaxHerds * 0.0
        temp <- paste(rep(parse_list[12], this_array_dim1), collapse=" , ")
        temp <- paste(parse_list[4], " = { { ", temp, " } } ", sep="")
        print(temp)
        csl$init[i] <- temp
        csl$delim[i] <- ""
        if (major_section != "initial"){
          csl$tail[i] <- paste("//", paste(parse_list[odds+1], collapse=" "), csl$tail[i]) # put original in tail
        }
        if (this_array_dim2 > 1){
          stop("illegal array initialisation")
        }
        csl$handled[i] <- TRUE
      } else { # normal array fill
        will_continue <- csl$cont[i] > ""
        if (length(parse_list) == 6 & will_continue){ # handle empty first lines e.g. CONSTANT Event = &
          temp <- paste(parse_list[4], " = { { ", sep="")
          this_array_row <- this_array_row - 1 # this line doesn't count
        } else { # first line contains a whole row
          outer <- if_else(this_array_dims == 2, "{ {", "{")
          ender <- if_else(will_continue, ",", if_else(this_array_dims == 2, "} }", "}"))
          k <- seq(8, length(parse_list), 2)
          if (parse_list[tail(k,1)] == ","){
            k <- seq(8, length(parse_list)-2, 2)
          }
          if ((length(k)+1)/2 != this_array_dim1){
            stop("wrong number of elements")
          }
          temp <- paste(parse_list[k], collapse=" ")
          temp <- paste(parse_list[4], " = ", outer, " { ", temp, " } ", ender, sep="")
        }
        print(temp)
        csl$init[i] <- temp
        csl$delim[i] <- ""
        if (major_section != "initial"){
          csl$tail[i] <- paste("//", paste(parse_list[odds+1], collapse=" "), csl$tail[i]) # put original in tail
        }
        if (this_array_row == this_array_dim2 & will_continue){
          stop("should not continue")
        } else {
          this_array_row <- this_array_row + 1
        }
        csl$handled[i] <- TRUE
      }
    } else { # array continuation FIXME assumes each line is a row of data
      will_continue <- csl$cont[i] > ""
      ender <- if_else(will_continue, ",", "} }")
      k <- seq(2, length(parse_list), 2)
      if (parse_list[tail(k,1)] == ","){
        k <- seq(2, length(parse_list)-2, 2)
      }
      if ((length(k)+1)/2 != this_array_dim1){
        stop("wrong number of elements")
      }
      temp <- paste(parse_list[k], collapse=" ")
      temp <- paste(" { ", temp, " } ", ender, sep="")
      print(temp)
      csl$init[i] <- temp
      csl$delim[i] <- ";"
      if (major_section != "initial"){
        csl$tail[i] <- paste("//", paste(parse_list[odds+1], collapse=" "), csl$tail[i]) # put original in tail
      }
      if (this_array_row == this_array_dim2 & will_continue){
        stop("should not continue")
      } else {
        this_array_row <- this_array_row + 1
      }
      csl$handled[i] <- TRUE
    }

  }
  #### handle line_type = if ####
  if (csl$line_type[i] %in% c("if")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    } else {
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = ifthen, endif ####
  if (csl$line_type[i] %in% c("ifthen", "endif")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      # no delimiter!
    } else {
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      # no delimiter!
    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = else ####
  if (csl$line_type[i] %in% c("else")){

    if (major_section == "initial"){
      csl$init[i] <- "} else {"
      # no delimiter!
    } else {
      csl$calc[i] <- "} else {"
      # no delimiter!
    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = elseif ####
  if (csl$line_type[i] %in% c("elseifthen")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      # no delimiter!
    } else {
      csl$calc[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      # no delimiter!
    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = program, derivative, initial, discrete, dynamic, terminal ####
  if (csl$line_type[i] %in% c("program", "initial", "derivative", "discrete", "dynamic", "terminal")){

   csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
   csl$handled[i] <- TRUE

  }
  #### handle line_type = end ####
  if (csl$line_type[i] %in% c("end")){

    # see what it matches
    j <- csl$stack[i]
    if (csl$line_type[j] %in% c("program", "initial", "derivative", "discrete", "dynamic", "terminal")){

      csl$tail[i] <- paste("// end of", csl$line_type[j], csl$tail[i])

    } else { # closes an ifthen or procedural

      if (major_section == "initial"){
        csl$init[i] <- "}"
      } else {
        csl$calc[i] <- "}"
      }

    }
    csl$handled[i] <- TRUE

  }
  #### handle line_type = comment, blank ####
  if (csl$line_type[i] %in% c("comment", "blank")){

    csl$handled[i] <- TRUE

  }

  # next
  i <- i + 1

} # end loop

# add collected info back into tokens
setdiff(token_variable, token_list) # FIXME should be nothing
tokens <- data.frame(
  name=token_list,
  lower=names(token_list),
  decl_line=token_decl_line,
  decl_type=token_decl_type,
  set_line=token_set_line,
  used_line=token_used_line,
  constant=token_constant,
  value=token_constant_value,
  variable=token_variable
  ) %>%
  arrange(decl_line)

# save progress
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
  View(filter(csl, line_type=="constant"))
}


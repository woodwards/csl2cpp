#### second pass - standardise tokens, identify decl/init/calc/integ ####
# thoughts:
# separate information into header, declaration, initialisation, calculation for C++
# have to duplicate information since CSL doesn't have separate declaration section
# combine separate declaration and initialisation of constants in CSL, has to be one line in C++
# do we want to keep constants or make everything variable?
# convert some CSL tokens into C++ equivalents
# keep track of lines
# http://www.neurophys.wisc.edu/comp/docs/not017/
# are we going to completely automate, or only mostly.
# some things are hard to automate but fairly easy to translate manually (goto, mod, pow)

temp_file <- paste(path_name, "parse_checkpoint2.RData", sep="/")
load(file=temp_file) # recover progress

cat(file=stderr(), "separating declarations, initialisation, calculation", "\n")

csl <- csl %>%
  mutate(
    section = NA,
    static = "",
    type = "",
    decl = "",
    init = "",
    calc = "",
    integ = "",
    delim = ""
  )

# change comment marker to C++
csl$tail <- str_trim(str_replace(csl$tail, "^! ?", "// "))

# seems to be easier to access named lists than tibble rows
token_list <- setNames(tokens$name, tokens$lower) # indexed by lower case
reserved <- c(keyword, "aint", "t", "cos",
              "integ", "sin", "max", "exp", "min", "acos", "asin", "atan", "log")
token_list[names(token_list) %in% reserved] <- str_to_lower(token_list[names(token_list) %in% reserved])
token_list[c("then", "end", "endif")] <- c("{", "}", "}")
token_list[c("integer", "logical", "doubleprecision", "real", "character")] <- c("int", "bool", "double", "double", "char")

# keeping track of tokens
token_decl_line <- setNames(rep(0L, length(token_list)), token_list) # indexed by standard case
token_decl_type <- setNames(rep("", length(token_list)), token_list) # indexed by standard case
token_set_line <- setNames(rep(0L, length(token_list)), token_list) # indexed by standard case
token_set_line[names(token_list) %in% reserved] <- 1 # assume reserved words are set
token_used_line <- setNames(rep(0L, length(token_list)), token_list) # indexed by standard case

# boolean and a few other conversions (WARNING: new values come first, old names come second!)
bool_list <- setNames(c("true", "false", "||", "&&", "~", ">=", ">", "<=", "<", "==", "!="),
                      c(".true.", ".false.", ".or.", ".and.", ".not.", ".ge.", ".gt.", ".le.", ".lt.", ".eq.", ".ne."))

# arrays
arrays <- character()

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
  # convert to case sensitive and C++ tokens
  odds <- seq(1, length(parse_list)-1, 2)
  j <- which(str_detect(parse_list[odds], "token")) * 2 - 1
  if (length(j)>0){
    parse_list[j+1] <- token_list[str_to_lower(parse_list[j+1])] # convert tokens to standard case
  }
  # convert bool (e.g. .GE.) to C++
  odds <- seq(1, length(parse_list)-1, 2)
  j <- which(str_detect(parse_list[odds], "bool")) * 2 - 1
  if (length(j)>0){
    parse_list[j+1] <- bool_list[str_to_lower(parse_list[j+1])] # convert bool
  }
  # "parameter", remove first and last brackets (FIXME only handles simple cases)
  if (csl$line_type[i] == "parameter"){
    k <- setdiff( seq(1, length(parse_list)), c(3, 4, length(parse_list)-1, length(parse_list)))
    parse_list <- parse_list[k]
  }

  # make concatenated version of original
  odds <- seq(1, length(parse_list)-1, 2)
  parse_str <- paste(parse_list[odds+1], collapse=" ")

  #### handle line_type = constant, integer, doubleprecision, logical, parameter ####
  if (csl$line_type[i] %in% c("constant", "integer", "doubleprecision", "logical", "parameter")){

    # these are explicit declarations and constant definitions
    # constant and parameter variables may have been declared previously
    # other variables are declared implictly by being used

    # is it an array declaration? FIXME rather simplistic
    odds <- seq(1, length(parse_list)-1, 2)
    j <- which( parse_list[odds] == "openbracket" ) * 2 - 1
    is_array <- length(j) > 0

    #### array declaration ####
    if (is_array){ # array declaration only

      # commas indicate number of dimensions (FIXME only handles simple cases)
      odds <- seq(1, length(parse_list)-1, 2)
      j <- which( parse_list[odds] == "comma" ) * 2 - 1
      if (length(j) == 0){

        # single dimension
        message <- paste("array:", parse_str)
        arrays <- c(arrays, parse_str)
        cat("line", i, message, "\n")
        csl$static[i] <- "static constexpr "
        csl$type[i] <- paste("std::array< double ,", parse_list[8], "> ")
        csl$decl[i] <- parse_list[4]
        csl$delim[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_decl_line[parse_list[4]] <- i
        token_decl_type[parse_list[4]] <- parse_str

      } else if (length(j) == 1){

        # two dimensions
        message <- paste("array:", parse_str)
        arrays <- c(arrays, parse_str)
        cat("line", i, message, "\n")
        csl$static[i] <- "static constexpr "
        csl$type[i] <- paste("std::array< std::array< double ,", parse_list[8], "> ,", parse_list[12], "> ")
        csl$decl[i] <- parse_list[4]
        csl$delim[i] <- ";"
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail
        token_decl_line[parse_list[4]] <- i
        token_decl_type[parse_list[4]] <- parse_str

      } else {

        # unhandled
        stop()

      }

    } # end array declaration

    #### non-array declaration and intialisation ####
    # csl$line_type[i] %in% c("constant", "integer", "doubleprecision", "logical", "parameter")
    if (is_array == FALSE){

      # C++ type
      this_type <- if_else(csl$line_type[i] %in% c("constant", "parameter"), "double", parse_list[2])

      # identify variables
      parse_list[1:2] <- c("comma" , ",") # put fake comma at start of list to make it easier to parse
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which( parse_list[odds] == "token" & lag(parse_list[odds], 1) == "comma") * 2 - 1 # find variables

      # declare variables
      csl$static[i] <- if_else(csl$line_type[i] %in% c("constant", "parameter"), "static constexpr", "")
      csl$type[i] <- this_type
      csl$decl[i] <- paste(parse_list[k+1], collapse=" , ") # variable list
      csl$delim[i] <- ";"
      if ((csl$line_type[i] %in% c("constant", "parameter")) | !str_detect(parse_str, "=")){
        csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # if no init, put original in tail
      }
      # deal with previous declarations of these variables
      bad <- token_decl_line[parse_list[k+1]] != 0 # find already declared
      if (any(bad)){
        message <- paste("redeclaration:", paste(parse_list[k+1][bad], collapse=" "))
        cat("line", i, message, "\n")
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
          cat("removed declaration of", j, "on line", token_decl_line[j], "\n")
        }
      }
      token_decl_line[parse_list[k+1]] <- i
      token_decl_type[parse_list[k+1]] <- csl$type[i]

      # now do initialisation (either as constexpr or in init section)
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
      # find the bits of the parse_list that are no declaration-only
      k <- seq(4, length(parse_list), 2)
      j <- which( parse_list == "token" & lag(parse_list, 2) == "comma" &
                    ( lead(parse_list, 2) != "equals" | is.na(lead(parse_list, 2)) ) ) + 1L
      k <- setdiff(k, union(j, j-2L)) # remove token and preceding comma
      if (csl$line_type[i] %in% c("constant", "parameter")){
        csl$decl[i] <- paste( parse_list[k], collapse=" ") # replace decl with constexpr decl
      } else {
        csl$init[i] <- str_replace_all( paste( parse_list[k], collapse=" "), ",", ";") # initial assignment
      }

    } # end non-array declaration and initialisation

  #### handle line_type = integ ####
  } else if (csl$line_type[i] %in% c("integ")){

    # FIXME only handles integ statements of particular simple forms
    csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

    # a = integ( b , c ), no expressions allowed
    if (str_to_lower(paste(parse_list[c(4, 6, 8, 12, 16, 18)], collapse="")) ==
        "=integ(,)na"){

      j <- which(str_to_lower(parse_list) == "integ") # 6 actually
      jj <- parse_list[j-4]
      # declare state variable
      if (token_decl_line[jj] != 0){
        message <- paste("previously declared integ:", jj)
        cat("line", i, message, "\n")
        # don't declare again
      } else {
        csl$static[i] <- ""
        csl$type[i] <- "double"
        csl$decl[i] <- jj
        csl$delim[i] <- ";"
        token_decl_line[jj] <- i
        token_decl_type[jj] <- "implicit"
      }
      # initial condition of state variable
      if (token_set_line[jj] != 0){
        message <- paste("previously initialised integ:", jj)
        cat("line", i, message, "\n")
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

    # } else if (str_to_lower(paste(parse_list[c(4, 6, 8, 12, 14, 16, 20, 24, 26, 28)], collapse="")) ==
    #            "=max(,integ(,))na"){



    } else {

      # can't handle this type
      odds <- seq(1, length(parse_list)-1, 2)
      message <- paste("unhandled form of integ:", paste(parse_list[odds+1], collapse=" "))
      cat("line", i, message, "\n")

    }

  #### handle line_type = assignment ####
  } else if (csl$line_type[i] %in% c("assignment")){

    # implicit declaration of lhs
    odds <- seq(1, length(parse_list)-1, 2)
    k <- which( parse_list[odds] == "token" & lead(parse_list[odds], 1) == "equals" ) * 2 - 1
    bad <- token_decl_line[parse_list[k+1]] != 0 # already declared (actually not bad)
    if (!bad){
      csl$static[i] <- ""
      csl$type[i] <- "double"
      csl$decl[i] <- parse_list[k+1]
      csl$delim[i] <- ";"
      token_decl_line[parse_list[k+1]] <- i
      token_decl_type[parse_list[k+1]] <- "implicit"
    }
    # assignment
    token_set_line[parse_list[k+1]] <- i
    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      # initial section should not need sorting
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    } else {
      # dynamic section might need sorting
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
      odds <- seq(1, length(parse_list)-1, 2)
      k <- which(parse_list[odds] == "token" & lead(parse_list[odds], 1) != "equals") * 2 - 1
      # bad <- token_set_line[parse_list[k]] == 0
      # token_used_line[parse_list[k][!bad]] <- i
      token_used_line[parse_list[k+1]] <- i
    }

    #### handle line_type = if ####
  } else if (csl$line_type[i] %in% c("if")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    } else {
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      csl$delim[i] <- ";"
    }

    #### handle line_type = ifthen, endif ####
  } else if (csl$line_type[i] %in% c("ifthen", "endif")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste(parse_list[odds+1], collapse=" ")
      # no delimiter!
    } else {
      csl$calc[i] <- paste(parse_list[odds+1], collapse=" ")
      # no delimiter!
    }

    #### handle line_type = else ####
  } else if (csl$line_type[i] %in% c("else")){

    if (major_section == "initial"){
      csl$init[i] <- "} else {"
      # no delimiter!
    } else {
      csl$calc[i] <- "} else {"
      # no delimiter!
    }

    #### handle line_type = elseif ####
  } else if (csl$line_type[i] %in% c("elseif")){

    odds <- seq(1, length(parse_list)-1, 2)
    if (major_section == "initial"){
      csl$init[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      # no delimiter!
    } else {
      csl$calc[i] <- paste("}", paste(parse_list[odds+1], collapse=" "))
      # no delimiter!
    }

    #### handle line_type = program, derivative, initial, discrete, dynamic, terminal ####
  } else if (csl$line_type[i] %in% c("program", "derivative", "initial", "discrete", "dynamic", "terminal")){

   csl$tail[i] <- paste("//", parse_str, csl$tail[i]) # put original in tail

    #### handle line_type = end ####
  } else if (csl$line_type[i] %in% c("end")){

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

    #### handle line_type = comment, blank ####
  } else if (csl$line_type[i] %in% c("comment", "blank")){

    # do nothing?

    #### unhandled line_type ####
  } else {

    if (csl$body[i]>""){
      csl$tail[i] <- paste("// UNHANDLED", parse_str, csl$tail[i])
    }

  }

  # next
  i <- i + 1

} # end loop

# add collected info back into tokens
tokens <- data.frame(
  name=token_list,
  lower=names(token_list),
  decl_line=token_decl_line,
  decl_type=token_decl_type,
  set_line=token_set_line,
  used_line=token_used_line
  ) %>%
  arrange(decl_line)


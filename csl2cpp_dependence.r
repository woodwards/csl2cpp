#### determine uninitialised variables ####
# thoughts:
# see ACSL Reference Manual Version 11 p3-1, p3-3 (mistake on p3-2)
# order of execution:
#   INITIAL, set state, control variables, DERIVATIVE (sorted), DISCRETE, DYNAMIC, TERMINAL
# constant statement can be anywhere in the code "they are not executable" (p3-2)
# integ statement can be anywhere in the code, initial conditions applied after INITIAL (p3-1, 3-3)
# derivt statement similarly
# DYNAMIC block statements are actually after DERIVATIVE and DISCRETE
# a handful of variables are "assumed", i.e. used without being set first
# ACSLX initialises all variables to weird values but we want to avoid this
# some might also be set in discrete, but it's difficult to analyse whether this will occur
# need to use tokens$set_line to see if it's a problem
# also after every sort operation, the variable dependence might change!

# scratch notes
# > unique(csl$line_type[csl$set>""|csl$used>""])
# [1] B"constant"    "assign"      A"parameter"   "if"          "ifthen"      "procedural"  "do"
# [8] "arrayassign" "elseifthen"  "schedule"    "algorithm"   "nsteps"      "maxterval"   "integ"
# [15] "ifgoto"

csl_dependence <- function(csl, tokens, silent=TRUE){

  #
  csl$assumed <- ""

  # extract model state and rate variables
  integ <- csl$integ[csl$integ > ""]
  state <- str_match(integ, "^[:alpha:]+[[:alnum:]_]*")[,1]
  rate <- str_trim(str_replace(integ, "^[:alpha:]+[[:alnum:]_]*", ""))
  rate <- str_replace_all(rate, "= ", "")
  i <- which(str_detect(rate, "^max \\( ")) # handle = max ( , ) form
  temp <- str_split(rate, " ") # returns list of vectors of strings
  for (ii in i){
    rate[ii] <- temp[[ii]][5]
  }

  # create running list
  token_set_line <- setNames(rep(NA, nrow(tokens)), tokens$name) # when does a var become available?
  token_set_status <- setNames(rep("uninit", nrow(tokens)), tokens$name) # how is it set?

  # declarations of value (e.g. parameter, constant)
  # "algorithm", "nsteps", "maxterval", "cinterval", "minterval" currently also treated like parameter
  rows <- which(csl$line_type %in% c("parameter", "constant",
                                     "algorithm", "nsteps", "maxterval", "cinterval", "minterval"))
  set <- setdiff(unique(unlist(str_split(csl$set[rows], ",", simplify=FALSE))), "")
  token_set_status[set] <- "set"
  token_set_line[set] <- 1

  # add fake variable to avoid an empty set list on procedurals
  token_set_status["procedural"] <- "set"
  token_set_line["procedural"] <- 1

  # work through remaining code (sections have already been reorganised)
  active <- (csl$set>"" | csl$used>"") &
    !(csl$line_type %in% c("parameter", "constant", "procedural", "integ", "derivt"))
  rows <- which(active)
  did_continue <- FALSE
  for (i in rows){
    # set state variables?
    if (is.na(token_set_line["t"]) & !(csl$line_type[i] %in% c("header", "initial"))){
      # integ statements set state (and t) at end of INITIAL section(s?)
      # derivt statements calculate derivative at end of each time step
      # only set ones that are not already flagged, we want to know if any are previous assumed
      set <- c("t", state)
      bad <- token_set_status[set] == "uninit"
      token_set_status[set[bad]] <- "set"
      token_set_line[set[bad]] <- i
    }
    # handle continuations
    if (did_continue){ # accumulate variable lists
      set <- c(set, comma_split(csl$set[i]))
      used <- c(used, comma_split(csl$used[i]))
    } else { # split variable lists
      set <- comma_split(csl$set[i])
      used <- comma_split(csl$used[i])
      line_type <- csl$line_type[i]
    }
    will_continue <- csl$cont[i] > ""
    if (will_continue){ # go to next line
      did_continue <- TRUE
    } else {
      # analyse line
      set <- set[set>""]
      if (any(set %in% c("t"))){
        stop(paste("code line", csl$line_number[i], ": assignment to t in", csl$section[i], "\n"))
      }
      if (any(set %in% c(state)) & !silent){
        cat(paste("code line", csl$line_number[i], ": assignment to state variable in", csl$section[i], "\n"))
      }
      if (any(set %in% c(rate)) & csl$section[i]!="derivative" & !silent){
        cat(paste("code line", csl$line_number[i], ": assignment to rate variable in", csl$section[i], "\n"))
      }
      used <- used[used>""]
      if (any(used %in% c("t")) & !(csl$section[i] %in% c("dynamic", "discrete", "derivative"))){
        stop(paste("code line", csl$line_number[i], ": use of t in", csl$section[i], "\n"))
      }
      if (any(used %in% c(state)) & !(csl$section[i] %in% c("dynamic", "discrete", "derivative")) & !silent){
        cat(paste("code line", csl$line_number[i], ": use of state variable in", csl$section[i], "\n"))
      }
      if (!any(used>"")){ # set only (e.g. initialisation with constant)
        if (csl$section[i] != "discrete"){ # assume discrete sections do not set anything
          bad <- token_set_status[set] == "uninit"
          token_set_status[set[bad]] <- "set"
          token_set_line[set[bad]] <- i
        }
      } else if (!any(set>"")){ # used only (e.g. ifthen)
        bad <- token_set_status[used] == "uninit"
        token_set_status[used[bad]] <- "assumed"
        token_set_line[used[bad]] <- i
        csl$assumed[i] <- paste_sort(used[bad])
      } else { # both set and used (e.g. assignment)
        bad <- token_set_status[used] == "uninit"
        token_set_status[used[bad]] <- "assumed"
        token_set_line[used[bad]] <- i
        csl$assumed[i] <- paste_sort(used[bad])
        if (csl$section[i] != "discrete"){ # assume discrete sections do not set anything
          if (any(token_set_status[used]!="set")){
            token_set_status[set] <- "from_assumed"
            token_set_line[set] <- i
          } else { # all set
            token_set_status[set] <- "set"
            token_set_line[set] <- i
          }
        }
      }
      did_continue <- FALSE
    }
  }

  # save results in df
  tokens <- tokens %>%
    mutate(set_line=unname(token_set_line),
           set_status=unname(token_set_status))

  return(tokens)

} # end function



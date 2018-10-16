#### third pass - determine uninitialised variables ####
# thoughts:
# see ACSL Reference Manual Version 11
# order of execution: INITIAL, DERIVATIVE (sorted), DISCRETE, DYNAMIC
# constant statement can be anywhere in the code "they are not executable" (p3-2)
# DYNAMIC block statements are actually after DERIVATIVE and DISCRETE

temp_file <- paste(path_name, "checkpoint_after_parse_two.RData", sep="/")
load(file=temp_file) # recover progress

cat("determining uninitialised variables", "\n")

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

#
token_set_line <- setNames(rep(NA, nrow(tokens)), tokens$name) # when does a var become available?
token_set_status <- setNames(rep("uninit", nrow(tokens)), tokens$name) # how is it set?
token_set_status[tokens$name[!is.na(tokens$decl_value)]] <- "set" # parameter/constexpr value set in declarations
token_set_line[tokens$name[!is.na(tokens$decl_value)]] <- 1 # parameter/constexpr value set in declarations
token_set_status["procedural"] <- "set" # only used to avoid an empty set list on procedurals
token_set_line["procedural"] <- 1 # only used to avoid an empty set list on procedurals# token_set_status["t"] <- "set" # set by driver at start of dynamic section

# declarations of value
rows <- which(csl$line_type %in% c("constant", "parameter")) #, "integ")) # not sure whether to include integ
set <- setdiff(unique(unlist(str_split(csl$set[rows], ",", simplify=FALSE))), "")
token_set_status[set] <- "set"
token_set_line[set] <- 1

# now work through code ignoring sorting (which has not yet been done)
rows <- 1:nrow(csl)
csl$assumed <- ""
did_continue <- FALSE
for (i in rows){
  if (!(csl$line_type[i] %in% c("header", "initial"))){
    if (is.na(token_set_line["t"])){
      token_set_status["t"] <- "set"
      token_set_line["t"] <- i
    }
  }
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
  } else if (csl$line_type[i]!="procedural") { # analyse
    set <- set[set>""]
    if (any(set %in% c("t"))){
      stop(paste("code line", csl$line_number[i], ": assignment to t in", csl$section[i], "\n"))
    }
    if (any(set %in% c(state)) & csl$line_type[i]!="integ"){
      cat(paste("code line", csl$line_number[i], ": assignment to state variable in", csl$section[i], "\n"))
    }
    if (any(set %in% c(rate)) & csl$section[i]!="derivative"){
      cat(paste("code line", csl$line_number[i], ": assignment to rate variable in", csl$section[i], "\n"))
    }
    used <- used[used>""]
    if (any(used %in% c("t")) & !(csl$section[i] %in% c("dynamic", "discrete", "derivative"))){
      stop(paste("code line", csl$line_number[i], ": use of t in", csl$section[i], "\n"))
    }
    if (any(used %in% c(state)) & !(csl$section[i] %in% c("dynamic", "discrete", "derivative"))){
      cat(paste("code line", csl$line_number[i], ": use of state variable in", csl$section[i], "\n"))
    }
    if (!any(used>"")){ # set only (e.g. initialisation with constant)
      bad <- token_set_status[set] == "uninit"
      token_set_status[set[bad]] <- "set"
      token_set_line[set[bad]] <- i
    } else if (!any(set>"")){ # used only (e.g. in ifthen)
      bad <- token_set_status[used] == "uninit"
      token_set_status[used[bad]] <- "assumed"
      token_set_line[used[bad]] <- i
      csl$assumed[i] <- paste_sort(used[bad])
    } else { # set and used (e.g. assignment)
      bad <- token_set_status[used] == "uninit"
      token_set_status[used[bad]] <- "assumed"
      token_set_line[used[bad]] <- i
      csl$assumed[i] <- paste_sort(used[bad])
      if (any(token_set_status[used]!="set")){
        token_set_status[set] <- "from_assumed"
        token_set_line[set] <- i
      } else { # all set
        token_set_status[set] <- "set" # note : assumes any procedural declaration is correct
        token_set_line[set] <- i
      }
    }
    did_continue <- FALSE
  }
}

# save results in df
tokens <- tokens %>%
  mutate(set_line=unname(token_set_line),
         set_status=unname(token_set_status))

# save progress
rm(list=setdiff(ls(), c("csl", "tokens", "path_name", "silent", lsf.str())))
temp_file <- paste(path_name, "checkpoint_after_parse_three.RData", sep="/")
save.image(temp_file)

# a handful of variables are "assumed", i.e. used without being set first
# some might also be set in discrete, but it's difficult to analyse whether this will occur
# need to use tokens$set_line to see if it's a problem

# do overall input outpus analysis of current section
# (includes variables hidden inside procedurals)
# set_init <- names(token_set_status)[token_set_status!="uninit"] # known status
# set_discrete <- unique(unlist(str_split(csl$set[csl$section=="discrete"], ","))) # not guaranteed
# used_discrete <- unique(unlist(str_split(csl$used[csl$section=="discrete"], ","))) # not guaranteed
# setdiff(set_discrete, set_init)
# setdiff(used_discrete, set_init)
# setdiff(set_discrete, used_discrete)
# used_here <- unique(unlist(str_split(index$used, ","))) # ignoring sorting
# set_here <- unique(unlist(str_split(index$set, ","))) # ignoring sorting
# used_but_not_set <- setdiff(used_here, set_here) # can be considered constants (includes t and state)
# set_but_not_used <- setdiff(set_here, c(used_here, rate)) # can be considered post-processing
# setdiff(used_but_not_set, names(token_set_status)[token_set_status!="uninit"])

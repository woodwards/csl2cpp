#### third pass - sorting dynamic lines into calc and post processing ####
# thoughts:
# taking unnecessary lines out of calculate_rates will speed it up a lot.
#

temp_file <- paste(path_name, "checkpoint_after_parse_two.RData", sep="/")
load(file=temp_file) # recover progress

cat(file=stderr(), "sorting derivative section code", "\n")
override_procedural_declarations <- FALSE
cat(file=stderr(), "override procedural declarations :", override_procedural_declarations, "\n")

# provides index to csl to allow easier sorting
i <- which(csl$section=="derivative")
base <- min(csl$block[i]) # lines in this block can be sorted
index <- data_frame(line = i,
                    block = csl$block[i],
                    begin = i,
                    end = i,
                    code = csl$calc[i],
                    line_type = csl$line_type[i],
                    set = csl$set[i], # outputs
                    used = csl$used[i], # inputs
                    cont = csl$cont[i], # continuation
                    unset = "", # unsorted
                    unsetline = "", # where unset is last set
                    notused = "",
                    newline = "",
                    moved = FALSE
                    )

# paste two comma separated strings (not vectorised)
paste_two <- function(a, b){
  if (a=="" | b==""){
    return(max(a, b, na.rm=TRUE))
  } else {
    all <- unique(c(str_split(a, ",", simplify=TRUE)[1,], str_split(b, ",", simplify=TRUE)[1,]))
    return(str_c(all, collapse=","))
  }
}

# collapse continuations
cont_lines <- which(index$cont>"")
while (length(cont_lines)>0){
  i <- max(cont_lines) + 1 # continuation character is on previous line
  index$end[i-1] <- index$end[i]
  index$code[i-1] <- "*** collapsed continuation ***"
  index$set[i-1] <- paste_two(index$set[i-1], index$set[i])
  index$used[i-1] <- paste_two(index$used[i-1], index$used[i])
  index$cont[i-1] <- ""
  index <- index[-i, ] # remove line
  cont_lines <- which(index$cont>"")
}

# collapse blocks
non_base <- index$block != base & lag(index$block,1) != base # valid lines to collapse
while (any(non_base)){
  i <- max(which( index$block == max(index$block[non_base]) )) # most indented block
  index$end[i-1] <- index$end[i]
  index$code[i-1] <- "*** collapsed block ***"
  # stopifnot(index$line_type[i-1] != "procedural")
  if (index$line_type[i-1] != "procedural" | override_procedural_declarations){ # procedural set=used list normally overrides reality
    index$set[i-1] <- paste_two(index$set[i-1], index$set[i])
    index$used[i-1] <- paste_two(index$used[i-1], index$used[i])
  }
  index <- index[-i, ] # remove line
  non_base <- index$block != base & lag(index$block,1) != base # valid lines to collapse
}

# collapse comments into block below
comments <- index$code == ""
while (any(comments)){
  i <- which(comments)[[1]] # first comment
  if (i<nrow(index)){
    index$begin[i+1] <- index$begin[i]
  } else { # last line needs special treatment
    index$end[i-1] <- index$end[i]
  }
  index <- index[-i, ] # remove line
  comments <- index$code == ""
}

# analyse variables (also used in csl2cpp_make.r) FIXME do I need this actually?
integ <- csl$integ[csl$integ > ""]
state <- str_match(integ, "^[:alpha:]+[[:alnum:]_]*")[,1]
rate <- str_trim(str_replace(integ, "^[:alpha:]+[[:alnum:]_]*", "")) # expressions, not just var names
rate <- str_replace_all(rate, "= ", "")
# handle "max ( const , ? )"
i <- which(str_detect(rate, "^max \\( "))
temp <- str_split(rate, " ") # list of vectors of strings
for (ii in i){
  rate[ii] <- temp[[ii]][5]
}
n_state <- length(state)
constant <- tokens$constant[tokens$constant>""]
variable <- tokens$variable[tokens$variable>"" & tokens$decl_type!="string" & tokens$constant==""]
n_variable <- length(variable)

# overall analysis
used_all <- unique(unlist(str_split(index$used, ",")))
used_all <- c(used_all, rate)
set_all <- unique(unlist(str_split(index$set, ",")))
used_but_not_set <- setdiff(used_all, set_all) # can be considered constants (includes t and state)
set_but_not_used <- setdiff(set_all, used_all) # can be considered post-processing

# see if any were not initialised
uninitialised <- which(token_list %in% used_but_not_set & token_set_line == 0)

# loop until no problems
jump_max <- "1"
pass <- 0
# stop()
while(any(jump_max>"")){

  # count passes
  pass <- pass + 1

  # locate variable usage
  var_set <- c() # set so far (excluding those which are not used in derivative)
  for (i in 1:nrow(index)){
    # record any set that will be used
    if (index$set[i]>""){
      set <- str_split(index$set[i], ",", simplify=TRUE)
      set <- setdiff(set, set_but_not_used) # ignore "post processing"
      var_set[set] <- index$line[i]
    }
  }
  var_set_all <- var_set
  # find unset variables
  var_set <- c()
  for (i in 1:nrow(index)){
    # record any set that will be used
    if (index$set[i]>""){
      set <- str_split(index$set[i], ",", simplify=TRUE)
      notused <- intersect(set, set_but_not_used)
      index$notused[i] <- paste(notused, collapse=",")
      set <- setdiff(set, set_but_not_used) # ignore "post processing"
      var_set[set] <- index$line[i]
    }
    # record any used but not set
    if (index$used[i]>""){
      used <- str_split(index$used[i], ",", simplify=TRUE)
      used <- setdiff(used, used_but_not_set) # ignore "constants"
      used <- setdiff(used, names(var_set))
      if (length(used)>0){ # record any used but unset
        index$unset[i] <- paste(used, collapse=",")
        index$unsetline[i] <- paste(var_set_all[used], collapse=",") # line number where last set
      } else {
        index$unset[i] <- ""
        index$unsetline[i] <- ""
      }
    }
  }

  # sort
  index$newline <- 1:nrow(index)
  jump_list <- str_split(index$unsetline, ",")
  jump_max <- unlist(lapply(jump_list, function(x) max(unlist(x)))) # max jump for each line
  jump_line <- jump_max > ""
  jump_i <- match( as.integer(jump_max) , index$line ) + runif(nrow(index)) # avoid identical
  cat("sorting pass", pass, ":" , sum(jump_line), "jumps remaining\n")
  if (any(jump_line)){
    j <- jump_line # or chose one
    index$newline[j] <- i[j]
    index$moved[j] <- TRUE
    index <- arrange(index, newline)
  }

}

# save progress
temp_file <- paste(path_name, "checkpoint_after_parse_three.RData", sep="/")
save.image(temp_file)



#### third pass - sorting dynamic lines into calc and post processing ####
# thoughts:
# taking unnecessary lines out of calculate_rates will speed it up a lot.
#

temp_file <- paste(path_name, "checkpoint_after_parse_two.RData", sep="/")
load(file=temp_file) # recover progress

cat("sorting derivative section code", "\n")

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

# paste and unique and sort a vector of comma separated strings (not vectorised)
paste_sort <- function(x){
  x <- str_split(x[!is.na(x)], ",", simplify=TRUE)
  x <- sort(setdiff(unique(x), ""))
  x <- max(str_c(x, collapse=","),"")
  return(x)
}

# sort var lists
for (i in 1:nrow(index)){
  index$set[i] <- paste_sort(index$set[i])
  index$used[i] <- paste_sort(index$used[i])
}

# collapse continuations
cont_lines <- which(index$cont>"")
while (length(cont_lines)>0){
  i <- max(cont_lines) + 1 # continuation character is on previous line
  index$end[i-1] <- index$end[i]
  index$code[i-1] <- "*** collapsed continuation ***"
  index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
  index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
  index$cont[i-1] <- ""
  index <- index[-i, ] # remove line
  cont_lines <- which(index$cont>"")
}

# collapse blocks upwards (the logic is a bit tricky to get right)
hyphens_base <- str_count(base, "-")
# 1. >base blocks that have identical block to previous
# 2. lines at the highest indent level >base+1
hyphens <- str_count(index$block, "-")
collapsible1 <- hyphens > hyphens_base & index$block == lag(index$block, 1)
collapsible2 <- hyphens > hyphens_base + 1 & hyphens == max(hyphens)
while (any(collapsible1 | collapsible2)){
  i <- max(which(collapsible1 | collapsible2)) # choose a line
  index$end[i-1] <- index$end[i]
  if (index$line_type[i-1] != "procedural"){
    index$code[i-1] <- "*** collapsed block ***"
    index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
    index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
  } else { # analyse procedural declaration
    index$code[i-1] <- "*** collapsed procedural ***"
    # act_set <- paste_sort(c(index$set[i], "procedural"))
    # act_used <- index$used[i]
    # if (act_set == index$set[i-1] & act_used == index$used[i-1]){
    #   cat("line", index$line[i-1], ": procedural honest\n")
    # } else {
    #   cat("line", index$line[i-1], ": procedural dishonest\n")
    #   cat("declared set  :", index$set[i-1], "\n")
    #   cat("actual   set  :", act_set, "\n")
    #   cat("declared used :", index$used[i-1], "\n")
    #   cat("actual used   :", act_used, "\n")
    # }
    # override_procedural_declaration <- FALSE # would be nice but sort fails
    # if (override_procedural_declaration){
    #   index$set[i-1] <- act_set
    #   index$used[i-1] <- act_used
    # }
  }
  index <- index[-i, ] # remove line
  hyphens <- str_count(index$block, "-")
  collapsible1 <- hyphens > hyphens_base & index$block == lag(index$block, 1)
  collapsible2 <- hyphens > hyphens_base + 1 & hyphens == max(hyphens)
}

# collapse non-active lines into line below (has to be done after blocks)
inactive <- index$set=="" | index$used==""
while (any(inactive)){
  i <- which(inactive)[[1]] # first comment
  if (i<nrow(index)){
    index$begin[i+1] <- index$begin[i]
  } else { # last line needs special treatment
    index$end[i-1] <- index$end[i]
  }
  index <- index[-i, ] # remove line
  inactive <- index$set=="" | index$used==""
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
      index$notused[i] <- paste_sort(notused)
      set <- setdiff(set, set_but_not_used) # ignore "post processing"
      var_set[set] <- index$line[i]
    }
    # record any used but not set
    if (index$used[i]>""){
      used <- str_split(index$used[i], ",", simplify=TRUE)
      used <- setdiff(used, used_but_not_set) # ignore "constants"
      used <- setdiff(used, names(var_set))
      if (length(used)>0){ # record any used but unset
        index$unset[i] <- paste_sort(used)
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
    j <- which(jump_line) # all lines
    # j <- sample(which(jump_line),1) # choose jump(s)
    index$newline[j] <- jump_i[j]
    index$moved[j] <- TRUE
    index <- arrange(index, newline)
  }

}

# save progress
temp_file <- paste(path_name, "checkpoint_after_parse_three.RData", sep="/")
save.image(temp_file)



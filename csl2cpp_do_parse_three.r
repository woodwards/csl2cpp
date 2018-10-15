#### third pass - sorting dynamic lines into calc and post processing ####
# thoughts:
# some ACSL model might have several sections to be sorted, currently only handle one
# taking unnecessary lines out of calculate_rates to post_processing could speed it up a lot.
# procedurals group lines together and make it harder to sort
# can't handle implicit systems
# user should ensure variables are initialised and equations/procedurals are sortable
# but if not, we still want a result
# need to handle variables that are not initialised (ACSLX sets these to strange values, in C++ they are NA)
# how to handle unsortable lines?

temp_file <- paste(path_name, "checkpoint_after_parse_two.RData", sep="/")
load(file=temp_file) # recover progress

cat("sorting derivative section code", "\n")

override_proc_set <- TRUE # would be nice but sort fails
override_proc_used <- FALSE # would be nice but sort fails
sorting_method <- "simon"
# sorting_method <- "acslx"
# sorting_method <- "none"
cat("override procedural set list :", override_proc_set, "\n")
cat("override procedural used list :", override_proc_used, "\n")
cat("sorting method :", sorting_method, "\n")

# paste and unique and sort a vector of comma separated strings (not vectorised)
# supplying a second argument also allows you to remove elements (using setdiff)
paste_sort <- function(x,y=""){
  x <- str_split(x[!is.na(x)], ",", simplify=TRUE)
  y <- str_split(y[!is.na(y)], ",", simplify=TRUE)
  x <- setdiff(x, y) # remove elements of y
  x <- sort(setdiff(unique(x), ""))
  x <- max(str_c(x, collapse=","),"")
  return(x)
}
# do reverse of paste_sort() (not vectorised)
comma_split <- function(s){
  if (is.na(s)) { "" } else { str_split(s, ",", simplify=TRUE)[1,] }
}

#### analyse variable initialisation ###
cat("analyse initialisation\n")
status <- setNames(rep("uninit", nrow(tokens)), tokens$name)
status[tokens$name[!is.na(tokens$decl_value)]] <- "set" # parameter/constexpr value set in declarations
status["procedural"] <- "set" # only used to avoid empty set list on procedurals
status["t"] <- "set" # set by driver
# in theory the status of variables can change through the code
rows <- which(csl$section %in% c("header", "initial") |
              csl$line_type %in% c("constant", "integer", "logical", "doubleprecision",
                                   "real", "character"))
csl$assumed <- ""
did_continue <- FALSE
for (i in rows){
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
  } else { # will not continue, so analyse
    set <- set[set>""]
    used <- used[used>""]
    if (!any(used>"")){ # set only (e.g. initialisation with constant)
      bad <- status[set] == "uninit"
      status[set[bad]] <- "set"
    } else if (!any(set>"")){ # used only (e.g. ifthen)
      bad <- status[used] == "uninit"
      status[used[bad]] <- "assumed"
      csl$assumed[i] <- paste_sort(used[bad])
    } else { # set and used (e.g. assignment)
      bad <- status[used] == "uninit"
      status[used[bad]] <- "assumed"
      csl$assumed[i] <- paste_sort(used[bad])
      if (any(status[used]!="set")){
        status[set] <- "from_assumed"
      } else { # all set
        status[set] <- "set" # note : assumes any procedural declaration is correct
      }
    }
    did_continue <- FALSE
  }
}
temp <- data_frame(name=names(status),
                   status=status,
                   type=tokens$decl_type[match(names(status), tokens$name)]) %>%
  arrange(status)
cat("variables used without initialisation :", paste(names(status)[status=="assumed"], collapse=", "), "\n")
# a handful of variables are "assumed", i.e. used without being set first
# they are all double in our case
# some might also be set in discrete, but it's difficult to analyse whether this will occur

# provides index to csl to allow easier sorting
i <- which(csl$section=="derivative")
if (max(i)-min(i)+1!=length(i)) {
  stop("multiple derivative blocks not handled")
}
base <- min(csl$block[i]) # lines in this block can be sorted
index <- data_frame(line = i,
                    block = csl$block[i],
                    begin = i,
                    end = i,
                    code = csl$calc[i],
                    line_type = csl$line_type[i],
                    set = csl$set[i], # outputs
                    set_hid = "",
                    used = csl$used[i], # inputs
                    used_hid = "",
                    cont = csl$cont[i], # continuation
                    unset = "", # unsorted
                    unsetline = "", # where unset is last set
                    notused = "",
                    newline = "",
                    saved = 0,
                    dep = "" # equation dependency
                    )

# these lines have no effect in sorted code block (at most they effect decl and init)
inactive <- c("integ", "comment", "blank", "derivative", "end",
              "algorithm", "maxterval", "minterval", "cinterval", "nsteps", "termt",
              "constant", "integer", "logical", "doubleprecision", "real", "character")
index$used[index$line_type %in% inactive] <- ""
index$set[index$line_type %in% inactive] <- ""
index$dep[index$line_type %in% inactive] <- "inactive"

#### collapse continuations ####
cat("collapse continuations", "\n")
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

#### collapse ifthen and do ####
# these are basically multline assignment statement
cat("collapse ifthen and do", "\n")
hyphens_base <- str_count(base, "-")
hyphens <- str_count(index$block, "-")
collapsible1 <- index$block == lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do")
collapsible2 <- index$block > lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do") &
                index$block > lead(index$block, 1) #& lead(index$line_type, 1) %in% c("else", "endif", "elseifthen", "enddo")
collapsible <- collapsible1 | collapsible2
collapsible[is.na(collapsible)] <- FALSE
while (any(collapsible)){
  i <- max(which(collapsible)) # choose a line
  index$end[i-1] <- index$end[i]
  index$code[i-1] <- paste("*** collapsed", index$line_type[i-1], "***")
  index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
  index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
  index <- index[-i, ] # remove line
  hyphens <- str_count(index$block, "-")
  collapsible1 <- index$block == lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do")
  collapsible2 <- index$block > lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do") &
    index$block > lead(index$block, 1) #& lead(index$line_type, 1) %in% c("else", "endif", "elseifthen", "enddo")
  collapsible <- collapsible1 | collapsible2
  collapsible[is.na(collapsible)] <- FALSE
}

#### collapse procedurals with goto, ifgoto ####
cat("collapse procedurals with goto, ifgoto", "\n")
has_goto <- unique(index$block[index$line_type %in% c("goto", "ifgoto")])
for (blocki in has_goto){ # collapse these blocks
  lines <- which(index$block==blocki)
  lines <- min(lines):max(lines)
  if (length(lines)>1){ # not collapsed yet
    cat("collapsing", blocki, "\n")
    # stopifnot(blocki!="1-2-5-17")
    i <- head(lines, 1)
    stopifnot(index$line_type[i]=="procedural")
    j <- tail(lines, -1)
    index$end[i] <- index$end[tail(j, 1)]
    index$code[i] <- paste("*** collapsed procedural ***")
    if (override_proc_set){
      index$set[i] <- paste_sort(index$set[j]) # ignore procedural declaration
    } else {
      index$set_hid[i] <- paste_sort(index$set[j], index$set[i]) # these were hid
    }
    if (override_proc_used){
      index$used[i] <- paste_sort(index$used[j]) # ignore procedural declaration
    } else {
      index$used_hid[i] <- paste_sort(index$used[j], index$used[i]) # these were hid
    }
    has_for_rate <- any(index$dep[j] %in% c("for_rate"))
    index$dep[i] <- if_else(has_for_rate, "for_rate", "")
    index <- index[-j, ] # remove lines
  }
}

#### collapse other procedurals ####
cat("collapse other procedurals", "\n")
has_proc <- unique(index$block[index$line_type %in% c("procedural")])
for (blocki in has_proc){ # collapse these blocks
  lines <- which(index$block==blocki)
  lines <- min(lines):max(lines)
  if (length(lines)>1){ # not collapsed yet
    cat("collapsing", blocki, "\n")
    # stopifnot(blocki!="1-2-5-27")
    i <- head(lines, 1)
    stopifnot(index$line_type[i]=="procedural")
    j <- tail(lines, -1)
    index$end[i] <- index$end[tail(j, 1)]
    index$code[i] <- paste("*** collapsed procedural ***")
    if (override_proc_set){
      index$set[i] <- paste_sort(index$set[j]) # ignore procedural declaration
    } else {
      index$set_hid[i] <- paste_sort(index$set[j], index$set[i]) # these were hid
    }
    if (override_proc_used){
      index$used[i] <- paste_sort(index$used[j]) # ignore procedural declaration
    } else {
      index$used_hid[i] <- paste_sort(index$used[j], index$used[i]) # these were hid
    }
    has_for_rate <- any(index$dep[j] %in% c("for_rate"))
    index$dep[i] <- if_else(has_for_rate, "for_rate", "")
    index <- index[-j, ] # remove lines
  }
}

#### analyse equation dependency ####

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

# do overall input outpus analysis of current section
# (includes variables hidden inside procedurals)
set_init <- names(status)[status!="uninit"] # known status
set_discrete <- unique(unlist(str_split(csl$set[csl$section=="discrete"], ","))) # not guaranteed
used_here <- unique(unlist(str_split(index$used, ","))) # ignoring sorting
set_here <- unique(unlist(str_split(index$set, ","))) # ignoring sorting
used_but_not_set <- setdiff(used_here, set_here) # can be considered constants (includes t and state)
set_but_not_used <- setdiff(set_here, c(used_here, rate)) # can be considered post-processing
setdiff(used_but_not_set, names(status)[status!="uninit"])

# identify which equations are needed to calculate the rate FIXME track variable lists
# keep a list of variables set, needed
cat("analyse equation dependency", "\n")
var_dep <- c()
var_dep[rate] <- "rate"
var_dep[state] <- "state"
var_dep["t"] <- "t"
sweep <- c(seq(nrow(index), 1, -1), seq(2, nrow(index), 1))
nsweep <- 0
change <- TRUE
while (change){
  nsweep <- nsweep + 1
  cat("sweep", nsweep, "\n")
  change <- FALSE
  for (i in sweep[index$dep[sweep]==""]){
    if (index$line_type[i] %in% inactive) {
      index$dep[i] <- "inactive"
    } else { # is a known variable getting set?
      set <- paste_sort(c(index$set[i], index$set_hid[i]))
      set <- comma_split(set)
      set_types <- var_dep[set]
      if (any(set_types %in% c("state", "t"))){
        stop(paste("index line", i, ": illegally set state variable or t"))
      } else if (any(set_types %in% c("rate", "for_rate"))){
        index$dep[i] <- "for_rate"
        used <- paste_sort(c(index$used[i], index$used_hid[i]))
        used <- comma_split(used)
        used_types <- var_dep[used]
        if (any(is.na(used_types))){
          var_dep[used[is.na(used_types)]] <- "for_rate" # new var
          change <- TRUE
        }
      }
    }
  } # loop
} # while

#### collapse non-active lines into line below ####
# has to be done after blocks
cat("collapse non-active lines", "\n")
inactive <- index$dep=="inactive"
while (any(inactive)){
  i <- which(inactive)[[1]]
  if (i<nrow(index)){
    index$begin[i+1] <- index$begin[i]
  } else { # last line needs special treatment
    index$end[i-1] <- index$end[i]
  }
  index <- index[-i, ] # remove line
  inactive <- index$dep=="inactive"
}

#### simon's sorting algorithm ####
if (sorting_method=="simon"){
  # loop until no problems
  cat("sort using simon's method\n")
  jump_to <- "1"
  pass <- 0
  while(any(jump_to>"")){

    if (TRUE){

      # count passes
      pass <- pass + 1
      for_rate <- which(index$dep == "for_rate")
      index$unset <- ""
      index$unsetline <- ""

      # locate lines where variables are set
      var_set_all <- c() # set so far (excluding those which are not used in derivative)
      for (i in for_rate){
        # record any set that will be used
        if (index$set[i]>""){
          set <- index$set[i]
          set <- comma_split(set)
          set <- setdiff(set, set_but_not_used) # ignore "post processing"
          var_set_all[set] <- index$line[i]
        }
      }
      # find unset variables
      var_set <- c()
      for (i in for_rate){
        # record any set that will be used
        if (index$set[i]>""){
          set <- index$set[i]
          set <- comma_split(set)
          set <- setdiff(set, set_but_not_used) # ignore "post processing"
          var_set[set] <- index$line[i]
        }
        # record any used but not yet set
        if (index$used[i]>""){
          used <- index$used[i]
          used <- comma_split(used)
          used <- setdiff(used, used_but_not_set) # ignore "constants"
          used <- setdiff(used, names(var_set)) # set so far
          if (length(used)>0){ # record any used but currently unset
            index$unset[i] <- paste_sort(used)
            index$unsetline[i] <- paste(var_set_all[used], collapse=",") # line number where last set
          }
        }
      }

      # sort
      index$newline <- 1:nrow(index)
      jump_list <- str_split(index$unsetline, ",") # returns matrix of strings
      jump_to <- unlist(lapply(jump_list, function(x) max(unlist(x)))) # jump for each line
      jump_line <- jump_to > ""
      jump_i <- match( as.integer(jump_to) , index$line ) + runif(nrow(index)) # avoid identical
      cat("sorting pass", pass, ":" , sum(jump_line), "jumps remaining\n")

    }

    if (any(jump_line)){

      j <- which(jump_line) # all lines
      # j <- head(which(jump_line), 1) # choose jumps
      index$newline[j] <- jump_i[j]
      index <- arrange(index, newline)

    }

  }

  # find unset variables remaining (using actual procedural set used lists)
  for_rate <- which(index$dep == "for_rate")
  index$unset <- ""
  var_set <- c()
  for (i in for_rate){
    # record any set
    set <- paste_sort(c(index$set[i], index$set_hid[i]))
    if (set>""){
      set <- comma_split(set)
      var_set[set] <- i
    }
    # check actual used
    used <- paste_sort(c(index$used[i], index$used_hid[i]))
    if (used>""){
      used <- comma_split(used)
      used <- setdiff(used, used_but_not_set) # ignore "constants"
      used <- setdiff(used, names(var_set)) # set so far
      if (length(used)>0){ # record any used but currently unset
        index$unset[i] <- paste_sort(used)
      }
    }
  }
  uninitialised <- paste_sort(index$unset[for_rate])
  cat("warning, uninitialised variables :", uninitialised, "\n")

} # end of simon's method

#### acslx sorting method ####
if (sorting_method=="acslx"){
  # index <- filter(index, dep=="for_rate")
  # locate variable usage
  cat("sort using acslx method\n")
  var_set_all <- c()
  for (i in 1:nrow(index)){
    if (index$set[i]>""){
      set <- comma_split(index$set[i])
      var_set_all[set] <- i
    }
  }
  var_set <- c()
  var_set[used_but_not_set] <- 0
  # var_set[c("dOx", "GlTO")] <- 0
  saved <- 0
  i <- 1
  while (i <= nrow(index)){

    if (index$saved[i]>0){ # already saved

      i <- i + 1

    } else { # unsaved

      # set
      if (index$set[i]>""){
        set <- comma_split(index$set[i])
      } else {
        set <- ""
      }

      # used but not available
      if (index$used[i]>""){
        used <- comma_split(index$used[i])
        used <- setdiff(used, used_but_not_set) # ignore "constants" including t and state
        used <- setdiff(used, names(var_set)) # ignore already set
        index$unset[i] <- paste_sort(used)
        index$unsetline[i] <- paste(var_set_all[used], collapse=",") # line number where last set
      } else {
        used <- c()
        index$unset[i] <- ""
        index$unsetline[i] <- ""
      }

      # save line if all used variables are set
      if (index$dep[i]==""){
        saved <- saved + 1
        index$saved[i] <- saved # save line
        # cat("saved", saved, "\n")
      } else if (length(used)==0){
        saved <- saved + 1
        index$saved[i] <- saved # save line
        # cat("saved", saved, "\n")
        var_set[set] <- saved # expand list of set vars
        i <- 1 # go back to top and recheck unsaved
      } else {
        i <-  i + 1 # don't save, continue
      }

    } # end if

  } # end while
  failed <- index$saved == 0
  cat("failed to sort", ceiling(sum(failed)/nrow(index)*100), "%\n")
  index <- arrange(index, saved) # sort lines
} # end of ACSLX method

#### save progress ####
temp_file <- paste(path_name, "checkpoint_after_parse_three.RData", sep="/")
save.image(temp_file)



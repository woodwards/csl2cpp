#### third pass - sorting dynamic lines into calc and post processing ####
# thoughts:
# some ACSL model might have several sections to be sorted
# taking unnecessary lines out of calculate_rates to post_processing could speed it up a lot.
# procedurals group lines together and make it harder to sort
# procedurals may lie about what variables are set used
# can't handle implicit systems
# user should ensure variables are initialised and equations/procedurals are sortable
# but if not, we still want a result
# how to handle unsortable lines?

temp_file <- paste(output_dir, "checkpoint_after_parse_two.RData", sep="/")
load(file=temp_file) # recover progress

cat("sorting derivative section code", "\n")

# options
collapse_included <- FALSE # might prevent sorting (experimental)
override_proc_set <- TRUE # helps sort
override_proc_used <- FALSE # would be nice but sort might fail
# sorting_method <- "simon" # only works for DERIVATIVE, when we know the outputs of the section
sorting_method <- "acslx"
# sorting_method <- "none"
cat("unsortable included blocks :", collapse_included, "\n")
cat("override procedural set list :", override_proc_set, "\n")
cat("override procedural used list :", override_proc_used, "\n")
cat("sorting method :", sorting_method, "\n")

# reorganise major sections into execution order
# FIXME could cause errors if SORT keyword in DYNAMIC section?
# FIXME can't handle multiple DYNAMIC sections
cat("reorganise major sections\n")
major_sections <- c("header", "initial", "dynamic", "derivative", "discrete", "terminal", "mfile")
if (!all(csl$section %in% major_sections)){
  stop("unknown major section")
}
new_i <- c(
  which(csl$section %in% c("header")),
  which(csl$section %in% c("initial")),
  which(csl$section %in% c("discrete")), # possibly executed before time step
  which(csl$section %in% c("derivative")),
  which(csl$section %in% c("dynamic")), # executed after time step
  which(csl$section %in% c("terminal")),
  which(csl$section %in% c("mfile"))
)
csl <- csl[new_i,] # sort in this order
csl$dep <- "" # add equation dependence to rate calculations

# get model state, rate and derivt variables
integ <- csl$integ[csl$line_type %in% c("integ", "intvc")]
state <- str_match(integ, "^[:alpha:]+[[:alnum:]_]*")[,1]
rate <- str_trim(str_replace(integ, "^[:alpha:]+[[:alnum:]_]*", ""))
rate <- str_replace_all(rate, "= ", "")
derivt <- csl$integ[csl$line_type %in% c("derivt")]
slope <- str_match(derivt, "^[:alpha:]+[[:alnum:]_]*")[,1]
slopeof <- str_trim(str_replace(derivt, "^[:alpha:]+[[:alnum:]_]*", ""))
slopeof <- str_replace_all(slopeof, "= ", "")

# deal with constant rate
rate <- rate[is.na(as_numeric(rate, NA_real_))]

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

# load dependence function
source("csl2cpp_dependence.r")

# warn user
if (sum(csl$line_type=="derivative")>1) {
  cat("sorting of multiple derivative blocks not tested\n")
} else if (sum(csl$line_type=="sort")>0){
  cat("sorting of sort blocks not tested\n")
}

#### choose block to sort ####
sortable <- which(csl$line_type %in% c("derivative", "sort"))

# sort next block
while (length(sortable)>0){

  # set up
  i <- sortable[[1]] # start line for sorting
  base_block <- csl$block[i] # lines in this block (after i) will be sorted
  base_type <- csl$line_type[i]
  base_i <- which(csl$block==base_block)
  base_i <- seq(max(i, min(base_i)), max(base_i))
  cat("sorting", csl$line_type[i], "block", base_block, ",", length(base_i), "lines\n")
  csl$line_type[i] <- paste(csl$line_type[i], "_sorted", sep="")  # prevent resorting

  # report files involved
  sorted_file_names <- unique(csl$file_name[base_i])
  cat(paste("\t", sorted_file_names, "\n"))

  # provide index to allow easier sorting
  i <- base_i
  index <- tibble(line = i,
				  file_name = csl$file_name[i],
				  line_number = csl$line_number[i],
				  block = csl$block[i],
				  begin = i,
				  end = i,
				  code = csl$calc[i], # only works on derivative code!
				  line_type = csl$line_type[i],
				  sort = TRUE,
				  set = csl$set[i], # outputs
				  set_hid = "",
				  used = csl$used[i], # inputs
				  used_hid = "",
				  cont = csl$cont[i], # continuation
				  unset = "", # unsorted
				  unsetline = "", # where unset is last set
				  assumed = "",
				  newline = "",
				  saved = 0,
				  dep = "" # equation dependency
  )

  # these lines have no effect on sorting of code (at most they effect decl and init)
  inactive <- c("integ", "intvc", "comment", "blank", "derivative", "termt", "sort", "derivt",
                "derivative_sorted", "sort_sorted", "included", "end",
                "algorithm", "maxterval", "minterval", "cinterval", "nsteps",
                "constant", "parameter",
                "integer", "dimension", "logical", "doubleprecision", "real", "character")
  # View(filter(index, index$line_type %in% inactive))
  index$set[index$line_type %in% inactive] <- "" # these are set elsewhere
  index$used[index$line_type %in% inactive] <- "" # these are used elsewhere
  index$dep[index$line_type %in% inactive] <- "inactive"

  #### collapse continuations ####
  cat("collapse continuations", "\n")
  cont_lines <- which(index$cont>"" & index$sort)
  # View(index[cont_lines,])
  while (length(cont_lines)>0){
    i <- max(cont_lines) + 1 # continuation character is on previous line
    index$end[i-1] <- index$end[i]
    index$code[i-1] <- "*** collapsed continuation ***"
    index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
    index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
    stopifnot(index$dep[i] %in% c(index$dep[i-1], "inactive"))
    index$cont[i-1] <- ""
    index <- index[-i, ] # remove line
    cont_lines <- which(index$cont>"")
  }

  #### collapse ifthen and do ####
  # these are basically multline assignment statements
  cat("collapse ifthen and do", "\n")
  hyphens_base <- str_count(base_block, "-")
  hyphens <- str_count(index$block, "-")
  collapsible1 <- index$block == lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do")
  collapsible2 <- index$block > lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do") &
                  index$block > lead(index$block, 1) #& lead(index$line_type, 1) %in% c("else", "endif", "elseifthen", "enddo")
  collapsible <- (collapsible1 | collapsible2) & index$sort
  collapsible[is.na(collapsible)] <- FALSE
  while (any(collapsible)){
    i <- max(which(collapsible)) # choose a line
    index$end[i-1] <- index$end[i]
    index$code[i-1] <- paste("*** collapsed", index$line_type[i-1], "***")
    index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
    index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
    stopifnot(index$dep[i] %in% c(index$dep[i-1], "inactive"))
    index <- index[-i, ] # remove line
    hyphens <- str_count(index$block, "-")
    collapsible1 <- index$block == lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do")
    collapsible2 <- index$block > lag(index$block, 1) & lag(index$line_type, 1) %in% c("ifthen", "do") &
      index$block > lead(index$block, 1) #& lead(index$line_type, 1) %in% c("else", "endif", "elseifthen", "enddo")
    collapsible <- (collapsible1 | collapsible2) & index$sort
    collapsible[is.na(collapsible)] <- FALSE
  }

  #### analyse variable dependency in derivative ####
  cat("analyse variable dependency in derivative - pre sort", "\n")
  tokens <- csl_dependence(csl, tokens, silent=TRUE)
  used_here <- setdiff(unique(unlist(str_split(index$used[index$sort], ","))), "")
  set_here <- setdiff(unique(unlist(str_split(index$set[index$sort], ","))), "")
  set_but_not_used <- setdiff(set_here, c(used_here, rate, slopeof))
  available_here <- tokens$name[tokens$set_line<min(base_i)]
  available_here <- setdiff(available_here, c(rate, slopeof)) # rate and slopeof should be calculated (usually, they could be constant)
  assumed_here <- setdiff(c(used_here, rate, slopeof) , c(available_here, state, set_here, "t"))
  if (length(assumed_here)>0){
    message <- paste("*** illegal uninitialised variables :", paste(assumed_here, collapse=" "))
    cat(message, "\n")
  }
  used_but_not_set <- setdiff(c(used_here, rate, slopeof), c(set_here, available_here, assumed_here, state, "t"))

  #### analyse equation dependency in derivative ####
  if (base_type=="derivative"){   # only applied to derivative section
    cat("analyse equation dependency in derivative", "\n")
    var_dep <- c()
    var_dep[rate] <- "rate"
    var_dep[slopeof] <- "rate" # we also require slopeof for numerical derivative
    var_dep[state] <- "state"
    var_dep["t"] <- "t"
    i <- which(index$sort)
    sweep <- c(seq(max(i), min(i), -1), seq(min(i)+1, max(i), 1)) # sweep both ways
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
          set <- comma_split(paste_sort(c(index$set[i])))
          set_types <- var_dep[set]
          if (any(set_types %in% c("state", "t"))){
            stop(paste(index$file_name[i], index$line_number[i], "ff : illegally set state variable or t\n"))
          } else if (any(set_types %in% c("rate", "for_rate"))){
            index$dep[i] <- "for_rate"
            used <- comma_split(paste_sort(c(index$used[i])))
            used_types <- var_dep[used]
            if (any(is.na(used_types))){
              var_dep[used[is.na(used_types)]] <- "for_rate" # new var
              change <- TRUE
            }
          }
        }
      } # loop
    } # while
  }

  #### collapse procedurals with goto, ifgoto ####
  cat("collapse procedurals with goto, ifgoto", "\n")
  has_goto <- unique(index$block[index$line_type %in% c("goto", "ifgoto")])
  for (blocki in has_goto){ # collapse these blocks where index$sort==TRUE
    lines <- which(index$block==blocki & index$sort)
    if (length(lines)>1){ # not collapsed yet
      lines <- min(lines):max(lines)
      # cat("collapsing", blocki, "\n")
      cat(blocki, " ")
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
  if (length(has_goto)>0) cat("\n")

  #### collapse other procedurals ####
  cat("collapse other procedurals", "\n")
  has_proc <- unique(index$block[index$line_type %in% c("procedural")])
  for (blocki in has_proc){ # collapse these blocks where index$sort==TRUE
    lines <- which(index$block==blocki & index$sort)
    if (length(lines)>1){ # not collapsed yet
      lines <- min(lines):max(lines)
      # cat("collapsing", blocki, "\n")
      cat(blocki, " ")
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
  if (length(has_proc)>0) cat("\n")

  #### collapse inactive lines except included ####
  # has to be done after blocks
  cat("collapse inactive lines except included \n")
  collapse_down <- c("comment", "derivative", "sort", "derivative_sorted", "sort_sorted",
                      "constant", "parameter",
                      "integer", "dimension", "logical", "doubleprecision", "real", "character")
  collapse_up <- c("blank", "termt", "integ", "intvc", "derivt",
                   "algorithm", "nsteps", "maxterval", "minterval", "cinterval")
  dont_collapse <- setdiff(inactive, c(collapse_down, collapse_up))
  inactive <- index$line_type %in% c(collapse_down, collapse_up) & index$sort
  while (any(inactive)){
    i <- which(inactive)[[1]]
    if (i!=1 && (index$line_type[i] %in% collapse_up || i==nrow(index))){
      index$end[i-1] <- index$end[i] # collapse upwards
    } else {
      index$begin[i+1] <- index$begin[i] # collapse downwards
    }
    index <- index[-i, ] # remove line
    inactive <- index$line_type %in% c(collapse_down, collapse_up) & index$sort
  }

  #### collapse included blocks ####
  # View(filter(index, line_type %in% c("included", "end")))
  # this feature is experimental and not fully operational
  # the intention is to see which included blocks do not get shuffled
  if (collapse_included){
    cat("collapse included blocks\n")
    hyphens_base <- str_count(base_block, "-")
    hyphens <- str_count(index$block, "-")
    collapsible1 <- index$block == lag(index$block, 1) & lag(index$line_type, 1) %in% c("included")
    collapsible2 <- index$block > lag(index$block, 1) & lag(index$line_type, 1) %in% c("included") &
      index$block > lead(index$block, 1)
    collapsible2 <- FALSE
    collapsible3 <- (index$set == "" & index$set_hid == "" & index$used == "" & index$used_hid == "") |
      (lag(index$set, 1) == "" & lag(index$set_hid, 1) == "" & lag(index$used, 1) == "" & lag(index$used_hid, 1) == "")
    collapsible <- (collapsible1 | collapsible2) & collapsible3 & index$sort
    collapsible[is.na(collapsible)] <- FALSE
    stop()
    while (any(collapsible)){
      i <- max(which(collapsible)) # choose a line
      index$end[i-1] <- index$end[i]
      index$code[i-1] <- paste("*** collapsed", index$line_type[i-1], index$file_name[i], "***")
      index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
      index$set_hid[i-1] <- paste_sort(c(index$set_hid[i-1], index$set_hid[i]))
      index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
      index$used_hid[i-1] <- paste_sort(c(index$used_hid[i-1], index$used_hid[i]))
      index <- index[-i, ] # remove line
      hyphens <- str_count(index$block, "-")
      collapsible1 <- index$block == lag(index$block, 1) & lag(index$line_type, 1) %in% c("included")
      collapsible2 <- index$block > lag(index$block, 1) & lag(index$line_type, 1) %in% c("included") &
        index$block > lead(index$block, 1)
      collapsible2 <- FALSE
      collapsible3 <- (index$set == "" & index$set_hid == "" & index$used == "" & index$used_hid == "") |
        (lag(index$set, 1) == "" & lag(index$set_hid, 1) == "" & lag(index$used, 1) == "" & lag(index$used_hid, 1) == "")
      collapsible <- (collapsible1 | collapsible2) & collapsible3 & index$sort
      collapsible[is.na(collapsible)] <- FALSE
    }
  }

  #### collapse included and end lines ####
  cat("collapse included and end lines\n")
  collapse_down <- c("included")
  collapse_up <- c("end")
  inactive <- index$line_type %in% c(collapse_down, collapse_up) & index$sort
  while (any(inactive)){
    i <- which(inactive)[[1]]
    if (i!=1 && (index$line_type[i] %in% collapse_up || i==nrow(index))){
      index$end[i-1] <- index$end[i] # collapse upwards
    } else {
      index$begin[i+1] <- index$begin[i] # collapse downwards
    }
    index <- index[-i, ] # remove line
    inactive <- index$line_type %in% c(collapse_down, collapse_up) & index$sort
  }

  #### simon's sorting algorithm ####
  # if (sorting_method=="simon"){
  #
  #   # loop until no jumps left
  #   cat("sort using simon's method\n")
  #   jump_to <- "1"
  #   pass <- 0
  #   while(any(jump_to>"")){
  #
  #     if (TRUE){
  #
  #       # count passes
  #       pass <- pass + 1
  #       for_rate <- which(index$dep == "for_rate" & index$sort)
  #       index$unset <- ""
  #       index$unsetline <- ""
  #
  #       # locate lines where variables are set
  #       var_set_all <- c() # set so far (excluding those which are not used in derivative)
  #       for (i in for_rate){
  #         # record any set that will be used
  #         if (index$set[i]>""){
  #           set <- index$set[i]
  #           set <- comma_split(set)
  #           set <- setdiff(set, set_but_not_used)
  #           var_set_all[set] <- index$line[i]
  #         }
  #       }
  #       # find unset variables
  #       var_set <- c()
  #       for (i in for_rate){
  #         # record any set that will be used
  #         if (index$set[i]>""){
  #           set <- index$set[i]
  #           set <- comma_split(set)
  #           set <- setdiff(set, set_but_not_used)
  #           var_set[set] <- index$line[i]
  #         }
  #         # record any used but not yet set
  #         if (index$used[i]>""){
  #           used <- index$used[i]
  #           used <- comma_split(used)
  #           used <- setdiff(used, used_but_not_set)
  #           used <- setdiff(used, names(var_set)) # set so far
  #           if (length(used)>0){ # record any used but currently unset
  #             index$unset[i] <- paste_sort(used)
  #             index$unsetline[i] <- paste(var_set_all[used], collapse=",") # line number where last set
  #           }
  #         }
  #       }
  #
  #       # sort
  #       index$newline <- 1:nrow(index)
  #       jump_list <- str_split(index$unsetline, ",") # returns matrix of strings
  #       jump_to <- unlist(lapply(jump_list, function(x) max(unlist(x)))) # jump for each line
  #       jump_line <- jump_to > ""
  #       jump_i <- match( as.integer(jump_to) , index$line ) + runif(nrow(index)) # avoid identical
  #       cat("sorting pass", pass, ":" , sum(jump_line), "jumps remaining\n")
  #
  #     }
  #
  #     if (any(jump_line)){
  #
  #       j <- which(jump_line) # all lines
  #       # j <- head(which(jump_line), 1) # choose jumps
  #       index$newline[j] <- jump_i[j]
  #       index <- arrange(index, newline)
  #
  #     }
  #
  #   }
  #
  # } # end of simon's method

  #### acslx sorting method ####
  if (sorting_method=="acslx"){

    cat("sort using acslx method\n")
    sort_rows <- which(index$sort)
    index$saved <- seq(nrow(index)) # used if index is whole csl
    index$saved[sort_rows] <- 0
    next_save_at <- min(sort_rows)

    # locate variable set lines in this section
    var_set_all <- c()
    for (i in sort_rows){
      if (index$set[i]>""){
        set <- comma_split(index$set[i])
        var_set_all[set] <- i
      }
    }
    # variables available at beginning of sort section
    var_set <- c()
    var_set[available_here] <- 0 # includes all previously assumed variables
    var_set[assumed_here] <- 0
    var_set[state] <- 0
    var_set["t"] <- 0
    # track variables in this section
    i <- min(sort_rows)
    while (i <= max(sort_rows)){

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
          used <- setdiff(used, c(names(var_set), set)) # ignore already set or assumed or set here
          phantom <- used[is.na(var_set_all[used])] # variable in procedural header does not exist
          if (length(phantom)>0){
            message <- paste("phantom variables :", paste(phantom, collapse=" "))
            cat("\n", message, "\n")
            var_set[phantom] <- next_save_at
            used <- setdiff(used, phantom)
          }
          index$unset[i] <- paste_sort(used)
          index$unsetline[i] <- paste_sort(var_set_all[used])
        } else {
          used <- c()
          index$unset[i] <- ""
          index$unsetline[i] <- ""
        }

        # save line if all used variables are now available
        if (length(used)==0){
          index$saved[i] <- next_save_at # save line
          cat(next_save_at, " ")
          var_set[set[set>""]] <- next_save_at # expand list of set vars with current line
          next_save_at <- next_save_at + 1
          i <- min(sort_rows) # go back to start and recheck unsaved
        } else {
          i <-  i + 1 # don't save, continue
        }

      } # end if

    } # end while
    cat("\n")
    failed <- sum(index$saved==0 & index$sort)
    if (failed>0){
      cat("failed to sort", failed, "of", length(sort_rows), "lines\n")
      unset <- comma_split(paste_sort(index$unset))
      message <- paste("unset variables :", paste(unset, collapse=" "))
      cat(message, "\n")
      stop("failed to sort")
    } else {
      cat("successfully sorted", length(sort_rows), "lines\n")
    }
    index <- arrange(index, saved) # sort lines

  } # end of ACSLX method

  #### analyse variable dependency in derivative ####
  # (FIXME should include set_hid and used_hid)
  # cat("analyse variable dependency in derivative - post sort", "\n")
  # tokens <- csl_dependence(csl, tokens, silent=TRUE)
  # used_here <- setdiff(unique(unlist(str_split(index$used[index$sort], ","))), "")
  # set_here <- setdiff(unique(unlist(str_split(index$set[index$sort], ","))), "")
  # set_but_not_used <- setdiff(set_here, c(used_here, rate, slopeof))
  # available_here <- tokens$name[tokens$set_line<min(base_i)]
  # available_here <- setdiff(available_here, c(rate, slopeof)) # rate and slopeof should be calculated (usually, they could be constant)
  # assumed_here <- setdiff(c(used_here, rate, slopeof) , c(available_here, state, set_here, "t"))
  # if (length(assumed_here)>0){
  #   message <- paste("*** illegal uninitialised variables :", paste(assumed_here, collapse=" "))
  #   cat(message, "\n")
  # }
  # used_but_not_set <- setdiff(c(used_here, rate, slopeof), c(set_here, available_here, assumed_here, state, "t"))
  #
  # # try to find variables that are used before they are set in derivative section (this is illegal)
  # if (base_type=="derivative"){
  #   both_here <- intersect(set_here, used_here)
  #   illegal <- c()
  #   for (j in both_here){
  #     set_line <- which(str_detect(index$set, paste("(?<![[:alnum:]_])", j, "(?![[:alnum:]_])", sep="")))
  #     used_line <- which(str_detect(index$used, paste("(?<![[:alnum:]_])", j, "(?![[:alnum:]_])", sep="")))
  #     if (used_line[[1]]<=set_line[[1]]){
  #       illegal <- c(illegal, j)
  #     }
  #   }
  #   if (length(illegal)>0){
  #     message <- paste("*** illegally updated variables :", paste(illegal, collapse=" "))
  #     cat(message, "\n")
  #   }
  # }

  #### post-sort collapse ####
  cat("post-sort collapse\n")
  hyphens_base <- str_count(base_block, "-")
  hyphens <- str_count(index$block, "-")
  collapsible1 <- index$file_name == lag(index$file_name, 1) & index$begin == lag(index$end + 1, 1)
  collapsible <- collapsible1 & index$sort
  collapsible[is.na(collapsible)] <- FALSE
  while (any(collapsible)){
    i <- max(which(collapsible)) # choose a line
    index$end[i-1] <- index$end[i]
    index$code[i-1] <- if_else(index$code[i-1]=="" || index$code[i]=="", max(index$code[i-1], index$code[i]), " *** collapsed ***")
    index$line_type[i-1] <- "collapsed"
    index$set[i-1] <- paste_sort(c(index$set[i-1], index$set[i]))
    index$set_hid[i-1] <- paste_sort(c(index$set_hid[i-1], index$set_hid[i]))
    index$used[i-1] <- paste_sort(c(index$used[i-1], index$used[i]))
    index$used_hid[i-1] <- paste_sort(c(index$used_hid[i-1], index$used_hid[i]))
    index <- index[-i, ] # remove line
    hyphens <- str_count(index$block, "-")
    collapsible1 <- index$file_name == lag(index$file_name, 1) & index$begin == lag(index$end + 1, 1)
    collapsible <- collapsible1 & index$sort
    collapsible[is.na(collapsible)] <- FALSE
  }

  #### use index to sort csl ####
  prev_i <- seq(nrow(csl)) # where each new line will come from
  dep_i <- csl$dep # new values of dep column
  pos <- min(base_i)
  for (i in seq(nrow(index))){
    lines <- index$begin[i]:index$end[i]
    pend <- pos + length(lines) - 1
    prev_i[pos:pend] <- lines
    dep_i[pos:pend] <- index$dep[i]
    pos <- pend + 1
  }
  csl <- csl[prev_i,]
  csl$dep <- dep_i

  #### any more blocks to sort? ####
  sortable <- which(csl$line_type %in% c("derivative", "sort"))

} # end while

#### finally reanalyse variable dependence ####
cat("analyse variable dependency in derivative - all csl", "\n")
tokens <- csl_dependence(csl, tokens, silent=FALSE)
assumed_all <- tokens$name[tokens$set_status=="assumed"]
if (length(assumed_all)>0){
  message <- paste("*** global uninitialised variables :", paste(assumed_all, collapse=" "))
  cat(message, "\n")
}

#### save progress ####
rm(list=setdiff(ls(), c("csl", "tokens", "output_dir", "model_name", "silent", lsf.str())))
temp_file <- paste(output_dir, "checkpoint_after_parse_three.RData", sep="/")
save.image(temp_file)



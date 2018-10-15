#### fourth pass - determine unitialised variables ####
# thoughts:

temp_file <- paste(path_name, "checkpoint_after_parse_three.RData", sep="/")
load(file=temp_file) # recover progress

cat("determining unitialised variables", "\n")

#### analyse variable initialisation ###
# see ACSL Reference Manual Version 11
# Order of executation: INITIAL, DERIVATIVE (sorted), DISCRETE, DYNAMIC
# constant statement can be anywhere in the code "they are not executable" (p3-2)
# DYNAMIC block statements are actually after DERIVATIVE and DISCRETE

cat("analyse initialisation\n")
status <- setNames(rep("uninit", nrow(tokens)), tokens$name)
status[tokens$name[!is.na(tokens$decl_value)]] <- "set" # parameter/constexpr value set in declarations
status["procedural"] <- "set" # only used to avoid empty set list on procedurals
status["t"] <- "set" # set by driver
# in theory the status of variables can change through the code
rows <- which(csl$section %in% c("header", "initial", "dynamic", "discrete", "derivative") |
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

# do overall input outpus analysis of current section
# (includes variables hidden inside procedurals)
set_init <- names(status)[status!="uninit"] # known status
set_discrete <- unique(unlist(str_split(csl$set[csl$section=="discrete"], ","))) # not guaranteed
used_discrete <- unique(unlist(str_split(csl$used[csl$section=="discrete"], ","))) # not guaranteed
setdiff(set_discrete, set_init)
setdiff(used_discrete, set_init)
setdiff(set_discrete, used_discrete)
used_here <- unique(unlist(str_split(index$used, ","))) # ignoring sorting
set_here <- unique(unlist(str_split(index$set, ","))) # ignoring sorting
used_but_not_set <- setdiff(used_here, set_here) # can be considered constants (includes t and state)
set_but_not_used <- setdiff(set_here, c(used_here, rate)) # can be considered post-processing
setdiff(used_but_not_set, names(status)[status!="uninit"])

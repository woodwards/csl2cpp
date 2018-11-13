#### first pass - split code, identify line_type, gather tokens ####
# thoughts:
# divide each line into label, head, body, continuation, tail
# identify keywords and type of each line
# parse each line into a list of recognised parts (drop whitespace and tidy up a bit)
# calculate indent and connect begin and end markers (using stack)

temp_file <- paste(output_dir, "checkpoint_after_read.RData", sep="/")
load(file=temp_file) # recover progress

cat("parsing code for tokens, line type, indent", "\n")

source("csl2cpp_parse.r") # load functions

silent=FALSE
split_lines=TRUE

csl <- csl %>%
  mutate(
    line_type = NA,
    indent = NA,
    stack = NA,
    block = "",
    length = NA,
    label = NA,
    head = NA,
    body = NA,
    cont = NA,
    tail = NA,
    parse_list = NA
  )

# start of line keywords (e.g. not including INTEG, INTVC, DERIVT)
# don't try to handle indenting for has_label, goto, if_goto
# for do loops, replace the continue with enddo
# (these structures must be protected from sorting by being placed inside procedurals)
declaration <- c("constant", "parameter",
                 "algorithm", "nsteps", "maxterval", "minterval", "cinterval",
                 "character", "integer", "logical", "doubleprecision", "real", "dimension")
keyword1 <- c("program", "derivative", "initial", "discrete", "dynamic", "procedural", "terminal", "do", "mfile", "included") # +if_then, increase indent
keyword2 <- c("end", "endif", "enddo") # decrease indent
keyword3 <- c("termt", "schedule", "interval", "if", "goto", "continue", "sort") # + has_label + if_goto, no change to indent
keyword4 <- c("else") # +else_if_then, decrease and increase indent
keyword <- c(declaration, keyword1, keyword2, keyword3, keyword4)

# stack for matching line numbers of keyword1/ifthen and keyword2(end/endif) (FIXME what about goto?)
stack <- c()
indent <- 0 # current indent level
max_indent <- 10
block <- rep(0, max_indent) # ids for blocks at different indent levels
do_labels <- c()

# create regex strings for detection (these could be made faster by working on split code)
label_str <- "^[:blank:]*[0-9]+[:blank:]*\\:|^[:blank:]*[:alpha:]+[[:alnum:]_]*[:blank:]*\\:"
keyword_str <- paste("^[:blank:]*", keyword, "(?![[:alnum:]_])", sep="", collapse="|")
array_assign_str <- "token.+openbracket.+closebracket.+equals"
integ_str <- "token.+equals.+integ.+openbracket.+closebracket"
intvc_str <- "token.+equals.+intvc.+openbracket.+closebracket"
derivt_str <- "token.+equals.+derivt.+openbracket.+closebracket"
if_then_str <- "openbracket.+closebracket.+token.+then"
if_goto_str <- "openbracket.+closebracket.+token.+goto"
has_then_str <- "token.+then"
has_goto_str <- "token.+goto"
else_if_then_str <- "token.+else.+token.+if.+openbracket.+closebracket.+then"

# prepare loop
token      <- vector("character", 12000)
token_line <- vector("integer", 12000)
tokeni <- 1
is_continuation <- FALSE
i <- 1
pre_cont_i <- i # line number of top of continuation block
while (i <= nrow(csl)){ # loop through lines (this allows inserting rows into csl)

  # report progress
  if ((i %% 200 == 1 || i == nrow(csl)) && !silent){
    cat("line", i, "of", nrow(csl), "\n")
  }

  # get line
  this_line <- csl[i, ]
  this_line_body <- str_trim(this_line$code)

  # use max() as a compact way to convert NA to ""

  # label
  this_line_label <- max(str_extract(this_line_body, label_str), "", na.rm=TRUE)
  if (this_line_label > ""){
    has_label <- TRUE
    this_line_body <- str_replace(this_line_body, this_line_label, "") # strip label
    # is label end of do loop?
    if (str_to_lower(str_replace(this_line_label, ":", "")) %in% do_labels){
      this_line_body <- str_replace(this_line_body, regex("continue", ignore_case=TRUE), "enddo")
    }
  } else {
    has_label <- FALSE
  }

  # collapse various items
  this_line_body <- str_replace(this_line_body, regex("end[:blank:]*if", ignore_case=TRUE), "ENDIF")
  this_line_body <- str_replace(this_line_body, regex( "go[:blank:]*to", ignore_case=TRUE), "GOTO")

  # head
  this_line_head <- max(str_extract(this_line_body, regex(keyword_str, ignore_case=TRUE)), "", na.rm=TRUE)
  if (this_line_head > ""){
    this_line_body <- str_replace(this_line_body, this_line_head, "") # strip head
    this_line_head <- str_to_lower(this_line_head)
  }

  # tail
  this_line_tail <- max(str_extract(this_line_body, "!.*$"), "", na.rm=TRUE)
  if (this_line_tail > ""){
    this_line_body <- str_replace(this_line_body, "!.*$", "") # strip trailing comment
    this_line_tail <- this_line_tail
    has_tail = TRUE
  } else {
    has_tail = FALSE
  }

  # continuation
  this_line_cont <- max(str_extract(this_line_body, "&[:blank:]*$"), "", na.rm=TRUE)
  if (this_line_cont > ""){
    this_line_body <- str_replace(this_line_body, "&[:blank:]*$", "") # strip trailing &
  }

  # trailing ; or $
  this_line_body <- str_replace(this_line_body, "\\$[:blank:]*$", "")
  this_line_body <- str_replace(this_line_body,   ";[:blank:]*$", "")

  # store parts
  csl$label[i] <- this_line_label
  csl$head[i] <- this_line_head
  csl$body[i] <- this_line_body
  csl$cont[i] <- this_line_cont
  csl$tail[i] <- this_line_tail

  # split semicolon lines (easy in Molly)
  # this avoids multiple assignments on one line
  split_semicolon <- split_lines && str_detect(this_line_body, ";")
  if (split_semicolon){
    cat(csl$file_name[i], csl$line_number[i], "dropping semicolon", "\n")
    this_line <- csl[i, ]
    csl <- insert_row(csl, this_line, i + 1) # duplicate this_line
    temp <- str_split(this_line_body, ";")[[1]]
    this_line_body <- temp[1]
    csl$body[i] <- this_line_body
    # create a false line containing  the remaining code
    csl$code[i+1] <- paste(this_line_label,
                               this_line_head,
                               paste(temp[-1], collapse=" ; "),
                               this_line_cont)
    csl$seq_number[i+1] <- NA
    csl$line_number[i+1] <- NA
  }

  # parse line into recognised pieces
  code <- str_c(this_line_head, this_line_body)
  parse_list <- code_split(code)

  # convert parse list to string
  parse_str <- obj_to_str(parse_list)
  csl$parse_list[i] <- parse_str
  csl$length[i] <- length(parse_list) / 2

  # get first 1-2 items
  type1 <- parse_list[[1]] # get first item
  value1 <- str_to_lower(parse_list[[2]])
  if (length(parse_list) > 2){
    type2 <- parse_list[[3]] # get second item
    value2 <- str_to_lower(parse_list[[4]])
  } else {
    type2 <- ""
    value2 <- ""
  }
  # identify preliminary line type from first 1-2 items
  csl$line_type[i] <- case_when(
    is_continuation ~ "continuation", # i.e. same as previous line
    type1 == "token" && value1 %in% keyword ~ value1, # line_type <- keyword
    type1 == "blank" & has_tail ~ "comment", # comment only
    type1 == "blank" ~ "blank", # blank line
    type1 == "token" && type2 == "equals" ~  "assign",
    str_detect(str_to_lower(parse_str), array_assign_str) ~ "arrayassign",
    TRUE ~ "unknown" # other
  )
  # assign -> integ, intvc, derivt
  if (csl$line_type[i]=="assign"){
    if (str_detect(str_to_lower(parse_str), integ_str)) {
      csl$line_type[i] <- "integ"
    } else if (str_detect(str_to_lower(parse_str), intvc_str)) {
      csl$line_type[i] <- "intvc"
    } else if (str_detect(str_to_lower(parse_str), derivt_str)) {
      csl$line_type[i] <- "derivt"
    }
  }
  # if -> ifthen, ifgoto
  if_then <- FALSE
  if (csl$line_type[i]=="if"){
    if (str_detect(str_to_lower(parse_str), if_then_str)) {
      csl$line_type[i] <- "ifthen"
      if_then <- TRUE
    } else if (str_detect(str_to_lower(parse_str), if_goto_str)) {
      csl$line_type[i] <- "ifgoto"
    }
  }
  # else -> elseifthen
  else_if_then <- FALSE
  if (csl$line_type[i]=="else"){
    if (str_detect(str_to_lower(parse_str), else_if_then_str)) {
      csl$line_type[i] <- "elseifthen"
      else_if_then <- TRUE
    }
  }
  # continuation modifies if
  if (is_continuation & csl$line_type[pre_cont_i]=="if"){
    if (str_detect(str_to_lower(parse_str), has_then_str)){
      csl$line_type[pre_cont_i] <- "ifthen"
      if_then <- TRUE
    } else if (str_detect(str_to_lower(parse_str), has_goto_str)){
      csl$line_type[pre_cont_i] <- "ifgoto"
    }
  }
  # continuation modifies else
  if (is_continuation & csl$line_type[pre_cont_i]=="else"){
    if (str_detect(str_to_lower(parse_str), has_then_str)){
      csl$line_type[pre_cont_i] <- "elseifthen"
      else_if_then <- TRUE
    }
  }
  # collect do labels
  if (csl$line_type[i]=="do"){
    do_labels <- c(do_labels, value2)
  }

  # indent (no attempt to indent goto, ifgoto)
  if ((type1 == "token" && value1 %in% keyword1) || if_then){ # increase indent
    csl$indent[i] <- indent
    indent <- indent + 1
    stack <- c(i, stack)
    block[indent] <- block[indent] + 1
    block[(indent+1):max_indent] <- 0
    csl$block[i] <- paste(block[1:indent], collapse="-")
    if (is_continuation & if_then){ # carry indent and block back to pre_cont_i
      csl$indent[(pre_cont_i+1):(i-1)] <- csl$indent[i] + 2
      csl$block[pre_cont_i:(i-1)] <- csl$block[i]
    }
  } else if ((type1 == "token" && value1 %in% keyword4) || else_if_then){ # decrease and increase indent
    indent <- indent - 1
    csl$indent[i] <- indent
    indent <- indent + 1
    csl$block[i] <- paste(block[1:indent], collapse="-")
  } else if (type1 == "token" && value1 %in% keyword2){ # decrease indent
    csl$block[i] <- paste(block[1:indent], collapse="-")
    indent <- indent - 1
    if (indent<0){
      print("FIXME : indent < 0")
      indent <- indent + 1
    }
    csl$indent[i] <- indent
    csl$stack[i] <- stack[1]
    csl$stack[stack[1]] <- i
    stack <- tail(stack, -1) # removes first item
  } else { # no permanent change to indent
    csl$indent[i] <- indent + ifelse(is_continuation, 1, 0)
    csl$block[i] <- paste(block[1:indent], collapse="-")
  }

  # gather tokens
  for (itemi in seq(1, length(parse_list)-1, 2)){
    if (itemi == 1){
      prevtype1 <- ""
      prevvalue1 <- ""
      type1 <- parse_list[[itemi]] # get this item
      value1 <- parse_list[[itemi+1]]
    } else {
      prevtype1 <- type1
      prevvalue1 <- value1
      type1 <- type2
      value1 <- value2
    }
    if (itemi + 2 < length(parse_list)){
      type2 <- parse_list[[itemi+2]] # get next item
      value2 <- parse_list[[itemi+3]]
    } else {
      type2 <- ""
      value2 <- ""
    }
    if (type1 == "token"){
      token[tokeni] <- value1
      token_line[tokeni] <- i
      tokeni <- tokeni + 1
      if (tokeni > length(token)){
        print("token arrays not big enough")
      }
    }
  }

  # next line
  is_continuation <- this_line_cont != ""
  if (is_continuation){
    i <- i + 1
    # pre_cont_i <- pre_cont_i
  } else {
    i <- i + 1
    pre_cont_i <- i
  }

} # end while

# token table
tokens <- data.frame(name = token, line = token_line, stringsAsFactors = FALSE) %>%
  filter(name > "") %>%
  mutate(lower = str_to_lower(name)) %>%
  group_by(lower) %>%
  summarise(
    name = head(name, 1),
    lines = obj_to_str(line)
    # lines = paste(line, collapse=",")
  )

# save progress
rm(list=setdiff(ls(), c("csl", "tokens", "output_dir", "model_name", "silent", lsf.str())))
temp_file <- paste(output_dir, "checkpoint_after_parse_one.RData", sep="/")
save.image(temp_file)

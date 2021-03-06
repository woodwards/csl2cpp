
# read csl file and includes
read_csl <- function(input_dir, csl_file, m_files=""){

  # options
  drop_comments <- FALSE
  ignore_path <- FALSE
  cat(paste("base directory", input_dir), "\n")
  cat(paste("drop_comments", drop_comments), "\n")
  cat(paste("ignore_path", ignore_path), "\n")
  cat(paste("reading csl file", csl_file), "\n")

  # set working directory
  original_dir <- getwd()
  if (!is.null(input_dir)){
    setwd(input_dir)
  }

  # read base file
  file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
  # path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation
  # file_path <- paste(path_name, "/", file_name, sep="")
  # csl <- read_lines(file_path) %>%
  csl <- read_lines(csl_file) %>%
    iconv(to="ASCII//TRANSLIT") %>% # remove accents
    enframe(name=NULL) %>%
    rename(code=value) %>%
    mutate(seq_number=as.double(seq(n())),
           line_number=as.double(seq(n())),
           file_name=file_name)

  # find includes
  incli <- which(str_detect(str_to_lower(csl$code), "^[:blank:]*include[:blank:]+")) # find include statements

  while(length(incli) > 0){

    # read include file
    thisi <- incli[1]
    cat(paste(csl$file_name[thisi], csl$line_number[thisi], csl$code[thisi], "\n"))
    file_name <- str_extract(csl$code[thisi], "[:alpha:]+[[:alnum:]_[:space:]]*\\.csl")
    # file_path <- paste(path_name, "/", file_name, sep="")
    file_path <- str_extract(csl$code[thisi], "(?<=\').+(?=\')")
    first <- csl$seq_number[thisi]
    last <- csl$seq_number[thisi+1]
    # cat(file_name, "\n")
    include_csl <- read_lines(file_path) %>%
      iconv(to="ASCII//TRANSLIT") %>% # remove accents
      enframe(name=NULL) %>%
      rename(code=value) %>%
      mutate(seq_number=first + as.double(seq(n()) + 1) / (n() + 3) * (last - first),
             line_number=as.double(seq(n())),
             file_name=file_name)

    # create opening and closing lines
    opening <- csl[thisi, ] %>%
      mutate(seq_number=first + 1 / (nrow(include_csl) + 3) * (last - first),
             line_number=0,
             code=paste("INCLUDED ! ", code),
             file_name=include_csl$file_name[1])
    closing <- csl[thisi, ] %>%
      mutate(seq_number=first + (nrow(include_csl) + 2) / (nrow(include_csl) + 3) * (last - first),
             line_number=nrow(include_csl) + 1,
             code=paste("END ! ", code),
             file_name=include_csl$file_name[1])
    csl$code[thisi] <- paste("! ", csl$code[thisi]) # comment out original include statement

    # update includes
    incli <- setdiff(incli, thisi)
    incli_new <- which(str_detect(str_to_lower(include_csl$code), "^[:blank:]*include[:blank:]+"))
    if (length(incli_new)>0){
      incli_new <- incli_new + nrow(csl) + nrow(opening)
      incli <- c(incli, incli_new)
    }

    # bind
    csl <- rbind(csl, opening, include_csl, closing)

  }

  csl <- csl %>%
    arrange(seq_number)

  if (drop_comments){ # drop comments and blank lines
    csl <- csl %>%
      filter(!str_detect(code, "^[:blank:]*\\!") & !str_detect(code, "^[:blank:]*$"))
  }

  # append m_files
  for (file_name in m_files){

    # read file
    cat(paste("reading", file_name), "\n")
    # file_path <- paste(path_name, "/", file_name, sep="")
    # include_mfile <- read_lines(file_path) %>%
    include_mfile <- read_lines(file_name) %>%
      iconv(to="ASCII//TRANSLIT") %>% # remove accents
      enframe(name=NULL) %>%
      rename(code=value)
    include_mfile <- rbind(tibble(code=paste("MFILE !", file_name)),
                           include_mfile,
                           tibble(code=paste("END !", file_name)))
    include_mfile$file_name <- file_name
    include_mfile$line_number <- 1:nrow(include_mfile)
    include_mfile$seq_number <- 1:nrow(include_mfile) + nrow(csl)

    # modify comments from % to !
    include_mfile$code <- str_replace(include_mfile$code, "%", "!")

    # append to csl
    csl <- rbind(csl, include_mfile)

  }

  # restore original working directory
  if (!is.null(original_dir)){
    setwd(original_dir)
  }

  return(csl)

}


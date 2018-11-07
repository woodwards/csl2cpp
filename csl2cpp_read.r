
# read csl file and includes
read_csl <- function(csl_file, m_files=""){

  #
  drop_comments=FALSE
  cat(file=stderr(), paste("reading", csl_file), "\n")
  cat(file=stderr(), paste("drop_comments", drop_comments), "\n")
  cat(file=stderr(), paste("ignore_path", TRUE), "\n")
  # cat(file=stderr(), "WARNING: All INCLUDE files must be in the same directory *** paths ignored ***\n")

  # read file
  file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
  path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation
  file_path <- paste(path_name, "/", file_name, sep="")
  csl <- read_lines(file_path) %>%
    iconv(to="ASCII//TRANSLIT") %>% # remove accents
    as_tibble() %>%
    rename(code=value) %>%
    mutate(seq_number=as.double(seq(n())),
           line_number=as.double(seq(n())),
           file_name=file_name)

  # find includes
  incli <- which(str_detect(str_to_lower(csl$code), "^[:blank:]*include[:blank:]+")) # find include statements

  while(length(incli) > 0){

    # read include file
    thisi <- incli[1]
    cat(paste(thisi, csl$code[thisi], "\n"))
    file_name <- str_extract(csl$code[thisi], "[:alpha:]+[[:alnum:]_]*\\.csl")
    file_path <- paste(path_name, "/", file_name, sep="")
    first <- csl$seq_number[thisi]
    last <- csl$seq_number[thisi+1]
    # cat(file=stderr(), file_name, "\n")
    include_csl <- read_lines(file_path) %>%
      iconv(to="ASCII//TRANSLIT") %>% # remove accents
      as_tibble() %>%
      rename(code=value) %>%
      mutate(seq_number=first + as.double(seq(n()) + 1) / (n() + 3) * (last - first),
             line_number=as.double(seq(n())),
             file_name=file_name)

    # create opening and closing lines
    opening <- csl[thisi, ] %>%
      mutate(seq_number=first + 1 / (nrow(include_csl) + 3) * (last - first),
             line_number=0,
             code=paste("INCLUDE ! ", code),
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
    cat(file=stderr(), paste("reading", file_name), "\n")
    file_path <- paste(path_name, "/", file_name, sep="")
    include_mfile <- read_lines(file_path) %>%
      iconv(to="ASCII//TRANSLIT") %>% # remove accents
      as_tibble() %>%
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

  return(csl)

}


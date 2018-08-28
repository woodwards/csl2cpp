# read csl file and includes

read_csl <- function(csl_file, silent=FALSE, drop_comments=FALSE){

  #
  cat(file=stderr(), paste("reading", csl_file), "\n")
  cat(file=stderr(), paste("silent", silent), "\n")
  cat(file=stderr(), paste("drop_comments", drop_comments), "\n")
  cat(file=stderr(), "WARNING: All INCLUDE files must be in the same directory *** paths ignored ***\n")

  # read file
  file_name <- str_extract(csl_file,  "[:alpha:]+[[:alnum:]_]*\\.csl")
  path_name <- str_extract(csl_file, "^[:alpha:]+[[:alnum:]_]*") # fails if path has punctuation
  file_path <- paste(path_name, "/", file_name, sep="")
  csl <- read_lines(file_path) %>%
    iconv(to="ASCII//TRANSLIT") %>% # remove accents
    as_tibble() %>%
    rename(code=value) %>%
    mutate(line_number=as.double(seq(n())),
           file_name=file_name)

  # find includes
  incli <- which(str_detect(str_to_lower(csl$code), "^[:blank:]*include[:blank:]+")) # find include statements

  while(length(incli) > 0){

    # read file
    thisi <- incli[1]
    if (!silent)
      cat(paste(thisi, csl$code[thisi], "\n"))
    file_name <- str_extract(csl$code[thisi], "[:alpha:]+[[:alnum:]_]*\\.csl")
    file_path <- paste(path_name, "/", file_name, sep="")
    first <- csl$line_number[thisi]
    last <- csl$line_number[thisi+1]
    # cat(file=stderr(), file_name, "\n")
    include_csl <- read_lines(file_path) %>%
      iconv(to="ASCII//TRANSLIT") %>% # remove accents
      as_tibble() %>%
      rename(code=value) %>%
      mutate(line_number=first + as.double(seq(n())) / (n() + 1) * (last - first),
             file_name=file_name)

    # append
    csl <- rbind(csl, include_csl)
    csl$code[thisi] <- paste("! DONE ", csl$code[thisi]) # comment out include statement

    # find includes
    incli <- which(str_detect(str_to_lower(csl$code), "^[:blank:]*include[:blank:]+")) # find include statements

  }

  csl <- csl %>%
    arrange(line_number)

  if (drop_comments){ # drop comments and blank lines
    csl <- csl %>%
      filter(!str_detect(code, "^[:blank:]*\\!") & !str_detect(code, "^[:blank:]*$"))
  }

  return(csl)

}


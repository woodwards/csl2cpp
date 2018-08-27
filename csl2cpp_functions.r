# csl2cpp_functions.r

# split a line of code into tokens, strings, comments, etc
code_split <- function(code){
  remaining <- code
  outlist <- vector("list", 20)
  outi <- 1
  patterns <- c( # special characters .!?\(){}
    space="^[:blank:]+",
    comment="^!.*",
    token="^[:alpha:]+[[:alnum:]_]*",
    assign="^\\=",
    bracket="^\\(|^\\)",
    semicolon="^;",
    ampersand="^&",
    comma="^\\,",
    logical="^\\.or\\.|^\\.OR\\.|^\\.and\\.|^\\.AND\\.|^\\.not\\.|^\\.NOT\\.|
             ^\\.eqv\\.|^\\.EQV\\.|^\\.neqv\\.|^\\.NEQV\\.|
             ^\\.ge\\.|^\\.GE\\.|^\\.gt\\.|^\\.GT\\.|
             ^\\.le\\.|^\\.LE\\.|^\\.lt\\.|^\\.LT\\.|
             ^\\.eq\\.|^\\.EQ\\.|^\\.ne\\.|^\\.NE\\.",
    operator="^[\\/\\+\\-\\*]|\\*=\\*",
    number="^[-+]?[0-9]*\\.?[0-9]+([eEdD][-+]?[0-9]+)?" # https://www.regular-expressions.info/floatingpoint.html
  )
  if (str_detect(remaining, "^[:blank:]*$")){ # blank line
    outlist[[outi]] <- "blank"
    outi <- outi + 1
    remaining <- ""
  } else while (str_length(remaining) > 0){ # loop through elements
    next_match <- str_match(remaining, patterns)[,1]
    matchj <- which(!is.na(next_match))
    if (length(matchj) != 1){ # matches 0 or more than 1 pattern
      outlist[[outi]] <- paste("unknown", remaining)
      outi <- outi + 1
      remaining <- ""
    } else { # matches 1 pattern
      if (names(patterns)[matchj] != "space"){ # don't save space
        outlist[[outi]] <- paste(names(patterns)[matchj], next_match[matchj])
        outi <- outi + 1
      }
      remaining <- str_replace(remaining, patterns[matchj], "")
    }
  }
  # return(capture.output(dput(compact(outlist)))) # return list as string
  return(compact(outlist)) # return list
}
if (FALSE){
  code <- "sev55 = 1 + 1 !do something!''"
  x <- code_split(code)
  code <- "CONSTANT       xic = 0.0    , xdic = 0.0"
  x <- code_split(code)
  code = "xdd     =(mass*g - k*xd - a*x)/mass"
  x <- code_split(code)
  code = ""
  x <- code_split(code)
}


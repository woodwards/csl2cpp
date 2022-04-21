
usethis::git_remotes()

gitcreds::gitcreds_set()

usethis::use_git_remote(
  "origin",
  "https://github.com/woodwards/csl2cpp.git",
  overwrite = TRUE
)

usethis::git_remotes()

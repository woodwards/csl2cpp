#### third pass - sorting dynamic lines into calc and post processing ####
# thoughts:
# taking unnecessary lines out of calculate_rates will speed it up a lot.
#

temp_file <- paste(path_name, "checkpoint_after_parse_two.RData", sep="/")
load(file=temp_file) # recover progress

cat(file=stderr(), "sorting dynamic section code", "\n")

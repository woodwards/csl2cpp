# write cpp based on parsed csl

write_cpp <- function(cpp, output_dir, model_name, ext_name){

	# open cpp file
	cpp_file_name <- paste(output_dir, "/", model_name, ".", ext_name, sep="")
	cpp_file <- file(cpp_file_name, open='w') # delete duplicate log file if necessary
	close(cpp_file)
	cpp_file <- file(cpp_file_name, open='a') # open file for writing

	# remove some blank lines
	blank <- str_detect(cpp, "^[:blank:]*\\n")
	keeps <- !blank | !lag(blank,1)
	keeps[is.na(keeps)] <- TRUE

	# write lines
	cat(file=cpp_file, cpp[keeps], sep="")

	# close cpp file
	close(cpp_file)

}

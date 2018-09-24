# write cpp based on parsed csl

write_cpp <- function(cpp, path_name, model_name){

	# open cpp file
	cpp_file_name <- paste(path_name, "/", model_name, ".cpp", sep="")
	cpp_file <- file(cpp_file_name, open='w') # delete duplicate log file if necessary
	close(cpp_file)
	cpp_file <- file(cpp_file_name, open='a') # open file for writing

	# write lines
	cat(file=cpp_file, cpp, sep="")

	# close cpp file
	close(cpp_file)

}

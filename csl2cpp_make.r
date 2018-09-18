# make cpp based on parsed csl

put_lines <- function(cpp, indent, lines){
	first_row <- attr(cpp, "row") + 1
	last_row <- first_row+length(lines)-1
	tab <- paste(rep("\t", indent), sep="", collapse="")
	cpp[first_row:last_row] <- paste(tab, lines, "\n", sep="") # multiple lines
	# cpp[first_row:last_row] <- paste(tab, lines, sep="") # multiple lines
	attr(cpp, "row") <- last_row
	return(cpp)
}

make_cpp <- function(csl, model_name, silent=FALSE){

	n_state <- 3
	n_variable <- 22

	# create an empty vector of strings
	cpp <- vector("character", nrow(csl))
	attr(cpp, "row") <- 0 # create an attribute for the row counter

	# header
	lines <- c("#include <unordered_map>",
			   "#include <string>",
			   "#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE",
			   "#include <boost/numeric/odeint.hpp>",
			   "#include <boost/array.hpp>", "")
	cpp <- put_lines(cpp, 0, lines)

	# class
	cpp <- put_lines(cpp, 0, c(paste("class", model_name, "{"), ""))
	cpp <- put_lines(cpp, 0, c("public:", ""))
	lines <- c("// unordered_map gives user efficient access to variables by name",
			   "std::unordered_map< std::string , double > variable;")
	cpp <- put_lines(cpp, 1, lines)
	cpp <- put_lines(cpp, 0, c("", "private:", ""))
	lines <- c("// declare state_type and system_time",
			   paste("typedef boost::array< double ,", n_state, "> state_type;"),
			   "double system_time;", "",
			   "// specify number of variables available to user",
			   paste("static constexpr int system_variables =", n_variable,";"), "",
			   "// declare boost::odeint stepper",
			   "typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;",
			   "stepper_type stepper;")
	cpp <- put_lines(cpp, 1, lines)

	# declare variables

	# constructor
	cpp <- put_lines(cpp, 0, c("", "public:", ""))
	lines <- c("// constructor",
			   paste(model_name, "() {"), "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c("// reserve buckets to minimise storage and avoid rehashing",
			   "variable.reserve( system_variables );")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}"))

	# initialise model

	# close class
	cpp <- put_lines(cpp, 0, c("", "public:", ""))
	lines <- c("int advance_model( double end_time , double time_step ){", "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c("double a_time;",
			   "state_type a_state;",
			   "int nsteps;",
			   "a_time = system_time;",
			   "a_state = get_state();","",
			   "// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class",
			   "nsteps = boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , end_time , time_step );", "",
			   "system_time = end_time;",
			   "set_state( a_state );",
			   "calculate_rate();",
			   "return( nsteps );")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}"))
	cpp <- put_lines(cpp, 0, c("", "}; // end class", ""))
	return(cpp)

}


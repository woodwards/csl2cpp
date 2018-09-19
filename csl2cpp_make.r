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

	# count variables
	integ <- str_split(paste(csl$integ[!is.na(csl$integ)], collapse=","), ",", simplify=TRUE)
	state <- str_match(integ, "^[:alpha:]+[[:alnum:]_]*")[,1]
	rate <- str_replace(integ, "^[:alpha:]+[[:alnum:]_]*", "")
	n_state <- length(state)
	variable <- str_split(paste(csl$declare[!is.na(csl$declare)], collapse=","), ",", simplify=TRUE)[1,]
	variable <- str_trim(variable)
	n_variable <- length(variable)

	# create an empty vector of strings
	cpp <- vector("character", nrow(csl) + 200) # estimate output length
	attr(cpp, "row") <- 0 # create an attribute for the row counter

	# header
	lines <- c("// *******************************************************",
			   "// this module is automatically generated from an R script",
			   "// *******************************************************",
			   "#include <unordered_map>",
			   "#include <string>",
			   "#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE",
			   "#include <boost/numeric/odeint.hpp>",
			   "#include <boost/array.hpp>", "")
	cpp <- put_lines(cpp, 0, lines)

	# class
	cpp <- put_lines(cpp, 0, c(paste("class", model_name, "{"), "",
	                           "private:", ""))
	lines <- c("// specify number of variables",
              paste("static constexpr int n_state_variables =", n_state,";"),
              paste("static constexpr int n_visible_variables =", n_variable,";"), "",
              "// declare state_type and system_time",
              paste("typedef boost::array< double , n_state_variables > state_type;"),
              "double system_time;", "",
              "// declare boost::odeint stepper",
              "typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;",
              "stepper_type stepper;", "")
	cpp <- put_lines(cpp, 1, lines)

	# declare state variables
  lines <- c("// state variables",
             paste("double", state, ";"), "")
  cpp <- put_lines(cpp, 1, lines)

	# declare other variables
  lines <- c("// other variables",
             paste("double", setdiff(variable, state), ";"), "")
  cpp <- put_lines(cpp, 1, lines)

  # get state
  cpp <- put_lines(cpp, 1, c("// get state",
                             "state_type get_state ( ) {"))
  cpp <- put_lines(cpp, 2, c("", "state_type a_state;", "", "// return current state"))
  lines <- paste("a_state[", 0:(n_state-1), "] = ", state, ";", sep="")
  cpp <- put_lines(cpp, 2, lines)
  cpp <- put_lines(cpp, 2, c("", "return( a_state );"))
  cpp <- put_lines(cpp, 1, c("", "}", ""))

  # set state
  cpp <- put_lines(cpp, 1, c("// set state",
                             "void set_state ( state_type a_state ) {"))
  cpp <- put_lines(cpp, 2, c("", "// set state"))
  lines <- paste(state, " = a_state[", 0:(n_state-1), "];", sep="")
  cpp <- put_lines(cpp, 2, lines)
  cpp <- put_lines(cpp, 1, c("", "}"))

	# constructor
	cpp <- put_lines(cpp, 0, c("", "public:", ""))
	lines <- c("// unordered_map gives user efficient access to variables by name",
	           "std::unordered_map< std::string , double > variable;", "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c("// constructor",
			   paste(model_name, "( ) {"), "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c("// reserve buckets to minimise storage and avoid rehashing",
			   "variable.reserve( n_visible_variables );")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}", ""))

	# initialise model
	cpp <- put_lines(cpp, 1, c("void initialise_model ( double a_system_time ) {"))
	cpp <- put_lines(cpp, 2, c("", "// initialise system_time",
	                           "system_time = a_system_time;", "",
	                           "// initialise model"))
	# lines <- paste(csl$init[!is.na(csl$init)], ";",
	#                csl$tail[csl$tail_loc=="init"]) # with tail comments
	rows <- !is.na(csl$init) | csl$tail_loc=="init"
	lines <- paste(pmax(csl$init[rows], "", na.rm=TRUE),
	               if_else(is.na(csl$init[rows]), "", ";"),
	               csl$tail[rows], sep="") # with tail comments
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}", ""))

	# pull variables from model
	cpp <- put_lines(cpp, 1, "void pull_variables_from_model ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// pull system time",
	                           "variable[\"system_time\"] = system_time;", "",
	                           "// pull model variables"))
	lines <- paste("variable[\"", variable, "\"] = ", variable, ";", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}", ""))

	# push variables to model
	cpp <- put_lines(cpp, 1, "void push_variables_to_model ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// push system time",
	                           "system_time = variable[\"system_time\"];", "",
	                           "// push model variables"))
	lines <- paste(variable, " = variable[\"", variable, "\"];", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}", ""))

	# calculate rate
	cpp <- put_lines(cpp, 1, "void calculate_rate ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// calculations"))
	rows <- !is.na(csl$calc) | csl$tail_loc=="calc"
	lines <- paste(pmax(csl$calc[rows], "", na.rm=TRUE),
	               if_else(is.na(csl$calc[rows]), "", ";"),
	               csl$tail[rows], sep="") # with tail comments
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}", ""))

	# rate operator
	cpp <- put_lines(cpp, 1, c("// called by boost::odeint::integrate()",
	                           "void operator()( const state_type &a_state , state_type &a_rate, double a_time ){"))
	cpp <- put_lines(cpp, 2, c("", "// set state",
	                           "system_time = a_time;"))
	lines <- paste(state, " = a_state[", 0:(n_state-1), "];", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 2, c("", "// calculate rate",
	                           "calculate_rate();", "",
	                           "// return rate"))
	lines <- paste("a_rate[", 0:(n_state-1), "]", rate, ";", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "}", ""))

	# close class
	lines <- c("int advance_model ( double end_time , double time_step ) {", "")
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


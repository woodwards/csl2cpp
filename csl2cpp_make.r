# make cpp based on parsed csl

vrep <- function(what, times){ # vectorised string rep
  sapply(times, function(x) paste(rep(what, x), collapse=""))
}

put_lines <- function(cpp, indent, lines){
	first_row <- attr(cpp, "row") + 1
	last_row <- first_row+length(lines)-1
	tab <- vrep("\t", indent)
	cpp[first_row:last_row] <- paste(tab, lines, "\n", sep="") # multiple lines
	attr(cpp, "row") <- last_row
	return(cpp)
}

smoosh <- function(...){
  # like paste but remove excess spaces
  str_c(..., sep=" ") %>%
    str_squish()
}

make_cpp <- function(csl, model_name){

	# count variables
	integ <- str_split(paste(csl$integ[csl$integ > ""], collapse=","), ",", simplify=TRUE)
	state <- str_match(integ, "^[:alpha:]+[[:alnum:]_]*")[,1]
	rate <- str_trim(str_replace(integ, "^[:alpha:]+[[:alnum:]_]*", ""))
	n_state <- length(state)
	decl <- str_trim(str_split(paste(csl$decl[csl$decl > ""], collapse=","), ",", simplify=TRUE))
	constant <- str_detect(decl, "=")
	variable <- str_match(decl, "^[:alpha:]+[[:alnum:]_]*")[,1]
	n_variable <- length(variable)

	# browser()

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
			   # "#include <boost/array.hpp>", # try std::array instead as recommended by boost
			   "#include <array>",
			   "")
	cpp <- put_lines(cpp, 0, lines)

	# header comments
	rows <- csl$section == "header"
	lines <- csl$tail[rows]
	cpp <- put_lines(cpp, 0, lines)

	# class
	cpp <- put_lines(cpp, 0, c("", paste("class", model_name, "{"), "",
	                           "private:", ""))

	# class properties
	lines <- c("// specify number of variables",
              paste("static constexpr int n_state_variables =", n_state,";"),
              paste("static constexpr int n_visible_variables =", n_variable,"; // FIXME"), "",
              "// declare state_type and t",
	           # paste("typedef boost::array< double , n_state_variables > state_type;"),
	           paste("typedef std::array< double , n_state_variables > state_type;"),
	           "double t;", "",
              "// declare boost::odeint stepper",
              "typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;",
              "stepper_type stepper;", "")
	cpp <- put_lines(cpp, 1, lines)

  # declare model variables
	cpp <- put_lines(cpp, 1, "// declare model variables")
	rows <- csl$decl > ""
	lines <- if_else(csl$decl[rows] > "",
	                 smoosh(csl$static[rows], csl$type[rows], csl$decl[rows], csl$delim[rows], csl$tail[rows]),
	                 csl$tail[rows])
  cpp <- put_lines(cpp, 1, lines)

  # get state
  cpp <- put_lines(cpp, 1, c("", "state_type get_state ( ) {"))
  cpp <- put_lines(cpp, 2, c("", "state_type a_state;", "", "// return current state"))
  lines <- paste("a_state[", 0:(n_state-1), "] = ", state, ";", sep="")
  cpp <- put_lines(cpp, 2, lines)
  cpp <- put_lines(cpp, 2, c("", "return( a_state );"))
  cpp <- put_lines(cpp, 1, c("", "} // end get_state", ""))

  # set state
  cpp <- put_lines(cpp, 1, c("void set_state ( state_type a_state ) {"))
  cpp <- put_lines(cpp, 2, c("", "// set state"))
  lines <- paste(state, " = a_state[", 0:(n_state-1), "];", sep="")
  cpp <- put_lines(cpp, 2, lines)
  cpp <- put_lines(cpp, 1, c("", "} // end set_state"))

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
	cpp <- put_lines(cpp, 1, c("", "} // end constructor", ""))

	# initialise model
	cpp <- put_lines(cpp, 1, c("void initialise_model ( double a_time ) {"))
	cpp <- put_lines(cpp, 2, c("", "// initialise t", "t = a_time;", ""))
	rows <- csl$section == "initial" | csl$init > "" # include init lines from other places
  if (any(rows)){
  	cpp <- put_lines(cpp, 2, "// initialise model")
  	lines <- if_else(csl$init[rows] > "",
  	                 smoosh(csl$init[rows], csl$delim[rows], csl$tail[rows]),
  	                 csl$tail[rows])
  	indent <- 2 + pmax(0, csl$indent[rows] - min(csl$indent[rows]) - 1)
  	cpp <- put_lines(cpp, indent, lines)
  	cpp <- put_lines(cpp, 2, "")
  }
	cpp <- put_lines(cpp, 1, c("} // end initialise_model", ""))

	# pull variables from model (and constants?)
	cpp <- put_lines(cpp, 1, "void pull_variables_from_model ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// pull system time",
	                           "variable[\"t\"] = t;", "",
	                           "// pull model variables"))
	# pulling constant values is risky so require user to un-constant them
	lines <- paste("variable[\"", variable[!constant], "\"] = ", variable[!constant], ";", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end pull_variables_from_model", ""))

	# push variables to model
	cpp <- put_lines(cpp, 1, "void push_variables_to_model ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// push system time",
	                           "t = variable[\"t\"];", "",
	                           "// push model variables"))
	lines <- paste(variable[!constant], " = variable[\"", variable[!constant], "\"];", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end push_variables_to_model", ""))

	# calculate rate
	cpp <- put_lines(cpp, 1, "void calculate_rate ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// calculations"))
	rows <- csl$section == "derivative"
	lines <- if_else(csl$calc[rows] > "",
	                 smoosh(csl$calc[rows], csl$delim[rows], csl$tail[rows]),
	                 csl$tail[rows])
	indent <- 2 + pmax(0, csl$indent[rows] - min(csl$indent[rows]) - 2)
	cpp <- put_lines(cpp, indent, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end calculate_rate", ""))

	# rate operator
	cpp <- put_lines(cpp, 1, c("// called by boost::odeint::integrate()",
	                           "void operator()( const state_type &a_state , state_type &a_rate, double a_time ){"))
	cpp <- put_lines(cpp, 2, c("", "// set state",
	                           "t = a_time;"))
	lines <- paste(state, " = a_state[", 0:(n_state-1), "];", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 2, c("", "// calculate rate",
	                           "calculate_rate();", "",
	                           "// return rate"))
	lines <- paste("a_rate[", 0:(n_state-1), "]", rate, ";", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end operator", ""))

	# close class
	lines <- c("int advance_model ( double end_time , double time_step ) {", "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c("double a_time;",
			   "state_type a_state;",
			   "int nsteps;",
			   "a_time = t;",
			   "a_state = get_state();","",
			   "// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class",
			   "nsteps = boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , end_time , time_step );", "",
			   "t = end_time;",
			   "set_state( a_state );",
			   "calculate_rate();",
			   "return( nsteps );")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end advance_model"))
	cpp <- put_lines(cpp, 0, c("", "}; // end class", ""))
	return(cpp)

}


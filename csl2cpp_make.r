# make cpp based on parsed csl

vrep <- function(what, times){ # vectorised string rep
  sapply(times, function(x) paste(rep(what, x), collapse=""))
}

put_lines <- function(cpp, indent, lines){
  # insert rows
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

make_cpp <- function(csl, tokens, model_name, delay_post=FALSE){

  cat("delay post processing :", delay_post, "\n")

	# count variables
  integ <- csl$integ[csl$line_type == "integ"]
  state <- str_match(integ, "^[:alpha:]+[[:alnum:]_]*")[,1]
	rate <- str_trim(str_replace(integ, "^[:alpha:]+[[:alnum:]_]*", "")) # rate expressions
	n_state <- length(state)
	derivt <- csl$integ[csl$line_type == "derivt"]
	slope <- str_match(derivt, "^[:alpha:]+[[:alnum:]_]*")[,1]
	slopeof <- str_trim(str_replace(derivt, "^[:alpha:]+[[:alnum:]_]*", ""))
	slopeof <- str_replace_all(slopeof, "= ", "")
	variable <- tokens$name[tokens$decl_type %in% c("double", "int", "bool", "auto") & is.na(tokens$decl_value)]
	n_variable <- length(variable)
	cat("n state :", n_state, "\n")
	cat("n visible :", n_variable, "\n")

	# uninitialised variables after sort
	assumed_all <- tokens$name[tokens$set_status=="assumed"]

	# browser()

	# create an empty vector of strings
	cpp <- vector("character", nrow(csl) * 2 + 200) # estimate output length
	attr(cpp, "row") <- 0 # create an attribute for the row counter

	# module header
	lines <- c("// *******************************************************",
			   "// this module is automatically generated from an R script",
			   "// *******************************************************",
			   "#include <unordered_map>",
			   "#include <map>",
			   "#include <string>",
			   "#include <cmath>",
			   "#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE",
			   "#include <boost/numeric/odeint.hpp>",
			   # "#include <boost/array.hpp>", # try std::array instead as recommended by boost
			   "#include <array>",
			   "",
			   "using namespace std; // needed for math functions",
			   "")
	cpp <- put_lines(cpp, 0, lines)

	#### model header comments ####
	rows <- csl$section == "header"
	lines <- csl$tail[rows]
	cpp <- put_lines(cpp, 0, lines)
	cat("header comments :", sum(rows), "\n")

	# class declaration
	cpp <- put_lines(cpp, 0, c("", paste("class", model_name, "{"), "",
	                           "private:", ""))

	# class properties
	lines <- c("// specify number of variables",
              paste("static constexpr int n_state_variables =", n_state,";"),
              paste("static constexpr int n_visible_variables =", n_variable,";"), "",
              "// declare state_type and t",
	           paste("typedef std::array< double , n_state_variables > state_type;"),
	           "double t;", "",
	           "// declare event list",
	           "std::multimap< double , std::string > event_list;", "",
	           "// add event",
	           "void schedule( double event_time, std::string event_name ){", "",
	           "\tevent_list.insert( std::make_pair( event_time , event_name ) );", "", "}", "",
	           "// declare boost::odeint stepper",
             "typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;",
             "stepper_type stepper;", "")
	cpp <- put_lines(cpp, 1, lines)

  #### declare model variables ####
	cpp <- put_lines(cpp, 1, "// declare model variables")
	rows <- csl$decl > ""
	lines <- smoosh(csl$static[rows], csl$type[rows], csl$decl[rows], csl$dend[rows])
	cpp <- put_lines(cpp, 1, lines)
	cat("declaration lines :", sum(rows), "\n")
	# declare numerical derivatives
	if (slopeof>""){
	  lines <- smoosh("double", paste(slopeof, "_prev", sep=""), ";")
	  cpp <- put_lines(cpp, 1, lines)
	  lines <- smoosh("double", paste("t", "_prev", sep=""), ";")
	  cpp <- put_lines(cpp, 1, lines)
	}

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

	# constructor head
	cpp <- put_lines(cpp, 0, c("", "public:", ""))
	lines <- c("// unordered_map gives user efficient access to variables by name",
	           "std::unordered_map< std::string , double > variable;", "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c("// constructor",
	           paste(model_name, "( ) :"), "")
	cpp <- put_lines(cpp, 1, lines)

	# array initialisation list (class constructor list)
	cpp <- put_lines(cpp, 2, c("// array initialisation list",
	                           "// warning: these are executed in the order of the member declarations"))
	rows <- csl$ccl > ""
	csl$dend[max(which(rows))] <- "" # remove last comma
	lines <- smoosh(csl$ccl[rows], csl$dend[rows], csl$tail[rows])
	indent <- if_else(str_detect(lines, "^\\{"), 3, 2) # indent array initialisation lines
	cpp <- put_lines(cpp, indent, lines)

	# constructor body
	lines <- c("", "// constructor body",
	           "{", "",
	           "\t// reserve buckets to minimise storage and avoid rehashing",
	           "\tvariable.reserve( n_visible_variables );", "",
	           "} // end constructor", "")
	cpp <- put_lines(cpp, 1, lines)

	#### initialise model ####
	cpp <- put_lines(cpp, 1, c("void initialise_model ( double a_time ) {"))
	cpp <- put_lines(cpp, 2, c("", "// initialise t", "t = a_time;", ""))
	# uninitialised variables
	if (assumed_all>""){
  	cpp <- put_lines(cpp, 2, c("", "// initialise illegally used variables as per ACSLX"))
  	lines <- paste(assumed_all, "= 5.5555e33 ;")
  	cpp <- put_lines(cpp, 2, lines)
	}
	# model initialisation
	rows <- csl$section == "initial"
  if (any(rows)){
  	cpp <- put_lines(cpp, 2, c("", "// initial calculations"))
  	lines <- if_else(csl$init[rows] > "",
  	                 smoosh(csl$init[rows], csl$delim[rows], csl$tail[rows]),
  	                 csl$tail[rows])
  	indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
  	cpp <- put_lines(cpp, indent, lines)
  	cat("initial calculations lines :", sum(rows), "\n")
  }
	# state variable initial conditions
	rows <- csl$line_type == "integ"
	cpp <- put_lines(cpp, 2, c("", "// initialise state variables"))
	lines <- smoosh(csl$init[rows], csl$delim[rows])
	indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
	cpp <- put_lines(cpp, indent, lines)
	# numerical derivatives initial conditions
	rows <- csl$line_type == "derivt"
	if (any(rows)){
  	cpp <- put_lines(cpp, 2, c("", "// initialise numerical derivatives"))
  	lines <- smoosh(csl$init[rows], csl$delim[rows])
  	indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
  	cpp <- put_lines(cpp, indent, lines)
  	lines <- smoosh(paste(slopeof, "_prev", sep=""), "=", slopeof, ";")
  	cpp <- put_lines(cpp, 2, lines)
  	lines <- "t_prev = t ;"
  	cpp <- put_lines(cpp, 2, lines)
	}
	# close off
	cpp <- put_lines(cpp, 1, c("", "} // end initialise_model", ""))

	# pull variables from model (and constants?)
	cpp <- put_lines(cpp, 1, "void pull_variables_from_model ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// pull system time",
	                           "variable[\"t\"] = t;", "",
	                           "// pull model variables"))
	lines <- paste("variable[\"", variable, "\"] = ", variable, ";", sep="") # implicit type conversion
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end pull_variables_from_model", ""))

	# push variables to model
	cpp <- put_lines(cpp, 1, "void push_variables_to_model ( ) {")
	cpp <- put_lines(cpp, 2, c("", "// push system time",
	                           "t = variable[\"t\"];", "",
	                           "// push model variables"))
	lines <- paste(variable, " = variable[\"", variable, "\"];", sep="") # implicit type conversion
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end push_variables_to_model", ""))

	####  do events ####
	cpp <- put_lines(cpp, 1, c("void do_event ( string next_event ) {", ""))
	rows <- csl$section == "discrete"
	if (any(rows)){
	  cpp <- put_lines(cpp, 2, c("// find event"))
	  lines <- if_else(csl$disc[rows] > "",
  	                 smoosh(csl$disc[rows], csl$delim[rows], csl$tail[rows]),
  	                 csl$tail[rows])
	  indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
	  cpp <- put_lines(cpp, indent, lines)
	  cpp <- put_lines(cpp, 2, "")
	}
	cpp <- put_lines(cpp, 1, c("} // end do_event", ""))
	cat("event lines :", sum(rows), "\n")

	#### derivt function ####
	# cpp <- put_lines(cpp, 1, c("double derivt( double dx0, double x ) {", "",
	#                            "\treturn( 0.0 ) ;", "",
	#                            "}", ""))

	#### calculate rate ####
	cpp <- put_lines(cpp, 1, "void calculate_rate ( ) {")
	# rate calculation from derivative section
	cpp <- put_lines(cpp, 2, c("", "// derivative calculations"))
	rows <- which(csl$section == "derivative" & (csl$dep == "for_rate" | delay_post==FALSE))
	lines <- if_else(csl$calc[rows] > "",
	                 smoosh(csl$calc[rows], csl$delim[rows], csl$tail[rows]),
	                 csl$tail[rows])
	indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
	cpp <- put_lines(cpp, indent, lines)
	cat("calculate rate lines :", length(rows), "\n")
	cpp <- put_lines(cpp, 1, c("", "} // end calculate_rate", ""))

	#### post processing ####
	cpp <- put_lines(cpp, 1, "void post_processing ( ) {")
	# post processing from derivative section
	cpp <- put_lines(cpp, 2, c("", "// post processing calculations from derivative"))
	rows <- which(csl$section == "derivative" & (csl$dep == "" & delay_post==TRUE))
	lines <- if_else(csl$calc[rows] > "",
	                 smoosh(csl$calc[rows], csl$delim[rows], csl$tail[rows]),
	                 csl$tail[rows])
	indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
	cpp <- put_lines(cpp, indent, lines)
	cat("post processing 1 lines :", length(rows), "\n")
	# and also dynamic section
	cpp <- put_lines(cpp, 2, c("", "// post processing calculations from dynamic"))
	rows <- which(csl$section == "dynamic")
	lines <- if_else(csl$calc[rows] > "",
	                 smoosh(csl$calc[rows], csl$delim[rows], csl$tail[rows]),
	                 csl$tail[rows])
	indent <- ifelse(csl$label[rows]>"", 1, 2) + pmax(0, csl$indent[rows] - min(csl$indent[rows]))
	cpp <- put_lines(cpp, indent, lines)
	cat("post processing 2 lines :", length(rows), "\n")
	cpp <- put_lines(cpp, 1, c("", "} // end post_processing", ""))

	# rate operator
	cpp <- put_lines(cpp, 1, c("// rate operator called by boost::odeint::integrate()",
	                           "void operator()( const state_type &a_state , state_type &a_rate, double a_time ){", ""))
	cpp <- put_lines(cpp, 2, c("// calculate rate",
	                           "t = a_time;",
	                           "set_state( a_state );",
	                           "calculate_rate();", "",
	                           "// return rate"))
	lines <- paste("a_rate[", 0:(n_state-1), "] ", rate, ";", sep="")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end rate operator", ""))

	# observer operator
	cpp <- put_lines(cpp, 1, c("// observer operator called by boost::odeint::integrate()",
	                           "void operator()( const state_type &a_state , double a_time ){", ""))
	rows <- csl$line_type == "derivt"
	if (any(rows)){
  	cpp <- put_lines(cpp, 2, c("", "// calculate numerical derivatives"))
  	lines <- smoosh(slope, "= (", slopeof, "-", paste(slopeof, "_prev", sep=""), ") / ( t - t_prev ) ;")
  	cpp <- put_lines(cpp, 2, lines)
  	lines <- smoosh(paste(slopeof, "_prev", sep=""), "=", slopeof, ";")
  	cpp <- put_lines(cpp, 2, lines)
  	lines <- "t_prev = t ;"
  	cpp <- put_lines(cpp, 2, lines)
	}
	cpp <- put_lines(cpp, 1, c("", "} // end observer operator ", ""))

	# advance method
	lines <- c("int advance_model ( double end_time , double time_step ) {", "")
	cpp <- put_lines(cpp, 1, lines)
	lines <- c(
	  "double next_time = end_time;",
	  "static constexpr double eps = 0.00001;",
	  "state_type a_state;",
	  "double a_time;",
	  "int nsteps = 0;", "",
	  "while ( t < end_time ) {", "",
	  "\t// do current events",
	  "\tdo {",
	  "\t\tif ( event_list.begin() == event_list.end() ){",
	  "\t\t\t// no events",
	  "\t\t\tnext_time = end_time + 1;",
	  "\t\t} else if ( event_list.begin()->first < t - eps ) {",
	  "\t\t\t// missed event",
	  "\t\t\tevent_list.erase( event_list.begin() );",
	  "\t\t\tnext_time = t - 1;",
	  "\t\t} else if ( event_list.begin()->first < t + eps ) {",
	  "\t\t\tdo_event( event_list.begin()->second );",
	  "\t\t\tevent_list.erase( event_list.begin() );",
	  "\t\t\tnext_time = t - 1;",
	  "\t\t} else {",
	  "\t\t\t// next event",
	  "\t\t\tnext_time = event_list.begin()->first;",
	  "\t\t}",
	  "\t} while ( next_time < t + eps ) ;", "",
	  "\t// advance model to next event or end_time",
	  "\ta_state = get_state();",
	  "\ta_time = t;",
	  "\tnext_time = std::min( end_time, next_time );",
	  "\t// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class",
	  "\tnsteps += boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , next_time , time_step , *this );",
	  "\tset_state( a_state );",
	  "\tt = next_time;",
	  "\tcalculate_rate();",
	  "\tpost_processing();",
	  "", "}", "",
	  "return( nsteps );")
	cpp <- put_lines(cpp, 2, lines)
	cpp <- put_lines(cpp, 1, c("", "} // end advance_model"))
	cpp <- put_lines(cpp, 0, c("", "}; // end class", ""))

	cat("total lines :", length(cpp), "\n")

	return(cpp)

}


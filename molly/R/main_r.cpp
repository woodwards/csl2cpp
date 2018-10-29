#include <Rcpp.h>

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]
// Boost headers
// [[Rcpp::depends(BH)]]

#include "molly.cpp"

molly_class molly;

// [[Rcpp::export]]
void initialise_model( double start_time = 0.0 , bool set_debug_status = false ){
  	molly.initialise_model( start_time , set_debug_status );
}

// https://stackoverflow.com/questions/34181135/converting-an-stdmap-to-an-rcpplist
// [[Rcpp::export]]
Rcpp::NumericVector get_molly_variables(){
	Rcpp::NumericVector my_vector;
	molly.pull_variables_from_model();
	my_vector = molly.variable; // coerces unordered_map to named vector
    return my_vector;
}

// [[Rcpp::export]]
int advance_model( double to_time , double dt ){
  	int nsteps;
  	nsteps = molly.advance_model( to_time , dt );
  	return nsteps;
}


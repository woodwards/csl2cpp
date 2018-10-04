#include <Rcpp.h>

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]
// Boost headers
// [[Rcpp::depends(BH)]]

#include "molly.cpp"

molly my_molly;


// [[Rcpp::export]]
void initialise_model( double t ){
  	my_molly.initialise_model( t );
}

// [[Rcpp::export]]
void pull_variables_from_model(){
  	my_molly.pull_variables_from_model();
}

// https://stackoverflow.com/questions/34181135/converting-an-stdmap-to-an-rcpplist
// [[Rcpp::export]]
Rcpp::NumericVector get_molly_variables(){
	Rcpp::NumericVector my_vector;
    my_vector = my_molly.variable; // coerces unordered_map to named vector
    return my_vector;
}

// [[Rcpp::export]]
int advance_model( double t , double dt ){
  	int nsteps;
  	nsteps = my_molly.advance_model( t , dt );
  	return nsteps;
}


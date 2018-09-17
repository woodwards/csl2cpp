#include <Rcpp.h>

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]
// Boost headers
// [[Rcpp::depends(BH)]]

#include "spring.cpp"

spring my_spring;


// [[Rcpp::export]]
void initialise_model( double t ){
  	my_spring.initialise_model( t );
}

// [[Rcpp::export]]
void pull_variables_from_model(){
  	my_spring.pull_variables_from_model();
}

// https://stackoverflow.com/questions/34181135/converting-an-stdmap-to-an-rcpplist
// [[Rcpp::export]]
Rcpp::NumericVector get_spring_variables(){
	Rcpp::NumericVector my_vector;
    my_vector = my_spring.variable; // coerces unordered_map to named vector
    return my_vector;
}

// [[Rcpp::export]]
int advance_model( double t , double dt ){
  	int nsteps;
  	nsteps = my_spring.advance_model( t , dt );
  	return nsteps;
}


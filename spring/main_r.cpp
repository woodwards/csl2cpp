#include <Rcpp.h>

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]
// Boost headers
// [[Rcpp::depends(BH)]]

#include "spring.h"

spring_class spring;

// [[Rcpp::export]]
void initialise_model( double t = 0 , bool debug = false ){
  	spring.initialise_model( t , debug );
}

// [[Rcpp::export]]
void pull_variables_from_model(){
  	spring.pull_variables_from_model();
}

// https://stackoverflow.com/questions/34181135/converting-an-stdmap-to-an-rcpplist
// [[Rcpp::export]]
Rcpp::NumericVector get_spring_variables(){
	Rcpp::NumericVector my_vector;
    my_vector = spring.variable; // coerces unordered_map to named vector
    return my_vector;
}

// [[Rcpp::export]]
int advance_model( double t , double dt ){
  	int nsteps;
  	nsteps = spring.advance_model( t , dt );
  	return nsteps;
}


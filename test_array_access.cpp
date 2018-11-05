#include <unordered_map>
// #include <map>
// #include <cmath>
// #include <string>
#include <iostream>
#include <vector>
#include <Rcpp.h>
#include <functional>

// using namespace std;

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]

class model_class {

public:

// https://stackoverflow.com/questions/22957205/how-do-you-declare-a-pointer-to-a-c11-stdarray
// https://codereview.stackexchange.com/questions/142815/generic-matrix-type-in-c
// https://stackoverflow.com/questions/17663186/initializing-a-two-dimensional-stdvector

	// int iTotMeals = 22 ;
	// double TNdfIn = 8.8 ;
	//
	static constexpr auto MaxHerds = 1 ;
	static constexpr int MaxInitValues = 5 ;
	static constexpr int MaxAnimals = 2 ;

	// std::array< double , MaxHerds > MamCellsF ;
	// std::array< std::array< double , MaxInitValues > , MaxAnimals > InitCond  ;
  std::vector< double > MamCellsF ;
  std::vector< std::vector< double > > InitCond ;

public:

	// std::unordered_map< std::string , double > variable;
  std::unordered_map< std::string , std::reference_wrapper< std::vector< double > > > vector;
  std::unordered_map< std::string , std::vector< std::vector< double > >* > array;

  model_class ( ) :
  // constructor initialisation list (C++11)
     // InitCond { { 1.0 , 550 , 3.5 , 1 , 4 } ,
     //       { 2.0 , 625 , 3.5 , 5 , 4 } } ,
     // MamCellsF { 0.001 } // additional brackets make it 2d
    MamCellsF( MaxHerds , 0.07 ) ,
    InitCond( MaxAnimals , std::vector< double > ( MaxInitValues , 0.7 ) )
    // InitCond( { { 1.0 , 550 , 3.5 , 1 , 4 } ,
    // { 2.0 , 625 , 3.5 , 5 , 4 } } )
    // constructor body
  {
     // pull variables from model
     // variable["iTotMeals"] = iTotMeals;
     // variable["TNdfIn"] = TNdfIn;
     vector.insert({"MamCellsF", std::ref(MamCellsF)});
     array["InitCond"] = &InitCond;

     // initialise
     MamCellsF = { 0.001 , 0.002 }  ; // warning: automatic resizing
     InitCond = { { 1.0 , 550 , 3.5 , 1 , 4 } ,
     { 2.0 , 625 , 3.5 , 5 , 4 } } ;

  }

  void print_values(){

    // std::cout << variable["iTotMeals"] << std::endl;
    // std::cout << variable["TNdfIn"] << std::endl;

    double a = vector["MamCellsF"].get()[1];
    double b = (*array["InitCond"])[0][1];
    std::cout << a << std::endl;
    std::cout << b << std::endl;

  }

};

// [[Rcpp::export]]
void test()
{

  model_class model;
  model.print_values();

}

/*** R
# This is R code
test()
*/


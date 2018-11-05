#include <unordered_map>
#include <map>
#include <cmath>
#include <string>
#include <iostream>
#include <array>
#include <Rcpp.h>

using namespace std;

// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]

class model {

private:

  // https://stackoverflow.com/questions/10694689/how-to-initialize-an-array-in-c-objects
  // https://stackoverflow.com/questions/33714866/initialize-stdarray-of-classes-in-a-class-constructor

	static constexpr auto MaxHerds = 1 ;
	static constexpr int MaxInitValues = 5 ;
	static constexpr int MaxAnimals = 2 ;

	std::array< std::array< double , MaxInitValues > , MaxAnimals > InitCond  ;
	std::array< double , MaxHerds > MamCellsF ;

public:

  model()
  // constructor initialisation list (C++11)
  : InitCond { { { 1.0 , 550 , 3.5 , 1 , 4 } ,
          { 2.0 , 625 , 3.5 , 5 , 4 } } } ,
    MamCellsF { { 0.001 } }
  {
     // constructor body
  }

  double getone(){
    return InitCond[0][1] + MamCellsF[0];
  }

};

// [[Rcpp::export]]
void hello()
{

  model amodel;
  std::cout << std::to_string(amodel.getone()) << std::endl;

}

/*** R
# This is R code
hello()
*/

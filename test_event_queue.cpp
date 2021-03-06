#include <unordered_map>
#include <map>
#include <cmath>
#include <string>
#include <array>
#include <Rcpp.h>


// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]

// [[Rcpp::export]]
void hello()
{

  // std::array< double , 2 > a = {{1e4,2}};
  // std::array< std::array< double , 3 > , 2 > b = {{{1,2,3},
  //     {4,5,6}}};

  using namespace std;

  std::map< double, std::string > event_queue;
  std::string output;

  event_queue[1.1] = "event1";
  event_queue[2.2] = "event2";

  output = event_queue[1.1]; // works
  std::cout << output << std::endl;

  output = event_queue.begin()->second; // works
  std::cout << output << std::endl;

  if (event_queue.begin()->second == "event1"){ // works
    output = "true";
    event_queue.erase(event_queue.begin());
  } else {
    output = "false";
  }
  std::cout << output << std::endl;

  if (event_queue.begin()->second == "event2"){ // works
    output = "true";
  } else {
    output = "false";
  }
  std::cout << output << std::endl;

}

/*** R
# This is R code
hello()
*/

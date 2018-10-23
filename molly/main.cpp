#include <iostream>
#include <fstream>

#include "molly.cpp"

std::ofstream data("molly_cpp_output.tsv");

int main()
{

    double start_time = 0.0;
    double time_step = 1.0;
    double end_time = 300.0;
   	int nsteps = 0;

  	molly my_molly;
    my_molly.initialise_model( start_time );

    for( double time = start_time ; time < end_time ; time = time + time_step ){

		nsteps += my_molly.advance_model( time , time_step / 100.0 );
		my_molly.pull_variables_from_model();

		std::cout << nsteps << '\t' << my_molly.variable["t"] << '\t' << my_molly.variable["dEating"] << '\t' << my_molly.variable["WtPUter"] << '\t' << my_molly.variable["LhorAdip"] << std::endl;
     	     data << nsteps << '\t' << my_molly.variable["t"] << '\t' << my_molly.variable["dEating"] << '\t' << my_molly.variable["WtPUter"] << '\t' << my_molly.variable["LhorAdip"] << std::endl;

    }

  	return nsteps;

}


#include <iostream>
#include <fstream>

#include "spring.cpp"

std::ofstream data("spring_cpp_output.tsv");

int main()
{

    double start_time = 0.0;
    double time_step = 0.02; // default CINT = 0.1
    double end_time = 3.99;

    spring my_spring;
    my_spring.initialise_model( start_time );

	int nsteps;

    for( double time = start_time ; time < end_time ; time = time + time_step ){

		nsteps = my_spring.advance_model( time , time_step / 10.0 ); // default NSTP = 10
		my_spring.pull_variables_from_model();

		std::cout << nsteps << '\t' << my_spring.variable["system_time"] << '\t' << my_spring.variable["time"] << '\t' << my_spring.variable["xd"] << '\t' << my_spring.variable["x"] << std::endl;
     	     data << nsteps << '\t' << my_spring.variable["system_time"] << '\t' << my_spring.variable["time"] << '\t' << my_spring.variable["xd"] << '\t' << my_spring.variable["x"] << std::endl;

    }

}

#include <iostream>
#include <fstream>

#include "spring_extra.cpp"

using namespace std;

ofstream data("spring_cpp_output.tsv");

int main()
{

    double start_time = 0.0;
    double time_step = 0.02; // default CINT = 0.1
    double end_time = 3.99;

    spring_extra my_spring;
    my_spring.initialise_time( start_time );
    my_spring.initialise_state();
	double a_time;
	spring_extra::state_type a_state;

	int nsteps;

    for( double time = start_time ; time < end_time ; time = time + time_step ){

		nsteps = my_spring.advance( time , time_step / 10.0 ); // default NSTP = 10
		a_time = my_spring.get_time();
		a_state = my_spring.get_state();

		cout << nsteps << '\t' << a_time << '\t' << a_state[0] << '\t' << a_state[1] << '\t' << a_state[2] << endl;
		data << nsteps << '\t' << a_time << '\t' << a_state[0] << '\t' << a_state[1] << '\t' << a_state[2] << endl;

    }

}

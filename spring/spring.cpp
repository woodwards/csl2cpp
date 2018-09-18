#include <unordered_map>
#include <string>
#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include <boost/numeric/odeint.hpp>
#include <boost/array.hpp>

class spring {

public:

	// unordered_map gives user efficient access to variables by name
	std::unordered_map< std::string , double > variable;

private:

	// declare state_type and system_time
	typedef boost::array< double , 3 > state_type;
	double system_time;
	
	// specify number of variables available to user
	static constexpr int system_variables = 22 ;
	
	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;

public:

	// constructor
	spring () {
	
		// reserve buckets to minimise storage and avoid rehashing
		variable.reserve( system_variables );
	
	}

public:

	int advance_model( double end_time , double time_step ){
	
		double a_time;
		state_type a_state;
		int nsteps;
		a_time = system_time;
		a_state = get_state();
		
		// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class
		nsteps = boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , end_time , time_step );
		
		system_time = end_time;
		set_state( a_state );
		calculate_rate();
		return( nsteps );
	
	}

}; // end class


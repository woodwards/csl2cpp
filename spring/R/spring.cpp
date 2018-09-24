// *******************************************************
// this module is automatically generated from an R script
// *******************************************************
#include <unordered_map>
#include <string>
#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include <boost/numeric/odeint.hpp>
#include <boost/array.hpp>

class spring {

private:

	// specify number of variables
	static constexpr int n_state_variables = 3 ;
	static constexpr int n_visible_variables = 12 ;
	
	// declare state_type and system_time
	typedef boost::array< double , n_state_variables > state_type;
	double system_time;
	
	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;
	
	// declare model variables
	static constexpr double k = 0.12 ;
	static constexpr double a = 1.0 ;
	static constexpr double w = 1.0 , g = 9.81 ;
	static constexpr double mass = 0.03 ;
	double time ;
	double xdd ;
	static constexpr double xic = 0.0 , xdic = 0.0 ;
	double xd ;
	double x ;
	static constexpr double Tstp = 3.99 ;
	
	state_type get_state ( ) {
		
		state_type a_state;
		
		// return current state
		a_state[0] = time;
		a_state[1] = xd;
		a_state[2] = x;
		
		return( a_state );
	
	} // end get_state
	
	void set_state ( state_type a_state ) {
		
		// set state
		time = a_state[0];
		xd = a_state[1];
		x = a_state[2];
	
	} // end set_state

public:

	// unordered_map gives user efficient access to variables by name
	std::unordered_map< std::string , double > variable;
	
	// constructor
	spring ( ) {
	
		// reserve buckets to minimise storage and avoid rehashing
		variable.reserve( n_visible_variables );
	
	} // end constructor
	
	void initialise_model ( double a_system_time ) {
		
		// initialise system_time
		system_time = a_system_time;
		
		// initialise model
		time = 0.0; 
		xd = xdic; 
		x = xic; 
	
	} // end initialise_model
	
	void pull_variables_from_model ( ) {
		
		// pull system time
		variable["system_time"] = system_time;
		
		// pull model variables
		variable["time"] = time;
		variable["xdd"] = xdd;
		variable["xd"] = xd;
		variable["x"] = x;
	
	} // end pull_variables_from_model
	
	void push_variables_to_model ( ) {
		
		// push system time
		system_time = variable["system_time"];
		
		// push model variables
		time = variable["time"];
		xdd = variable["xdd"];
		xd = variable["xd"];
		x = variable["x"];
	
	} // end push_variables_to_model
	
	void calculate_rate ( ) {
		
		// calculations
		// -------spring damping problem. models releasing
		//       mass from initial conditions of zero
		//       velocity and displacement
		
		// -------define model default constants
		// simon's comment
		// *** semicolon ***
		// -------another way of changing the independent
		//       variable
		// -------calculate acceleration
		xdd = ( mass * g - k * xd - a * x ) / mass; 
		// -------integrate accel for velocity and position
		// -------specify termination condition
		
		// of derivative
		// of program
	
	} // end calculate_rate
	
	// called by boost::odeint::integrate()
	void operator()( const state_type &a_state , state_type &a_rate, double a_time ){
		
		// set state
		system_time = a_time;
		time = a_state[0];
		xd = a_state[1];
		x = a_state[2];
		
		// calculate rate
		calculate_rate();
		
		// return rate
		a_rate[0]= 1.0;
		a_rate[1]= xdd;
		a_rate[2]= xd;
	
	} // end operator
	
	int advance_model ( double end_time , double time_step ) {
	
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
	
	} // end advance_model

}; // end class


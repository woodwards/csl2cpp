// *******************************************************
// this module is automatically generated from an R script
// *******************************************************
#include <unordered_map>
#include <string>
#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include <boost/numeric/odeint.hpp>
#include <boost/array.hpp>

// program spring 

class spring {

private:

	// specify number of variables
	static constexpr int n_state_variables = 3 ;
	static constexpr int n_visible_variables = 12 ;
	
	// declare state_type and t
	typedef boost::array< double , n_state_variables > state_type;
	double t;
	
	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;
	
	// declare model variables
	static constexpr double k = 0.12 ; // constant k = 0.12 // simon's comment
	static constexpr double a = 1.0 ; // constant a = 1.0
	static constexpr double w = 1.0 , g = 9.81 ; // constant w = 1.0 , g = 9.81
	static constexpr double mass = 0.03 ; // constant mass = 0.03
	double time ; // time = integ ( 1.0 , 0.0 )
	double xdd ;
	static constexpr double xic = 0.0 , xdic = 0.0 ; // constant xic = 0.0 , xdic = 0.0
	double xd ; // xd = integ ( xdd , xdic )
	double x ; // x = integ ( xd , xic )
	static constexpr double Tstp = 3.99 ; // constant Tstp = 3.99
	
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
	
	void initialise_model ( double a_time ) {
		
		// initialise t
		t = a_time;
		
	} // end initialise_model
	
	void pull_variables_from_model ( ) {
		
		// pull system time
		variable["t"] = t;
		
		// pull model variables
		variable["time"] = time;
		variable["xdd"] = xdd;
		variable["xd"] = xd;
		variable["x"] = x;
	
	} // end pull_variables_from_model
	
	void push_variables_to_model ( ) {
		
		// push system time
		t = variable["t"];
		
		// push model variables
		time = variable["time"];
		xdd = variable["xdd"];
		xd = variable["xd"];
		x = variable["x"];
	
	} // end push_variables_to_model
	
	void calculate_rate ( ) {
		
		// calculations
		// derivative 
		// -------spring damping problem. models releasing
		//       mass from initial conditions of zero
		//       velocity and displacement
		
		
		// -------define model default constants
		// constant k = 0.12 // simon's comment
		// constant a = 1.0 
		// constant w = 1.0 , g = 9.81 
		// constant mass = 0.03 
		// -------another way of changing the independent
		//       variable
		// time = integ ( 1.0 , 0.0 ) 
		// -------calculate acceleration
		xdd = ( mass * g - k * xd - a * x ) / mass ;
		// -------integrate accel for velocity and position
		// constant xic = 0.0 , xdic = 0.0 
		// xd = integ ( xdd , xdic ) 
		// x = integ ( xd , xic ) 
		// -------specify termination condition
		// constant Tstp = 3.99 
		
		
		// end of derivative // of derivative
		// end of program // of program
	
	} // end calculate_rate
	
	// called by boost::odeint::integrate()
	void operator()( const state_type &a_state , state_type &a_rate, double a_time ){
		
		// set state
		t = a_time;
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
		a_time = t;
		a_state = get_state();
		
		// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class
		nsteps = boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , end_time , time_step );
		
		t = end_time;
		set_state( a_state );
		calculate_rate();
		return( nsteps );
	
	} // end advance_model

}; // end class


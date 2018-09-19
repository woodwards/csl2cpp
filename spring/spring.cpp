// *******************************************************
// this module is automatically generated from an R script
// *******************************************************
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
	static constexpr int system_variables = 12 ;
	
	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;
	
	// state variables
	double time ;
	double xd ;
	double x ;
	
	// other variables
	double k ;
	double a ;
	double w ;
	double g ;
	double mass ;
	double xdd ;
	double xic ;
	double xdic ;
	double tstp ;
	

public:

	// constructor
	spring ( ) {
	
		// reserve buckets to minimise storage and avoid rehashing
		variable.reserve( system_variables );
	
	}

	void initialise_model ( double a_system_time ) {
		
		// initialise system_time
		system_time = a_system_time;
		
		// initialisation
		k = 0.12 ;
		a = 1.0 ;
		w = 1.0 ; g = 9.81 ;
		mass = 0.03 ;
		time = 0.0 ;
		xic = 0.0 ; xdic = 0.0 ;
		xd = xdic ;
		x = xic ;
		tstp = 3.99 ;
	
	}
	

	void pull_variables_from_model ( ) {
		
		// pull system time
		variable["system_time"] = system_time;
		
		// pull model variables
		variable["k"] = k;
		variable["a"] = a;
		variable["w"] = w;
		variable["g"] = g;
		variable["mass"] = mass;
		variable["time"] = time;
		variable["xdd"] = xdd;
		variable["xic"] = xic;
		variable["xdic"] = xdic;
		variable["xd"] = xd;
		variable["x"] = x;
		variable["tstp"] = tstp;
	
	}
	

	void push_variables_to_model ( ) {
		
		// push system time
		system_time = variable["system_time"];
		
		// push model variables
		k = variable["k"];
		a = variable["a"];
		w = variable["w"];
		g = variable["g"];
		mass = variable["mass"];
		time = variable["time"];
		xdd = variable["xdd"];
		xic = variable["xic"];
		xdic = variable["xdic"];
		xd = variable["xd"];
		x = variable["x"];
		tstp = variable["tstp"];
	
	}
	

private:

	state_type get_state ( ) {
		
		state_type a_state;
		
		// return current state
		a_state[0] = time;
		a_state[1] = xd;
		a_state[2] = x;
		
		return( a_state );
	
	}
	
	void set_state ( state_type a_state ) {
		
		// set state
		time = a_state[0];
		xd = a_state[1];
		x = a_state[2];
	
	}

public:

	void calculate_rate ( ) {
		
		// calculations
		xdd = ( mass * g - k * xd - a * x ) / mass ;
	
	}
	
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
		a_rate[0] = 1.0;
		a_rate[1] = xdd;
		a_rate[2] = xd;
	
	}

public:

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
	
	}

}; // end class


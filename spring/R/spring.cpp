#include <unordered_map>
#include <string>
#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include <boost/numeric/odeint.hpp>
#include <boost/array.hpp>

class spring{

public:

	// unordered_map gives user efficient access to variables by name
	// https://stackoverflow.com/questions/8483985/obtaining-list-of-keys-and-values-from-unordered-map
	// will be more efficient if we intelligently reserve storage for the map at construction time
	// https://stackoverflow.com/questions/15135900/how-to-pass-a-mapdouble-t-from-c-to-r-with-rcpp
	std::unordered_map< std::string , double > variable;

private:

	// declare state_type and system_time
	typedef boost::array< double , 3 > state_type;
	double system_time;
	static constexpr int system_variables = 12; // number of variables available to user

	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;

	// state variables
	double time;
	double xd;
	double x;

	// constants
	static constexpr double k = 0.12;
	static constexpr double a = 1.0;
	static constexpr double w = 1.0;
	static constexpr double g = 9.81;
	static constexpr double mass = 0.03;
	static constexpr double xdic = 0.0;
	static constexpr double xic = 0.0;

	// other variables
	double xdd = -999;

public:

	// constructor
	spring(){

	    // reserve buckets to minimise storage and avoid rehashing
	    // note: only need variables the user needs to set or get
	    variable.reserve( system_variables );

	}

	void initialise_model( double a_system_time ){

		// initialise system time
		system_time = a_system_time;

		// initial calculations

		// initial conditions
		time = 0.0;
		xd = xdic;
		x = xic;

	}

	void pull_variables_from_model(){

		// generic system property
		variable["system_time"] = system_time;

		// state variables
		variable["time"] = time;
		variable["xd"] = xd;
		variable["x"] = x;

		// constants
		variable["k"] = k;
		variable["a"] = a;
		variable["w"] = w;
		variable["g"] = g;
		variable["mass"] = mass;
		variable["xdic"] = xdic;
		variable["xic"] = xic;

		// other variables
		variable["xdd"] = xdd;

	}

	void push_variables_to_model(){

		// generic system property
		system_time = variable["system_time"];

		// state variables
		time = variable["time"];
		xd = variable["xd"];
		x = variable["x"];

		// constants (cant change these)
//		k = variable["k"];
//		a = variable["a"];
//		w = variable["w"];
//		g = variable["g"];
//		mass = variable["mass"];
//		xdic = variable["xdic"];
//		xic = variable["xic"];

		// other variables
		xdd = variable["xdd"];

	}

private:

	state_type get_state(){

		state_type a_state;

		// return current state
		a_state[0] = time;
		a_state[1] = xd;
		a_state[2] = x;

		return( a_state );

	}

	void set_state( state_type a_state ){

		// set state
		time = a_state[0];
		xd = a_state[1];
		x = a_state[2];

	}

public:

	void calculate_rate(){

		// calculations
		xdd = ( mass * g - k * xd - a * x ) / mass;

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

}; // end class spring_type

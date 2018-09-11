#include <boost/numeric/odeint.hpp>

//using namespace std;
//using namespace boost::numeric::odeint;

class spring{

public:

	// declare state_type
	typedef boost::array< double , 3 > state_type;

<<<<<<< HEAD
	// container for variables
//	std::unordered_map<std::string, double> d;

=======
>>>>>>> 1440ac92eea6b7ebe1cf5ffb52818575a8b19891
	// generic system properties
	double system_time;

	// state variables
	double time;
	double xd;
	double x;

	// constants
	const double k = 0.12;
	const double a = 1.0;
	const double w = 1.0;
	const double g = 9.81;
	const double mass = 0.03;
	const double xdic = 0.0;
	const double xic = 0.0;

	// other variables
	double xdd;

//	void initialise_time( double a_time ){
//
//		// initialise system time
//		system_time = a_time;
//
//	}

	void initialise_state(){

		// initial calculations

		// set initial conditions
		time = 0.0;
		xd = xdic;
		x = xic;

	}

//	double get_time(){
//
//		return( system_time );
//
//	}

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

	void calculate_rate(){

		// calculations
		xdd = ( mass * g - k * xd - a * x ) / mass;

	}

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

//	int advance( const double end_time , const double time_step ){
//
//		double a_time;
//		state_type a_state;
//
//	    int nsteps;
//
//		a_time = get_time();
//		a_state = get_state();
//
//		nsteps = integrate_const( stepper , *this , a_state, a_time , end_time , time_step );
//
//		system_time = end_time;
//		set_state( a_state );
//
//		return(nsteps);
//
//	}

}; // end class spring_type

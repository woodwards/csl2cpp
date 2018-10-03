// *******************************************************
// this module is automatically generated from an R script
// *******************************************************
#include <unordered_map>
#include <map>
#include <string>
#include <cmath>
#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#include <boost/numeric/odeint.hpp>
#include <array>

// program spring
// end of program // of program

class spring {

private:

	// specify number of variables
	static constexpr int n_state_variables = 3 ;
	static constexpr int n_visible_variables = 13 ;

	// declare state_type and t
	typedef std::array< double , n_state_variables > state_type;
	double t;

	// event queue
	std::map< double , std::string > schedule;

	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;

	// declare model variables
	double cint ;
	double k ; // simon's comment
	double a ;
	double w , g ;
	double mass ;
	double time ; // time = integ ( 1.0 , 0.0 )
	double xdd ;
	double xic , xdic ;
	double xd ; // xd = integ ( xdd , xdic )
	double x ; // x = integ ( xd , xic )
	double Tstp ;

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

		// initialise model
		cint = 0.02 ;
		k = 0.12 ; // simon's comment
		a = 1.0 ;
		w = 1.0 ; g = 9.81 ;
		mass = 0.03 ;
		time = 0.0 ; // time = integ ( 1.0 , 0.0 )
		xic = 0.0 ; xdic = 0.0 ;
		xd = xdic ; // xd = integ ( xdd , xdic )
		x = xic ; // x = integ ( xd , xic )
		Tstp = 3.99 ;

	} // end initialise_model

	void pull_variables_from_model ( ) {

		// pull system time
		variable["t"] = t;

		// pull model variables
		variable["cint"] = cint;
		variable["k"] = k;
		variable["a"] = a;
		variable["g"] = g;
		variable["w"] = w;
		variable["mass"] = mass;
		variable["time"] = time;
		variable["xdd"] = xdd;
		variable["xdic"] = xdic;
		variable["xic"] = xic;
		variable["xd"] = xd;
		variable["x"] = x;
		variable["Tstp"] = Tstp;

	} // end pull_variables_from_model

	void push_variables_to_model ( ) {

		// push system time
		t = variable["t"];

		// push model variables
		cint = variable["cint"];
		k = variable["k"];
		a = variable["a"];
		g = variable["g"];
		w = variable["w"];
		mass = variable["mass"];
		time = variable["time"];
		xdd = variable["xdd"];
		xdic = variable["xdic"];
		xic = variable["xic"];
		xd = variable["xd"];
		x = variable["x"];
		Tstp = variable["Tstp"];

	} // end push_variables_to_model

	void do_event ( ) {

	} // end do_event

	void calculate_rate ( ) {

		// calculations
		// derivative
		// -------spring damping problem. models releasing
		//       mass from initial conditions of zero
		//       velocity and displacement


		// -------define model default constants
		// simon's comment



		// -------another way of changing the independent
		//       variable
		// time = integ ( 1.0 , 0.0 )
		// -------calculate acceleration
		xdd = ( mass * g - k * xd - a * x ) / mass ;
		// -------integrate accel for velocity and position

		// xd = integ ( xdd , xdic )
		// x = integ ( xd , xic )
		// -------specify termination condition

		// termt ( t >= Tstp , 'Time Limit' )

		// end of derivative // of derivative

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

		double next_time;
		state_type a_state;
		double a_time;
		int nsteps = 0;

		while ( t < end_time ) {

			// do current events
			do {
				if ( schedule.begin() == schedule.end() ){
					// no events
					next_time = end_time + 1;
				} else if ( schedule.begin()->first < t ) {
					// missed event
					schedule.erase( schedule.begin() );
					next_time = t - 1;
				} else if ( schedule.begin()->first == t ) {
					do_event();
					schedule.erase( schedule.begin() );
					next_time = t - 1;
				} else if ( schedule.begin()->first > t ) {
					// next event
					next_time = schedule.begin()->first;
				}
			} while ( next_time <= t) ;

			// advance model to next event or end_time
			a_state = get_state();
			a_time = t;
			next_time = std::min( end_time, next_time );
			// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class
			nsteps += boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , next_time , time_step );
			set_state( a_state );
			t = next_time;
			calculate_rate();

		}

		return( nsteps );

	} // end advance_model

}; // end class


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

using namespace std; // needed for math functions

// program spring 
// end of program // of program

class spring_class {

private:

	// switches
	bool print_debug_messages = false;
	
	// specify number of variables
	static constexpr int n_state_variables = 3 ;
	static constexpr int n_visible_variables = 12 ;
	
	// declare state_type and t
	typedef std::array< double , n_state_variables > state_type;
	double t;
	
	// declare event list
	std::multimap< double , std::string > event_list;
	
	// add event
	void schedule( double event_time, std::string event_name ){
	
		event_list.insert( std::make_pair( event_time , event_name ) );
	
	}
	
	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;
	
	// declare model variables
	static constexpr auto cint = 0.02 ;
	double k = 0.12 ;
	double a = 1.0 ;
	double w = 1.0 , g = 9.81 ;
	double mass = 0.03 ;
	double time ;
	double xdd ;
	double xic = 0.0 , xdic = 0.0 ;
	double xd ;
	double x ;
	double Tstp = 3.99 ;
	
	state_type get_state ( ) {
		
		state_type current_state;
		
		// return current state
		current_state[0] = time;
		current_state[1] = xd;
		current_state[2] = x;
		
		return( current_state );
	
	} // end get_state
	
	void set_state ( state_type this_state ) {
		
		// set state
		time = this_state[0];
		xd = this_state[1];
		x = this_state[2];
	
	} // end set_state

public:

	// unordered_map gives user efficient access to variables by name
	std::unordered_map< std::string , double > variable;
	
	// constructor
	spring_class ( )
	
	// constructor body
	{
	
		// reserve buckets to minimise storage and avoid rehashing
		variable.reserve( n_visible_variables );
	
	} // end constructor
	
	void initialise_model ( double start_time = 0.0 , bool set_debug_status = false ) {
		
		// initialise t
		t = start_time;
		
		// set debug status
		print_debug_messages = set_debug_status;
		
		// initialise state variables
		time = 0.0 ;
		xd = xdic ;
		x = xic ;
		
		// initial calculation of rates
		calculate_rate( );
		
	} // end initialise_model
	
	void pull_variables_from_model ( ) {
		
		// pull system time
		variable["t"] = t;
		
		// pull model variables
		variable["a"] = a;
		variable["g"] = g;
		variable["k"] = k;
		variable["mass"] = mass;
		variable["time"] = time;
		variable["Tstp"] = Tstp;
		variable["w"] = w;
		variable["x"] = x;
		variable["xd"] = xd;
		variable["xdd"] = xdd;
		variable["xdic"] = xdic;
		variable["xic"] = xic;
	
	} // end pull_variables_from_model
	
	void push_variables_to_model ( ) {
		
		// push system time
		t = variable["t"];
		
		// push model variables
		a = variable["a"];
		g = variable["g"];
		k = variable["k"];
		mass = variable["mass"];
		time = variable["time"];
		Tstp = variable["Tstp"];
		w = variable["w"];
		x = variable["x"];
		xd = variable["xd"];
		xdd = variable["xdd"];
		xdic = variable["xdic"];
		xic = variable["xic"];
	
	} // end push_variables_to_model
	
	void do_event ( string next_event ) {
	
	} // end do_event
	
	double do_current_events ( double max_next_time ) {
	
		static constexpr double eps = 0.00001;
		double next_time = max_next_time;
		
		if (print_debug_messages) {
			std::cout << "event check at" << '\t' << t << std::endl;
		}
		
		do {
			if ( event_list.begin( ) == event_list.end( ) ){
				// no events
				next_time = max_next_time + 1;
			} else if ( event_list.begin( )->first < t - eps ) {
				// missed event (handle bugs)
				event_list.erase( event_list.begin( ) );
				next_time = t - 1;
			} else if ( event_list.begin( )->first < t + eps ) {
				if (print_debug_messages) {
					std::cout << "event at" << '\t' << t << std::endl;
				}
				do_event( event_list.begin( )->second );
				event_list.erase( event_list.begin( ) );
				next_time = t - 1;
			} else {
				// next event
				next_time = event_list.begin( )->first;
			}
		} while ( next_time < t + eps );
		
		next_time = std::min( max_next_time, next_time );
		
		if (print_debug_messages) {
			std::cout << "next event at" << '\t' << next_time << std::endl;
		}
		
		return( next_time );
		
	} // end do_current_events
	
	void calculate_rate ( ) {
		
		// derivative calculations
		
		// derivative 
		// -------spring damping problem. models releasing
		//       mass from initial conditions of zero
		//       velocity and displacement
		
		// cinterval cint = 0.02 
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
		// termt ( t >= Tstp , 'Time Limit' ) 
		
		// end of derivative // of derivative
	
	} // end calculate_rate
	
	void post_processing ( ) {
		
	} // end post_processing
	
	// rate operator called by boost::odeint::integrate( )
	void operator( )( const state_type &odeint_state , state_type &odeint_rate, double odeint_time ){
	
		// calculate rate
		set_state( odeint_state );
		t = odeint_time;
		calculate_rate( );
		
		// return rate
		odeint_rate[0] = 1.0;
		odeint_rate[1] = xdd;
		odeint_rate[2] = xd;
	
	} // end rate operator
	
	// observer operator called by boost::odeint::integrate( )
	// https://stackoverflow.com/questions/12150160/odeint-streaming-observer-and-related-questions
	void operator( )( const state_type &odeint_state , double odeint_time ){
	
	} // end observer operator 
	
	int step_to_next_time ( double next_time , double time_step ) {
	
		state_type odeint_state;
		double odeint_time;
		int nsteps;
		
		if (print_debug_messages) {
			std::cout << "start integ at" << '\t' << t << std::endl;
		}
		
		odeint_state = get_state( );
		odeint_time = t;
		// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class
		nsteps = boost::numeric::odeint::integrate_const( stepper , std::ref(*this) , odeint_state, odeint_time , next_time , time_step , std::ref(*this) );
		set_state( odeint_state );
		t = next_time;
		
		return( nsteps );
		
	} // end step_to_next_time
	
	int advance_model ( double advance_time , double time_step ) {
	
		double next_time;
		int nsteps = 0;
		
		do {
		
			// do current events
			next_time = do_current_events( advance_time );
		
			// advance model to next event or advance_time
			nsteps += step_to_next_time( next_time , time_step );
		
		} while ( t < advance_time );
		
		post_processing( );
		
		return( nsteps );
	
	} // end advance_model

}; // end spring_class


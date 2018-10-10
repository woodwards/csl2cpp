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

// ************************ Molly Ingredient Input ********************************
// Derived from a version of Molly closest to Lee's Book with errors fixed
// and with arrays for feed and animal inputs.
// program MOLLY 
// constant version = 8.9555 
// constant numVersion = 5.2555 
// constant MyPi = 3.1555 

// INITIAL ! Documentaion only section to enable collapsing. The real initial follows right after...
//

// Molly5x
// Added a medium particle pool (MPart) to the rumen model to better represent particle
// outflow rates. Changed Sp and Lp names to SPart and LPart for consistency. 4-21-13, MDH
// Found one instance of DaysinMilk used to trap small numbers and changed to DayMilk
// Added some if statements to define several variables including RumDM at T=0 to avoid use before definition, i.e. IHor, MDH
// Added calculations of water balance and urine output including a bladder integral to determine time of day fo urination.
//
// Molly 8.8 Gil Nov 2012
//   Improvements for Extended Lactation:
//   kLhorPp replaced with fMamCellsPaPp that increases mam cells proliferaion as opposed
//      to kLhorPp which increased Lhor so a cow can sustain a multi-year lactaion (3y or more).
//      cells proliferation will peak with day-length-rate-of-change so the udder and
//      milk yield of successive years would peak - as before - together with DayLength.
//   Required Intake pushed up as days lengthens to support the udder
//      recovery during successive summers (Currently yield does not effect required intake
//      since we moved away from RFEED4, to avoid the cirularity of yield <-> intake)
//   PpLatitudeFactor introduced It is modulated to be zero at latitudes under
//      <23> (to be refined) as we do not want any PP effect in latitudes where daylength
//      is 11-13 hours) and also saturated at latitudes above <50> (to be refined), as we
//      do not want the PP effect at lat 60 to be significantly higher than at lat 40).
//   ObesityFactorLhor introduced, cows gaining LW above certain (high) BCS will suffer a
//      decline in LHOR (rather than just flattening). This is essential to get reasonable
//      fit to DairyNZ's EL trial data (where the better fed Friesian cows consistently
//      produces less milk, and obesity is the commonly assumed cause), in order to enable
//      the finer fit of the PP effect
//
// Molly 8.7555 Gil Nov 2012
//      Lactation callibration parameter values incorporated in the code
//
// Molly 8.7555 Gil Nov 2012
//   UdderFillFactor: introduced to allow extra reduction in 1X yield
//      around peak lactaion creating more flexibility for the model by
//      simulating the pressure in the udder as limiting yield (while so
//      far we've considered only the low frequency as a limiting factor)
//      However this avenue was estiomated as negligable by the optimiser
//      so I intend to remove it in future versions.
//
//   MilkingFrequencyAdjusted: introduced to enable slower recovery upon
//      transition from 1x to 2x, as the model is to fast to recover.
//
//   Q_likelihood_to_die_relative_to_A: was removed as it was consitently
//      estimated as 1 (which is a "do nothing" value, being a multiplier)
//
//   dNEP: (daily NEP) added as NEP became momentary
//      when intermittent milking was introduced
//
//   MilkingHours: array filled with 4 times a day so Molly
//      can now change MF between 1 and 4 woithout crashing
//
//   DailySummary: timing slightly changed X.99 to ensure it is done
//      before the day step stop (for WFM) which occurs around Y.00 +- MAXT)
//
//
// Molly 8.7555 Gil 29 Aug 12
//    Mindy supplementation finalised (preliminary attraction factor)
//    Most changes are in intermittent_eating_deriv.csl
//
// Molly 8.7 Gil July? 2012
//   Merge of Molly 8.5 and 8.6
//
// Molly 8.7555 GL Aug 2012
//    Folder structure changed so projects Mindy , Molly etc co-exist and share the common files
//
// Molly 8.5555 GL  July 2012
//    Merge with branch 'MindyStart'onGit.
//    That is, changes made by Mark early 2012 "Molly86 for Mindy" wer incorporated in the master
//    version which is  the big merge Mark + Gil March 2012 made to compile and run by Gil
//    Conflict wer in these files:
//    	Files/Intermittent_Eating_Init.csl
//    	Files/Intermittent_Eating_deriv.csl
//   	Files/Molly.apj
//    	Files/Molly_ProximateExpand_In_Deriv.csl
//    	Files/Molly_ProximateExpand_In_Init.csl
//    	Molly.csl
//
// Molly 8.6 MDH Dec 2011 **** BRANCHED FROM 8.4555 ****
//   Changes for Mindy
//

// Molly 8.5555 Gil May 2012
//    Lhor eqautions in MamCell_DairyNZ_in_deriv.scl were changed to use cAa / cAaBase instead of AA / AaBase.
//    Some integer numeric literals (e.g. 1) in double percision equations were changed to 1.0 in DairyNZ
//    include files (FdRat, MamCells). csl's behaviour in those cases was unperdictable until the change!!!
//
// Molly 8.5555 Gil April 2012 Gil Merged code made working
// 		First daily summary at t = 0 instead of T = 1
//    Some variables initialisations to zero added e.g. dMilkProd
// 		MamEnz equation moved to MamCells_MDH_in_deriv.csl / MamCells_DairyNZ_in_deriv.csl
//
// Molly 8.5555 March 2012 MDH/Gil Merged code
//
// Molly 8.5555 March 2012 Gil
//     Prepearation for merging with Mark
//     Latest interim parameter values (Fdrat_DairyNZ_constants.csl; Mamcells_dairyNZ_in_deriv.csl)
//     Molly.csl: Got rid of the warnings about re-computing parameters but ConceiveNow, DryOffNow now need to be reset to zero by the m file / WFM.
//     Added AbortPregNow constant top allow for abortion through M file (more doc inside)
//     Fdrat_DairyNz.csl:  Separated Dry and lactation cow maintenance requirement
//
// 		 WFM recent callibration (interim) main parameter values that are NOT in the code are:
//         BcsBase = 3.2555	 (in code = 3)
//         MilkProductionAgeAdjustment = 0.7555 (2y old)	|  0.9555	(3y old) |  1 (4+y old)  |  (in code = 1)
// 			   vmAaPmVis	    0.002555 (Freisian) | 	0.002555 (Jersey)	0.001555 (Holstein) | (in code = 0.002555)
//         vmGlLmVisPart	0.001555 (Freisian) | 	0.001555 (Jersey)	0.001555 (Holstein) | (in code = 0.001555)
//
// Molly 8.5555 Feb 2012 Gil
//     FdRatMolly calculations (Fdrat_DairyNZ.csl) converted to MJ rather than kgDM. IntakeVersion = 8.5555
//     FdRatMolly (Fdrat_DairyNZ.csl): compensatory feeding introduced (cows under / over target weight request / more / less feed)
//     LHOR / MamCells code extracted to include files
//         Old code MamCells_MDH_*.csl
//         New code MamCells_DairyNZ_*.csl
//     Revised lactation model (MamCells_DairyNZ*.csl), around LHOR and MamCells, improves predictions on 1x,3x and feed restricted periods
//     iFdRat (FdRat_MDH_in_init) restored (was iFdRat = 10.0 for a while...)
//     iFdRat (FdRat_DairyNZ_in_init) now calculated using the same code as FdRat, to reduce LW jump on day1 due to FdRat jump.
//     Pregnancy code moved up in the init section, because FdRat_DairyNZ requires to know pregnancy in order to calculate iFdRat and FdRat
//     Recomended MamCellsPart settings:  2 * KgMilkSolidsExpectedIn270Days - 140
//
// Molly 8.4555  May 2011 Gil
//     Completely new FdRatMolly calculations replacing RFEED4 etc. IntakeVersion = 8.4555
//     MatureMamCellsPart and age adjustment removed. Age adjustment responsibility handed back to WFM / user.
//     MamCellsPart is again the constant to change for production.
//     KgMilkSolidsExpectedIn270Days introduced, represents the genetic merit in the new required intake calculations
//     Recomended MamCellsPart settings: 1.4 *KgMilkSolidsExpectedIn270Days - 140.
//
//
// Molly 8.4555  3rd March 2011 Gil
//     WtUter equation: DayMilk changed to MOD(DayMilk+3000.0,3000.0)
//     to correct the wrong assumption that when non preg DayMilk is positive
//     as DayMilk = -21 give WtUter = 465kg(!) (When dry cow gets pregnant)
//     WtUter should always be 0 - 10kg. The +1000.0 is only needed due to Acsl's
//     weird behaviour of the MOD function
//
// Molly 8.5 PG Feb 2011
//    Include ProximateExpand (init / deriv) csl files instead of classic_nutr
//    Many digestive constants changed
//
// Molly 8.4555 Gil Jan 2011
//    MatureMamCellsPart is now the initialization constant, while MamCellsPart is derived from it.
//    (adjsting for age)
//
//   FdRat_CCP_IN_Deriv.csl renamed to FdRat_DairyNZ.csl plus code revised.
//
//    kMamCellsQA initial value and use changed.
//
//    Revised formula for FMamCellsQA. For twice a day milking should yield NO change
//    it does yield changes for once and thrice a day. Too sensitive and needs further improvement.
//    (attempts to fix the very low sensitivity of previous version to milking frequency)
//
//    Pablo's Walking code added but NOT incorporated (because it disturbed the intake / energy expanditure too much, needs work)
//
//
// Molly 8.3 GL May 2010
//  Pregnancy:
//    made conceptus weight / gravid uterus dependent on cow age and breed
//    Added
//      WeightConcBreedFactor
//      WeightConcAgeFactor
//      iAgeinYears
//      ageInYears
//    WtGrvUter used to always grow to 91 kg regrdless of cow age / size
//    This seems to suit only mature American H cows.
//    Defaults have not changed so
//    For mature American Holstein LEAVE as it is iAgeinYears = 4.0 ; WeightConcBreedFactor = 1.0.
//    For mature NZ Frisian set WeightConcBreedFactor to 0.7555 to get final GU 67kg (DairyNZ data)
//    For mature NZ Jersey  set WeightConcBreedFactor to 0.4555 to get final GU 43kg (DairyNZ data)
//    For 2 years old change iAgeinYears. This will drive WeightConcAgeFactor to be
//   	87.5% (all breeds) for 2y olds (DairyNZ data)
//   	97.5% (all breeds) for 2y olds (DairyNZ data)
//    As the default age is now 4 default GU will remain unchanged (AgeFactor is calculated to 1.0 for
//    the defult age = 4, and breedFactor is initialized to 1.0))
//
// Molly 8.2 GL
//  Meal feeding:
//     (with JM) Meal feeding available (equal size equally spaced only).
//      Default is continous feeding with:
//         FDTM  = 1.0 !! Length in days of one meal
//         FDINT = 1.0 !! Length of interval between meals in days
//      for two meals of approx half an hour every 12 hours set
//         FDTM  = 0.02
//         FDINT = 0.5
//
//  Timers of Lactation and pregnancy:
//     Introduced new timer system for gestation and lactation.
//     Deleted DIM include files as the new system should cater for all users
//     Recomended to intialise only StartDayGest to the day of pregnancy.
//     Possible but not recomended to start with negative StartDIM instead.
//     Not recomended to start Molly in milk as results are way off the rsults of
//     an eqivalent Molly that starts before calving (Old problem).
//     By Default GestLength=283, DaysOpen=82, DaysDry = 82 so Molly will be
//        pregnant 283 days followed by DaysOpen = 82
//        lactating 283 days followed by DaysDry = 82
//     for control from the m file over conception / dry off use:
//        DryOffNow = 1.0 to dry her off on the spot (but make sure DaysDry is zero to not take effect
//        ConceiveNow = 1.0 to get her pregnant on the spot (but make sure DaysOpen is very big enough to not take effect)
//
// Molly5v
// Altered milk out to use a schedule statement so as to not miss milking times.
// Found that OtGutCont calc does not work properly with intermitten FdRat. Switched to
// 	intakeDay, 5-12-09, MDH
// Changed the representation of daylength to improve accuracy. 5-12-09, MDH
// Added an additional daylength calc that included twilight for use with the within day
// 	intake equation, 5-12-09, MDH
// Added another intake equation that will support within day simulation of intake patterns
// 	and feeding grain separately from forage, MDH 4-12-09
// Changed the representation of DayGest to allow simulation of an abortion by resetting DaysOpen
// 	and Preg in the middle of a run. MDH 4-16-09

// Molly5u
// Changed Nint to Nintake to avoid problem with reserved keyword in Xtreme, 3/13/09
// Found that kMinH will assume negative values when udder fill exceeds MilkMax.
// 	Milk production is also a proplem with periodic milking as dmilk is
// 	directly affected by kminh and thus changes over the course of a day.
// 	Made a pool for kMinH to dampen the effect on active cells.  This
// 	maintains responsiveness to udder fill but removes hour to hour variation.
// 	Milk yield is now represented by dMilkProd.  Apr 23, 2008 MDH
// Fixed problems with DIM and GestDay calculations to allow cycling through
// 	multiple lactations. Apr. 21, 2008 MDH
// Revised and added to the Intake Prediction Equations
// 	NRC is the default dry cow equation with no other choices at this time.
// 	IntakeEqn=1, Roseler et al while lactating.  Apr 21, 2008

// Molly5t
// Changed the Include statement for Ingredient inputs to revised scheme in 5t files.
// Added vectors for experimental bias for passage of DM, OM, Ha, NDF, ADF, Lipid
// 	MiNP, NitP, and NaNMNP, 12-23-07, MDH

// 	Checking_Stability_Nutr_Inputs_CCP_In_Init.csl and
// 	Checking_Stability_Nutr_Inputs_CCP_In_Deriv.csl
// 	because cAc and cGl not calculated in Initial section. CCP 8-29-07

// Molly5s2
// Added code so that ME in calories can be converted to ME in Joules, CCP 8-20-07

// Molly5s1
// Altered BCS conversions, see Molly3 bk p. 14, CCP 7-26-07
// Discovered that Mark's BW includes WtGrvUter. Therefore use BW now, and remove LW
// 	from code, CCP 7-25-07

// Molly5s - Mark's latest version
// Introduced INCLUDE files DIM_Gest_CCP_In_Init.csl, DIM_Gest_CCP_In_Deriv.csl,
// 	Includes code to calculate WtUter when cow does not conceive, CCP 5-30-07
// iBW calculation from iLW and WtGrvUter added, CCP 5-23-07
// Changes to align Dexcel's Molly with Mark's 5s:
// 	Comment re Molly5q4 and 5q6 changed, CCP 5-14-07
// 	Parameter changes, see Molly5s.cmd_, CCP 5-22-07:
// 		K1MamCells=0.009
// 		K2MamCells=0.4555
// 		uTMamCells 0.03 to 0
// 		MAMCELLSF(2)=180.3555, sent by WFM
// 		LAMBDAMAMCELLS 0.002555 to 0.002555
// 		LAMBDAMAMCELLSF(2)=-0.0009555, sent by WFM
// 		PMAMENZCELL 11.8555 to 12.4555
// 		VMGLLMVISPART 0.001555 to 0.001555
// 		KVMGLLMDECAY 0.1555 to 0.1555
// 		KVMGLLMDEG 0.0003555 to 0.0003555
// 		KVMGLLMSYN 0.03555 to 0.04555
// 		VMAAPMVIS 0.002555 to 0.002555
// 		VMFATSADIP 0.4555 to 0.3555
// 		KGLCD 0.01555 to 0.01555
// 		KBLDURU 2162.5555 to 2133.5555
// 		XMAMENZLHOR 0.5555 to 0.6555
// 		KDAYLENGTH 0.2555 to 0.1555
// 		LHORPPF(2)=0.1555, sent by WFM
// 		KBASOTH 2.2555 to 2.1555
// 		KAHORGL 3.0e-3 to 0.003555
// 		THETA2 2.0 to 5.9555
// 		KCHOR1GL 3.0e-3 to 0.003555
// 		THETA4 1.0 to 4.1555
// 	Mark's code for DayGest & DIM not used, since these are sent by Wfm.
// 	LW not equal to BW now - removed, CCP 5-22-07
// Removed the following INCLUDE files and incorporated them into the main code:
// 	DIM_CCP_In_Init.csl, InitCond_CCP.csl, Genotype_Effect_CCP.csl,
// 	Check_iBW_iBCS_CCP.csl, DIM_CCP_In_Deriv.csl,MamEnz_CCP.csl, MUN_CCP.csl. CCP 5-22-07
//
// Molly5s
// Added vectors to hold experimental bias adjustement factors for CP, Fat, NDF,
// 	ADF, and Starch, 12-18-07, MDH
// Added MUN calculations. 4-14-07
// Changed PUN to BldUr to avoid confusion with true PUN needed to calculated MUN
// Gl, Ac, Aa, Fa, and Ur volume factors are flaky with GrvUter as they run off of
// 	EBW which changes dramatically the day of calving.  Changed these factors
// 	to use NonUterEBW which resolved the problem. MDH 4-14-07
// Added gravid uterus calculations based on the Ferrell model. MDH 4-5-07

// Molly5r
// Altered representation of AHor, AHor1, CHor, and CHor1 such that each has its
// 	own rate constant and changed Theta on CHor such that Theta4 is used for both
// 	CHor and CHor1 and Theta3 is used for Ahor1.  Now Theta 2 and 3 increase anabolic
// 	responses and Theta4 increases catabolic responses. 11-22-06 MDH

// Molly5q15
// Added 5q12 changes, CCP 5-10-07
// Some 'pruning' done, CCP 5-10-07
// Included check that iBW, iBCS don't give iWtAdip <= 0, see 'Check_iBW_iBCS_CCP.csl', CCP 5-10-07
// Altered code so that iFdRat not equal to 1e-8, and therefore gives a more realistic
// 	value for iRumVol, which in turn reduces the jump in BW between t=0 and t=1.
// 	See FdRat_CCP_In_Deriv.csl and FdRat_CCP_In_Deriv.csl, CCP 5-9-07

// Molly5q13
// MetabPP calculated incorrectly. See Marks's email 'MetabPP, 6/4/07'. CCP 4-11-07

// Molly5q12
// Mun added to code as an INCLUDE, MUN_CCP.csl. See p 7 of N notebk.
// 	Relevant parameters/constants not refitted though. How am I going to do this?
// 	Need to consult with Mark. CCP 3-28-07

// Molly5q11
// PunVol changed to VolPun to be consistent with naming of other metabolites.
// 	VolPun for T > 1.0e-6 changed to match VolPun for T <= 1.0e-6.
// 	cPun changed from (Pun/PunCor)/VolPun to Pun/VolPun, CCP 2-5-07
// Parameter changes, see emails: 	'Copy of <RE: Ruminal pH>' 1-10-07,
// 						'RE: parameters and manuscript' 1-12-07
// 		K2MamCells 0.04555 to 0.4555
// 		K1MamCells 0.01 to 0.009
// 		MamCellsF 162.9555 to 163.3555 for NA, see Genotype_Effect_CCP.csl, sent by WFM
// 		lambdaMamCells 0.002555 to 0.002555
// 		lambdaMamCellsF -0.0008 to -0.0007555 for NA, see Genotype_Effect_CCP.csl, sent by WFM
// 		uTMamCells 2.8555e-08 to 0.03
// 		PMamEnzCell 12.5555 to 11.8555
// 		VmGlLmVisPart 0.001555 to 0.001555
// 		kVmGlLmDecay 0.1555 to 0.1555
// 		kVmGlLmDeg 0.0003555 to 0.0003555
// 		kVmGlLmSyn 0.03555 to 0.03555
// 		VmAaPmVis 0.002555 to 0.002555
// 		VmFaTsAdip 0.4555 to 0.4555
// 		KGlCd 0.01555 to 0.01555
// 		KPunU 2162.7 to 2162.5555
// 		xMamEnzLHor 0.5555 to 0.5555
// 		KDayLength 0.2555 to 0.2555
// 		LHorPPF 0.1555 to 0.1555 for NA, see Genotype_Effect_CCP.csl, sent by WFM
// 		KbasOth 2.2555 to 2.2555
// 	CCP 1-12-07

// Molly5q10
// KCeCs, KHcCs restored as f(RumpH) on Mark's advice (email: 'RE: ME of feeds + EL runs',
// 	12-13-06). Need to revisit this issue.
// KCeCs and KHcCs fixed at 61.7555, 25.6555 regardless of RumpH, CCP 12-12-06

// Molly5q9
// Molly not recognising MAMCELLSF(1), LAMBDAMAMCELLSF(1), LHORPPF(1).
// 	Changed to MAMCELLSF, LAMBDAMAMCELLSF, LHORPPF respectively.
// 	See Genotype_Effect_CCP.csl also. CCP 12-11-06

// Molly5q8
// DIM, sign (AX reserved words) replaced by DayMilk and signe respectively,
// 	just to be safe. See:
// 	Molly5q8.csl, DIM_CCP_In_Init.csl, FdRat_CCP_In_Init.csl,
// 	DIM_CCP_In_Deriv.csl, FdRat_CCP_In_Deriv.csl, MamEnz_CCP.csl.
// 	CCP 11-29-06
// MamCells eqn changed as per Mark's email, 11-23-06,
// 	'RE: MamCells Calc and Initialising lactation', CCP 11-27-06.
// Provision added so that Molly does not crash when feed intake
// 	<= 0, see FdRat_CCP_In_Init.csl, FdRat_CCP_In_Deriv.csl,
// 	CCP 11-10-06

// Molly5q7
// Calculation of MamCellsF(1), lambdaMamCellsF(1), LHorPPF(1) changed
// 	for Wfm, incorporated as an Include, see 'Genotype_Effect_CCP.csl', CCP 9-29-06
// numVersion changed to version for Wfm, CCP 9-29-06
// Incorporated 'Checking_Stability_Nutr_Inputs_CCP_5q7.csl' as an Include.
// 	Besides checking for feed nutrient values < 0, and for cGl, cAc
// 	< 0, it has code to trap and correct div/0 for the 5 feed
// 	nutrients where this could be a problem, CCP 9-18-06
// For Classic and Proximate nutrient schemes kPiAa, kHaCs, KCeCs1, KHcCs1 constants.
// 	For Ingredient nutrient scheme (Mark uses this), they are variables. CCP 9-18-06
// 	As suggested by Mark, see his email 'RE: cmd file', 9-20-06, 10.02 am, I take
// 	means from ExtLact5p Inputs Printout. Need to revisit this issue.
// Forset=0, mixset=1, conset=0 for all feeds in WFM, CCP 9-25-06.
// 	See Email 'MixSet, etc', 25/09/2006, 01.5555 p.m.
// 	Need to revisit this issue, see Email 'RE: cmd file', 15/09/2006, 06.4555 a.m.
// 'INCLUDED' files introduced, CCP 9-14-06:
// 	Initial: DIM_CCP_In_Init.csl, InitCond_CCP.csl, Genotype_Effect_CCP.csl,
// 	Molly_Classic_Nutr_CCP.csl, FdRat_CCP_In_Init.csl, Checking_Stability_Nutr_Inputs_CCP.csl
// 	Derivative: DIM_CCP_In_Deriv.csl, FdRat_CCP_In_Deriv.csl, MamEnz_CCP.csl
// 	Dynamic: Checking_Stability_Nutr_Inputs_CCP.csl
// Following Constant changes made as per Molly5q3.cmd_, Sept 3, CCP 9-13-06
// 	IALG 5 to 8
// 	MAXT 0.006555 to 0.01
// 	OthDnaMx 0.1555 to 0.09
// 	VisDnaMx 0.1555 to 0.09
// 	FHcCs1 0.6555 to 0.4555, see Molly_Ingredient_In_Deriv.csl
// 	KFatPi 0.03 to 0.0007555
// 	KAmabs 12.4 to 23.06555
// 	KabsAc 10.5 to 6.2555
// 	KabsBu 10.5 to 10.3555
// 	KabsPr 10.5 to 11.6555
// 	FKRuAdf 10.0 to 59.7555, see Molly_Ingredient_In_Init.csl
// 	FKRuP 0 to 3.6555, see Molly_Ingredient_In_Init.csl
// 	FKRuSt 5.0 to 3.01555, see Molly_Ingredient_In_Init.csl
// 	LgutDCFa 0.9 to 0.6555
// 	LgutDCHa 0.7 to 0.8555
// 	LgutDCHb 0.1 to 0.1555
// 	LgutDCPi 0.4 to 0.6555
// 	kMamCellsQA 0.7555 to 0.3
// 	LHorCor 10 to 20
// 	BCSTarget 2.5 to 3.0
// 	FMamCellsQACr 0.5 to 1
// 	K2MamCells 0.1555 to 0.04555
// 	xLHorSensAdip 4.0 to 2.9555
// 	VmAaPOthOth 300 to 223.2555
// 	iPMamCellsA 0.5 to 0.7555
// 	K1MamCells 0.009 to 0.01
// 	fLm 0.04555 to 0.04555
// 	KbasVis 3.5 to 8.8555
// 	VmTsFaAdip 0.1 to 0.06555
// 	MamCellsPart 1000 to 792
// Following 17 parameters set from Molly5q3_MamCells_Pub_Summary.xls,
// 	Solution 2 (see email RE: cmd file, 15/09/2006, 06.4555 a.m.)
// 	CCP 9-15-06
// 	MamCellsF(2) 0 for NZ, 162.9555 for NA
// 	lambdaMamCells 0.002 to 0.002555
// 	lambdaMamCellsF(2) 0 for NZ, -0.0008 for NA
// 	uTMamCells 0.03 to 2.8555e-08
// 	PMamEnzCell 10 to 12.5555
// 	VmGlLmVisPart 0.002555 to 0.001555
// 	kVmGlLmDecay 0.03 to 0.1555
// 	kVmGlLmDeg 0.0005 to 0.0003555
// 	kVmGlLmSyn 0.005 to 0.03555
// 	VmAaPmVis 2.5E-3 to 0.002555
// 	VmFaTsAdip 0.1555 to 0.4555
// 	KGlCd 20.0E-3 to 0.01555
// 	KPunU 857.1 to 2162.7
// 	xMamEnzLHor 1.0 to 0.5555
// 	KDayLength 0.0 to 0.2555
// 	LHorPPF(2) 0 for NZ, 0.1555 for NA
// 	KbasOth 2.6555 to 2.2555
// FeedInFlag introduced to enable FdRat input from WFM.
// 	See FdRat_CCP_In_Init.csl, FdRat_CCP_In_Deriv.csl, CCP 9-8-06
// If cows dry for prolonged periods (iMamLmF or MamLm = 0.00001), then
// 	dMilk becomes very small and eventually is treated as zero. Effects
// 	propLm, fPm, fTm1 because dMilk is a divisor in their eqns.
// 	iMamLmF/MamLm set to zero for dry cow and dMilk = dMilk + 1e-8 in
// 	the above eqns. Minimal effect on previous work, CCP 9-1-06
// Added code so that program stops if feed fractions < 0.
// 	See 'Checking_Stability_Nutr_Inputs_CCP.csl'. No effect on previous work, CCP 9-1-06
// Added code so that program stops if cGl or cAc <= 0.
// 	See 'Checking_Stability_Nutr_Inputs_CCP.csl'
// 	No effect on previous work, CCP 9-1-06
// Code added by Robin McDougall, AX support, for debugging.
// 	No effect on previous work, CCP 9-1-06
// iBCS added to the 'initialisation' Procedural in a similar way to iDMilk.
// 	Means that first BCS is the observed, CCP 9-1-06
// Daily and total milk yield in litres also, CCP 9-1-06
// BCSTarget = 5.0 NZ scale (Holmes et al, 2002), which is equivalent to US 3.1.
// 	Will not effect previous work since BCSTarget then = 3.0 US, CCP 9-1-06
// iBCS, BCSTarget now given in NZ scale, CCP 9-1-06
// RumBuCor instead of RumPrCor in eqn for iRumBu. Both set to 1 so no problems
// 	with previous work, CCP 8-31-06
// if(GlobalDMIEqn.eq.0) changed from if(GlobalDMIEqn.ne.0).
// 	No effect on previous work, CCP 8-31-06

// Molly5q6
// Extracted the several feed input schemes to external files and use an INCLUDE
// statement to pull the desired code in. This greatly simplified the code.
// 8-23-06, MDH

// Molly5q4
// Extracted the several feed input schemes to external files to simplify the code.
// Currently need to cut and paste the sections in. This version has the
// nutrient input scheme. 8-23-06, MDH

// Molly5q3
// Need to add various BCS prediction equations to be consistent with NZ system.
// 	Also need to add gravid uterus equations for weight and nutrient expenditure.
// Fixed inappropriate use of breed factor on KLHorPP to affect KDaylength, 8-18-06
// Updated Methane equations to correct an error that was fixed 2/2/95
// 	by KC which was after I got my version of Molly.
// Fixed some MiCor and LPartCor factors.  Both set to 1 so no problems
// 	with previous work, 8-18-06 per CCP
// Changed the effects of LHor from mammary cell cycling to Penzcell

// Molly5q
// Added photoperiod effects

// Molly5p
// Changed MamCellsF from a multiplier to an addend, MDH 4-24-06
// Added MamCellsF(5) as an expt effect to MamCellsPart, MDH 4-21-06
// Altered the representation of the Vm for lactose such that it is representated
// using Dijkstras milk curve equation.  This allows for lesser lactose yield relative
// to protein and fat in early and late lactation and more at peak lactation.
// Changed kMinH effects from the individual milk component enzymes to the
// active and quiescent proportion fluxes.
// MDH 3-6-06

// Molly5o4
// Altered MamCells representation such that total MamCells is calculated from
// the equation of Dijkstra et al., the proportion of active cells is calculated
// using the approach of Vetharaniam et al. and Enzyme activity per cell is as
// originally represented. 2-27-06

// Molly5o3
// Removed grass, legume, corn silage scheme for setting kCeCs and kHcCs.  Forgot to
// 	remove and thus it was over-riding the calculation using RuAdf, 9-20-04.
// Added intercepts to rate constant calcs for ruminal degradation of Pi, Ha, and Ce
// 	and made ruminal retention times a constant to allow fitting. 9-19-04
// Added vectors to allow application of fixed effects for laboratory, 9-16-04

// Molly5o2
// Made the mean ruminal retention time and intercept for kPiAa calc constants so
// they could be fitted to data. 9-16-04
// Changed CP content of urea from 285 to 292 per J. Fadel comment, 9-2-04
// Added the code to support ingredient inputs. 7-12-04

// Molly5o1
// Added the newer Event array, fixed an error in calculating Nintake, removed duplicate
// iMilkAve definitions, removed EmptyEventAnimal reference, changed InitCond to the
// newer 5 element size, and added StartDIM code. 7-9-04

// Changed fNnFd to represent Nucleic Acids plus nonurea NPN as originally
// intended. MDH, 3/7/2000

// Added Parity and enabled discrimination by parity in DMI predictions
// and in MamCells and KLHor. MDH, 3-2-00

// Increased array size for events to allow 50 events.  MDH 11-16-99

// This version has new calculations for initial conditions
// including rumen DM, liquid and total volume, EBW and
// partitioning of EBW to adipose(Adip), lean body mass(Oth),
// and visceral(Vis) fractions.   8/99   NES

// Coding revisions for consistency and enhanced understanding,
// as described below, initiated on 5/25/99.  NES

// This is an aggregated version of the 550 kg cow described by
// Smith(1970) and Baldwin and Smith(JDS 54:583(1971)).
// This version was developed to simulate overall energy transactions,
// within day patterns of nutrient use, longer term day to day patterns
// of nutrient use throughout a lactation or growth cycle and
// as an aid in the design of energy balance experiments.
//  The body weight of the cow is 550 kg.  Her empty body weight(EBW)
// is 500 kg. Lean body mass (Oth) is 350 kg and includes skin, brain,
// kidney,muscle,skeleton and minor tissues. Adipose tissue (Adip) is
// 75 kg and is comprised of triacylglyceride(Ts,60kg) and cytoplasmic
// elements (wtcytAdip=15kg). Visceral weight (wtVis) is 75kg and includes
// blood, gut, liver, heart and mammary. Note that wtOth and wtAdip would
// be expected to decrease in early lactation while wtVis would increase.
//   Nutrient inputs were calculated for a 50:50 forage/concentrate
// ration and are continuous in the reference (default) state. Milk
// production is 30 kg/day and energy balance is zero. Milk has
// 3.5, 4.8, and 3.3% and 41.2, 23.4 and 22.4 MJ, respectively,
// of fat (expressed as tripalmitin and equivalent to 3.7% of true
// milk fat),lactose and protein.  Input/output comments prior to
// each subsection of the model refer to this specific
// feeding/production condition.

// This version has provisions which accommodate
// 1.  Different feeding frequencies e.g. one or more meals
//   per day or continuous feeding.
// 2.  A wide range of diets specified as input by users
// 3.  Alternative feeding strategies including specification of
//   amounts of feed offered and conditional changes in
//   feedstuffs offered.
// 4.  Provisions for abomasal infusion of casein, BST, T3,
//   and insulin administration.

// The default for this version is set for mid-lactation within day
// simulations.  For long term simulation runs overlays
// to increase pool sizes and reset mammary parameters are required.

// Explanation of coding, 5/25/99
// 1.Most entities described with 2 to 5 letter codes with first
// letter in caps and rest small.
// 2.Compartments:
// Rum	 Rumen	   Abo    Abomasum	   Milk Milk
// IntSm Sm intestine  IntLrg Large intestine  Fec Fecal
// Vis	 Viscera	   Adip   Adipose		   Mam Mammary
// Liv	 Liver         Git    Gastrointestine  Hrt Heart
// Bld	 Blood	   Mus    Muscle
// Oth Other(lean body mass)
// 3.Factors, initial conditions, rate constants, etc.:
// Cor	Correction factors	Hcomb	 Heat of combustion
// c	Concentrations		Vol	 Volume
// d	Differential equation	Vm	 Vmax(metabolic capacity)
// i	Initial condition		K	 Rate or affinity constant
// Mwt	Molecular weight		F	 Adjust/scaling factor(Bwt,etc)
// Ave	Average			f	 Fraction (eg. kg/kg, etc.)
// 4.Metabolites, nutrients, etc.:
//
// 5.Other coding:
// 	Liq	Liquid		Hor	Hormone		Enz	Enzyme
// END ! doc

// ******************************************************************
// 			Dynamic Section
// ******************************************************************

// Added by Robin McDougall, AX support, for debugging. CCP 7-20-06

// end of program // OF PROGRAM

class molly {

private:

	// specify number of variables
	static constexpr int n_state_variables = 66 ;
	static constexpr int n_visible_variables = 1778 ;
	
	// declare state_type and t
	typedef std::array< double , n_state_variables > state_type;
	double t;
	
	// event list
	std::multimap< double , std::string > event_list;
	
	// add event
	void schedule( double event_time, std::string event_name ){
	
		event_list.insert( std::make_pair( event_time , event_name ) );
	
	}
	
	// declare boost::odeint stepper
	typedef boost::numeric::odeint::runge_kutta4< state_type > stepper_type;
	stepper_type stepper;
	
	// declare model variables
	double version ; // constant version = 8.9555
	double numVersion ; // constant numVersion = 5.2555
	double MyPi ; // constant MyPi = 3.1555
	double MwtSc , MwtOa , MwtPe , MwtSt ;
	double MwtLiFd , MwtFl , MwtGy , MwtHc ;
	double MwtCe , MwtPs , MwtPi , MwtNn ;
	double MwtAc , MwtPr , MwtBu , MwtVa ;
	double MwtCh , MwtCs , MwtRumAa , MwtUr ;
	double MwtAm , MwtLa , MwtAs , MwtFa ;
	double MwtFaFd , MwtCH4 , MwtPOth , MwtN ;
	double MwtAa , MwtTs , MwtTm , MwtLm ;
	double MwtPVis ;
	double F1 ;
	double test ;
	double HcombCH4 ;
	double HcombAc ;
	double HcombPr ;
	double HcombBu ;
	double HcombGl ;
	double HcombGy ;
	double HcombFl ; // HcombFl is value for stearate
	double HcombFa ; // HcombFa=2.3555 is value for palmitate
	double HcombCs ;
	double HcombHc ;
	double HcombPs ;
	double HcombNn ;
	double HcombCh ;
	double HcombUr ;
	double HcombOa ;
	double HcombLiFd ;
	double HcombLa ;
	double HcombTg ;
	double HcombLm ;
	double HcombAa ;
	double HcombTp ;
	double HcombMi ;
	double HcombLg ;
	double HcombMiLi ;
	double MatBW ;
	double CsCor , RumAaCor , AmCor , HaCor ; // unit: unitless
	double HbCor , RumAcCor , RumPrCor , RumBuCor , RumLaCor ; // unit: unitless
	double LPartCor ; // unit: unitless
	double MICor , PICor , IndigFdCor , ASCor ; // unit: unitless
	double FLCor , MiHaCor , MIHbCor ; // unit: unitless
	double FaCor , AcCor , AaCor , BldUrCor ; // unit: unitless GlCor=10.0 moved to Constants.csl (Gil, Apr 14) to enable Mindy to overwrite it to 7.0
	double HcCor , CeCor ; // unit: unitless
	double GlCor ;
	int CurrentEvent , PreviousEvent ;
	int i , j , jj ;
	int GlobalDMIEqn ;
	int Expt ;
	bool logCriteria1 , logNewEvent , ilogNewEvent ;
	static constexpr auto MaxHerds = 1 ;
	static constexpr int MaxInitValues = 5 ; // must be 5 for the current structure of the initCond Array
	static constexpr int MaxEvents = 20 ; // must be 11 to satisfy default initialisation in proximate_expand_in_init.csl
	static constexpr int MaxAnimals = 2 ; // must be at least 2 to satisfy default initialisation of InitCond array 30 lines below
	double McalToMJ ;
	double dMilkVol ;
	double dMilkProd ;
	double MilkProdDiel ;
	double dLmProd ;
	double dPmProd ;
	double dTmProd ;
	double TVolMilk ;
	double TVolMilkYest ;
	double dNep ;
	double tNep ;
	double tNepYest ;
	double MastJawMoveBolus ;
	double Rest ;
	double STFLAG ;
	double IntakeYest ; // so the DailySummary , that is now scheduled on T = 0 as well, will do IntakeDay=IntakeTotal-IntakeYest properly
	double TransitSW ;
	double FdDMIn ;
	double DrnkWa ;
	double DrnkWaTot ;
	double DrnkWaDiel ;
	double DrnkWaYest ;
	double DrinkSW ;
	double WaUrineYest ;
	double UrinationCount ;
	double UrinationCountYest ;
	double UrinationCountDiel ;
	double UrinationVol ;
	double UrinationVolYest ;
	double UrinationVolDiel ;
	double iKHaCs , refIngrKHaCs ;
	double iKCeCs , iKHcCs , refIngrKAdfDeg ;
	double iKPiAa , refIngrKPiAa ; // Reference value for in situ rate scaling
	double iAnimal ;
	int Animal ;
	double iDayOfYear ; // Day of the year
	double DayofYear ;
	double iParity ;
	int Parity ;
	double GestLength , DaysOpen , DaysDry ;
	double StartDayGest ;
	double StartDIM ;
	double ConceiveNow , DryOffNow , AbortPregNow ;
	double iAgeInYears ;
	double WtConcBreedFactor ; // 1.0 keeps MDH calibration. Use 0.7555 for New Zealand Frisian, 0.4555 for Jersey. DairyNZ data
	double BCSBase ; // US baseline, for DNZ lactaion model (LHOR effect). See also BCS target that varies by DayMilk
	double AgeInYears ;
	double WtConcAgeFactor ; // GL. This yields 0.8555 for 2 years old; 0.9555 for 3 & 1.0 for 4+ Based on DairyNZ data
	double iStartDayGest ;
	double iDayGestDMI ; // used for iFdRat calculations
	double iStartDIM ;
	double ConceiveNowVariable ;
	double DryOffNowVariable ;
	double AbortPregNowVariable ;
	double tAtConception ;
	double DayGestBasic ;
	double DayGest ; // This prevents undeflows in some preg equations while she is ompty.
	double tAtCalving ;
	double DayMilk ;
	double fHeifers ; // Used for determining udder capacity and DMI
	std::array< std::array< double , MaxInitValues > , MaxAnimals > InitCond ;
	int FirstEvent ;
	int LastEvent ;
	double iBW ;
	double iBCS ;
	double iHerd ;
	int Herd ;
	std::array< double , MaxHerds > MamCellsF ; // Array to hold Herd Adjustment factors for MamCellsPart
	std::array< double , MaxHerds > lambdaMamCellsF ; // Array to hold Herd Adjustment factors for MamCellsPart
	std::array< double , MaxHerds > LHorPPF ; // Array to hold Herd Adjustment factors for LHor PP
	double fPUter ; // g CP/g wet weight, mean from Ferrell et al., 1976
	double iWtUter , kUterSyn ;
	double kUterSynDecay , kUterDeg ;
	double iWtConc , kConcSyn , kConcSynDecay ;
	double iWtPConc , kPConcSyn , kPConcSynDecay ;
	double Preg ;
	double NonPreg ;
	double WtUterPart ;
	double WtUter ;
	double WtPUter ; // kg protein in uterus
	double PUter ;
	double WtConc ;
	double WtGrvUter ;
	double fEndogLiFd ; // Assumed that endogenous fat = 2.7555% of DM
	double fAiFdBase ; // Average proportion of ADF Ash. Gil Sep 2014 made it diurnally dynamic in Mindy
	double afAiFd ;
	double SecondsPerDay ; // 24 * 60 * 60
	static constexpr int MaxEventNutrients = 12 ;
	static constexpr auto MaxCalculatedNutrients = 3 ;
	static constexpr int MaxProtFrac = 5 ;
	static constexpr int MaxCHOFrac = 5 ;
	std::array< std::array< double , MaxEventNutrients > , MaxEvents > Event ;
	std::array< std::array< double , MaxCalculatedNutrients > , MaxEvents > Ingr ;
	std::array< std::array< double , MaxProtFrac > , MaxEvents > IngrProtComp ;
	std::array< std::array< double , MaxCHOFrac > , MaxEvents > IngrCHOComp ;
	std::array< std::array< double , 8 > , MaxEvents > SugarAndDmPercent ;
	int CurrentFeed , CurrentSupplement ;
	double Silage ;
	static constexpr double GrassNZ = 1 ;
	static constexpr auto GrassSilage = 2 ;
	static constexpr double MaizeSilage = 3 ;
	static constexpr double TMR = 4 ;
	static constexpr double SoybeanMeal = 5 ;
	static constexpr double CerealStraw = 6 ;
	static constexpr double MaizeGrain = 7 ;
	static constexpr double BarleyGrain = 8 ;
	static constexpr double Lucerne = 9 ;
	static constexpr double LucerneSilage = 10 ;
	static constexpr auto Clover = 11 ;
	static constexpr auto FeedName = 1 ;
	static constexpr int IngrDM = 2 ;
	static constexpr int IngrCP = 3 ;
	static constexpr int IngrFat = 4 ;
	static constexpr int IngrSt = 5 ;
	static constexpr int IngrNDF = 6 ;
	static constexpr int IngrADF = 7 ;
	static constexpr int IngrLg = 8 ;
	static constexpr int IngrAsh = 9 ;
	static constexpr int IngrFor = 10 ;
	static constexpr auto IngrPsf = 11 ;
	static constexpr auto IngrEaseOfBreakdown = 12 ;
	static constexpr auto dmSummerSunrisePos = 1 ;
	static constexpr auto dmSummerSunsetPos = 2 ;
	static constexpr auto dmWinterSunrisePos = 3 ;
	static constexpr auto dmWinterSunsetPos = 4 ;
	static constexpr auto scSummerSunrisePos = 5 ;
	static constexpr auto scSummerSunsetPos = 6 ;
	static constexpr auto scWinterSunrisePos = 7 ;
	static constexpr auto scWinterSunsetPos = 8 ;
	double NumberOfFeeds ;
	static constexpr auto IngrSc = 1 ; // Calculated in a complicated way, as 'the rest' after accounting for all the others..
	static constexpr auto IngrHc = 2 ; // Calculated as NDF - ADF
	static constexpr auto IngrPeNdf = 3 ; // Calculated as PSF * NDF
	static constexpr int IngrCPs = 2 ; // This includes NPN, all in CP equivalents
	static constexpr int IngrRUP = 3 ;
	static constexpr int IngrNPN = 4 ; // this is NPN-CP, i.e. CP equivalents and includes the urea
	static constexpr int IngrUr = 5 ; // This is also expressed as CP equivalents
	static constexpr int IngrStS = 2 ;
	static constexpr int IngrRUSt = 3 ;
	static constexpr int IngrRUAdf = 4 ;
	static constexpr int IngrForNDF = 5 ;
	double fAiFd ; // average insoluble Ash contents Gil Sep 2014 this is no longer a contsant
	double fCPFd ; // These are currently only defined for the Bate NRC data set
	double FCPsFd ; // includes NPN sources
	double FUrFd ; // IngrUr is in CP equivalents. Changed to urea mass here
	double FNPNFd ; // CP equivalents including that from urea
	double FRUPFd ;
	double FPsFd ;
	double fPiFd ;
	double fNnFd ; // Nn mass, % of DM, assumes a C to N ratio the same as CP which is correct for nucleic acids
	double FCFatFd ; // These are currently only defined for the Bate NRC data set
	double fLiFd ;
	double fFatFd ;
	double fAshFd ; // These are currently only defined for the Bate NRC data set
	double fAsFd ;
	double fNDFFd ; // These are currently only defined for the Bate NRC data set
	double fADFFd ; // These are currently only defined for the Bate NRC data set
	double fRuAdfFd ;
	double fLgFd ;
	double fHcFd ;
	double fCeFd ;
	double fOmFd ;
	double fStFd ; // These are currently only defined for the Bate NRC data set
	double FStsFd ;
	double fRUStFd ;
	double PartFd ;
	double fScFd ;
	double StSol ;
	double SpeciesFactor ;
	static constexpr int MaxFdScreens = 14 ; // Allow 14 screens, Unit: mm
	static constexpr auto fdSlots = 15 ; // Name + 14 screens
	std::array< double , 15 > FdBinMeshSize ; // Unit: mm Standard bins min values (As described in 2.2.1 Gregorini 2014)
	std::array< double , MaxFdScreens > fBinFd ; // Unit: fraction (all the array elements sum up to 1.0). Proprtions of particles in each size range.
	std::array< double , MaxFdScreens > fBinFd1 ; // Unit: fraction (all the array elements sum up to 1.0). Proprtions of particles in each size range. working array for the chewing model
	std::array< std::array< double , fdSlots > , MaxEvents > iBinFd ; // Unit: percent (all the array elements sum up to 100.0). Percentages of particles in each size range. Table of all feeds before ingestion
	double LPartSize ; // Min size of the ruminal LPart pool, Unit: mm
	double MPartSize ; // Min size of the ruminal MPart pool, Unit: mm
	double CappingForIntake ; // NOT USED BY DO NOT DELETE. OVERWRITTEN BY breedAndAge.csv setting. We nneed it here so it enters the automatically generate cowParameters file, to become optimsable. Used in Smalltalk / WFM
	double IntakeVersion ; // Gil's required intake model April 2011 replacing RFEED4 etc.
	double GrowthPerDay ; // WFM sets it differently for 2 - 5 years old, breed & age dependent.
	double OnceADayMilkingAdjustment ; // DairyNZ observed ratio of 1x production vs 2x whole season. This will affect EnergyForMilk, while the cow is on MF other than twice a day
	double OnceADay2YearsOldAdjustment ; // DairyNZ Observed (Friesian) 2 years old production vs mature on once a day. (For Jersey this is set to 0.9555)
	double IntakeDeclineSlope ; // Decline in intake after DIM 110, as proportion of MaxFoodForMilk. WFM sets it diffrently for Jerseys and once-a-day cows
	double EnergyForDryCowMaintenancePower ; // Edith's parametrisation March 2012. Multiplies NonUterEbwTarget^0.7 gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively
	double EnergyForDryCowMaintenanceFactor ; // Edith's parametrisation March 2012. Multiplies NonUterEbwTarget^p gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively (p = EnergyForMaintenancePower)
	double EnergyForMilkingCowMaintenancePower ; // Multiplies NonUterEbwTarget^0.7 gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively
	double EnergyForMilkingCowMaintenanceFactor ; // Multiplies NonUterEbwTarget^p gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively (p = EnergyForMaintenancePower)
	double EnergyForMilkFactor ; // Multiplier of (MilkSolids270 + MilkSolids270a) / 2
	double EnergyForMilkPower ; // WFM sets it to 4.5 for Jerseys and once-a-day cows
	double EnergyForPregnancyFactor ; // Edith's parametrisation March 2012. Multiplier of WtGrUter
	double PeakIntakeDay ; // WFM sets it to 108 for Jerseys and once-a-day cows
	double kEnergyCompensation ; // 0.5 will roughly give 1% extra / less required intake for every 2% LW difference (reverse relationship! low conditin cows eat more!)
	double xOadIntakeTadIntake ; // OAD (once a day) cows eat more compared to their production. This parameter describes how close their requirement would to twice a day situation. If set to 1, they will require the same (in spite of their lower production) if set to 0, they will require less energy for milk in direct proprtion to their reduced production. Middle values will reflect the reality in which they eat more compared to their production, and thus gain condition.
	double SmoothingPeriodDays ; // Number of days after PeakIntakeDay, in which the transitional curve takes effect
	double FeedInFlag ; // Set to 1.0 for external feeding (m file / WFM)
	double FdRatWFM ; // Set to desired daily intake external feeding (m file / WFM)
	double MaxEnergyForMilk ; // Eneregy for milk at peak intake
	double iFdRat ;
	double NonUterEBW ;
	double NonUterEbwTarget ;
	double EnergyForActivity ; // (ActEnergyReq / MjoulesToATPConv / MEinMJ) / (0.02 * (17.0 * (MEinMJ/3.6/4.1555) - 2.0) + 0.5)
	double EnergyForPregnancy ; //
	double EnergyForGrowth ; // Assuming 50MJ required for 1kg LW gain
	double EnergyCompensation ; // Any extra/deficit in condition would decrease/increase intake, trying to simulate the extra hunger of low condition cows, and lower insentive to eat for fat cows
	double EnergyForMaintenance ;
	double EnergyForMilk ;
	double RequiredEnergy ; // Molly's required intake, in MJ
	double FdCapMolly ; // in kgDM actual, WFM will not feed her more forage (pasture & silage) then this amount, but may feed things like grains on top of that
	double FdRat ; // Allocated feed - m files or WFM
	double fd1 ;
	double fd2 ;
	double fd3 ;
	double EnergyCompenstaion ;
	double iNonUterEbwTarget ;
	double iNonUterEBW ;
	double IntakeDay ;
	int iTotMeals , TotMeals , TotMealsYest ;
	double TNdfIn ;
	double TNdfInYest ;
	double iGlF , iFaF , iAcF , iAaF ;
	double iVolGlF , iVolFaF , iVolAcF , iVolAaF ;
	double iHaF , iHcF , iCeF ;
	double iPiF , iIndigFdF ; // Reduced iIndigFdF to achieve model balance, MDH 5-28-13
	double iHbF ;
	double iLPartF ; // LPartF and MPartF reflect Shaver et al 1988 data
	double ifMPartRum , ifSPartRum ;
	double iMPartF ;
	double iSPartF ;
	double iAsF , iAmF , iMiF , iFlF , iCsF ;
	double iRumAcF , iRumPrF , iRumBuF , iRumAaF ;
	double iMiHaF , iMiHbF , iBldUrF , iVolBldUrF ;
	double iRumLaF ;
	double iOthDnaF , iVisDnaF ;
	double kInitRumVol ; // Exponetial deacy constant. When this k is set to 50 cows will be initialized as follows: (iFdRat,iRumVol) (10,64) (20,90)
	double MaxRumVol ; // Unit: kg. Average of lit references is 3.9 * FdRat but is higher at DMI<20 kg. NES 10/99. 0.05 is FdRat = 5% of bodyweight. Gil
	double RumVol ; // Unit: kg. Gil Sep 2014 MaxRumVol acheived at FdRat=5% of IBW, but smaller rations no longer give lineraly declining RumVol
	double RumDM ;
	double iRumLiqVol ;
	double RumLiqVol ;
	double iotGutCont ;
	double iEBW ; // Blood Vol should be removed from this?
	double iWtAdip ;
	double iWtCytAdip ;
	double iWtTsAdip ;
	double iNonFatEBW ;
	double iNonFatNonUterEBW ;
	double iWtOth ;
	double iWtVis ;
	double BWF ; // Changed to reflect conceptus space outside of maternal space, 4-14-07
	double iotWtOth ;
	double iotWtVis ;
	double ifDWt ;
	double iPOth ;
	double iPVis ;
	double iTsAdip ;
	double iabsEAve ; // jk 04/26/91
	double iGl ;
	double iVolGl ;
	double iFa ;
	double iVolFa ;
	double iAc ;
	double iVolAc ;
	double iAa ;
	double iVolAa ;
	double iBldUr ;
	double iVolBldUr ;
	double iHa ;
	double iLPart ;
	double iMPart ;
	double iHc ;
	double iCe ;
	double iHb ;
	double iPi ;
	double iIndigFd ;
	double iSPart ;
	double iRumLa ;
	double iCs ;
	double iAs ;
	double iAm ;
	double iMi ;
	double iFl ;
	double iRumAc ;
	double iRumPr ;
	double iRumBu ; // Changed from RumPrCor. CCP 8-31-06
	double iRumAa ;
	double iMiHa ;
	double iMiHb ;
	double iOthDna ; // ??This and iVisDna should use starting pools for Oth and Vis
	double iVisDna ; // to calculate iDna
	double iVmAcTs ;
	double SolDM ;
	double TotWaUrineLast ; // Accumulated urine water at the last urinartion event
	double TotNurLast ; // Accumulated urine N at the last urinartion event
	double BladderVol ; // Momentary water volume of the bladder contents.
	double NurConcentration ; // Concentration of N of the last discrete urination excretion
	double THETA5 , TAveMilkMam , MamMilkCor ;
	double iLHorF , iMamMilkAveF , iMamTmF ;
	double LHorBase ; // Used to be LhorCor
	double iLHor ;
	double iMamLmF , iMamPmF , iMilkAve , TAveMilk ;
	double CumulativeLowMfDays ;
	double DailyMfDiff ;
	double InMilk ;
	double iMamCells ;
	double iMamCellsA ;
	double iMamCellsQ ;
	double iMamCellsS ;
	double dWtAdipNew ;
	double BcsTarget ;
	double dLwExclUterGutAndGrowth ;
	double MilkingFrequencyLag ;
	double derivMilkingFrequencyLag ;
	double iRumVol ; // Only mamcells of DNZ needs iRumVol, so I put it here to minimise diff - Gil
	double BST , T3 , INS ;
	double kRetMilkI ;
	double MilkMax ;
	double KMilkI ;
	double iMamMilkAve ;
	double iMamTm ;
	double iMamPm ;
	double iMamLm ;
	double TotEatingYest ;
	double TotRumntnYest ;
	double TotRestYest ;
	double dEating ;
	double dRumntn ;
	double dRest ;
	double MilkInt , PResidMamMilk ;
	int MilkingIndex ;
	static constexpr int MaxMilkingPerDay = 4 ; // Can have variable MilkingFrequency up to this Max
	std::array< double , MaxMilkingPerDay > MilkingHours ; // Can be increased if desired maximal milking frequency is above 2
	double MilkingFrequency ;
	double NextMilkingT ;
	double MilkSW ;
	double EatingSupplementsSW ;
	double SupplementOnOffer ; // Basic Molly does not always eats entry 1 in the feed table, which is changes daily bexternally to reflect the daily diet (supps and grass mixed together to one feed)
	double HMM ;
	double RUMNTNEQ ;
	double WaPool ;
	double WaPoolTarget ;
	double EatingSW ;
	double AcquisitionJawMovesCurrent ;
	double BolusWeightTotalCurrent ;
	double MaxBolusWeight ;
	double EatSW5 ;
	double CurrStrat ;
	int CurrHerbage ;
	double StandardBw ; // Standard BW of a cow baseline for extrapolating in various equations
	double StandardMetabolicBw ; // Standard EBW of a cow baseline for extrapolating in various equations
	double BwCorrected ; // Unit: kg. Excl.gravid uetrus, corrected to average condition
	double BwCorrection ;
	string bufferslp ;
	string bufferint ;
	double ResidMamMilk ;
	double dNdfIn ;
	double MealsDay ;
	double SunriseToday ;
	double SunsetToday ;
	double TMilkLmYest ;
	double TMilkPmYest ;
	double TMilkTmYest ;
	double FdRatDiel ; // Unit: kgDM. Daily cummlative DM intake. Being reset to zero at midnight
	double IALG ;
	double NSTP ;
	double CINT ;
	double MAXT ;
	double TSTP ;
	double TIME ;
	double LastEv ;
	double LastSu ;
	double LastSw ;
	double LastEA ;
	double fAcSilage ; // Kg/Kg Corn Silage DM
	double fLaSilage ;
	double PcSilage ;
	double PcPeFd ; // Gil sept 2014, reducing all nutrients, to leave enough room for SC!! (Delagargde for instance has 921 g/kg for CP+SC+NDF which leaves only 8% for ST PE OA ASH!!!!)
	double fBuAc ;
	double fOaPe ; // Oa = Pe * .2 is an Unsupported Estimate mdh. reduced from 0.2 to 0.1555 - Gil
	double fAcFd ;
	double FLaFd ;
	double fBuSilage ;
	double fBuFd ;
	double fPeFd ;
	double fOaFd ; // Oa = Pe*.2 is an Unsupported Estimate
	double fRoughageFd ;
	double fDMFd ;
	double PiMeanRRT ; // Used to calculate kPiAa from RUP and CP
	double HaMeanRRT ; // Used to calculate kHaCs from RUSt and St
	double CeMeanRRT ; // Used to calculate kCeCs from RuADF and ADF
	double FKRuP ;
	double FKRuSt ;
	double FKRuAdf ;
	double FHcCs1 ; // Scale KHcCs1 to KCeCs1 based on previously derived estimates for ikHcCs and ikCeCs
	double slpKRuAdf ; // added slopes to each of the K calculations to keep the
	double slpKRUP ; // rate constants sensitive to input in situ rates. MDH 5-17-14
	double slpKRUST ; // The large intercept for RuADF removed all sensitivity.
	double KPiAa ; // 24h Degradation Rates for protein
	double KHaCs ; // Starch
	double KCeCs1 ; // Cellulose, Need RuAdf estimates to fully use.
	double KHcCs1 ; // Scale rate from Ce to achieve appropriate Hc rate
	double fRupCp ; // Rup as a fraction of CP
	double fPiCp ; // SolP as a fraction of CP
	double fPsCp ; // SolP as a fraction of CP
	double fNnCp ; // Nn as a fraction of CP
	double fUrCp ; // Urea as a fraction of CP
	double fRuStSt ; // RuST as a fraction of St
	double fStSSt ; // Soluble St as a fraction of St
	double fRuAdfAdf ; // RuAdf as a fraction of Adf
	double fLgAdf ; // Lignin as a fraction of Adf
	double RestSa ; // At 8 h resting, this equates to 70 ml/min which is below the 114 ml/min according to Maekawa et al.
	double RestWa ;
	double RumntnSa ;
	double EatSa ; // L/Kg FdDMin. Redefined by Mindy to be: EatSa=2.6555*(EBW**0.7555)*Eating
	double SaIn ;
	double EatWa ;
	double otGutCont ; // Use initial guess until the first days intake has been determined
	double OsMolF ; // OSMOLALITY FACTOR
	double RumLiqVolEQ ;
	double WaIn ;
	double RumOsMol ;
	double OsWaSlp , OsWaInt ; // Set the slope a little higher to keep osmolality down, MDH 2-19-14
	double OsWa ; // Set the intercept to 0 to achieve a center around 280 mOsmol
	double fRumDM ;
	double WaOut ;
	double dRumLiqVol ;
	double DilRate ; // Dilution rate divided by 24 to get %/hr
	double KWaFeces ; // assumed feces is 23% DM from Murphy 1992 review, JDS
	double kMilkAsh ; // Assumed 1% Ash in milk
	double kWaRespir ; // Assumed half the maximal respiratory rate cited in Murphy, 1992 JDS
	double kWaSweat ; // Assumed 25% the maximal sweating rate cited in Murphy, 1992 JDS
	double WaFeces ;
	double WaMilk ;
	double WaRespir ;
	double WaSweat ;
	double WaUrine ; // This will easily go negative if DRnkWa is inadequate. May at times during the day with intermittent feeding
	double WaConsumed ;
	double TotWaConsumed ; // TotWaConsumed = integ ( WaConsumed , 0.0 )
	double TotWaUrine ; // TotWaUrine = integ ( WaUrine , 0.0 )
	double MaxBladderVol ;
	double MilkDen ;
	double TVolMilkVol ; // L, CCP 9-1-06
	double CountDownDays ; // Minimum days that DayMilk should count down to next calving
	double IntakeTotal ; // IntakeTotal = integ ( FdRat , 0 )
	double kMastication ; // Unit: multiplier. Adaptor from actual mastication jaw movements to the arbitrary steps of the model which are on a finer scale.
	double kAcquisition ; // Unit: multiplier. Adaptor from actual acquisition jaw movements to the arbitrary steps of the model which are on a finer scale. Acquisition/Severing bites are expected to have very little effect (smaller k) compared to mastication bites.
	double kSpecies ; // Unit: power. SpeciesFactor simulates ease of breakdown of the feed. It reduces mastication and increases breakdown effiecncy (simulated by cheating: increasing steps in the model!). However tougher feed should result in larger particle size, so we must avoid kSpecies=1 which will neutralize the species effect (i.e. with SpeciesFactor=1 the decreased breakdown effeciecny would cancel out exactly the effect of the increased mastication of tougher feed)
	double kComminuteOralMin ; // Fitted to 21 datasets, Bailey 90, Loginbuhl 89, Shaver, Oshio, John&reid 87, however with mastication time left free as was unknown! Hence in future the k will be influenced by the feeds ease of breakdown, which will also reduce mastication time so end result will not change a lot.
	double kComminuteOralMax ; // -"-
	std::array< double , MaxFdScreens > kComminuteOral ; // Array of proprtions that pass from each bin to the next bin upon one mastication jaw mevement
	int MasticationSteps ;
	int Old ;
	double SumBinFd ;
	double MeanParticleSize ;
	double MedianParticleSize ;
	double fSPartSwal ;
	double fMPartSwal ;
	double fLPartSwal ;
	double fLPartMPartSwal ;
	double PsF ; // This needs to be revised, MDH Feb 10, 2014
	double fParticulateFd ;
	double iOxup , iFCM4Z , iME , iDMilk ;
	double ifPm , ifTm , iAtAdh ;
	double icFa , icAc , icGl ;
	double iTCH4 ;
	double iDEi , iMEi , VmAcTsAdip ;
	double rtPOx ;
	double fTm ;
	double FCM4Z ;
	double AtAdHT ;
	double ME ;
	double OXUP1 ;
	double MamMilkAve2 ;
	double EBW ;
	double BW ;
	double VolGl ; // Next4: Changed to non consider adipose Gil 2015.
	double VolFa ;
	double VolAc ;
	double VolAa ;
	double BldUrVol ;
	double DMilk ;
	double TFCM4z ; // TFCM4z = integ ( FCM4Z , iFCM4Z )
	double WtCytAdip ;
	double otWtOth ;
	double otWtVis ;
	double fPm ;
	double ObsME , ObsDE , ObsCH4 , ObsEUr ;
	double FatAdd ;
	double InfPrt ;
	double CWC ;
	double GE1Fd ;
	double GE2Fd ;
	double GE3Fd ;
	double GEFd ;
	double OaScSC , PeScSC , LiScSC , LaScSc , HcCeCe ;
	double FaScFd ;
	double fLPartNutIng ; // Proportion of LPartSwal
	double fLPartSt ; // proportion of starch, added to LPart 5-21-13, MDH
	double fLPartHc ; // proportion of hemicellulose
	double fLPartCe ; // proportion of cellulose
	double fLPartPi ; // proportion of insoluble protein
	double fLPartLg ; // proportion of lignin
	double fLPartAi ; // proportion of insoluble ash
	double fIndigFd ; // Combines lignin and silicates as Indigestible Feed (IndigFd)
	double fLPartIndigFd ; // proportion of IndigFd
	double fLgIndigFd ;
	double fAiIndigFd ;
	double fLPartADF ;
	double fLPartNDF ;
	double fMPartNutIng ;
	double fSPartNutIng ;
	double OaSc ;
	double PeSc ;
	double LiSc ;
	double FatSc ;
	double fScTFd ;
	double totFd ;
	double DailyDMin ;
	double TotDMin ; // TotDMin = integ ( DailyDMin , 1.0E-9 )
	double OminFd ;
	double NdfinFd ;
	double AdfinFd ;
	double RuAdfinFd ;
	double LginFd ;
	double RuStinFd ;
	double ScinFd ;
	double CPinFd ;
	double CPsinFd ;
	double RUPinFd ;
	double NpninFd ;
	double NninFd ;
	double CFatinFd ;
	double AshinFd ;
	double DAY ;
	double CWCF ;
	double RumntnF ; // Gil Aug 20102 moved RUMNTEQ to mindy_init (=0 for molly; = 1 for mindy)
	double MinLPRumntnF ; // Fraction of MBW. Set to achieve appropriate rumination times and LP pool sizes, added 04-25-2011, MDH
	double MEAN1 , MEAN2 , AMP1FT , AMP2FT ;
	double MinLPRumntn ;
	double Eating ;
	double Rumntn ;
	double TotRumntn ; // TotRumntn = integ ( Rumntn , 0.0 )
	double TotEating ; // TotEating = integ ( Eating , 0.0 )
	double TotRest ; // TotRest = integ ( Rest , 0.0 )
	double AaFvAc ;
	double AaFvPr ;
	double AaFvBu ;
	double LaAcAc ;
	double LaPrPr ;
	double AaFvFat ;
	double FORSET , MIXSET , CONSET ;
	double ScAcAc ;
	double ScPrPr ;
	double ScBuBu ;
	double ScLaLa ;
	double StAcAc ;
	double StPrPr ;
	double StBuBu ;
	double StLaLa ;
	double HcAcAc ;
	double HcPrPr ;
	double HcBuBu ;
	double CeAcAc ;
	double CePrPr ;
	double CeBuBu ;
	double ScAc ;
	double ScPr ;
	double ScBu ;
	double ScLa ;
	double StAc ;
	double StPr ;
	double StBu ;
	double StLa ;
	double vfaeff , RumpHCON , FIXDpH , RumpHBase ;
	double RumpH ;
	double TVFA ;
	double cVFA ; // cVFA in Moles/liter
	double KSPartP , KWAP ; // KSPartP is now a fraction of KWAP, MDH 5-25-13
	double KLPartRed ; // This pool was reduced in size and more dietary material is diverted through it
	double dLPart ;
	double LPartSwal ; // LARGE PARTICLES
	double LPartRed ;
	double LPart ; // LPart = integ ( dLPart , iLPart )
	double LPart1 ;
	double KMPartSPart ; // Set to achieve steady state on the base diet, MDH
	double pLPartMPartComm ; // ??Need to verify and update if needed
	double KMPartP ; // A proportion of Liq Flow. An initial guess, MDH
	double dMPart ;
	double MPartSwal ;
	double LPartMPart ;
	double MPartSPart ;
	double MPartDeg ;
	double MPartP ;
	double MPart ; // MPart = integ ( dMPart , iMPart )
	double LPartplusMPart ;
	double dSPart ;
	double SPartSwal ;
	double LPartSPart ;
	double SPartDeg ;
	double SPartP ;
	double SPart ; // SPart = integ ( dSPart , iSPart )
	double fLPart ;
	double fMPart ;
	double fLPartplusMPart ;
	double fSPart ;
	double fMPart1 ;
	double fSPart1 ;
	double RumPartSizeSlp ;
	double RumPartSizeInt ;
	double PartWidth ; // mm from Pierre Beukes
	double PartThick ;
	double kSurfaceArea ; // 27.6555mm2/mm3=mean SA per unit Vol for Shaver et al. 1988 data
	double RumPartSizeMean ;
	double RumLpMpSizeMean ;
	double RumLPartSizeMean ;
	double RumMPartSizeMean ;
	double RumSPartSizeMean ;
	double MPartSA ;
	double MPartVol ;
	double SPartSA ;
	double SPartVol ;
	double fSPartSA ; // Scale SPartSA per Vol to that for a standard SP pool
	double fMPartSA ; // Scale MPartSA per Vol to that for a standard SP pool
	double fPartSA ;
	double fPartP ;
	double KMiHa , KMiHb , VmMiHa ;
	double VmMiHb , KMiHaF , KMiHbF ;
	double Csin ; // Fractions of Cs entry
	double fCsHa ; // attributed to Ha at Hb
	double fCsHb ; // hydrolysis.
	double HaMiP ;
	double HbMiP ;
	double IndigFdMiP ; // Passage of microbes in
	double PiMiP ; // association with SPart
	double SPartMiPi ;
	double CsMiG ; // Proportion of microbial
	double HaMiG ; // growth attributable to
	double HbMiG ; // Cs formed from Ha and Hb hydrolysis
	double cMiHa ; // Concentration (kg/kg) of
	double cMiHb ; // microbes associated with Ha and Hb
	double SPartMiHa ; // Microbes attached
	double SPartMiHb ;
	double HaMiRum ; // Microbes already associated with SPart potentially
	double HbMiRum ; // released due to hydrolysis of particulate substrates
	double HaMiF ; // Fraction of potentially released microbes actually
	double HbMiF ; // dependent on fractions of Ha and Hb in SPart where
	double MiHaMi ; // Microbes on particles and those grown from Ha and Hb
	double MiHbMi ; // hydrolysis and fermentation which remain in association with SP.
	double dHaMi ;
	double dHbMi ;
	double HaMi ; // HaMi = integ ( dHaMi , iMiHa )
	double HbMi ; // HbMi = integ ( dHbMi , iMiHb )
	double dHa ;
	double StinFd ;
	double StCsFd ;
	double StHaFd ; // error as fLPart was applied to StinFd previously, MDH 5-27-13
	double LPartStHa ;
	double SPartHaCs ;
	double HaP ;
	double HaPT ; // To Fit Duodenal Data, Kg/d
	double Ha ; // Ha = integ ( dHa , iHa )
	double KFatHb ;
	double KHcCs ;
	double KCeCs ;
	double dHc ;
	double Hcin ;
	double RumHcin ;
	double LPartHcHc ; // HOLOCELLULOSE-Hb
	double SPartHcCs ;
	double HcP ;
	double Hc ; // Hc = integ ( dHc , iHc )
	double dCe ;
	double RumCein ;
	double Cein ;
	double LPartCeCe ; // HOLOCELLULOSE-Hb
	double SPartCeCs ;
	double CeP ;
	double Ce ; // Ce = integ ( dCe , iCe )
	double dHb ;
	double Hbin ;
	double LPartHbHb ;
	double SPartHbCs ;
	double HbP ;
	double Hb ; // Hb = integ ( dHb , iHb )
	double KFatPi ; // Sept. 20, 2004 solution against Bate5o2 data.
	double dPi ;
	double PiPiFd ;
	double LPartPiPi ;
	double SPartPiAa ;
	double PiP ;
	double TPRTin ;
	double Pi ; // Pi = integ ( dPi , iPi )
	double dIndigFd ;
	double IndigFdFd ;
	double LPartIndigFdIndigFd ; // INDIGESTIBLE FEED
	double IndigFdP ;
	double LgP ; // To Fit Duodenal Data Kg/d
	double AiP ;
	double IndigFd ; // IndigFd = integ ( dIndigFd , iIndigFd )
	double RumLg ;
	double VmCsFv , KCsFv ;
	double dCs ;
	double cCs ;
	double ScTCs ;
	double StCs ; // SOLUBLE CARBOHYDRATES
	double HaCs ; // CS
	double HcCs ; // Converts kg of hemicellulose to moles of hexose equivalents.
	double CeCs ;
	double CsFv ;
	double CsMi ;
	double CsP ;
	double Cs ; // Cs = integ ( dCs , iCs )
	double VmRumAaFv , cSaPs , KRumAaFv ;
	double dRumAa ;
	double cRumAa ;
	double PsAaFd ;
	double PiAa ;
	double RumAaP ; // AMINO ACIDS-RumAa
	double SaPsAa ;
	double RumAaFv ;
	double RumAaMi ;
	double RumAa ; // RumAa = integ ( dRumAa , iRumAa )
	double NnAmAM , AaFvAm , KAmabs , UrAmAm ;
	double VmBldUrAm , KBldUrAm , KiAm ;
	double dAm ;
	double UrAmFd ;
	double NnAmFd ; // RUMEN AMMONNIA-Am
	double AaAm ;
	double SaNnAm ;
	double BldUrAm ;
	double absRumAm ;
	double AmMi ;
	double AM2 ; // AM2 = integ ( dAm , iAm )
	double AM ; // Prevent a crash when AM goes negative. I don't have time to find the source
	double Am1 ;
	double cAm ;
	double cBldUr ;
	double fSaAs , KAsabs ;
	double InfNaCl , InfNaBicarb ;
	double dAs ;
	double AsAsFd ;
	double SaAs ;
	double InfAs ;
	double AsP ;
	double AshP ; // Total Duodenal Ash, Kg/d
	double absRumAs ;
	double cAs ;
	double As ; // As = integ ( dAs , iAs )
	double LiFlFd , LiChFd , FaFlFd ;
	double dFl ;
	double FlFd ;
	double Fl1Fd ;
	double FlMi ;
	double FaP ;
	double LipidP ; // Total Duodenal Lipid Flow, Kg/d
	double Fl ; // Fl = integ ( dFl , iFl )
	double KabsAc , KabsPr , KabsBu , KabsLa ;
	double dRumAc ;
	double FvAcFd ;
	double CsAc ;
	double CsFvAc ;
	double fScCs ;
	double fStCs ;
	double fHcCs ;
	double FCeCs ;
	double RumLaAc ;
	double RumAaAc ;
	double absRumAc ;
	double RumAcSynth ; // synthesized ruminal acetate
	double cRumAc ;
	double RumAcP ;
	double RumAc ; // RumAc = integ ( dRumAc , iRumAc )
	double RumAc1 ; // Uninflated Rumen Pool size for comparison to observed data
	double MPcAc ; // Mole percent(MPc)
	double InfRumPr ; // infused ruminal propionate, mol/d
	double dRumPr ;
	double CsPr ;
	double RumLaPr ;
	double CsFvPr ;
	double RumAaPr ;
	double RumPrP ;
	double RumPrSynth ;
	double cRumPr ;
	double absRumPr ;
	double RumPr ; // RumPr = integ ( dRumPr , iRumPr )
	double RumPr1 ; // Uninflated Rumen Pool size for comparison to observed data
	double MPcPr ;
	double dRumBu ;
	double CsBu ;
	double CsFvBu ;
	double RumAaBu ;
	double FvBuFd ;
	double absRumBu ;
	double RumBuP ;
	double RumBuSynth ;
	double cRumBu ;
	double RumBU ; // RumBU = integ ( dRumBu , iRumBu )
	double RumBu1 ; // Uninflated Rumen Pool size for comparison to observed data
	double MPcBu ;
	double KLaFv ;
	double dRumLa ;
	double CsLa ;
	double CsFvLa ;
	double FvLaFd ;
	double RumLaP ;
	double cRumLa ;
	double RumLaFv ;
	double absRumLa ;
	double RumLa ; // RumLa = integ ( dRumLa , iRumLa )
	double RumLa1 ; // Uninflated Rumen Pool size for comparison to observed data
	double RumYAtp , KYAtAa , KFGAm , LaFvAt ;
	double KFatFG ;
	double CsFvAt , AaFvAt ;
	double CsMiG1 , AmMiG1 , HyMiG1 , CdMiG1 ;
	double FlMiG ;
	double CsMiG2 , AmMiG2 , AaMiG2 , HyMiG2 ;
	double CdMiG2 ;
	double MiPiPI , MiNnNn , MiHaHA , MiLiLI ;
	double MiLiFA , MiLiBu , MiLiPr , MiLiCh , MiLiGl ;
	double MwtMiLi ;
	double MiMaAd ;
	double dMi ;
	double MiG ;
	double AtpG ;
	double AtpF ;
	double AtpC ;
	double AtpM ;
	double FGAm ;
	double FGFa ;
	double YAtp ;
	double YAtpAp ;
	double G1 ;
	double G2 ;
	double MiP ;
	double SPartMiP ;
	double WaMiP ;
	double LPartMi ;
	double SPartMi ;
	double WaMi ;
	double cMiSPart ;
	double cMiWa ;
	double Mi ; // Mi = integ ( dMi , iMi )
	double RumNit ;
	double RumCP ;
	double ADFIn ;
	double NDFIn ;
	double RumADF ; // Rumen pool size of ADF and NDF, kg
	double RumNDF ;
	double RumOM ;
	double fLPartNDF_NDF ;
	double fMPartNDF_NDF ;
	double fSPartNDF_NDF ;
	double ADFP ;
	double MPartADFP ;
	double SPartADFP ;
	double NDFP ;
	double MPartNDFP ;
	double SPartNDFP ;
	double NitP ; // Total Duodenal N Flow, Kg N/d
	double MiNP ;
	double CpP ; // Total CP Passage to SI, kg/d
	double NANP ; // No accomodation for Ammonia passage. Is this correct? Probably blown off by drying.
	double MiPP ; // Microbial CP Passage
	double NANMNP ;
	double MetabPP ; // Metabolizable Protein Passage, kg/d, original eqn wrong corrected 1-30-07 mdh
	double SolOmP ; // Added ChChFd to be consistent with DMP, MDH. Mar 31, 2014
	double LgutDCHa , DCMiPi , DCMiLi , LgutDCHb ;
	double LgutDCPi , LgutDCAs , LgutDCAi , LgutDCFa ;
	double DMP ;
	double ChChFd ;
	double LgutHaGl ;
	double MiGl ;
	double MiAa ;
	double MiLiDg ;
	double MiFa ;
	double LgutFaDg ;
	double MiBu ;
	double MiPr ;
	double MiLGl ;
	double MiCh ;
	double LgutHcFv ;
	double LgutHcAc ;
	double LgutHcPr ;
	double LgutHcBu ;
	double LgutCeFv ;
	double LgutCeAc ;
	double LgutCePr ;
	double LgutCeBu ;
	double LgutPiAa ;
	double LgutAs ;
	double LgutAi ;
	double FecHa ;
	double FecMiHa ;
	double FecHaT ;
	double FecHb ;
	double FecHC ;
	double FecCe ;
	double FecADF ;
	double FecNDF ;
	double FecLg ;
	double FecFa ;
	double FecMiLi ;
	double FecLipid ; // Total Fecal Lipid Flow, Kg/d
	double FecMiPi ; // KG.
	double FecMiNn ;
	double FecPi ;
	double FecPiT ;
	double FecPiTN ;
	double FecAsh ;
	double FecCh ;
	double FecOm ;
	double FecENG ;
	double FecDM ;
	double FecMPart ; // these do not account for intestinal digestion of MPart and SPart. Need to fix per above.
	double FecSPart ;
	double FecFMPart ;
	double FecFSPart ;
	double SolDMP ;
	double TOmP ;
	double TTOmP ;
	double OmPt ; // true OM passage. Not sure why the above do not follow the standard. Added this variable to maintain historical data references, MDH
	double OmPa ; // apparent OM passage from the rumen
	double RumDCOm ; // FOR TRUE ORGANIC MATTER
	double RumDCOmA ; // FOR APPARENT ORGANIC MATTER
	double RumDCPrt ;
	double RumDCN ;
	double RumDCndf ;
	double RumDCadf ;
	double RumDCHa ; // truely digested
	double RumDCHaT ; // apparent Ha digestion
	double RumDCHc ;
	double RumDCCe ;
	double RumDCHb ;
	double RumDCLiA ; // Apparently Digested in the Rumen
	double RumDCLiT ; // Truly Digested in the Rumen
	double DCDM ;
	double DCOm ;
	double DCPrt ;
	double DCHa ;
	double DCLipid ; // No Correction for Micribial Lipid
	double DCndf ;
	double DCadf ;
	double DCHb ;
	double DCLg ;
	double TStin ;
	double FdGEin ;
	double AccGEi ; // AccGEi = integ ( FdGEin , 1.0E-8 )
	double TDE ;
	double appDE ; // APPARRENT DIGESTIBLE ENERGY
	double DEI ; // DIGESTIBLE ENERGY INTAKE
	double DE ; // DIGESTIBLE ENERGY
	double AccDEi ; // AccDEi = integ ( DEI , 1.0E-8 )
	double CH4E ; // APPARRENT AND CORRECTED
	double EUr ; // METABOLIZABLE
	double MEI ; // ENERGY
	double AccMEi ; // AccMEi = integ ( MEI , 1.0E-8 )
	double ME1 ;
	double GE ;
	double HFerm ;
	double CorMEi ;
	double CorME ;
	double DCCe ;
	double Nintake ;
	double Nan ;
	double Ndiff ;
	double MiPrOm ; // kg CP/kg OM True
	double MirOma ; // Apparent
	double MiNOm ; // kg N/kg OM True
	double MiNOma ; // Apparent
	double absGl ;
	double AbsAa ;
	double absAc ;
	double absPr ;
	double absBu ;
	double AbsAm ;
	double absFa ;
	double absAs ;
	double absLa ;
	double AbsAcE ;
	double absPrE ;
	double absBuE ;
	double absFaE ;
	double absAaE ;
	double absGlE ;
	double absLaE ;
	double AbsE ;
	double Latitude ; // Positive for N Hemi, Neg for S Hemisphere
	double DaylightSavingShift ; // Unit: hours. Set to 1 if the management hours (milking hours, feeding times in Mindy) are expressed in daylight saving time with 1 hour shift.
	double DaylengthP1 ;
	double DayTwlengthP2 ;
	double DayTwLength ;
	double DaylengthP2 ;
	double DayLength ;
	double Sunlight ; // positive if the sun is up and negative it is below the horizon
	double Sunrise ;
	double SunSet ;
	double SunsetTodayTemp ;
	double kAHorGl , Theta2 ;
	double kAHor1Gl , Theta3 ;
	double kCHorGl , Theta4 ;
	double kCHor1Gl ;
	double AHor ; // Stimulates FaTsAdip, AaPOthOth, AaPVisVis, rtOx1 (GlOx)
	double AHor1 ; // Stimulates AcTsAdip
	double CHor ; // Not Used
	double CHor1 ; // Stimulates TsFaAdip
	double dWtUter ;
	double dWtPUter ;
	double WtUterSyn ;
	double WtPUterSyn ;
	double WtUterDeg ;
	double WtPUterDeg ;
	double AaPUter ;
	double PUterAa ;
	double WtConcSyn ;
	double WtPConc ;
	double WtPConcSyn ;
	double AaPConc ;
	double kAaGlGest ; // Yields approximately 2x use for Oxid as for prt syn per Bell 1995
	double WtPGrvUter ;
	double WtPGrvUterSyn ;
	double dWtGrvUter ;
	double dWtPGrvUter ;
	double AaPGest ;
	double AaGlGest ; // I think I have accounted for heat and ATP correctly
	double KMilk ;
	double MilkProductionAgeAdjustment ;
	double MamCellsPerKgMs270 ;
	double MamCellsPerKgMsAdjustment ;
	double KgMilkSolidsExpectedIn270Days ; // Also Key Determiner of required intake
	double MamCellsPart ; // WFM sets it to 2.5555 * KgMilkSolidsExpectedIn270Days - 200
	double K1MamCells ; // Decay rate for cell proliferation precalving from Dijkstra goat DNA
	double uTMamCells ; // Specific Cell proliferation rate at parturition from Dijkstra
	double lambdaMamCells ; // Mam Cell Death rate from Dijkstra Mexican cow (now used only precalving GL)
	double kMamAQBase ; // From Vetharaniam et al 2003
	double kMamCellsDeclineBase ; // Determines the base (2x) post peak decline of MamCells (Note: LHOR affcets Milk Production decline as well)
	double kMamCellsQAPrePeak ; // Very sensitive! determines the timing of the peak. Decraese delays
	double kMamCellsQAPostPeak ; // The ratio between this one and kMamCellsAQBase drives the proprtion of the A pool in its relatively stable period (post peak)
	double kMamCellsQAStart ; // Speeds up the first week's A pool growth => 1st week milk yield
	double kMamCellsQAKickStartDecay ; // Speeds up the first week's A pool growth => 1st week milk yield
	double kMamCellsTransitionDim ; // Affects mid lactation milk curve. Controls the centre DIM of the change from "pre" tp "post" kMamCellsQA
	double kMamCellsTransitionSteepness ; // Affects mid lactation milk curve. Controls the speed of transition from "pre" to "post"kMamCellsQA. 0.1 gives 90% of the change with the 60 days around kMamCellsTransitionDim. With 0.2 the 60 goes down to 30.
	double MamCellsDecayRateOfSenescence ; // This determines the mild curvature (convex) of the post peak mam cells decline, by slowing the absolute senescence rate gardually down to base turn over rate. Note that a typical near-constant persistnecy is exponantial decay by nature (e.g. 98% weekly persistency means every week is 98% of the previous => exponential decay.
	double BaseMamCellsTurnOver ; // Base rate of proliferation and senecence, has no real effect because it does not affect the net change of MamCells. Only purpose is to maintain biological correctness of 100% turnover by around 250 DIM (Capuco / Ruminant phisiology)
	double MamCellsProliferationDecayRate ; // This determines Peak mam cells. Not much to play with here as it is commonly accepted that day 10 in milk is the peak
	double kMamCellsUsMfDecay ; // Rate of the increased senecence while on low milking frequency. (works on a continuous scale between MF 1 and 2)
	double MilkIntPowerForFMamCelsQA1 ; // Changes the Q to A flux as MF changes. When decreased, difference between 1x,2x and 3x will grow.
	double MaxLossDueToLowMf ; // This sets the maximum proportion of MamCellPart that would be permanently lost due to long term low milking frequency. The loss is fast to start with and slows down the more days on low MF the cow goes through
	double PreCalvingMamCells ; // Oroginal Dijkstra
	double PostCalvingMamCells ;
	double MamCells ;
	double MEinMJ ; // Moved here to minimise diff with mark
	double LW ; // Unit: kg. Liveweight incl. milk ! Moved here to minimise diff with mark
	double fMamCellsQA ;
	double fMamCellsPA ; // Approximation of the growth rate derived from Dijkstra's MamCells eq.
	double fMamCellsUS ; // Approximation of the senescence rate derived from Dijkstra's MamCells eq, PLUS dymaically calculated increase loss due to low milking frequency
	double fMamCellsAS ; // Partition the total death rate between the Q and A pools
	double fMamCellsQS ; // Partition the total death rate between the Q and A pools
	double fMamCellsAQ ; // GL: removed KMINH as there are alternative explicit MF drivers
	double dMamCellsA ; // Second part empties the A pool withing few hours after dry off
	double dMamCellsQ ; // Second part brings Q pool to have all MamCells when dry
	double dMamCellsS ;
	double MamCellsA ; // MamCellsA = integ ( dMamCellsA , iMamCellsA )
	double MamCellsQ ; // MamCellsQ = integ ( dMamCellsQ , iMamCellsQ )
	double MamCellsS ; // MamCellsS = integ ( dMamCellsS , iMamCellsS ) // We don't really need this pool, only to verify that there is approx 100% turnover over 250 days of lactation
	double MamCellsQaKickStartFactor ;
	double MamCellsQaPreToPostFactor ;
	double kMamCellsQA ; // Baseline value
	double kMamCellsQaMfAdjustment ; // Stimulus adjustment, more cells go from Q to A as the milking frequency goes higher. Note that it gives 1 = no change for twice a day milking (MilkInt = 0.5).
	double LowMfDecay ; // The longer on low MF the slower the senescence
	double IncreasedUsDueToLowMf ;
	double dNonUterEBW ;
	double WtAdipNew ; // WtAdipNew = max ( 0. , integ ( dWtAdipNew , iWtAdip ) )
	double LhorTurnoverDays ; // Roughly the number days it would take LHOR to change to the full extent once the drivers state changed
	double kLHorSensAa ; // Linear slope component in the equation
	double kLHorSensGl ; // Linear slope component in the equation
	double wLHorSensAa ; // Relative importance of blood amino acids
	double wLHorSensAdip ; // Relative importance of adipose size.
	double wLHorSensGl ; // Relative importance of blood glocose.
	double xLHorSensAa ; // Curvature component of the sensitivity
	double xLHorSensAdip ; // Curvature component of the sensitivity
	double xLHorSensGl ; // Curvature component of the sensitivity
	double cAaBase ; // Baseline of cAA. When cAA = cAaBase => nil change to LHOR due to amino acids level. 10-2015 GL changed from 0.06555 to 0.005
	double cGlBase ; // Baseline of CGL. When cGL = cGlBase => nil change to LHOR due to glucose level
	double KDayLength ; // CCP Scalar for PP effect on LHor degradation
	double FixedLhorSW ; // Unit: 0 or 1. set to 1 to bypass the Lhor equation and use Lhor set to its baseline instead.
	double cGlTarget ; // Gl Has the same strong dip as BCS, so we want only deviation from the pattern t count topwards LHor syntheis.
	double LHor ;
	double LHor1 ; // LHor1 = integ ( dLHor , iLHor )
	double dLHor ;
	double VmLHorSyn ; // Denominator of LhorSyn. Set so that the division gives 1.0 when all drivers are on base line
	double LHorSyn1 ; // The expression in the brackets yields 1 when all drivers are on their baseline.
	double LHorSyn ; // Maintain base level while dry, the real game starts when she calves, otherwise we may have too much sesnitivity to the dry period situation of the cow
	double LHorDeg ; // CCP + GL (made turnover adjustable). This equation empties (1 + KLhorPP)/turnoverDays of LHOR pool
	double kLHor ; // Base degredadion and synthesis rate. Unit: LHOR units per day
	double LhorAa ;
	double LhorGl ;
	double LhorAdip ; // Contribution of Adippose to LhorSyn (not linear)
	double KLHorPP ; // CCP 12-11-06
	double BcsTargetNadir ; // Target BCS around peak lactaion. Calving, Calving + 365 days => BcsTarget = BcsBase; DIM 10 to 70: BCS = BcsTargetNadir; oOther DIM: linear - connect the dots.
	double BcsTargetDecay ; // Determines the exponential down curve of TargetBcs from calving to nadir
	double BcsTargetFactor ; // Changes beween 1 (calving) through 0 (~day 40) than up to 1 after 365 days, to create the BcsTarget
	double WtAdipTarget ; // MDH Target Adipose weight defended by the animal. GL made TegetBcs variable to refelect findings of BCS article, JR et al JDS 2007
	double CorrectedBW ; // Using iRumVol rather than RumVol, to avoid fluctuation
	double PMamEnzCell ; // This should be a fn of DayMilk
	double MamEnz ;
	double kMilkingFrequencyLagUp ;
	double kMilkingFrequencyLagDown ;
	double MilkingFrequencyAdjusted ; // Young cows do not perform as well as mature cows on OAD. Observed relationship incorporated, and we capture that in both reduced intake(FdRat_DairyNZ.csl), and here, by artificially reducing their milking frequency as if they are miked even less than once a day, to magnify the OAD effect (slow return from Q to A pools)
	double MilkingFrequencyBaseAdjustment ;
	double OnceADay2YearsOldAdjustment1 ;
	double MilkingFrequencyAgeAdjustment ;
	double MilkSolids270MfAdjusted ; // Expected yield on actual milking frequency
	double KMilkInhDeg , ikMilkInh ;
	double dKMilkInh ;
	double MilkInhSyn ; // Changed to prevent negative values, Apr 23, 2008 MDH
	double MilkInhDeg ;
	double KMinh ; // KMinh = integ ( dKMilkInh , ikMilkInh )
	double TgFaFa , AcAcTs , VmTsFaAdip , KTsFaAdip ;
	double Theta1 ;
	double KFaTsAdip , K1FaTs , K1TsFa ;
	double VmFaTsAdip , KFaTmVis , VmFaTmVis ;
	double FaTgTg , AcTgTg , K1FaTm ;
	double P1 ;
	double EXP10 ;
	double dTsAdip ;
	double TsFaAdip ;
	double cTs ;
	double FaTsF1 ;
	double AcTsF1 ;
	double WtAdip ;
	double dWtTsAdip ;
	double WtTsAdip ;
	double TsAdip ; // TsAdip = integ ( dTsAdip , iTsAdip )
	double dBCS ;
	double BCS ; // BW does not affect BCS within a run hence this equation vs init section
	double BCS_NZ ; // CCP's scale, 7-26-07
	double dFa ;
	double FaTsAdip ;
	double cFa ;
	double TsFaF1 ;
	double FaTmVis ;
	double Fa ; // Fa = integ ( dFa , iFa )
	double K1VAct , K2VAct ;
	double dVmAcTs ;
	double VmAcTs2 ; // VmAcTs2 = integ ( dVmAcTs , iVmAcTs )
	double VmAcTs ;
	double KAcTsAdip , VmAcTmVis , KAcTmVis ;
	double K1AcTm , K1AcTs , AaGlAc ;
	double dAc ;
	double AaAcV1 ;
	double AcTsAdip ;
	double cAc ;
	double AcTmVis ;
	double Ac ; // Ac = integ ( dAc , iAc )
	double WtAcTm ;
	double WtFaTm ;
	double dTm ; // Kg/d Produced
	double dMamTm ;
	double dMilkTm ; // Mammary MILK FAT
	double MamTm ; // MamTm = integ ( dMamTm , iMamTm ) // AND TOTAL YIELD
	double TMilkTm ; // TMilkTm = integ ( dMilkTm , 1.0E-8 )
	double PcTmFromScfa ;
	double ExpOth2 , KDnaOth , ExpV2 , KDnaVis ;
	double OthDnaMx , VisDnaMx ;
	double dOthDna ;
	double dVisDna ;
	double OthDna ; // OthDna = integ ( dOthDna , iOthDna )
	double VisDna ; // VisDna = integ ( dVisDna , iVisDna )
	double VmAaPOthOth ;
	double VmAaPVisVis , KAaPOthOth , KAaPVisVis ;
	double VmAaPmVis , VmAaGlVis , KAaGlVis ;
	double KPOthAaOth , KPVisAaVis , fDWt ;
	double AaGlUr , KAaPmVis , VsizF ;
	double dAa ;
	double dPOth ;
	double dPVis ;
	double POthAaOth ;
	double PVisAaVis ;
	double cPOth ;
	double cPVis ;
	double WtPOth ;
	double WtPVis ;
	double WtOth ;
	double dWtOth ;
	double WtVis ;
	double dWtVis ;
	double AaPOthOth ;
	double AaPVisVis ;
	double AaGlVis ; // GLUCONEOGENESIS
	double POthfSr ;
	double PVisfSr ;
	double POthfDr ;
	double PVisfDr ;
	double AaPmVis ;
	double cAa ;
	double PVis ; // PVis = integ ( dPVis , iPVis )
	double POth ; // POth = integ ( dPOth , iPOth )
	double AA1 ; // AA1 = integ ( dAa , iAa )
	double AA ;
	double AmUrUr , KBldUrU ;
	double dBldUr ;
	double AaUrVis ;
	double AaUrGest ;
	double AmUr ;
	double BldUrRumAm ;
	double SaNRumAm ;
	double dUrea ;
	double BldUr1 ; // BldUr1 = integ ( dBldUr , iBldUr )
	double BldUr ; // Prevent a crash when AM goes negative. I don't have time to find the source
	double cPun ; // Converts cBldUr from mol Urea/l to mg N/dl.
	double cMun ; // From Kauffman & St-Pierre, 2001, JDS, 84: 2284-2294 (mg/dl)
	double BldUrMUN ; // (moles urea/day transferred from Bld to milk)
	double dPm ; // Kg/d produced
	double dMamPm ;
	double dMilkPm ;
	double MamPm ; // MamPm = integ ( dMamPm , iMamPm ) // Mammary PROTEIN
	double TMilkPm ; // TMilkPm = integ ( dMilkPm , 1.0E-8 ) // AND TOTAL YIELD
	double AaGlGl , PrGlGl , LaGlGl , GyGlGl ;
	double KGlLmVis , VmGlTpAdip ;
	double KGlTpAdip , VmGlTpVis , KGlTpVis , fLaCdAdip ;
	double KGlLaOth , VmGlLaOth , fLaCdOth , GlGlHy ;
	double pGlHyAdip , pGlHyVis , fPrGl , TgGyGy , GlLaLa ;
	double GlHyTp , GlTpTp , TpTpTs , GlGyGY , KAaLmVis ;
	double TpTpTm ;
	double dGl ;
	double upGl ;
	double AaGlV1 ;
	double PrGlV1 ;
	double PrGlVis ;
	double LaGlV1 ;
	double RumLaGl ;
	double gGlLa ;
	double GyGlVis ;
	double GyGlV1 ;
	double VmGlLmVisPart ;
	double kVmGlLmSyn , kVmGlLmDecay , kVmGlLmDeg ;
	double VmGlLm1Vis ;
	double GLLmVis ; // cAa should affect the Vm not Ks
	double GlHyAdip ;
	double GlHyVis ;
	double fGlHyAdip ;
	double fGlHyVis ;
	double GlTpAdip ;
	double GlTpVis ;
	double GlLaOth ;
	double TpinAdip ;
	double GlTpF1 ;
	double TpinVis ;
	double GlTpV1 ;
	double LainOth ;
	double GlLaB1 ;
	double TpLaAdip ;
	double TpCdVis ;
	double TpTsAdip ;
	double TpTmVis ;
	double AcTmV1 ;
	double FaTmV1 ;
	double LaCdAdip ;
	double LaGlAdip ;
	double GlGyT ;
	double LaCdOth ;
	double LaGlOth ;
	double Gl ; // Gl = integ ( dGl , iGl )
	double cGl ;
	double GlLmLm , fLm ;
	double dLm ; // Kg/d produced
	double dMamLm ;
	double dMilkLm ;
	double MamLm ; // MamLm = integ ( dMamLm , iMamLm ) // Mammary LACTOSE
	double TMilkLm ; // TMilkLm = integ ( dMilkLm , 1.0E-8 ) // AND TOTAL YIELD
	double MamMilk ;
	double dMamMilkAve ;
	double MamMilkAve ; // MamMilkAve = integ ( dMamMilkAve , iMamMilkAve )
	double GlCdAt , AcCdAt , FaCdAt , GyGlAt ;
	double PrCdAt , TpCdAt , LaCdAt , TpLaAt , GlLaAt ;
	double BuCdAt , HyAtAt ;
	double AaPxAD , GlHyAD , GlTpAD , LaGlAd ;
	double TpTgAD , AcFaAd , TcHyAd , GlLmAd , PrGlAd ;
	double absGlAd , absAaAd ;
	double OxAcCd , OxPrCd , OxBuCd , OxGlCd , OxLaCd ;
	double OxTpCd , OxFaCd , OxPrGl ;
	double AcCdCd , PrCdCd , BuCdCd , GlCdCd , GlHyCd ;
	double FaCdCd , LaCdCd , TpCdCd ;
	double TAveabsE ;
	double dabsEAve ;
	double absEAve ; // absEAve = integ ( dabsEAve , iabsEAve )
	double absEF ;
	double EBW1 ;
	double dEBW1 ; // WtCytAdip does not change during a run.
	double BW1 ;
	double NonFatEBW ;
	double NonFatNonUterEBW ;
	double KNaOth , HyAcFa , AaGlH ; // KbasOth was moved so it can be defined diferently by Mindy
	double KbasOth ; // Basal energy expenditure rate coefficient bundles average activity inside
	double eerActivityAtp ; // Unit: mole/d Activity Energy Expenditure Rate. Molly does not calculate activity explicitly
	double AtAd ;
	double AdAt ;
	double AtAdOth ;
	double AtAdB1 ;
	double basalOth ; // GL Apr 2014: KBasalOth reduced from ~2.2 to XXX and eerActivityATP introduced
	double OldBasalOth ;
	double KNaAtOth ;
	double AdAtOth ;
	double AdAtB1 ;
	double AdAtB2 ;
	double basHtOth ; // Basal
	double AaPOthHt ; // Protein TO
	double MHtOth ;
	double KbasAdip , KNaAdip ;
	double AtAdAdip ;
	double basalAdip ;
	double KNaAtAdip ;
	double AtAdF1 ;
	double AtAdF2 ;
	double AtAdF3 ;
	double AtAdF4 ;
	double AdAtAdip ;
	double AdAtF1 ;
	double AdAtF2 ;
	double basHtAdip ; // Basal
	double HtF2 ; // Gl to Tp
	double HtF3 ; // Ts TO
	double MHtAdip ;
	double KbasVis , KNaVis , KidWrk , HrtWrk , ResWrk ;
	double AtAmUr ;
	double AtAdVis ;
	double basalVis ;
	double KNaAtVis ;
	double AtAdV1 ;
	double AtAdV2 ;
	double AtAdV3 ;
	double AtAdV4 ;
	double AtAdV5 ;
	double AtAdV6 ;
	double AtAdV7 ;
	double AtAdV8 ;
	double LaGlVis ;
	double AtAdV9 ;
	double AtAd10 ;
	double AtAd11 ;
	double AtAd12 ;
	double AtAd13 ;
	double AtAd14 ;
	double ATAd15 ;
	double AdAtVis ;
	double AdAtV1 ;
	double AdAtV2 ;
	double AdAtV3 ;
	double AdAtV4 ;
	double PrCdVis ;
	double BuCdVis ;
	double AdAtV5 ;
	double basHtVis ; // basal
	double HtV2 ; // Protein TO
	double HtV3 ; // Gl to Tp
	double HtV4 ; // La to Gl
	double HtV5 ; // kidney work
	double HtV6 ; // heart work
	double HtV7 ; // respiration
	double HiV8 ; // urea synthesis
	double MHtVis ;
	double fGrvUterTO ; // All oxidation in GrvUter driven by protein T/O
	double AtAdGestGrth ; // Protein deposition cost
	double AtAdGestTO ; // Protein T/O estimate
	double AtAdGest ;
	double MHtGestGrth ;
	double MHtGestTO ;
	double MHtGest ; // Maintence Heat of Gestation
	double EGrvUterCLF ;
	double KAcCd , KGlCd , KFaCd ;
	double ndAt ; // TOTAL OXIDATION
	double ndOx ;
	double rtOx1 ;
	double rtOx2 ;
	double GlCd ;
	double FaCd ;
	double AcCd ;
	double rtPO ;
	double TcHyAdip ; // correction for NADH
	double TcHyVis ; // from ICD in TCA
	double dOx ;
	double AtHt1 ;
	double AtHt2 ;
	double AtHt3 ; // gluconeogenesis from Gy
	double AtHt4 ;
	double AtHt5 ;
	double AtHt6 ;
	double AtHt7 ;
	double AtHt8 ;
	double AtHt ;
	double AtAdH1 ;
	double dN ; // in kg N
	double Nin ;
	double UrNFd ;
	double NSal ;
	double Nabs ;
	double NUr ;
	double NurTotal ; // NurTotal = integ ( NUr , 0.0 )
	double NBody ;
	double NMilk ;
	double NFec ;
	double Nout ;
	double Nret1 ; // Nret1 should = Nret2 should = dN
	double Nret2 ;
	double Ndig ; // should equal Nabs
	double Nbal ;
	double RumDPrta ;
	double ELm ;
	double EPm ; // Changed from dMilkPm (milkout) to dPm, dTm, and dLm
	double ETm ; // MDH, 5-6-14
	double NEP ; // this is NEl, NEp would include growth and pregnancy
	double NetEff ;
	double propLm ;
	double fTm1 ;
	double PcLm ;
	double PcPm ;
	double PcTm ;
	double FCM3h ;
	double FCM4z1 ;
	double MntHP ; // ??There is a problem with maintenance HP. Way too high, MDH, 5-6-14
	double MEMBW ;
	double THP1 ;
	double fGestEPrt ; // Derived from (AaPGest*HcombAa)/EGrvUterCLF
	double dOthE ;
	double dAdipE ;
	double dVisE ;
	double dGestE ; // Matched Ferrell's estimate which will be slightly
	double dMaint ;
	double dHiM4 ;
	double EB ;
	double THP2 ;
	double CorNEP ;
	double CH4EFd ; // methane (kcal)/kg feed
	double fCH4E ;
	double fCH4DE ;
	double fCH4ME ;
	double AaFvHy ;
	double KHyEruct ; // Based on AgResearch Chamber data
	double KHyOther ; // Based on AgResearch Chamber data
	double KumarMigEq ; // Pablo requested May 2016 to be able to diable Mig effect on hidrogen
	double dTCH4 ;
	double dCsFvH ;
	double dCsHy ;
	double dRumAaHy ;
	double dHyFlF ;
	double dHyMi ; // This equation has been wrong since 1995. It was lacking MiG. Added MiG multiplier per Vetharanium 6-24-14, MDHdHyFlF=FlFd*2.0
	double dTHy ;
	double dHyEruct ; // Hydrogen eructation, g/d = mol/d, added 5-14-14, MDH
	double dHyOther ; // Undefined loss of Hy, mol/d, added 5-19-14, MDH
	double dDCH4 ; // Unit: moles/d
	double dCH4Kg ; // Unit: Kg/d of Methane
	double dCH4g ; // Unit: g/d of Methane
	double TCH4 ; // TCH4 = integ ( dTCH4 , iTCH4 ) // TCH4 is in moles
	double CH4KGY ; // total kg methane
	double CH4Milk ; // kg methane/kg milk
	double TCH4E ; // total kcal methane
	double netME ; // added 4/16/92 kcd
	double CH4GEi ; // to use for EPA CH4
	double CH4DEi ; // calculations
	double CH4MEi ; // added 7/23/92
	double fFIM ;
	double mult ;
	double BCH4 ;
	double TBCH4 ; // TBCH4 = integ ( BCH4 , 1.0E-8 )
	double TBCH41 ;
	double fBCH4E ;
	double fBCH4D ;
	double fBCH4M ;
	double BCH4Fd ;
	double MCH4E ;
	double MCH4kg ; // units kg - eq. changed 2/2/95-kc
	double TMCH4E ; // TMCH4E = integ ( MCH4E , 1.0E-8 ) // TMCH4 to TMCH4E - 2/2/95
	double TMCH42 ; // units kg - 2/2/95-kc
	double fMCH4E ; // energy equ. using MCH4E - 2/2/95
	double fMCH4D ;
	double fMCH4M ;
	double EPart ;
	double GlLmHt ; // Lactose synthesis
	double AaPmHt ; // milk protein synthesis
	double AcTsH1 ; // Acetate energy input
	double AcTsH2 ; // Gl energy loss in PC
	double AcTsH3 ; // activation of Gl for PC
	double AcTsH4 ; // Tp reduction and Fa esterification
	double AcTsH5 ; // NADPH from TCA + AtAd in Fa synthesis
	double AcTsH6 ; // Tp energy incorporated
	double AcTsH7 ; // Energy in Ts
	double AcTsHt ;
	double AcTmH1 ; // Acetate energy input
	double AcTmH2 ; // Gl energy loss in PC
	double AcTmH3 ; // Activation of Gl for PC
	double AcTmH4 ; // NADPH from TCA and AtAd in FAS
	double AFTmH5 ; // Tp reduction and Fa esterification
	double AFTmH6 ; // Tp energy input
	double FaTmH7 ; // Fa energy input
	double FaTmH8 ; // Cost of Fa synthesis in adipose
	double rtFa1 ; // proportion of Fa entering central pool formed from Ac
	double AfTmH9 ; // Glycerol energy recovered
	double HiP1 ;
	double HiM1 ;
	double PrGlHt ;
	double AaGlHt ;
	double absGlHt ; // Cost of absorbtion
	double aPAGlH ; // Total Hi of Gl
	double GLMilk1 ; // Gl to PC in adipose
	double GlMilk2 ; // Gl to PC in mammary
	double GlMilk3 ; // Gl to Tm and Gy in viscera
	double GlMilk ;
	double rtGl1 ; // Proportion of Gl entry to milk
	double HiP2 ; // Hi of Gl assigned to production
	double HiM2 ; // Hi of Gl assigned to maintenance
	double AaTO ; // Note that only visceral protein TO contributes to Aa TO
	double absAaHt ;
	double rtAa1 ;
	double HiP3 ;
	double HiM3 ;
	double dHiP4 ;
	double HFermM ;
	double HiM ;
	double HiP ;
	double RQEQ ;
	double AaFvCd ;
	double CsFvCd ;
	double CsCd ;
	double RumAaCd ;
	double CdMi ;
	double TCd ;
	double dRumCd ;
	double MwtCd ;
	double dCd ;
	double dCdKg ;
	double RQ1 ;
	double GlCdT ;
	double GlTO ;
	double ObsMEi ;
	double ObsDEi ;
	double ObsCH4E ;
	double dTCH4E ;
	double EUrFd ;
	double ObsPredME ;
	double ObsPredDE ;
	double ObsPredCH4 ;
	double ObsPredEUr ;
	double Somedummyvariable ; // --- use this dummy assignment statement to set a debug breakpoint. Can equal any variable or number.
	
	state_type get_state ( ) {
		
		state_type a_state;
		
		// return current state
		a_state[0] = TotWaConsumed;
		a_state[1] = TotWaUrine;
		a_state[2] = IntakeTotal;
		a_state[3] = TFCM4z;
		a_state[4] = TotDMin;
		a_state[5] = TNdfIn;
		a_state[6] = TotRumntn;
		a_state[7] = TotEating;
		a_state[8] = TotRest;
		a_state[9] = LPart;
		a_state[10] = MPart;
		a_state[11] = SPart;
		a_state[12] = HaMi;
		a_state[13] = HbMi;
		a_state[14] = Ha;
		a_state[15] = Hc;
		a_state[16] = Ce;
		a_state[17] = Hb;
		a_state[18] = Pi;
		a_state[19] = IndigFd;
		a_state[20] = Cs;
		a_state[21] = RumAa;
		a_state[22] = AM2;
		a_state[23] = As;
		a_state[24] = Fl;
		a_state[25] = RumAc;
		a_state[26] = RumPr;
		a_state[27] = RumBU;
		a_state[28] = RumLa;
		a_state[29] = Mi;
		a_state[30] = AccGEi;
		a_state[31] = AccDEi;
		a_state[32] = AccMEi;
		a_state[33] = MamCellsA;
		a_state[34] = MamCellsQ;
		a_state[35] = MamCellsS;
		a_state[36] = CumulativeLowMfDays;
		a_state[37] = WtAdipNew;
		a_state[38] = NonUterEbwTarget;
		a_state[39] = LHor1;
		a_state[40] = MilkingFrequencyLag;
		a_state[41] = KMinh;
		a_state[42] = TsAdip;
		a_state[43] = Fa;
		a_state[44] = VmAcTs2;
		a_state[45] = Ac;
		a_state[46] = MamTm;
		a_state[47] = TMilkTm;
		a_state[48] = OthDna;
		a_state[49] = VisDna;
		a_state[50] = PVis;
		a_state[51] = POth;
		a_state[52] = AA1;
		a_state[53] = BldUr1;
		a_state[54] = MamPm;
		a_state[55] = TMilkPm;
		a_state[56] = Gl;
		a_state[57] = MamLm;
		a_state[58] = TMilkLm;
		a_state[59] = MamMilkAve;
		a_state[60] = absEAve;
		a_state[61] = NurTotal;
		a_state[62] = tNep;
		a_state[63] = TCH4;
		a_state[64] = TBCH4;
		a_state[65] = TMCH4E;
		
		return( a_state );
	
	} // end get_state
	
	void set_state ( state_type a_state ) {
		
		// set state
		TotWaConsumed = a_state[0];
		TotWaUrine = a_state[1];
		IntakeTotal = a_state[2];
		TFCM4z = a_state[3];
		TotDMin = a_state[4];
		TNdfIn = a_state[5];
		TotRumntn = a_state[6];
		TotEating = a_state[7];
		TotRest = a_state[8];
		LPart = a_state[9];
		MPart = a_state[10];
		SPart = a_state[11];
		HaMi = a_state[12];
		HbMi = a_state[13];
		Ha = a_state[14];
		Hc = a_state[15];
		Ce = a_state[16];
		Hb = a_state[17];
		Pi = a_state[18];
		IndigFd = a_state[19];
		Cs = a_state[20];
		RumAa = a_state[21];
		AM2 = a_state[22];
		As = a_state[23];
		Fl = a_state[24];
		RumAc = a_state[25];
		RumPr = a_state[26];
		RumBU = a_state[27];
		RumLa = a_state[28];
		Mi = a_state[29];
		AccGEi = a_state[30];
		AccDEi = a_state[31];
		AccMEi = a_state[32];
		MamCellsA = a_state[33];
		MamCellsQ = a_state[34];
		MamCellsS = a_state[35];
		CumulativeLowMfDays = a_state[36];
		WtAdipNew = a_state[37];
		NonUterEbwTarget = a_state[38];
		LHor1 = a_state[39];
		MilkingFrequencyLag = a_state[40];
		KMinh = a_state[41];
		TsAdip = a_state[42];
		Fa = a_state[43];
		VmAcTs2 = a_state[44];
		Ac = a_state[45];
		MamTm = a_state[46];
		TMilkTm = a_state[47];
		OthDna = a_state[48];
		VisDna = a_state[49];
		PVis = a_state[50];
		POth = a_state[51];
		AA1 = a_state[52];
		BldUr1 = a_state[53];
		MamPm = a_state[54];
		TMilkPm = a_state[55];
		Gl = a_state[56];
		MamLm = a_state[57];
		TMilkLm = a_state[58];
		MamMilkAve = a_state[59];
		absEAve = a_state[60];
		NurTotal = a_state[61];
		tNep = a_state[62];
		TCH4 = a_state[63];
		TBCH4 = a_state[64];
		TMCH4E = a_state[65];
	
	} // end set_state

public:

	// unordered_map gives user efficient access to variables by name
	std::unordered_map< std::string , double > variable;
	
	// constructor head
	molly ( ) :
	
		// array initialisation list
		// warning: these are executed in the order of the member declarations
		InitCond { { { 1.0 , 550 , 3.5 , 1 , 4 } ,
			{ 2.0 , 625 , 3.5 , 5 , 4 } } } ,
		MamCellsF { { 0.0 } } , // Prefill the array with 0
		lambdaMamCellsF { { 0.0 } } , // Prefill the array with 0
		LHorPPF { { 0.0 } } , // Prefill the array with 0
		Event { {
			{ GrassNZ , 17 , 19 , 3.5 , 1.9 , 49.0 , 30.0 , 6.4 , 8.0 , 99 , 0.4555 , 1.0 } ,
			{ GrassSilage , 25 , 14.8 , 3.5 , 1.0 , 50.6 , 32.6 , 5.0 , 8.0 , 100 , 0.5555 , 1.0 } ,
			{ MaizeSilage , 33 , 9.3 , 3.5 , 20.6 , 48.6 , 29.6 , 2.3 , 4.8 , 100 , 0.5555 , 1.4 } ,
			{ TMR , 100 , 17.8 , 7.1 , 23.2 , 34.2 , 23.7 , 4.4 , 8.2 , 0 , 0.4555 , 1.0 } ,
			{ SoybeanMeal , 90 , 54.2 , 10.0 , 5.0 , 13.4 , 8.7 , 1.8 , 5.9 , 0 , 0.4555 , 0.2 } ,
			{ CerealStraw , 88 , 4 , 1.6 , 1.0 , 84.5 , 54.5 , 12.0 , 7.0 , 100 , 0.4555 , 1.4 } ,
			{ MaizeGrain , 89 , 10.7 , 3.5 , 64.8 , 16.4 , 10.3 , 2.1 , 3.2 , 0 , 0.4555 , 0.2 } ,
			{ BarleyGrain , 86 , 12.3 , 2.5 , 57.0 , 22.4 , 7.4 , 2.2 , 2.6 , 0 , 0.4555 , 0.2 } ,
			{ Lucerne , 100 , 27 , 4.0 , 1.0 , 45.0 , 28.0 , 5.0 , 10.0 , 100 , 0.4555 , 0.6 } ,
			{ LucerneSilage , 35 , 19.5 , 7.0 , 0.5 , 57.0 , 35.0 , 6.0 , 9.0 , 100 , 0.5555 , 0.6 } ,
			{ Clover , 13 , 25.1 , 3.2 , 3.8 , 47.0 , 24.0 , 4.0 , 8.1 , 100 , 0.7555 , 0.6 } } } , // Temorary "remedy" for calibrating the attraction coefficients
		SugarAndDmPercent { {
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } ,
			{ 19.0 , 34.0 , 12.0 , 10.9 , 27.4 , 33.5 , 9.8 , 10.9 } } } ,
		IngrProtComp { {
			{ GrassNZ , 55.0 , 16.7 , 0.01555 , 1e-9 } ,
			{ GrassSilage , 54.8 , 13.4 , 0.6555 , 0 } ,
			{ MaizeSilage , 57.0 , 60.2 , 4.6555 , 0 } ,
			{ TMR , 6.2 , 40.0 , 2.9555 , 0 } ,
			{ SoybeanMeal , 29.0 , 50.0 , 1.05 , 0 } , // RUP should restore to 40
			{ CerealStraw , 65.0 , 25.0 , 2.5555 , 0 } ,
			{ MaizeGrain , 32.7 , 49.0 , 1.5555 , 0 } ,
			{ BarleyGrain , 31.0 , 55.0 , 1.4555 , 0 } ,
			{ Lucerne , 52.0 , 19.0 , 2.6555 , 0 } ,
			{ LucerneSilage , 49.0 , 32.2 , 2.5555 , 0 } ,
			{ Clover , 50.8 , 35.0 , 1.5555 , 0 } } } ,
		IngrCHOComp { {
			{ GrassNZ , 30 , 10 , 60 , 99 } ,
			{ GrassSilage , 35 , 15 , 50 , 99 } ,
			{ MaizeSilage , 35 , 25 , 50 , 60 } ,
			{ TMR , 30 , 25 , 50 , 35 } ,
			{ SoybeanMeal , 10 , 25 , 50 , 1 } ,
			{ CerealStraw , 30 , 25 , 50 , 99 } ,
			{ MaizeGrain , 25 , 25 , 50 , 1 } ,
			{ BarleyGrain , 35 , 10 , 50 , 1 } ,
			{ Lucerne , 20 , 10 , 50 , 99 } ,
			{ LucerneSilage , 30 , 25 , 50 , 99 } ,
			{ Clover , 20 , 10 , 28 , 99 } } } ,
		FdBinMeshSize { { 0 , 0.03555 , 0.07555 , 0.1555 , 0.3 , 0.6 , 1.2 , 2.4 , 4.8 , 9.6 , 19.2 , 38.4 , 76.8 , 153.6 , 307.2 } } , // Unit: mm
		iBinFd { {
			{ GrassNZ , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 } ,
			{ GrassSilage , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ MaizeSilage , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ TMR , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ SoybeanMeal , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ CerealStraw , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ MaizeGrain , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ BarleyGrain , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ Lucerne , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ LucerneSilage , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } ,
			{ Clover , 0 , 10 , 10 , 10 , 10 , 10 , 10 , 10 , 0 , 0 , 0 , 0 , 0 , 0 } } } , // proprtion in each size range
		MilkingHours { { 6 , 14 , 18 , 22 } } // Fill desired milking times in hours on a 24h clock. Bad idea, change this to days, MDH?? Please no renamed variable to Hours instead (GL)
	
	// constructor body
	{
	
		// reserve buckets to minimise storage and avoid rehashing
		variable.reserve( n_visible_variables );
	
	} // end constructor
	
	void initialise_model ( double a_time ) {
		
		// initialise t
		t = a_time;
		
		// initialise model
		version = 8.9555 ; // constant version = 8.9555
		numVersion = 5.2555 ; // constant numVersion = 5.2555
		MyPi = 3.1555 ; // constant MyPi = 3.1555
		// initial 
		
		// BASIC UNITS
		// TIME IN DAYS
		// POOL SIZES IN KG(LPart,SPart,SolDM,IndigFd,Ha,Hb,Pi,As,Mi) OR
		// MOLES
		// CONCENTRATIONS IN MOLES/KG or LITER
		// MOLECULAR WEIGHTS IN KG/MOLE
		// FLUXES IN KG OR MOLES PER DAY
		
		// *********************** PHYSICAL CONSTANTS ****************************
		// MOLECULAR WEIGHTS(Mwt)in Kg/Mole
		// Fl=long chain fatty acids,Am=ammonia,Wa=water,Ch=choline
		MwtSc = 0.1555 ; MwtOa = 0.1555 ; MwtPe = 0.1555 ; MwtSt = 0.1555 ;
		MwtLiFd = 0.6555 ; MwtFl = 0.2555 ; MwtGy = 0.09555 ; MwtHc = 0.1555 ;
		MwtCe = 0.1555 ; MwtPs = 0.1555 ; MwtPi = 0.1555 ; MwtNn = 0.3555 ;
		MwtAc = 0.06555 ; MwtPr = 0.07555 ; MwtBu = 0.08555 ; MwtVa = 0.1555 ;
		MwtCh = 0.1555 ; MwtCs = 0.1555 ; MwtRumAa = 0.1555 ; MwtUr = 0.06555 ;
		MwtAm = 0.01555 ; MwtLa = 0.09555 ; MwtAs = 0.08555 ; MwtFa = 0.2555 ;
		MwtFaFd = 0.8555 ; MwtCH4 = 0.01555 ; MwtPOth = 0.1555 ; MwtN = 0.01555 ;
		MwtAa = 0.1555 ; MwtTs = 0.8555 ; MwtTm = 0.8555 ; MwtLm = 0.3555 ;
		MwtPVis = 0.1555 ;
		
		// HEATS OF COMBUSTION (Hcomb)
		// When F1 is set at 1.0, energy values are in Mcal/mole
		// when set at 4.1555,values are in MJ/mole
		F1 = 1.0 ;
		// Me=methane,Mi=microbes,Fl=unsaturated fatty acids,Fa=
		// saturated fatty acids(STEARATE).
		
		test = F1 ;
		HcombCH4 = 0.2555 * F1 ;
		HcombAc = 0.2555 * F1 ;
		HcombPr = 0.3555 * F1 ;
		HcombBu = 0.5555 * F1 ;
		HcombGl = 0.6555 * F1 ;
		HcombGy = 0.3555 * F1 ;
		HcombFl = 2.6555 * F1 ; // HcombFl is value for stearate
		HcombFa = 2.7555 * F1 ; // HcombFa=2.3555 is value for palmitate
		HcombCs = 0.6555 * F1 ;
		HcombHc = 0.5555 * F1 ;
		HcombPs = 0.6555 * F1 ;
		HcombNn = 1.2 * F1 ;
		HcombCh = 0.4 * F1 ;
		HcombUr = 0.1555 * F1 ;
		HcombOa = 0.3555 * F1 ;
		HcombLiFd = 5.2555 * F1 ;
		HcombLa = 0.3555 * F1 ;
		HcombTg = 7.5555 * F1 ;
		HcombLm = 1.3555 * F1 ;
		HcombAa = 0.6555 * F1 ;
		HcombTp = 0.3555 * F1 ;
		HcombMi = 5.3555 * F1 ;
		HcombLg = 8.3 * F1 ;
		HcombMiLi = 4.5555 * F1 ;
		// Values for HcombMi and HcombLg are in Mcal or MJ/KG
		
		MatBW = 650.0 ;
		
		// CORRECTION FACTORS FOR SHORT TERM WITHIN DAY SIMULATIONS
		// Pool sizes of Cs, RumAa, Am, Aa and Gl inflated to increase
		// solution speeds (MAXT).
		// THIS IS THE DEFAULT CONDITION OF THE MODEL
		
		CsCor = 10.0 ; RumAaCor = 10.0 ; AmCor = 10.0 ; HaCor = 1.0 ; // unit: unitless
		HbCor = 1.0 ; RumAcCor = 1.0 ; RumPrCor = 1.0 ; RumBuCor = 1.0 ; RumLaCor = 1.0 ; // unit: unitless
		LPartCor = 1.0 ; // unit: unitless
		MICor = 1.0 ; PICor = 1.0 ; IndigFdCor = 1.0 ; ASCor = 1.0 ; // unit: unitless
		FLCor = 1.0 ; MiHaCor = 1.0 ; MIHbCor = 1.0 ; // unit: unitless
		FaCor = 1.0 ; AcCor = 1.0 ; AaCor = 10.0 ; BldUrCor = 1.0 ; // unit: unitless GlCor=10.0 moved to Constants.csl (Gil, Apr 14) to enable Mindy to overwrite it to 7.0
		HcCor = 1.0 ; CeCor = 1.0 ; // unit: unitless
		
		// BEGIN  INCLUDE 'Constants.csl'
		
		// Constants that need to differ in Mindy can be put here
		
		GlCor = 10.0 ;
		
		// END  INCLUDE 'Constants.csl'
		
		// **************** FEED DESCRIPTION **********************
		
		// must be 5 for the current structure of the initCond Array
		// must be 11 to satisfy default initialisation in proximate_expand_in_init.csl
		// must be at least 2 to satisfy default initialisation of InitCond array 30 lines below
		
		McalToMJ = 4.1555 ;
		// GIL added May 2012 after merge, to initialize some variables values during day 1
		dMilkVol = 0 ;
		dMilkProd = 0 ;
		MilkProdDiel = 0 ;
		dLmProd = 0 ;
		dPmProd = 0 ;
		dTmProd = 0 ;
		TVolMilk = 0 ;
		TVolMilkYest = 0 ;
		dNep = 0.0 ;
		tNep = 0.0 ;
		tNepYest = 0.0 ;
		MastJawMoveBolus = 0.0 ;
		
		Rest = 0.0 ;
		STFLAG = 0.0 ;
		IntakeYest = -15.0 ; // so the DailySummary , that is now scheduled on T = 0 as well, will do IntakeDay=IntakeTotal-IntakeYest properly
		TransitSW = 0.0 ;
		FdDMIn = 1e-9 ;
		
		DrnkWa = 0.0 ;
		DrnkWaTot = 0.0 ;
		DrnkWaDiel = 0.0 ;
		DrnkWaYest = 0.0 ;
		DrinkSW = 0.0 ;
		
		WaUrineYest = 0.0 ;
		
		UrinationCount = 0.0 ;
		UrinationCountYest = 0.0 ;
		UrinationCountDiel = 0.0 ;
		
		UrinationVol = 0.0 ;
		UrinationVolYest = 0.0 ;
		UrinationVolDiel = 0.0 ;
		
		// Reference ruminal Degradation Rates used to calculate actual rates
		iKHaCs = 11.02555 ; refIngrKHaCs = 1.3555 ;
		iKCeCs = 8.9555 ; iKHcCs = 6.1555 ; refIngrKAdfDeg = 0.7555 ;
		iKPiAa = 1.7555 ; refIngrKPiAa = 3.8 ; // Reference value for in situ rate scaling
		// KPiAa is a function of in situ degradation rates
		
		ilogNewEvent = true ;
		logNewEvent = ilogNewEvent ; // Set flag to calculate Nutrient inputs to the model
		// Flag is toggled in Criteria and Ingredient sections
		
		// Arrays are loaded row 1 to n within column 1 followed by row 1
		// to n within column 2 followed by row ... within column z
		
		// BEGIN  INCLUDE 'Experimental_Bias_Vectors_in_init.csl'
		
		// Not needed in this project
		// END  INCLUDE 'Experimental_Bias_Vectors_in_init.csl'
		
		// ****************** InitCond - Initial Animal Settings ************************
		iAnimal = 1.0 ;
		
		Animal = iAnimal ;
		
		iDayOfYear = 252.0 ; // Day of the year
		// 1 Jan=1, 2 Jan=2,..,1 June=152. CCP 8-24-06
		DayofYear = iDayOfYear ;
		
		iParity = 2.0 ;
		
		Parity = iParity ;
		if ( iParity > 2 ) Parity = 2 ;
		if ( iParity < 1 ) Parity = 1 ;
		
		// !!!!!!!!!!!!!! Initialize Gestation and Lactation !!!!!!!!! Gil Feb / 2010 !!!!!!!
		// ConceiveNow, DryOffNow are hooks for external (m file / WFM) control over Molly.
		// If these constants are left as 0.0 (Standalone) Molly works the old way:
		// Conceive <DaysOpen> after  last calving
		// Dry Off  <DaysDry>  before next calving
		
		GestLength = 283 ; DaysOpen = 82 ; DaysDry = 82 ;
		StartDayGest = 230 ;
		StartDIM = 0 ;
		ConceiveNow = 0.0 ; DryOffNow = 0.0 ; AbortPregNow = 0.0 ;
		iAgeInYears = 5.0 ;
		WtConcBreedFactor = 1.0 ; // 1.0 keeps MDH calibration. Use 0.7555 for New Zealand Frisian, 0.4555 for Jersey. DairyNZ data
		BCSBase = 3.0 ; // US baseline, for DNZ lactaion model (LHOR effect). See also BCS target that varies by DayMilk
		
		AgeInYears = iAgeInYears ;
		WtConcAgeFactor = 1.0 - 0.02555 * ( 4.001 - min ( 4.0 , iAgeInYears ) ) *pow(1,1)* 2.3555 ; // GL. This yields 0.8555 for 2 years old; 0.9555 for 3 & 1.0 for 4+ Based on DairyNZ data
		iStartDayGest = StartDayGest ;
		iDayGestDMI = iStartDayGest ; // used for iFdRat calculations
		iStartDIM = StartDIM ;
		
		ConceiveNowVariable = 0 ;
		DryOffNowVariable = 0 ;
		AbortPregNowVariable = 0 ;
		
		if ( iStartDIM < 0 ) { // Old style init with negative startDIM
			iStartDayGest = GestLength + iStartDIM ;
		}
		
		if ( iStartDIM < iStartDayGest ) {
			iStartDIM = iStartDayGest - GestLength ; // dry about to calve => countdown
			// else: Late lactation early preg. Not really used as she does not start nicely inMilk so pre-run from pre-calving is used instead
		}
		
		if ( iStartDayGest >= 0 ) {
			tAtConception = ( -1 ) * iStartDayGest ;
			DayGestBasic = iStartDayGest ;
			DayGest = max ( DayGestBasic , -300. ) ; // This prevents undeflows in some preg equations while she is ompty.
			
		} else {
			tAtConception = 2999.0 ;
		}
		
		if ( iStartDIM >= 0 ) {
			tAtCalving = ( -1 ) * iStartDIM ;
			DayMilk = iStartDIM ;
		} else {
			tAtCalving = 2999.0 ;
			DayMilk = t - ( tAtConception + GestLength ) ;
		}
		
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		fHeifers = .3 ; // Used for determining udder capacity and DMI
		
		// The InitCond array = Animal ibw iBCS EventStartPos NumberofEvents
		
		FirstEvent = InitCond [ Animal - 1 ][ 4 - 1 ] ;
		
		LastEvent = FirstEvent + ( InitCond [ Animal - 1 ][ 5 - 1 ] -1 ) ;
		CurrentEvent = FirstEvent ;
		PreviousEvent = FirstEvent ;
		
		iBW = InitCond [ Animal - 1 ][ 2 - 1 ] ;
		iBCS = InitCond [ Animal - 1 ][ 3 - 1 ] ;
		
		// ******* Genetic Scalars ******
		iHerd = 1.0 ;
		
		Herd = iHerd ;
		// Array to hold Herd Adjustment factors for MamCellsPart
		// Prefill the array with 0
		// Array to hold Herd Adjustment factors for MamCellsPart
		// Prefill the array with 0
		// Array to hold Herd Adjustment factors for LHor PP
		// Prefill the array with 0
		
		// ********** EventCriteria - Evaluation Criteria for executing an event ************
		// EventCriteria Code used only by Mark,  moved to currentEvent.csl
		
		// The include files and statements right below are NO LONGER IN USE. I LEFT THEM HERE TO REDUCE DIFF WITH MARK
		// Use an Include statement here for Nutrient input schemes
		// Classic Input Scheme
		// INCLUDE 'Molly_Classic_Nutr_In_Init.csl'
		// INCLUDE 'Molly_Classic_Nutr2_Init_PG.csl'
		// Proximate Input Scheme
		// INCLUDE 'Molly_Proximate_In_Init.csl'
		// INCLUDE 'Molly_ProximateExpand_In_Init_5x.csl'
		// Include Ingredient Input Scheme
		// INCLUDE 'Molly5t_Ingredient_In_Init.csl'
		// !!!!!!! Gil Aug 2011 gestation init is now here, after i moved it up
		// because DairyNZ's iFdRat calculations requires WtGvdUter
		// Gestation settings and Initial Gravid Uteris weight, kg
		// This submodel is from the grvuter submodel based on Ferrel model fitted to Bell data.
		fPUter = .1327 ; // g CP/g wet weight, mean from Ferrell et al., 1976
		iWtUter = .204 ; kUterSyn = 0.01555 ;
		kUterSynDecay = 0.00002555 ; kUterDeg = 0.2 ;
		iWtConc = 0.4555 ; kConcSyn = 0.02555 ; kConcSynDecay = 0.00002555 ;
		iWtPConc = .00077 ; kPConcSyn = 0.05555 ; kPConcSynDecay = 0.00007555 ;
		
		// Pregnancy switches and Day in Gestation (DayGest) calculation
		Preg = 0 ;
		NonPreg = 1 ;
		if ( DayGest > 0 ) {
			Preg = 1 ;
			NonPreg = 0 ;
		}
		
		WtUterPart = iWtUter * exp ( ( kUterSyn - kUterSynDecay * GestLength ) * GestLength ) ;
		// Breed and Age Factors added by Gil, 2010. Need to document on the next publication, MDH
		// The Mod function below was added by Gil to avoid a problem with large
		// uterus for an empty cow or steer.  See Gil's comment above ~8.4, MDH
		WtConcAgeFactor = 1.0 - 0.02555 * ( 4.001 - min ( 4.0 , iAgeInYears ) ) *pow(1,1)* 2.3555 ; // GL. This yields 0.8555 for 2 years old; 0.9555 for 3 & 1.0 for 4+ Based on DairyNZ data
		WtUter = WtConcBreedFactor * WtConcAgeFactor *
			( ( iWtUter * exp ( ( kUterSyn - kUterSynDecay * DayGest ) * DayGest ) ) * Preg
			+ ( ( WtUterPart - iWtUter ) * exp ( - kUterDeg * fmod ( DayMilk +3000.0 , 3000.0 ) ) + iWtUter ) * NonPreg ) ;
		
		WtPUter = WtUter * fPUter ; // kg protein in uterus
		PUter = WtPUter / MwtPVis ;
		WtConc = WtConcBreedFactor * WtConcAgeFactor *
			iWtConc * exp ( ( kConcSyn - kConcSynDecay * DayGest ) * DayGest ) ;
		WtGrvUter = WtConc + WtUter ;
		
		// ** Include Proximate Nutrient inputs in the Initial Section
		// ** Added feed particle size description
		
		// ********** Nutrients in Ration ******************
		// BEGIN  INCLUDE '..\Molly_ProximateExpand_In_Init.csl'   !Take from the shared root folder, all projects are currently using ProximateExpand
		// ** Include Proximate Nutrient inputs in the Initial Section
		// ********** Nutrients in Ration ******************
		// Define the nutrient input arrays
		
		fEndogLiFd = .0275 ; // Assumed that endogenous fat = 2.7555% of DM
		fAiFdBase = 0.01555 ; // Average proportion of ADF Ash. Gil Sep 2014 made it diurnally dynamic in Mindy
		afAiFd = fAiFdBase ;
		
		SecondsPerDay = 86400 ; // 24 * 60 * 60
		
		CurrentFeed = 1 ;
		CurrentSupplement = 0 ;
		
		// The following are parameter definitions for the nutrients used in Event array references
		// This allows changes in the array order without having to change all the subsequent code
		
		// Nutrient positions in the Event arrray
		
		// Sugar and Dry Matter positions in the SugarAndDmPercent array
		
		NumberOfFeeds = 11 ; // Must be smaller than MaxEvents, Event array used up to its name by MDH but as a Feed Array by DairyNZ
		
		// Calculated in a complicated way, as 'the rest' after accounting for all the others..
		// Calculated as NDF - ADF
		// Calculated as PSF * NDF
		
		// Event(FeedName DM CP Fat StarchplusSugars NDF ADF Lignin Ash Roughage Psf)
		// The first event is a standard grass diet for DairyNZ
		// from Bryant et al., An Fd Sci Tech., 2012, Samson cultivar, morning composition
		// FeedName     DM    CP   Fat    St  NDF   ADF   Lg   Ash Roug  Psf EaseOfBreakdown
		
			// Temorary "remedy" for calibrating the attraction coefficients
		
		// dmSummerSunrise #dmSummerSunset #dmWinterSunrise #dmWinterSunset #scSummerSunrise #scSummerSunset #scWinterSunrise #scWinterSunset)
		
		// Nutrient positions for the IngrProtComp array
		
		// Parameter(FeedName=1) defined above
		// This includes NPN, all in CP equivalents
		
		// this is NPN-CP, i.e. CP equivalents and includes the urea
		// This is also expressed as CP equivalents
		
		// Protein Composition, % of CP in CP equivalents
		
			// RUP should restore to 40
			
		// CHO composition, % of parent CHO
		// Nutrient positions for the IngrCHOComp array
		
		// Parameter(FeedName=1) defined above
		
		// Initialize some calculated freactions that we need to have for all feeds not just the current one so
		// Mindy will be able to asses a food for attraction before she switches to it
		// with some more work all fd varibales can be cached in this array, so when Molly switches feed
		// the new values are readily avaialable.
		
		// procedural ( ) 
		for ( int i = 1 ; i <= NumberOfFeeds ; ++ i ) { // do AnotherLoop i = 1,NumberOfFeeds
			CurrentFeed = i ;
			// BEGIN  Include '../Molly_ProximateExpand_Conversion.csl' ! the included code converts the Ingr* to f*Fd
			
			// This is the body of the procedural from proximateExpand in deriv
			
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			
			fAiFd = fAiFdBase ; // average insoluble Ash contents Gil Sep 2014 this is no longer a contsant
			
			//  Feed Nitrogen
			fCPFd = Event [ CurrentFeed - 1 ][ IngrCP - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
			// fCPFd=Event(IngrCP,CurrentFeed)/100;	!Original equation not corrected for bias in the input data
			FCPsFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrCPs - 1 ] / 100 ; // includes NPN sources
			FUrFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrUr - 1 ] / 100 / 2.8555 ; // IngrUr is in CP equivalents. Changed to urea mass here
			FNPNFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrNPN - 1 ] / 100 ; // CP equivalents including that from urea
			FRUPFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrRUP - 1 ] / 100 ;
			FPsFd = FCPsFd - FNPNFd ;
			fPiFd = fCPFd - FCPsFd ;
			if ( fPiFd <= 0 ) { // This should not happen, but just in case a bad number gets entered
				FCPsFd = FCPsFd + fPiFd ;
				fPiFd = 1e-12 ;
			}
			fNnFd = FNPNFd - ( FUrFd * 2.8555 ) ; // Nn mass, % of DM, assumes a C to N ratio the same as CP which is correct for nucleic acids
			if ( fNnFd < 0 ) fNnFd = 0 ;
			
			// Feed Lipid
			FCFatFd = Event [ CurrentFeed - 1 ][ IngrFat - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
			// 	fCFatFd=Event(IngrFat,CurrentFeed) / 100
			
			fLiFd = fEndogLiFd ;
			fFatFd = FCFatFd - fEndogLiFd ;
			
			// Feed Ash
			fAshFd = Event [ CurrentFeed - 1 ][ IngrAsh - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
			// 	fAshFd=Event(IngrAsh,CurrentFeed)/100
			fAsFd = fAshFd - fAiFd ;
			
			// Feed Carbohydrate
			fNDFFd = Event [ CurrentFeed - 1 ][ IngrNDF - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
			fADFFd = Event [ CurrentFeed - 1 ][ IngrADF - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
			// 	fNdfFd=Event(IngrNDF,CurrentFeed)/100
			// 	fAdfFd=Event(IngrADF,CurrentFeed)/100
			if ( fADFFd == 0 ) fADFFd = 1e-12 ;
			fRuAdfFd = fADFFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrRUAdf - 1 ] / 100 ;
			if ( fRuAdfFd > fADFFd ) fRuAdfFd = fADFFd -1e-12 ;
			
			fLgFd = Event [ CurrentFeed - 1 ][ IngrLg - 1 ] / 100 ;
			fHcFd = fNDFFd - fADFFd ;
			fCeFd = fADFFd - fLgFd - fAiFd ;
			fOmFd = 1.0 - fAshFd ;
			
			fStFd = Event [ CurrentFeed - 1 ][ IngrSt - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
			// 	fStFd=Event(IngrSt,CurrentFeed) / 100
			if ( fStFd <= 0 ) fStFd = 1.0e-12 ;
			FStsFd = fStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrStS - 1 ] / 100 ;
			if ( FStsFd <= 0 ) FStsFd = 0.0 ;
			fRUStFd = fStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrRUSt - 1 ] / 100 ;
			if ( fRUStFd > fStFd ) fRUStFd = fStFd -1e-12 ;
			
			if ( fFatFd < 0 ) { // trap negative values for fFatFd
				fFatFd = 0 ;
				fLiFd = ( Event [ CurrentFeed - 1 ][ IngrFat - 1 ] / 100 ) ;
			}
			
			PartFd = fPiFd + FPsFd + fNnFd + FUrFd + fAcFd + FLaFd + fBuFd + fPeFd + fOaFd +
				fHcFd + fCeFd + fLgFd + fLiFd + fFatFd + fAsFd + fAiFd ;
			
			// Calculates fSCFd and fStFd and traps negative added fat values
			fScFd = 1 - PartFd - fStFd ;
			if ( fScFd < 0 ) {
				if ( fStFd > - fScFd ) {
					fStFd = fStFd + fScFd ;
					FStsFd = fStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrStS - 1 ] / 100 ;
					fRUStFd = fRUStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrRUSt - 1 ] / 100 ;
					fScFd = 0 ;
				} else {
					fStFd = 1.0e-12 ;
					FStsFd = 0.0 ;
					fRUStFd = 1.0e-13 ;
					fScFd = 0 ;
				}
			}
			StSol = FStsFd / fStFd ;
			SpeciesFactor = Event [ CurrentFeed - 1 ][ IngrEaseOfBreakdown - 1 ] ;
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// END  Include '../Molly_ProximateExpand_Conversion.csl' ! the included code converts the Ingr* to f*Fd
			Ingr [ i - 1 ][ IngrSc - 1 ] = fScFd * 100.0 ;
			Ingr [ i - 1 ][ IngrHc - 1 ] = fHcFd * 100.0 ;
			Ingr [ i - 1 ][ IngrPeNdf - 1 ] = Event [ i - 1 ][ IngrNDF - 1 ] * ( 1.0 - Event [ i - 1 ][ IngrPsf - 1 ] ) ;
		} // AnotherLoop: Continue
		CurrentEvent = 1 ;
		// end of procedural 
		
		// Particle size description of the feed inputs
		
		// Allow 14 screens, Unit: mm
		// Name + 14 screens
		// Unit: mm  Standard bins min values (As described in 2.2.1 Gregorini 2014)
		// Unit: fraction (all the array elements sum up to 1.0). Proprtions of particles in each size range.
		// Unit: fraction (all the array elements sum up to 1.0). Proprtions of particles in each size range. working array for the chewing model
		// Unit: percent (all the array elements sum up to 100.0). Percentages of particles in each size range. Table of all feeds before ingestion
		
		// Standard bins including the 1.2 and 4.8 thresholds. It is a working array for the particle-size-reduction-by-mastication part of the chewing model
		// Unit: mm
		
			// proprtion in each size range
		
		LPartSize = 4.8 ; // Min size of the ruminal LPart pool, Unit: mm
		MPartSize = 1.2 ; // Min size of the ruminal MPart pool, Unit: mm
		
		// ** End of the Include Section
		// END  INCLUDE '..\Molly_ProximateExpand_In_Init.csl'   !Take from the shared root folder, all projects are currently using ProximateExpand
		
		// ******************* Initial Feed Rate (fdrat) *************************
		// BEGIN  INCLUDE 'FdRat_Constants.csl'
		
		CappingForIntake = 0.04555 ; // NOT USED BY DO NOT DELETE. OVERWRITTEN BY breedAndAge.csv setting. We nneed it here so it enters the automatically generate cowParameters file, to become optimsable. Used in Smalltalk / WFM
		IntakeVersion = 8.7555 ; // Gil's required intake model April 2011 replacing RFEED4 etc.
		GrowthPerDay = 0 ; // WFM sets it differently for 2 - 5 years old, breed & age dependent.
		OnceADayMilkingAdjustment = 0.8555 ; // DairyNZ observed ratio of 1x production vs 2x whole season. This will affect EnergyForMilk, while the cow is on MF other than twice a day
		OnceADay2YearsOldAdjustment = 0.7555 ; // DairyNZ Observed (Friesian) 2 years old production vs mature on once a day. (For Jersey this is set to 0.9555)
		IntakeDeclineSlope = 0.002555 ; // Decline in intake after DIM 110, as proportion of MaxFoodForMilk. WFM sets it diffrently for Jerseys and once-a-day cows
		EnergyForDryCowMaintenancePower = 0.7555 ; // Edith's parametrisation March 2012. Multiplies NonUterEbwTarget^0.7 gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively
		EnergyForDryCowMaintenanceFactor = 1.03555 ; // Edith's parametrisation March 2012. Multiplies NonUterEbwTarget^p gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively (p = EnergyForMaintenancePower)
		EnergyForMilkingCowMaintenancePower = 0.7 ; // Multiplies NonUterEbwTarget^0.7 gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively
		EnergyForMilkingCowMaintenanceFactor = 1.6555 ; // Multiplies NonUterEbwTarget^p gives 2.01%/1.9555% of NonUterEbwTarget for a 550/600kg cow respectively (p = EnergyForMaintenancePower)
		EnergyForMilkFactor = 0.1555 ; // Multiplier of (MilkSolids270 + MilkSolids270a) / 2
		EnergyForMilkPower = 3.0 ; // WFM sets it to 4.5 for Jerseys and once-a-day cows
		EnergyForPregnancyFactor = 0.03555 ; // Edith's parametrisation March 2012. Multiplier of WtGrUter
		PeakIntakeDay = 77 ; // WFM sets it to 108 for Jerseys and once-a-day cows
		kEnergyCompensation = 0.5555 ; // 0.5 will roughly give 1% extra / less required intake for every 2% LW difference (reverse relationship! low conditin cows eat more!)
		xOadIntakeTadIntake = 0.1555 ; // OAD (once a day) cows eat more compared to their production. This parameter describes how close their requirement would to twice a day situation. If set to 1, they will require the same (in spite of their lower production) if set to 0, they will require less energy for milk in direct proprtion to their reduced production. Middle values will reflect the reality in which they eat more compared to their production, and thus gain condition.
		SmoothingPeriodDays = 20 ; // Number of days after PeakIntakeDay, in which the transitional curve takes effect
		FeedInFlag = 0.0 ; // Set to 1.0 for external feeding (m file / WFM)
		FdRatWFM = 20.0 ; // Set to desired daily intake external feeding (m file / WFM)
		
		// Eneregy for milk at peak intake
		
		// END  INCLUDE 'FdRat_Constants.csl'
		// BEGIN  INCLUDE 'FdRat_Init.csl'
		
		// iterate to get close approximation of the mutualy dependent iFdRat <=> iNotUterEbwTarget
		
		iFdRat = 10 ;
		
		NonUterEBW = iBW - iFdRat * ( 4.4 + 1 ) - WtGrvUter ;
		NonUterEbwTarget = NonUterEBW - 36.0 * ( iBCS - BCSBase ) ;
		// BEGIN  INCLUDE 'FdRat_deriv.csl'
		
		// Mindy sets feedInFlag to 2, but still she must not have the if statement
		// below even though it is inactive it messes things up
		// Hence the separation deriv (molly) vs. deriv_basic (mindy need RequiredEnergy too)
		
		// BEGIN  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		// procedural ( MaxEnergyForMilk = MilkSolids270MfAdjusted ) 
		
		MaxEnergyForMilk = EnergyForMilkFactor *
			( ( 1.0 - xOadIntakeTadIntake ) * MilkSolids270MfAdjusted +
			xOadIntakeTadIntake * KgMilkSolidsExpectedIn270Days ) ; // Once a days seem toeat more compared to their production.
		// end of procedural 
		
		// procedural ( RequiredEnergy , FdCapMolly = DayMilk , NonUterEbwTarget , NonUterEBW ) 
		
		EnergyForActivity = 0 ; // (ActEnergyReq / MjoulesToATPConv / MEinMJ) / (0.02 * (17.0 * (MEinMJ/3.6/4.1555) - 2.0) + 0.5)
		EnergyForPregnancy = EnergyForPregnancyFactor * 11.5 * WtGrvUter ; //
		EnergyForGrowth = GrowthPerDay * 50.0 ; // Assuming 50MJ required for 1kg LW gain
		EnergyCompensation = ( NonUterEbwTarget / NonUterEBW ) *pow(1,1)* kEnergyCompensation ; // Any extra/deficit in condition would decrease/increase intake, trying to simulate the extra hunger of low condition cows, and lower insentive to eat for fat cows
		// ** 0.5 gives approx 1% more food for every 2% LW deficit in the non extreme range, and vice versa. ** 1 would give 1:1 relationship.
		if ( DayMilk <= 0 ) { // Dry Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForDryCowMaintenancePower * EnergyForDryCowMaintenanceFactor ;
			EnergyForMilk = 0 ;
		} else { // Milking Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForMilkingCowMaintenancePower * EnergyForMilkingCowMaintenanceFactor ;
			if ( DayMilk <= PeakIntakeDay ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - ( 1.0 - DayMilk / PeakIntakeDay ) *pow(1,1)* EnergyForMilkPower ) ;
			} else if ( DayMilk <= ( PeakIntakeDay + SmoothingPeriodDays ) ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 ) * ( ( DayMilk - PeakIntakeDay ) / SmoothingPeriodDays ) *pow(1,1)* 2.0 ) ;
			} else {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 + ( DayMilk - ( PeakIntakeDay + SmoothingPeriodDays ) ) ) ) ;
			}
		}
		
		RequiredEnergy = ( EnergyForMaintenance // Molly's required intake, in MJ
			+ EnergyForMilk //
			+ EnergyForPregnancy //
			+ EnergyForGrowth //
			+ EnergyForActivity ) // Currently this one zero, avarage activity assumed and bundled in rEnergeyForMaintenance
			* EnergyCompensation ; // Smaller than one for fat cows, largerthan 1 for skinny, on the long run brings cows that differ only in condition, to a similar state.
		//
		FdCapMolly = NonUterEbwTarget * CappingForIntake ; // in kgDM actual, WFM will not feed her more forage (pasture & silage) then this amount, but may feed things like grains on top of that
		//
		// end of procedural 
		
		// END  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		if ( FeedInFlag == 1.0 ) { //
			FdRat = FdRatWFM ; // Allocated feed - m files or WFM
		} else { //
			FdRat = RequiredEnergy / 11.5 ; // Fully fed automatically (standalone)
		}
		
		// END  INCLUDE 'FdRat_deriv.csl'
		iFdRat = RequiredEnergy / 11.5 ;
		fd1 = iFdRat ;
		
		NonUterEBW = iBW - iFdRat * ( 4.4 + 1 ) - WtGrvUter ;
		NonUterEbwTarget = NonUterEBW - 36.0 * ( iBCS - BCSBase ) ;
		// BEGIN  INCLUDE 'FdRat_deriv.csl'
		
		// Mindy sets feedInFlag to 2, but still she must not have the if statement
		// below even though it is inactive it messes things up
		// Hence the separation deriv (molly) vs. deriv_basic (mindy need RequiredEnergy too)
		
		// BEGIN  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		// procedural ( MaxEnergyForMilk = MilkSolids270MfAdjusted ) 
		
		MaxEnergyForMilk = EnergyForMilkFactor *
			( ( 1.0 - xOadIntakeTadIntake ) * MilkSolids270MfAdjusted +
			xOadIntakeTadIntake * KgMilkSolidsExpectedIn270Days ) ; // Once a days seem toeat more compared to their production.
		// end of procedural 
		
		// procedural ( RequiredEnergy , FdCapMolly = DayMilk , NonUterEbwTarget , NonUterEBW ) 
		
		EnergyForActivity = 0 ; // (ActEnergyReq / MjoulesToATPConv / MEinMJ) / (0.02 * (17.0 * (MEinMJ/3.6/4.1555) - 2.0) + 0.5)
		EnergyForPregnancy = EnergyForPregnancyFactor * 11.5 * WtGrvUter ; //
		EnergyForGrowth = GrowthPerDay * 50.0 ; // Assuming 50MJ required for 1kg LW gain
		EnergyCompensation = ( NonUterEbwTarget / NonUterEBW ) *pow(1,1)* kEnergyCompensation ; // Any extra/deficit in condition would decrease/increase intake, trying to simulate the extra hunger of low condition cows, and lower insentive to eat for fat cows
		// ** 0.5 gives approx 1% more food for every 2% LW deficit in the non extreme range, and vice versa. ** 1 would give 1:1 relationship.
		if ( DayMilk <= 0 ) { // Dry Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForDryCowMaintenancePower * EnergyForDryCowMaintenanceFactor ;
			EnergyForMilk = 0 ;
		} else { // Milking Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForMilkingCowMaintenancePower * EnergyForMilkingCowMaintenanceFactor ;
			if ( DayMilk <= PeakIntakeDay ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - ( 1.0 - DayMilk / PeakIntakeDay ) *pow(1,1)* EnergyForMilkPower ) ;
			} else if ( DayMilk <= ( PeakIntakeDay + SmoothingPeriodDays ) ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 ) * ( ( DayMilk - PeakIntakeDay ) / SmoothingPeriodDays ) *pow(1,1)* 2.0 ) ;
			} else {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 + ( DayMilk - ( PeakIntakeDay + SmoothingPeriodDays ) ) ) ) ;
			}
		}
		
		RequiredEnergy = ( EnergyForMaintenance // Molly's required intake, in MJ
			+ EnergyForMilk //
			+ EnergyForPregnancy //
			+ EnergyForGrowth //
			+ EnergyForActivity ) // Currently this one zero, avarage activity assumed and bundled in rEnergeyForMaintenance
			* EnergyCompensation ; // Smaller than one for fat cows, largerthan 1 for skinny, on the long run brings cows that differ only in condition, to a similar state.
		//
		FdCapMolly = NonUterEbwTarget * CappingForIntake ; // in kgDM actual, WFM will not feed her more forage (pasture & silage) then this amount, but may feed things like grains on top of that
		//
		// end of procedural 
		
		// END  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		if ( FeedInFlag == 1.0 ) { //
			FdRat = FdRatWFM ; // Allocated feed - m files or WFM
		} else { //
			FdRat = RequiredEnergy / 11.5 ; // Fully fed automatically (standalone)
		}
		
		// END  INCLUDE 'FdRat_deriv.csl'
		iFdRat = RequiredEnergy / 11.5 ;
		fd2 = iFdRat ;
		
		NonUterEBW = iBW - iFdRat * ( 4.4 + 1 ) - WtGrvUter ;
		NonUterEbwTarget = NonUterEBW - 36.0 * ( iBCS - BCSBase ) ;
		// BEGIN  INCLUDE 'FdRat_deriv.csl'
		
		// Mindy sets feedInFlag to 2, but still she must not have the if statement
		// below even though it is inactive it messes things up
		// Hence the separation deriv (molly) vs. deriv_basic (mindy need RequiredEnergy too)
		
		// BEGIN  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		// procedural ( MaxEnergyForMilk = MilkSolids270MfAdjusted ) 
		
		MaxEnergyForMilk = EnergyForMilkFactor *
			( ( 1.0 - xOadIntakeTadIntake ) * MilkSolids270MfAdjusted +
			xOadIntakeTadIntake * KgMilkSolidsExpectedIn270Days ) ; // Once a days seem toeat more compared to their production.
		// end of procedural 
		
		// procedural ( RequiredEnergy , FdCapMolly = DayMilk , NonUterEbwTarget , NonUterEBW ) 
		
		EnergyForActivity = 0 ; // (ActEnergyReq / MjoulesToATPConv / MEinMJ) / (0.02 * (17.0 * (MEinMJ/3.6/4.1555) - 2.0) + 0.5)
		EnergyForPregnancy = EnergyForPregnancyFactor * 11.5 * WtGrvUter ; //
		EnergyForGrowth = GrowthPerDay * 50.0 ; // Assuming 50MJ required for 1kg LW gain
		EnergyCompensation = ( NonUterEbwTarget / NonUterEBW ) *pow(1,1)* kEnergyCompensation ; // Any extra/deficit in condition would decrease/increase intake, trying to simulate the extra hunger of low condition cows, and lower insentive to eat for fat cows
		// ** 0.5 gives approx 1% more food for every 2% LW deficit in the non extreme range, and vice versa. ** 1 would give 1:1 relationship.
		if ( DayMilk <= 0 ) { // Dry Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForDryCowMaintenancePower * EnergyForDryCowMaintenanceFactor ;
			EnergyForMilk = 0 ;
		} else { // Milking Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForMilkingCowMaintenancePower * EnergyForMilkingCowMaintenanceFactor ;
			if ( DayMilk <= PeakIntakeDay ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - ( 1.0 - DayMilk / PeakIntakeDay ) *pow(1,1)* EnergyForMilkPower ) ;
			} else if ( DayMilk <= ( PeakIntakeDay + SmoothingPeriodDays ) ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 ) * ( ( DayMilk - PeakIntakeDay ) / SmoothingPeriodDays ) *pow(1,1)* 2.0 ) ;
			} else {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 + ( DayMilk - ( PeakIntakeDay + SmoothingPeriodDays ) ) ) ) ;
			}
		}
		
		RequiredEnergy = ( EnergyForMaintenance // Molly's required intake, in MJ
			+ EnergyForMilk //
			+ EnergyForPregnancy //
			+ EnergyForGrowth //
			+ EnergyForActivity ) // Currently this one zero, avarage activity assumed and bundled in rEnergeyForMaintenance
			* EnergyCompensation ; // Smaller than one for fat cows, largerthan 1 for skinny, on the long run brings cows that differ only in condition, to a similar state.
		//
		FdCapMolly = NonUterEbwTarget * CappingForIntake ; // in kgDM actual, WFM will not feed her more forage (pasture & silage) then this amount, but may feed things like grains on top of that
		//
		// end of procedural 
		
		// END  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		if ( FeedInFlag == 1.0 ) { //
			FdRat = FdRatWFM ; // Allocated feed - m files or WFM
		} else { //
			FdRat = RequiredEnergy / 11.5 ; // Fully fed automatically (standalone)
		}
		
		// END  INCLUDE 'FdRat_deriv.csl'
		iFdRat = RequiredEnergy / 11.5 ;
		fd3 = iFdRat ;
		
		NonUterEBW = iBW - iFdRat * ( 4.4 + 1 ) - WtGrvUter ;
		NonUterEbwTarget = NonUterEBW - 36.0 * ( iBCS - BCSBase ) ;
		// BEGIN  INCLUDE 'FdRat_deriv.csl'
		
		// Mindy sets feedInFlag to 2, but still she must not have the if statement
		// below even though it is inactive it messes things up
		// Hence the separation deriv (molly) vs. deriv_basic (mindy need RequiredEnergy too)
		
		// BEGIN  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		// procedural ( MaxEnergyForMilk = MilkSolids270MfAdjusted ) 
		
		MaxEnergyForMilk = EnergyForMilkFactor *
			( ( 1.0 - xOadIntakeTadIntake ) * MilkSolids270MfAdjusted +
			xOadIntakeTadIntake * KgMilkSolidsExpectedIn270Days ) ; // Once a days seem toeat more compared to their production.
		// end of procedural 
		
		// procedural ( RequiredEnergy , FdCapMolly = DayMilk , NonUterEbwTarget , NonUterEBW ) 
		
		EnergyForActivity = 0 ; // (ActEnergyReq / MjoulesToATPConv / MEinMJ) / (0.02 * (17.0 * (MEinMJ/3.6/4.1555) - 2.0) + 0.5)
		EnergyForPregnancy = EnergyForPregnancyFactor * 11.5 * WtGrvUter ; //
		EnergyForGrowth = GrowthPerDay * 50.0 ; // Assuming 50MJ required for 1kg LW gain
		EnergyCompensation = ( NonUterEbwTarget / NonUterEBW ) *pow(1,1)* kEnergyCompensation ; // Any extra/deficit in condition would decrease/increase intake, trying to simulate the extra hunger of low condition cows, and lower insentive to eat for fat cows
		// ** 0.5 gives approx 1% more food for every 2% LW deficit in the non extreme range, and vice versa. ** 1 would give 1:1 relationship.
		if ( DayMilk <= 0 ) { // Dry Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForDryCowMaintenancePower * EnergyForDryCowMaintenanceFactor ;
			EnergyForMilk = 0 ;
		} else { // Milking Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForMilkingCowMaintenancePower * EnergyForMilkingCowMaintenanceFactor ;
			if ( DayMilk <= PeakIntakeDay ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - ( 1.0 - DayMilk / PeakIntakeDay ) *pow(1,1)* EnergyForMilkPower ) ;
			} else if ( DayMilk <= ( PeakIntakeDay + SmoothingPeriodDays ) ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 ) * ( ( DayMilk - PeakIntakeDay ) / SmoothingPeriodDays ) *pow(1,1)* 2.0 ) ;
			} else {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 + ( DayMilk - ( PeakIntakeDay + SmoothingPeriodDays ) ) ) ) ;
			}
		}
		
		RequiredEnergy = ( EnergyForMaintenance // Molly's required intake, in MJ
			+ EnergyForMilk //
			+ EnergyForPregnancy //
			+ EnergyForGrowth //
			+ EnergyForActivity ) // Currently this one zero, avarage activity assumed and bundled in rEnergeyForMaintenance
			* EnergyCompensation ; // Smaller than one for fat cows, largerthan 1 for skinny, on the long run brings cows that differ only in condition, to a similar state.
		//
		FdCapMolly = NonUterEbwTarget * CappingForIntake ; // in kgDM actual, WFM will not feed her more forage (pasture & silage) then this amount, but may feed things like grains on top of that
		//
		// end of procedural 
		
		// END  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		if ( FeedInFlag == 1.0 ) { //
			FdRat = FdRatWFM ; // Allocated feed - m files or WFM
		} else { //
			FdRat = RequiredEnergy / 11.5 ; // Fully fed automatically (standalone)
		}
		
		// END  INCLUDE 'FdRat_deriv.csl'
		iFdRat = RequiredEnergy / 11.5 ;
		EnergyCompenstaion = 1.0 ;
		
		FdRat = RequiredEnergy / 11.5 ;
		iNonUterEbwTarget = NonUterEbwTarget ;
		iNonUterEBW = NonUterEBW ;
		
		// END  INCLUDE 'FdRat_Init.csl'
		
		// Next block was merged From Molly86 for Mindy. Gil July 2012
		IntakeDay = iFdRat ;
		
		iTotMeals = 0 ;
		TotMeals = iTotMeals ;
		TotMealsYest = iTotMeals ;
		TNdfIn = 0.0 ;
		TNdfInYest = 0.0 ;
		
		// ****** COMPUTATION OF INITIAL CONDITIONS FOR STATE VARIABLES *******
		// Initial conditions are set to allow for changes in initial
		// empty bodyweight.
		// This was done by dividing original pool sizes by 500 (iEBW). Thus,
		// iGl pool was 4.5E-2 and became (4.5E-2/500.0)=9.0E-5.Initial volumes
		// for blood metabolites are similarly defined but in addition
		// have become dynamic variables dependent on EBW changes during
		// solution.
		
		// FOR MID-LACTATION REFERENCE (DEFAULT) STATE
		
		iGlF = 9.0E-5 ; iFaF = 5.0E-4 ; iAcF = 1.1555E-3 ; iAaF = 7.5E-5 ;
		iVolGlF = 3.0E-2 ; iVolFaF = 1.0 ; iVolAcF = 0.6555 ; iVolAaF = 3.0E-2 ;
		// The following 5 factors were set too low in Moly5v.  INcreased to 173% of 5v to reflect Shaver et al 1988 data
		iHaF = 2.9555E-3 ; iHcF = 2.5555e-3 ; iCeF = 2.5555e-3 ;
		iPiF = 1.4555E-3 ; iIndigFdF = 2.0E-3 ; // Reduced iIndigFdF to achieve model balance, MDH 5-28-13
		iHbF = iHcF + iCeF ;
		iLPartF = 1.5555E-3 ; // LPartF and MPartF reflect Shaver et al 1988 data
		ifMPartRum = 0.3555 ; ifSPartRum = 0.5555 ;
		iMPartF = ( iHaF + iHcF + iCeF + iPiF + iIndigFdF ) * ifMPartRum / ( ifMPartRum + ifSPartRum ) ;
		iSPartF = ( iHaF + iHcF + iCeF + iPiF + iIndigFdF ) * ifSPartRum / ( ifMPartRum + ifSPartRum ) ;
		
		iAsF = 1.5555E-3 ; iAmF = 1.6E-3 ; iMiF = 2.7555E-3 ; iFlF = 0.9555E-3 ; iCsF = 0.5555E-3 ;
		iRumAcF = 8.0E-3 ; iRumPrF = 3.5E-3 ; iRumBuF = 1.2E-3 ; iRumAaF = 0.1555E-3 ;
		iMiHaF = 0.3555E-3 ; iMiHbF = 0.5555E-3 ; iBldUrF = 7.0E-3 ; iVolBldUrF = 1.0 ;
		iRumLaF = 2.0E-9 ;
		iOthDnaF = 1.5e-4 ; iVisDnaF = 1.5e-4 ;
		// iOthDnaF and iVisDnaF set at 1.5E-4 based upon a rough estimates of
		// maxima for lactating cows jk
		
		// For initiation of full lactation simulations initial rumen pools
		// must be adjusted to lower intakes characteristic of pregnancy.
		// Example values for an intake of 7.0kg/day follow:
		
		// CONSTANT iHaF=0.7555E-3,iLPartF=2.7555E-3,iHbF=1.4555E-3,iPiF=0.4555E-3
		// CONSTANT iIndigFdF=0.8E-3,iCsF=0.1555E-3,iAsF=1.0E-3,iAmF=1.08E-3
		// CONSTANT iMiF=1.04E-3,iFlF=0.3555E-3,iRumAcF=3.8555E-3,iRumPrF=1.6555E-3
		// CONSTANT iRumBuF=0.5555E-3,iRumAaF=0.6E-3,iMiHaF=0.1555E-3,iMiHbF=0.1555E-3
		// CONSTANT iRumLaF=1.0E-8
		
		kInitRumVol = 100 ; // Exponetial deacy constant. When this k is set to 50 cows will be initialized as follows: (iFdRat,iRumVol) (10,64) (20,90)
		MaxRumVol = 3.9 * 0.05 * iBW ; // Unit: kg. Average of lit references is 3.9 * FdRat but is higher at DMI<20 kg. NES 10/99. 0.05 is FdRat = 5% of bodyweight. Gil
		RumVol = ( 1 - exp ( - kInitRumVol * iFdRat / iBW ) ) * MaxRumVol ; // Unit: kg. Gil Sep 2014 MaxRumVol acheived at FdRat=5% of IBW, but smaller rations no longer give lineraly declining RumVol
		
		RumDM = RumVol * 0.1555 ;
		iRumLiqVol = RumVol - RumDM ;
		RumLiqVol = iRumLiqVol ;
		// Rumen DM of 14.7% based on data from above references for lactating
		// cows.   NES 8/99
		iotGutCont = 1 * iFdRat ;
		// Other gut contents (otGutCont) equal to feed DM intake based on data
		// from Chilliard 1991, JDS 74:3103,and Gibb 1992,An Prod 55:339. NES 8/99
		
		// Gestaion code moved from here upwards, to maintain correct order with DNZ fdrat code
		
		iEBW = iBW - RumVol - iotGutCont ; // Blood Vol should be removed from this?
		
		// Initial wt Adipose based on equation from Waltner,et al 1994,JDS
		// 77:2570. All of the remainder of the partitioning of non-fat EBW based
		// on calculations and concepts from the original model. Cytisolic
		// portion of adipose is 20% of Adip and is constant. Lean body mass(Oth)
		// and Viscera(Vis) are 82% and 18%, respectively, of non-fat EBW.
		// Computation of mass of Oth and Vis assumes that protein+H2O is 70% of
		// wt at 25% dry matter (fractional dry wt or fDWt) and the remaining 30%
		// of initial mass is constant (otWtOth and otWtVis). POth and PVis
		// represent the turnover proteins of these body fractions and thus are
		// not constant. TsAdip represents the storage triglycerides of adipose
		// and thus are not constant.
		iWtAdip = ( 0.2555 * iBW ) + ( 36 * iBCS ) -122.1 ;
		iWtCytAdip = iWtAdip * 0.2 ;
		iWtTsAdip = iWtAdip - iWtCytAdip ;
		iNonFatEBW = iEBW - iWtAdip ;
		// WtGrvUter added Apr 5, 2007 to refect gestation status.
		iNonFatNonUterEBW = iNonFatEBW - WtGrvUter ;
		iNonUterEBW = iEBW - WtGrvUter ;
		iWtOth = iNonFatNonUterEBW * 0.8555 ;
		iWtVis = iNonFatNonUterEBW - iWtOth ;
		BWF = iNonUterEBW ; // Changed to reflect conceptus space outside of maternal space, 4-14-07
		
		iotWtOth = iWtOth * 0.3 ;
		iotWtVis = iWtVis * 0.3 ;
		ifDWt = 0.2555 ;
		iPOth = ( iWtOth - iotWtOth ) * ifDWt / MwtPOth ;
		iPVis = ( iWtVis - iotWtVis ) * ifDWt / MwtPVis ;
		iTsAdip = iWtTsAdip / MwtTs ;
		
		iabsEAve = 0.4555 * ( BWF *pow(1,1)* 0.7555 ) * F1 ; // jk 04/26/91
		iGl = iGlF * BWF * GlCor ;
		iVolGl = iVolGlF * BWF * GlCor ;
		iFa = iFaF * BWF * FaCor ;
		iVolFa = iVolFaF * BWF * FaCor ;
		iAc = iAcF * BWF * AcCor ;
		iVolAc = iVolAcF * BWF * AcCor ;
		iAa = iAaF * BWF * AaCor ;
		iVolAa = iVolAaF * BWF * AaCor ;
		iBldUr = iBldUrF * BWF * BldUrCor ;
		iVolBldUr = iVolBldUrF * BWF * BldUrCor ;
		iHa = iHaF * BWF * HaCor ;
		iLPart = iLPartF * BWF * LPartCor ;
		iMPart = iMPartF * BWF ;
		iHc = iHcF * BWF * HcCor ;
		iCe = iCeF * BWF * CeCor ;
		iHb = iHbF * BWF * HbCor ;
		iPi = iPiF * BWF * PICor ;
		iIndigFd = iIndigFdF * BWF * IndigFdCor ;
		iSPart = iSPartF * BWF ;
		iRumLa = iRumLaF * BWF * RumLaCor ;
		iCs = iCsF * BWF * CsCor ;
		iAs = iAsF * BWF * ASCor ;
		iAm = iAmF * BWF * AmCor ;
		iMi = iMiF * BWF * MICor ;
		iFl = iFlF * BWF * FLCor ;
		iRumAc = iRumAcF * BWF * RumAcCor ;
		iRumPr = iRumPrF * BWF * RumPrCor ;
		iRumBu = iRumBuF * BWF * RumBuCor ; // Changed from RumPrCor. CCP 8-31-06
		iRumAa = iRumAaF * BWF * RumAaCor ;
		iMiHa = iMiHaF * BWF * MiHaCor ;
		iMiHb = iMiHbF * BWF * MIHbCor ;
		iOthDna = iOthDnaF * BWF ; // ??This and iVisDna should use starting pools for Oth and Vis
		iVisDna = iVisDnaF * BWF ; // to calculate iDna
		iVmAcTs = VmAcTsAdip * ( iEBW *pow(1,1)* 0.7555 ) ;
		
		// Provide some initial calculated values to allow the first time step
		SolDM = iCs * MwtCs / CsCor + iAm * MwtAm / AmCor + iRumPr * MwtPr / RumPrCor + iRumBu * MwtBu
			/ RumBuCor + iRumAa * MwtRumAa / RumAaCor + iAs / ASCor + iFl * MwtFl / FLCor +
			iRumAc * MwtAc / RumAcCor + iRumLa * MwtLa / RumLaCor ;
		
		// Intiialize urine excretion accumulator variable
		UrinationVol = 0.0 ; // Volume of the last urination
		TotWaUrineLast = 0.0 ; // Accumulated urine water at the last urinartion event
		TotNurLast = 0.0 ; // Accumulated urine N at the last urinartion event
		BladderVol = 0.0 ; // Momentary water volume of the bladder contents.
		NurConcentration = 0.0 ; // Concentration of N of the last discrete urination excretion
		
		// Leftovers from Neal and Thornley Equations
		THETA5 = 10.0 ; TAveMilkMam = 0.04555 ; MamMilkCor = 1.0 ;
		// Initiate mammary parameters
		iLHorF = 1.0 ; iMamMilkAveF = 1.0 ; iMamTmF = 0.01 ;
		LHorBase = 20 ; // Used to be LhorCor
		iLHor = iLHorF * LHorBase ;
		iMamLmF = 0.1 ; iMamPmF = 0.01 ; iMilkAve = 10.0 ; TAveMilk = 0.1555 ;
		
		// BEGIN  INCLUDE 'MamCells_init.csl'
		
		CumulativeLowMfDays = 0 ;
		DailyMfDiff = 0 ;
		
		InMilk = 0 ;
		iMamCells = MamCellsPart ;
		iMamCellsA = 1e-12 ;
		iMamCellsQ = MamCellsPart ;
		iMamCellsS = 0 ;
		dWtAdipNew = 0 ;
		BcsTarget = 3.0 ;
		dLwExclUterGutAndGrowth = 0 ;
		dWtAdipNew = 0 ;
		MilkingFrequencyLag = 0.0 ;
		derivMilkingFrequencyLag = 0.0 ;
		iRumVol = RumVol ; // Only mamcells of DNZ needs iRumVol, so I put it here to minimise diff - Gil
		iNonUterEbwTarget = iEBW - WtGrvUter - 36.0 * ( iBCS - BCSBase ) ; // Needed for DNZ/mamcells_deriv
		NonUterEbwTarget = iNonUterEbwTarget ;
		// END  INCLUDE 'MamCells_init.csl'
		
		// Genetic Potential and hormone administration
		// Hormone injections expressed in multiples of endogenous
		// secretion.
		BST = 1.0 ; T3 = 1.0 ; INS = 1.0 ;
		kRetMilkI = 4.5555 ;
		
		// Mammary PARAMETERS
		
		MilkMax = 30.0 * MamMilkCor ;
		KMilkI = kRetMilkI * MamMilkCor ;
		iMamMilkAve = iMamMilkAveF * MamMilkCor ;
		iMamTm = iMamTmF * MamMilkCor ;
		iMamPm = iMamPmF * MamMilkCor ;
		iMamLm = iMamLmF * MamMilkCor ;
		
		// Next block (up to END of INITIAL) was merged From Molly86 for Mindy. Gil July 2012
		// Initialize milk production variables
		TVolMilkYest = 0.0 ;
		dMilkProd = 0.0 ;
		MilkProdDiel = 0.0 ;
		
		// Initialize Eating Time variables
		TotEatingYest = 0.0 ;
		TotRumntnYest = 0.0 ;
		TotRestYest = 0.0 ;
		dEating = 0.0 ;
		dRumntn = 0.0 ;
		dRest = 0.0 ;
		
		MilkInt = 0.5 ; PResidMamMilk = 0.1 ;
		// Bruckmaier, 2003 observed 10% residual milk (PResidMamMilk) without oxytocin injections
		// Seems like it should be a fixed amount regardless of Mammary Fill
		
		// Schedule milkings and summary: Make sure last milking hour is early enough to be finished before midnight when the daily summary is exceuted
		
		// Can have variable MilkingFrequency up to this Max
		// Can be increased if desired maximal milking frequency is above 2
		
		// Include initial settings for the within day grazing equations
		// Moved this code down so iNonFatEBW could be used in Mindy, 2-26-2012, MDH
		// Fill desired milking times in hours on a 24h clock. Bad idea, change this to days, MDH?? Please no renamed variable to Hours instead (GL)
		MilkingIndex = 1 ; // Index of milking within the day. Goes from 1,2,..., MilkingFrequency, and repeats.
		MilkingFrequency = 1 / MilkInt ;
		
		NextMilkingT = -1.0 ;
		schedule ( t + 0.01 , "NewDay" ) ; // This happens after midnight every day and sets the first milking every day she is in milk
		MilkSW = 0.0 ;
		
		schedule ( t + 0.0 , "DailySummary" ) ; // Gil July 2012 changed to from 1 to 0, to make many graphs work OK before T=1
		
		// BEGIN  INCLUDE 'Intermittent_Eating_Init.csl'
		
		// No intermittent eating for basic Molly
		EatingSupplementsSW = 0.0 ;
		SupplementOnOffer = 0 ; // Basic Molly does not always eats entry 1 in the feed table, which is changes daily bexternally to reflect the daily diet (supps and grass mixed together to one feed)
		MastJawMoveBolus = 5.0 ; // In Mindy it is a function of hunger, ndf, specific feed and bolus size. Bolus size in turn is determined by the size of the cow, availability of feed (e.g. short grass => more severing movements => more saliva => smaller bolus) %DM, and specific saliva inducing properties of the feed
		// END  INCLUDE 'Intermittent_Eating_Init.csl'
		// BEGIN  INCLUDE 'Mindy_Init.csl'
		
		// No Mindy code in this project
		HMM = 1.0 ;
		RUMNTNEQ = 0.0 ;
		
		// Some no-op thingies provided dynamically by Mindy,
		// but must provide something for Molly as well,
		// As the chewing sub-model is in Molly.csl (for Mark)
		WaPool = 300 ;
		WaPoolTarget = 300 ;
		EatingSW = 1.0 ;
		AcquisitionJawMovesCurrent = 10 ;
		BolusWeightTotalCurrent = 200 ;
		MaxBolusWeight = 200 ;
		EatSW5 = 1.0 ;
		CurrStrat = 1.0 ;
		
		CurrHerbage = 1 ;
		// END  INCLUDE 'Mindy_Init.csl'
		
		StandardBw = 560 ; // Standard BW of a cow baseline for extrapolating in various equations
		StandardMetabolicBw = StandardBw *pow(1,1)* 0.7555 ; // Standard EBW of a cow baseline for extrapolating in various equations
		BwCorrected = NonUterEbwTarget * 1.2555 ; // Unit: kg. Excl.gravid uetrus, corrected to average condition
		// *1.2555 adds an average gutfil. (From Illius & Gordon 1992 Total ruminant Gut DM = 0.002555 of BW, and assuming 14% DM in rumen contents (Van Vuuren 1993) we get:
		// BW - (BW * 0.02555 / 0.1555) = EBW    =>    BW * (1 - 0.02555/0.1555) = EBW   =>  BW = 1.2555 EBW
		BwCorrection = BwCorrected / StandardBw ;
		
		// end of initial // OF INITIAL SECTION
		bufferslp = "RumPartSizeSlp is positive - particle size dist is likely non-biological" ;
		bufferint = "RumPartSizeInt is very positive - particle size dist may be non-biological" ;
		IALG = 4 ;
		NSTP = 1 ;
		CINT = 0.001 ;
		MAXT = 0.005 ;
		TSTP = 305 ;
		CurrentEvent = 1 ;
			fAcSilage = 0.007555 ; // Kg/Kg Corn Silage DM
			fLaSilage = 0.06 ;
			PcSilage = 0.0 ;
			PcPeFd = 3 ; // Gil sept 2014, reducing all nutrients, to leave enough room for SC!! (Delagargde for instance has 921 g/kg for CP+SC+NDF which leaves only 8% for ST PE OA ASH!!!!)
			fBuAc = 0.2 ;
			fOaPe = 0.1555 ; // Oa = Pe * .2 is an Unsupported Estimate mdh. reduced from 0.2 to 0.1555 - Gil
		PiMeanRRT = 8.0 ; // Used to calculate kPiAa from RUP and CP
		HaMeanRRT = 8.0 ; // Used to calculate kHaCs from RUSt and St
		CeMeanRRT = 8.0 ; // Used to calculate kCeCs from RuADF and ADF
		FKRuP = 2.8555 ;
		FKRuSt = 3.1555 ;
		FKRuAdf = 5.4555 ;
		FHcCs1 = 0.9555 ; // Scale KHcCs1 to KCeCs1 based on previously derived estimates for ikHcCs and ikCeCs
		slpKRuAdf = 1.0 ; // added slopes to each of the K calculations to keep the
		slpKRUP = 1.0 ; // rate constants sensitive to input in situ rates. MDH 5-17-14
		slpKRUST = 1.0 ; // The large intercept for RuADF removed all sensitivity.
		OsMolF = 1.7555 ; // OSMOLALITY FACTOR
		RumLiqVolEQ = 0.0 ;
		OsWaSlp = 0.9555 ; OsWaInt = 0 ; // Set the slope a little higher to keep osmolality down, MDH 2-19-14
		KWaFeces = 0.7555 ; // assumed feces is 23% DM from Murphy 1992 review, JDS
		kMilkAsh = 0.01 ; // Assumed 1% Ash in milk
		kWaRespir = 0.03555 ; // Assumed half the maximal respiratory rate cited in Murphy, 1992 JDS
		kWaSweat = 0.01555 ; // Assumed 25% the maximal sweating rate cited in Murphy, 1992 JDS
		TotWaConsumed = 0.0 ; // TotWaConsumed = integ ( WaConsumed , 0.0 )
		TotWaUrine = 0.0 ; // TotWaUrine = integ ( WaUrine , 0.0 )
		MaxBladderVol = 2.0 ;
		MilkDen = 1.03 ;
		CountDownDays = 30 ; // Minimum days that DayMilk should count down to next calving
		IntakeTotal = 0 ; // IntakeTotal = integ ( FdRat , 0 )
		kMastication = 1.5555 ; // Unit: multiplier. Adaptor from actual mastication jaw movements to the arbitrary steps of the model which are on a finer scale.
		kSpecies = 1.3 ; // Unit: power. SpeciesFactor simulates ease of breakdown of the feed. It reduces mastication and increases breakdown effiecncy (simulated by cheating: increasing steps in the model!). However tougher feed should result in larger particle size, so we must avoid kSpecies=1 which will neutralize the species effect (i.e. with SpeciesFactor=1 the decreased breakdown effeciecny would cancel out exactly the effect of the increased mastication of tougher feed)
		kComminuteOralMin = 0.03555 ; // Fitted to 21 datasets, Bailey 90, Loginbuhl 89, Shaver, Oshio, John&reid 87, however with mastication time left free as was unknown! Hence in future the k will be influenced by the feeds ease of breakdown, which will also reduce mastication time so end result will not change a lot.
		kComminuteOralMax = 0.9555 ; // -"-
		iOxup = 180.4 ; iFCM4Z = 44.08 ; iME = 2.7 ; iDMilk = 20.0 ;
		ifPm = 0.03555 ; ifTm = 0.03555 ; iAtAdh = 0.01555 ;
		icFa = 0.5E-3 ; icAc = 1.8E-3 ; icGl = 3.0E-3 ;
		iTCH4 = 1.0E-9 ;
		iDEi = 70 ; iMEi = 60.0 ; VmAcTsAdip = 0.3555 ;
		TFCM4z = iFCM4Z ; // TFCM4z = integ ( FCM4Z , iFCM4Z )
		ObsME = 2.5555 ; ObsDE = 3.00 ; ObsCH4 = 0.2555 ; ObsEUr = 0.1555 ;
		FatAdd = 0.0 ;
		InfPrt = 0.0 ;
		OaScSC = 0.6555 ; PeScSC = 0.9555 ; LiScSC = 0.1555 ; LaScSc = 0.9555 ; HcCeCe = 1.02 ;
		FaScFd = 0.09555 ;
		TotDMin = 1.0E-9 ; // TotDMin = integ ( DailyDMin , 1.0E-9 )
		TNdfIn = 1.0E-9 ; // TNdfIn = integ ( NdfinFd , 1.0E-9 )
		RumntnF = 0.3555 ; // Gil Aug 20102 moved RUMNTEQ to mindy_init (=0 for molly; = 1 for mindy)
		MinLPRumntnF = 0.03555 ; // Fraction of MBW. Set to achieve appropriate rumination times and LP pool sizes, added 04-25-2011, MDH
		MEAN1 = 0.0 ; MEAN2 = 0.05555 ; AMP1FT = 0.1555 ; AMP2FT = 0.1555 ;
		TotRumntn = 0.0 ; // TotRumntn = integ ( Rumntn , 0.0 )
		TotEating = 0.0 ; // TotEating = integ ( Eating , 0.0 )
		TotRest = 0.0 ; // TotRest = integ ( Rest , 0.0 )
		AaFvAc = 0.6555 ;
		AaFvPr = 0.6555 ;
		AaFvBu = 0.2555 ;
		LaAcAc = 0.8555 ;
		FORSET = 0.0 ; MIXSET = 1.0 ; CONSET = 0.0 ;
		vfaeff = 15.0 ; RumpHCON = 1.0 ; FIXDpH = 0.0 ; RumpHBase = 6.5555 ;
		KSPartP = 0.3555 ; KWAP = 3.5 ; // KSPartP is now a fraction of KWAP, MDH 5-25-13
		KLPartRed = 20.5 ; // This pool was reduced in size and more dietary material is diverted through it
		LPart = iLPart ; // LPart = integ ( dLPart , iLPart )
		KMPartSPart = 4.5 ; // Set to achieve steady state on the base diet, MDH
		pLPartMPartComm = 0.9 ; // ??Need to verify and update if needed
		KMPartP = 0.05 ; // A proportion of Liq Flow. An initial guess, MDH
		MPart = iMPart ; // MPart = integ ( dMPart , iMPart )
		SPart = iSPart ; // SPart = integ ( dSPart , iSPart )
		PartWidth = 0.3 ; // mm from Pierre Beukes
		PartThick = 0.1 ;
		kSurfaceArea = 27.6555 ; // 27.6555mm2/mm3=mean SA per unit Vol for Shaver et al. 1988 data
		KMiHa = 1.5555 ; KMiHb = 0.4555 ; VmMiHa = 0.8555 ;
		VmMiHb = 0.8555 ; KMiHaF = 0.09555 ; KMiHbF = 0.04555 ;
		HaMi = iMiHa ; // HaMi = integ ( dHaMi , iMiHa )
		HbMi = iMiHb ; // HbMi = integ ( dHbMi , iMiHb )
		Ha = iHa ; // Ha = integ ( dHa , iHa )
		KFatHb = 0.03 ;
		Hc = iHc ; // Hc = integ ( dHc , iHc )
		Ce = iCe ; // Ce = integ ( dCe , iCe )
		Hb = iHb ; // Hb = integ ( dHb , iHb )
		KFatPi = 0.0007555 ; // Sept. 20, 2004 solution against Bate5o2 data.
		Pi = iPi ; // Pi = integ ( dPi , iPi )
		IndigFd = iIndigFd ; // IndigFd = integ ( dIndigFd , iIndigFd )
		VmCsFv = 1000 ; KCsFv = 0.009 ;
		Cs = iCs ; // Cs = integ ( dCs , iCs )
		VmRumAaFv = 407.0 ; cSaPs = 0.002 ; KRumAaFv = 0.006555 ;
		RumAa = iRumAa ; // RumAa = integ ( dRumAa , iRumAa )
		NnAmAM = 3.8 ; AaFvAm = 1.3555 ; KAmabs = 23.06555 ; UrAmAm = 2.0 ;
		VmBldUrAm = 5.6555e-2 ; KBldUrAm = 0.007 ; KiAm = 0.003 ;
		AM2 = iAm ; // AM2 = integ ( dAm , iAm )
		fSaAs = 0.008555 ; KAsabs = 58.0 ;
		InfNaCl = 0.0 ; InfNaBicarb = 0.0 ;
		As = iAs ; // As = integ ( dAs , iAs )
		LiFlFd = 1.8 ; LiChFd = 0.1555 ; FaFlFd = 3.0 ;
		Fl = iFl ; // Fl = integ ( dFl , iFl )
		KabsAc = 6.2555 ; KabsPr = 11.6555 ; KabsBu = 10.3555 ; KabsLa = 0.1 ;
		RumAc = iRumAc ; // RumAc = integ ( dRumAc , iRumAc )
		InfRumPr = 0.0 ; // infused ruminal propionate, mol/d
		RumPr = iRumPr ; // RumPr = integ ( dRumPr , iRumPr )
		RumBU = iRumBu ; // RumBU = integ ( dRumBu , iRumBu )
		KLaFv = 0.5 ;
		RumLa = iRumLa ; // RumLa = integ ( dRumLa , iRumLa )
		RumYAtp = 0.01555 ; KYAtAa = 0.0001 ; KFGAm = 0.001555 ; LaFvAt = 1.0 ;
		KFatFG = 0.03 ;
		CsFvAt = 4.0 ; AaFvAt = 0.9555 ;
		CsMiG1 = 6.1555 ; AmMiG1 = 7.3555 ; HyMiG1 = 2.7555 ; CdMiG1 = 0.5555 ;
		CsMiG2 = 2.1555 ; AmMiG2 = 1.1555 ; AaMiG2 = 4.9555 ; HyMiG2 = -0.4555 ;
		MiPiPI = 0.5555 ; MiNnNn = 0.09555 ; MiHaHA = 0.2555 ; MiLiLI = 0.1555 ;
		MiLiFA = 1.2 ; MiLiBu = 0.5 ; MiLiPr = 1.0 ; MiLiCh = 0.8 ; MiLiGl = 0.5 ;
		Mi = iMi ; // Mi = integ ( dMi , iMi )
		LgutDCHa = 0.8555 ; DCMiPi = 0.7555 ; DCMiLi = 0.7 ; LgutDCHb = 0.1555 ;
		LgutDCPi = 0.6555 ; LgutDCAs = 0.8555 ; LgutDCAi = 0.1555 ; LgutDCFa = 0.6555 ;
		AccGEi = 1.0E-8 ; // AccGEi = integ ( FdGEin , 1.0E-8 )
		AccDEi = 1.0E-8 ; // AccDEi = integ ( DEI , 1.0E-8 )
		AccMEi = 1.0E-8 ; // AccMEi = integ ( MEI , 1.0E-8 )
		Latitude = 38.0 ; // Positive for N Hemi, Neg for S Hemisphere
		DaylightSavingShift = 0 ; // Unit: hours. Set to 1 if the management hours (milking hours, feeding times in Mindy) are expressed in daylight saving time with 1 hour shift.
		kAHorGl = 0.003555 ; Theta2 = 5.9555 ;
		kAHor1Gl = 3.0e-3 ; Theta3 = 1.0 ;
		kCHorGl = 3.0e-3 ; Theta4 = 4.1555 ;
		kCHor1Gl = 0.003555 ;
		kAaGlGest = 6.0 ; // Yields approximately 2x use for Oxid as for prt syn per Bell 1995
		KMilk = 100.0 ;
		MilkProductionAgeAdjustment = 1.0 ;
		MamCellsPerKgMs270 = 2.3 ;
		MamCellsPerKgMsAdjustment = 200 ;
		KgMilkSolidsExpectedIn270Days = 472 ; // Also Key Determiner of required intake
		MamCellsPart = 1000 ; // WFM sets it to 2.5555 * KgMilkSolidsExpectedIn270Days - 200
		K1MamCells = 0.009 ; // Decay rate for cell proliferation precalving from Dijkstra goat DNA
		uTMamCells = 0.03 ; // Specific Cell proliferation rate at parturition from Dijkstra
		lambdaMamCells = 0.002555 ; // Mam Cell Death rate from Dijkstra Mexican cow (now used only precalving GL)
		kMamAQBase = 0.1555 ; // From Vetharaniam et al 2003
		kMamCellsDeclineBase = 0.003 ; // Determines the base (2x) post peak decline of MamCells (Note: LHOR affcets Milk Production decline as well)
		kMamCellsQAPrePeak = 0.4555 ; // Very sensitive! determines the timing of the peak. Decraese delays
		kMamCellsQAPostPeak = 0.2555 ; // The ratio between this one and kMamCellsAQBase drives the proprtion of the A pool in its relatively stable period (post peak)
		kMamCellsQAStart = 2.0 ; // Speeds up the first week's A pool growth => 1st week milk yield
		kMamCellsQAKickStartDecay = 2.0 ; // Speeds up the first week's A pool growth => 1st week milk yield
		kMamCellsTransitionDim = 16.0 ; // Affects mid lactation milk curve. Controls the centre DIM of the change from "pre" tp "post" kMamCellsQA
		kMamCellsTransitionSteepness = 0.04555 ; // Affects mid lactation milk curve. Controls the speed of transition from "pre" to "post"kMamCellsQA. 0.1 gives 90% of the change with the 60 days around kMamCellsTransitionDim. With 0.2 the 60 goes down to 30.
		MamCellsDecayRateOfSenescence = 0.003 ; // This determines the mild curvature (convex) of the post peak mam cells decline, by slowing the absolute senescence rate gardually down to base turn over rate. Note that a typical near-constant persistnecy is exponantial decay by nature (e.g. 98% weekly persistency means every week is 98% of the previous => exponential decay.
		BaseMamCellsTurnOver = 0.001 ; // Base rate of proliferation and senecence, has no real effect because it does not affect the net change of MamCells. Only purpose is to maintain biological correctness of 100% turnover by around 250 DIM (Capuco / Ruminant phisiology)
		MamCellsProliferationDecayRate = 0.2 ; // This determines Peak mam cells. Not much to play with here as it is commonly accepted that day 10 in milk is the peak
		kMamCellsUsMfDecay = 0.02555 ; // Rate of the increased senecence while on low milking frequency. (works on a continuous scale between MF 1 and 2)
		MilkIntPowerForFMamCelsQA1 = 1.1 ; // Changes the Q to A flux as MF changes. When decreased, difference between 1x,2x and 3x will grow.
		MaxLossDueToLowMf = 0.1555 ; // This sets the maximum proportion of MamCellPart that would be permanently lost due to long term low milking frequency. The loss is fast to start with and slows down the more days on low MF the cow goes through
		MamCellsA = iMamCellsA ; // MamCellsA = integ ( dMamCellsA , iMamCellsA )
		MamCellsQ = iMamCellsQ ; // MamCellsQ = integ ( dMamCellsQ , iMamCellsQ )
		MamCellsS = iMamCellsS ; // MamCellsS = integ ( dMamCellsS , iMamCellsS ) // We don't really need this pool, only to verify that there is approx 100% turnover over 250 days of lactation
		CumulativeLowMfDays = 0 ; // CumulativeLowMfDays = integ ( DailyMfDiff , 0 ) // Keep track of the cummulative number of 1x eqivalent days on low liking freq, e.g 20 days on 1.5x or 10 days on 1x both grow cumulativeLowMfDiff by 10.
		WtAdipNew = max ( 0. , iWtAdip ) ; // WtAdipNew = max ( 0. , integ ( dWtAdipNew , iWtAdip ) )
		NonUterEbwTarget = iNonUterEbwTarget ; // NonUterEbwTarget = integ ( GrowthPerDay , iNonUterEbwTarget )
		LhorTurnoverDays = 20.0 ; // Roughly the number days it would take LHOR to change to the full extent once the drivers state changed
		kLHorSensAa = 2.0 ; // Linear slope component in the equation
		kLHorSensGl = 2.0 ; // Linear slope component in the equation
		wLHorSensAa = 1.6 ; // Relative importance of blood amino acids
		wLHorSensAdip = 0.4 ; // Relative importance of adipose size.
		wLHorSensGl = 1.3 ; // Relative importance of blood glocose.
		xLHorSensAa = 0.8 ; // Curvature component of the sensitivity
		xLHorSensAdip = 0.2555 ; // Curvature component of the sensitivity
		xLHorSensGl = 0.5 ; // Curvature component of the sensitivity
		cAaBase = 0.003555 ; // Baseline of cAA. When cAA = cAaBase => nil change to LHOR due to amino acids level. 10-2015 GL changed from 0.06555 to 0.005
		cGlBase = 0.01 ; // Baseline of CGL. When cGL = cGlBase => nil change to LHOR due to glucose level
		KDayLength = 0.1555 ; // CCP Scalar for PP effect on LHor degradation
		FixedLhorSW = 0.0 ; // Unit: 0 or 1. set to 1 to bypass the Lhor equation and use Lhor set to its baseline instead.
		LHor1 = iLHor ; // LHor1 = integ ( dLHor , iLHor )
		BcsTargetNadir = 2.02 ; // Target BCS around peak lactaion. Calving, Calving + 365 days => BcsTarget = BcsBase; DIM 10 to 70: BCS = BcsTargetNadir; oOther DIM: linear - connect the dots.
		BcsTargetDecay = 0.2 ; // Determines the exponential down curve of TargetBcs from calving to nadir
		PMamEnzCell = 12.4555 ; // This should be a fn of DayMilk
		kMilkingFrequencyLagUp = 0.01555 ;
		kMilkingFrequencyLagDown = 1.5555 ;
		MilkingFrequencyLag = 2 ; // MilkingFrequencyLag = integ ( derivMilkingFrequencyLag , 2 ) // Lagging Mf moves down fast but slow to catch up after MF changed upwards.
		KMilkInhDeg = 1.0 ; ikMilkInh = 0.8 ;
		KMinh = ikMilkInh ; // KMinh = integ ( dKMilkInh , ikMilkInh )
		TgFaFa = 3.0 ; AcAcTs = 24.0 ; VmTsFaAdip = 0.06555 ; KTsFaAdip = 0.2 ;
		Theta1 = 5.0 ;
		KFaTsAdip = 5.0E-4 ; K1FaTs = 2.0E-3 ; K1TsFa = 5.0E-4 ;
		VmFaTsAdip = 0.3555 ; KFaTmVis = 5.0E-4 ; VmFaTmVis = 1.0E-3 ;
		FaTgTg = 0.3555 ; AcTgTg = 0.04555 ; K1FaTm = 1.5E-3 ;
		P1 = 2.0 ;
		EXP10 = 2.0 ;
		TsAdip = iTsAdip ; // TsAdip = integ ( dTsAdip , iTsAdip )
		Fa = iFa ; // Fa = integ ( dFa , iFa )
		K1VAct = 7.0 ; K2VAct = 0.3555 ;
		VmAcTs2 = iVmAcTs ; // VmAcTs2 = integ ( dVmAcTs , iVmAcTs )
		KAcTsAdip = 1.8E-3 ; VmAcTmVis = 7.0E-3 ; KAcTmVis = 1.8E-3 ;
			K1AcTm = 1.0E-3 ; K1AcTs = 1.0E-3 ; AaGlAc = 0.6 ;
		Ac = iAc ; // Ac = integ ( dAc , iAc )
		MamTm = iMamTm ; // MamTm = integ ( dMamTm , iMamTm ) // AND TOTAL YIELD
		TMilkTm = 1.0E-8 ; // TMilkTm = integ ( dMilkTm , 1.0E-8 )
		ExpOth2 = 1.1555 ; KDnaOth = 7.3555E-4 ; ExpV2 = 0.9555 ; KDnaVis = 1.7e-4 ;
			OthDnaMx = 0.09 ; VisDnaMx = 0.09 ;
		OthDna = iOthDna ; // OthDna = integ ( dOthDna , iOthDna )
		VisDna = iVisDna ; // VisDna = integ ( dVisDna , iVisDna )
		VmAaPOthOth = 223.2555 ;
			VmAaPVisVis = 200 ; KAaPOthOth = 2.5E-3 ; KAaPVisVis = 2.5E-3 ;
			VmAaPmVis = 0.002555 ; VmAaGlVis = 0.2555 ; KAaGlVis = 10.0E-3 ;
			KPOthAaOth = 0.03555 ; KPVisAaVis = 0.09555 ; fDWt = 0.2555 ;
			AaGlUr = 0.6555 ; KAaPmVis = 2.1E-3 ; VsizF = 80.0 ;
		PVis = iPVis ; // PVis = integ ( dPVis , iPVis )
		POth = iPOth ; // POth = integ ( dPOth , iPOth )
		AA1 = iAa ; // AA1 = integ ( dAa , iAa )
		AmUrUr = 0.5 ; KBldUrU = 2133.5555 ;
		BldUr1 = iBldUr ; // BldUr1 = integ ( dBldUr , iBldUr )
		MamPm = iMamPm ; // MamPm = integ ( dMamPm , iMamPm ) // Mammary PROTEIN
		TMilkPm = 1.0E-8 ; // TMilkPm = integ ( dMilkPm , 1.0E-8 ) // AND TOTAL YIELD
		AaGlGl = 0.4555 ; PrGlGl = 0.5 ; LaGlGl = 0.5 ; GyGlGl = 0.5 ;
			KGlLmVis = 3.0E-3 ; VmGlTpAdip = 3.7555e-2 ;
			KGlTpAdip = 3.0E-3 ; VmGlTpVis = 9.4555e-3 ; KGlTpVis = 3.0E-3 ; fLaCdAdip = 0.2555 ;
			KGlLaOth = 1.5E-3 ; VmGlLaOth = 4.6555e-2 ; fLaCdOth = 0.2555 ; GlGlHy = 0.2555 ;
			pGlHyAdip = 0.6 ; pGlHyVis = 0.6 ; fPrGl = 0.7 ; TgGyGy = 1.0 ; GlLaLa = 2.0 ;
			GlHyTp = 1.0 ; GlTpTp = 2.0 ; TpTpTs = 1.0 ; GlGyGY = 2.0 ; KAaLmVis = 2.0E-3 ;
			TpTpTm = 1.0 ;
		VmGlLmVisPart = 0.001555 ;
		kVmGlLmSyn = 0.04555 ; kVmGlLmDecay = 0.1555 ; kVmGlLmDeg = 0.0003555 ;
		Gl = iGl ; // Gl = integ ( dGl , iGl )
		GlLmLm = 0.5 ; fLm = 0.04555 ;
		MamLm = iMamLm ; // MamLm = integ ( dMamLm , iMamLm ) // Mammary LACTOSE
		TMilkLm = 1.0E-8 ; // TMilkLm = integ ( dMilkLm , 1.0E-8 ) // AND TOTAL YIELD
		MamMilkAve = iMamMilkAve ; // MamMilkAve = integ ( dMamMilkAve , iMamMilkAve )
		GlCdAt = 38.0 ; AcCdAt = 10.0 ; FaCdAt = 129.0 ; GyGlAt = 2.0 ;
			PrCdAt = 18.5 ; TpCdAt = 20.0 ; LaCdAt = 18.0 ; TpLaAt = 2.0 ; GlLaAt = 2.0 ;
			BuCdAt = 25.0 ; HyAtAt = 3.0 ;
		AaPxAD = 5.0 ; GlHyAD = 1.0 ; GlTpAD = 2.0 ; LaGlAd = 3.0 ;
			TpTgAD = 9.0 ; AcFaAd = 2.8555 ; TcHyAd = 5.2555 ; GlLmAd = 1.0 ; PrGlAd = 2.0 ;
			absGlAd = 1.0 ; absAaAd = 1.0 ;
		OxAcCd = 2.0 ; OxPrCd = 3.5 ; OxBuCd = 5.0 ; OxGlCd = 6.0 ; OxLaCd = 3.0 ;
			OxTpCd = 3.0 ; OxFaCd = 23.0 ; OxPrGl = 1.0 ;
		AcCdCd = 2.0 ; PrCdCd = 3.0 ; BuCdCd = 4.0 ; GlCdCd = 6.0 ; GlHyCd = 3.0 ;
			FaCdCd = 16.0 ; LaCdCd = 3.0 ; TpCdCd = 3.0 ;
		TAveabsE = 0.05 ;
		absEAve = iabsEAve ; // absEAve = integ ( dabsEAve , iabsEAve )
		KNaOth = 0.1555 ; HyAcFa = 1.7555 ; AaGlH = 0.5555 ; // KbasOth was moved so it can be defined diferently by Mindy
		KbasOth = 2.1555 ; // Basal energy expenditure rate coefficient bundles average activity inside
		eerActivityAtp = 0.0 ; // Unit: mole/d Activity Energy Expenditure Rate. Molly does not calculate activity explicitly
		KbasAdip = 1.1555 ; KNaAdip = 0.1555 ;
		KbasVis = 8.8555 ; KNaVis = 0.2555 ; KidWrk = 0.3555 ; HrtWrk = 0.09555 ; ResWrk = 0.3555 ;
		AtAmUr = 4.0 ;
		fGrvUterTO = 0.6 ; // All oxidation in GrvUter driven by protein T/O
		KAcCd = 2.0E-3 ; KGlCd = 0.01555 ; KFaCd = 2.2555E-3 ;
		NurTotal = 0.0 ; // NurTotal = integ ( NUr , 0.0 )
		tNep = 0 ; // tNep = integ ( NEP , 0 )
		fGestEPrt = 0.6555 ; // Derived from (AaPGest*HcombAa)/EGrvUterCLF
		AaFvHy = 1.1555 ;
		KHyEruct = 0.02 ; // Based on AgResearch Chamber data
		KHyOther = 0.1555 ; // Based on AgResearch Chamber data
		KumarMigEq = 1.0 ; // Pablo requested May 2016 to be able to diable Mig effect on hidrogen
		TCH4 = iTCH4 ; // TCH4 = integ ( dTCH4 , iTCH4 ) // TCH4 is in moles
		TBCH4 = 1.0E-8 ; // TBCH4 = integ ( BCH4 , 1.0E-8 )
		TMCH4E = 1.0E-8 ; // TMCH4E = integ ( MCH4E , 1.0E-8 ) // TMCH4 to TMCH4E - 2/2/95
		EPart = 1.0 ;
		RQEQ = 1.0 ;
		AaFvCd = 1.2555 ;
		MwtCd = 44 ;
			schedule ( t + 1.0 , "breakpoint4debug" ) ; // --- this is the interval that this discrete section will be executed.
		
	} // end initialise_model
	
	void pull_variables_from_model ( ) {
		
		// pull system time
		variable["t"] = t;
		
		// pull model variables
		variable["Expt"] = Expt;
		variable["GlobalDMIEqn"] = GlobalDMIEqn;
		variable["jj"] = jj;
		variable["logCriteria1"] = logCriteria1;
		variable["Old"] = Old;
		variable["Silage"] = Silage;
		variable["HyMiG2"] = HyMiG2;
		variable["AA"] = AA;
		variable["AA1"] = AA1;
		variable["AaAcV1"] = AaAcV1;
		variable["AaAm"] = AaAm;
		variable["AaFvFat"] = AaFvFat;
		variable["AaGlGest"] = AaGlGest;
		variable["AaGlHt"] = AaGlHt;
		variable["AaGlV1"] = AaGlV1;
		variable["AaGlVis"] = AaGlVis;
		variable["AaPConc"] = AaPConc;
		variable["AaPGest"] = AaPGest;
		variable["AaPmHt"] = AaPmHt;
		variable["AaPmVis"] = AaPmVis;
		variable["AaPOthHt"] = AaPOthHt;
		variable["AaPOthOth"] = AaPOthOth;
		variable["AaPUter"] = AaPUter;
		variable["AaPVisVis"] = AaPVisVis;
		variable["AaTO"] = AaTO;
		variable["AaUrGest"] = AaUrGest;
		variable["AaUrVis"] = AaUrVis;
		variable["AbortPregNow"] = AbortPregNow;
		variable["AbortPregNowVariable"] = AbortPregNowVariable;
		variable["AbsAa"] = AbsAa;
		variable["absAaE"] = absAaE;
		variable["absAaHt"] = absAaHt;
		variable["absAc"] = absAc;
		variable["AbsAcE"] = AbsAcE;
		variable["AbsAm"] = AbsAm;
		variable["absAs"] = absAs;
		variable["absBu"] = absBu;
		variable["absBuE"] = absBuE;
		variable["AbsE"] = AbsE;
		variable["absEAve"] = absEAve;
		variable["absEF"] = absEF;
		variable["absFa"] = absFa;
		variable["absFaE"] = absFaE;
		variable["absGl"] = absGl;
		variable["absGlE"] = absGlE;
		variable["absGlHt"] = absGlHt;
		variable["absLa"] = absLa;
		variable["absLaE"] = absLaE;
		variable["absPr"] = absPr;
		variable["absPrE"] = absPrE;
		variable["absRumAc"] = absRumAc;
		variable["absRumAm"] = absRumAm;
		variable["absRumAs"] = absRumAs;
		variable["absRumBu"] = absRumBu;
		variable["absRumLa"] = absRumLa;
		variable["absRumPr"] = absRumPr;
		variable["Ac"] = Ac;
		variable["AcCd"] = AcCd;
		variable["AccDEi"] = AccDEi;
		variable["AccGEi"] = AccGEi;
		variable["AccMEi"] = AccMEi;
		variable["AcTmH1"] = AcTmH1;
		variable["AcTmH2"] = AcTmH2;
		variable["AcTmH3"] = AcTmH3;
		variable["AcTmH4"] = AcTmH4;
		variable["AcTmV1"] = AcTmV1;
		variable["AcTmVis"] = AcTmVis;
		variable["AcTsAdip"] = AcTsAdip;
		variable["AcTsF1"] = AcTsF1;
		variable["AcTsH1"] = AcTsH1;
		variable["AcTsH2"] = AcTsH2;
		variable["AcTsH3"] = AcTsH3;
		variable["AcTsH4"] = AcTsH4;
		variable["AcTsH5"] = AcTsH5;
		variable["AcTsH6"] = AcTsH6;
		variable["AcTsH7"] = AcTsH7;
		variable["AcTsHt"] = AcTsHt;
		variable["AdAt"] = AdAt;
		variable["AdAtAdip"] = AdAtAdip;
		variable["AdAtB1"] = AdAtB1;
		variable["AdAtB2"] = AdAtB2;
		variable["AdAtF1"] = AdAtF1;
		variable["AdAtF2"] = AdAtF2;
		variable["AdAtOth"] = AdAtOth;
		variable["AdAtV1"] = AdAtV1;
		variable["AdAtV2"] = AdAtV2;
		variable["AdAtV3"] = AdAtV3;
		variable["AdAtV4"] = AdAtV4;
		variable["AdAtV5"] = AdAtV5;
		variable["AdAtVis"] = AdAtVis;
		variable["ADFIn"] = ADFIn;
		variable["AdfinFd"] = AdfinFd;
		variable["ADFP"] = ADFP;
		variable["afAiFd"] = afAiFd;
		variable["AFTmH5"] = AFTmH5;
		variable["AFTmH6"] = AFTmH6;
		variable["AfTmH9"] = AfTmH9;
		variable["AgeInYears"] = AgeInYears;
		variable["AHor"] = AHor;
		variable["AHor1"] = AHor1;
		variable["AiP"] = AiP;
		variable["AM"] = AM;
		variable["Am1"] = Am1;
		variable["AM2"] = AM2;
		variable["AmMi"] = AmMi;
		variable["AmUr"] = AmUr;
		variable["Animal"] = Animal;
		variable["aPAGlH"] = aPAGlH;
		variable["appDE"] = appDE;
		variable["As"] = As;
		variable["AsAsFd"] = AsAsFd;
		variable["AshinFd"] = AshinFd;
		variable["AshP"] = AshP;
		variable["AsP"] = AsP;
		variable["AtAd"] = AtAd;
		variable["AtAd10"] = AtAd10;
		variable["AtAd11"] = AtAd11;
		variable["AtAd12"] = AtAd12;
		variable["AtAd13"] = AtAd13;
		variable["AtAd14"] = AtAd14;
		variable["ATAd15"] = ATAd15;
		variable["AtAdAdip"] = AtAdAdip;
		variable["AtAdB1"] = AtAdB1;
		variable["AtAdF1"] = AtAdF1;
		variable["AtAdF2"] = AtAdF2;
		variable["AtAdF3"] = AtAdF3;
		variable["AtAdF4"] = AtAdF4;
		variable["AtAdGest"] = AtAdGest;
		variable["AtAdGestGrth"] = AtAdGestGrth;
		variable["AtAdGestTO"] = AtAdGestTO;
		variable["AtAdH1"] = AtAdH1;
		variable["AtAdHT"] = AtAdHT;
		variable["AtAdOth"] = AtAdOth;
		variable["AtAdV1"] = AtAdV1;
		variable["AtAdV2"] = AtAdV2;
		variable["AtAdV3"] = AtAdV3;
		variable["AtAdV4"] = AtAdV4;
		variable["AtAdV5"] = AtAdV5;
		variable["AtAdV6"] = AtAdV6;
		variable["AtAdV7"] = AtAdV7;
		variable["AtAdV8"] = AtAdV8;
		variable["AtAdV9"] = AtAdV9;
		variable["AtAdVis"] = AtAdVis;
		variable["AtHt"] = AtHt;
		variable["AtHt1"] = AtHt1;
		variable["AtHt2"] = AtHt2;
		variable["AtHt3"] = AtHt3;
		variable["AtHt4"] = AtHt4;
		variable["AtHt5"] = AtHt5;
		variable["AtHt6"] = AtHt6;
		variable["AtHt7"] = AtHt7;
		variable["AtHt8"] = AtHt8;
		variable["AtpC"] = AtpC;
		variable["AtpF"] = AtpF;
		variable["AtpG"] = AtpG;
		variable["AtpM"] = AtpM;
		variable["basalAdip"] = basalAdip;
		variable["basalOth"] = basalOth;
		variable["basalVis"] = basalVis;
		variable["basHtAdip"] = basHtAdip;
		variable["basHtOth"] = basHtOth;
		variable["basHtVis"] = basHtVis;
		variable["BCH4"] = BCH4;
		variable["BCH4Fd"] = BCH4Fd;
		variable["BCS"] = BCS;
		variable["BCS_NZ"] = BCS_NZ;
		variable["BcsTarget"] = BcsTarget;
		variable["BcsTargetFactor"] = BcsTargetFactor;
		variable["BladderVol"] = BladderVol;
		variable["BldUr"] = BldUr;
		variable["BldUr1"] = BldUr1;
		variable["BldUrAm"] = BldUrAm;
		variable["BldUrMUN"] = BldUrMUN;
		variable["BldUrRumAm"] = BldUrRumAm;
		variable["BldUrVol"] = BldUrVol;
		variable["BuCdVis"] = BuCdVis;
		variable["BW"] = BW;
		variable["BW1"] = BW1;
		variable["BwCorrected"] = BwCorrected;
		variable["BwCorrection"] = BwCorrection;
		variable["BWF"] = BWF;
		variable["cAa"] = cAa;
		variable["cAc"] = cAc;
		variable["cAm"] = cAm;
		variable["cAs"] = cAs;
		variable["cBldUr"] = cBldUr;
		variable["cCs"] = cCs;
		variable["CdMi"] = CdMi;
		variable["CdMiG2"] = CdMiG2;
		variable["Ce"] = Ce;
		variable["CeAcAc"] = CeAcAc;
		variable["CeBuBu"] = CeBuBu;
		variable["CeCs"] = CeCs;
		variable["Cein"] = Cein;
		variable["CeP"] = CeP;
		variable["CePrPr"] = CePrPr;
		variable["cFa"] = cFa;
		variable["CFatinFd"] = CFatinFd;
		variable["cGl"] = cGl;
		variable["cGlTarget"] = cGlTarget;
		variable["CH4DEi"] = CH4DEi;
		variable["CH4E"] = CH4E;
		variable["CH4EFd"] = CH4EFd;
		variable["CH4GEi"] = CH4GEi;
		variable["CH4KGY"] = CH4KGY;
		variable["CH4MEi"] = CH4MEi;
		variable["CH4Milk"] = CH4Milk;
		variable["ChChFd"] = ChChFd;
		variable["CHor"] = CHor;
		variable["CHor1"] = CHor1;
		variable["cMiHa"] = cMiHa;
		variable["cMiHb"] = cMiHb;
		variable["cMiSPart"] = cMiSPart;
		variable["cMiWa"] = cMiWa;
		variable["cMun"] = cMun;
		variable["ConceiveNow"] = ConceiveNow;
		variable["ConceiveNowVariable"] = ConceiveNowVariable;
		variable["CONSET"] = CONSET;
		variable["CorME"] = CorME;
		variable["CorMEi"] = CorMEi;
		variable["CorNEP"] = CorNEP;
		variable["CorrectedBW"] = CorrectedBW;
		variable["CPinFd"] = CPinFd;
		variable["cPOth"] = cPOth;
		variable["CpP"] = CpP;
		variable["CPsinFd"] = CPsinFd;
		variable["cPun"] = cPun;
		variable["cPVis"] = cPVis;
		variable["cRumAa"] = cRumAa;
		variable["cRumAc"] = cRumAc;
		variable["cRumBu"] = cRumBu;
		variable["cRumLa"] = cRumLa;
		variable["cRumPr"] = cRumPr;
		variable["Cs"] = Cs;
		variable["CsAc"] = CsAc;
		variable["CsBu"] = CsBu;
		variable["CsCd"] = CsCd;
		variable["CsFv"] = CsFv;
		variable["CsFvAc"] = CsFvAc;
		variable["CsFvBu"] = CsFvBu;
		variable["CsFvCd"] = CsFvCd;
		variable["CsFvLa"] = CsFvLa;
		variable["CsFvPr"] = CsFvPr;
		variable["Csin"] = Csin;
		variable["CsLa"] = CsLa;
		variable["CsMi"] = CsMi;
		variable["CsMiG"] = CsMiG;
		variable["CsP"] = CsP;
		variable["CsPr"] = CsPr;
		variable["cTs"] = cTs;
		variable["CumulativeLowMfDays"] = CumulativeLowMfDays;
		variable["CurrentFeed"] = CurrentFeed;
		variable["CurrentSupplement"] = CurrentSupplement;
		variable["cVFA"] = cVFA;
		variable["CWC"] = CWC;
		variable["CWCF"] = CWCF;
		variable["dAa"] = dAa;
		variable["dabsEAve"] = dabsEAve;
		variable["dAc"] = dAc;
		variable["dAdipE"] = dAdipE;
		variable["DailyDMin"] = DailyDMin;
		variable["DailyMfDiff"] = DailyMfDiff;
		variable["dAm"] = dAm;
		variable["dAs"] = dAs;
		variable["DAY"] = DAY;
		variable["DayGest"] = DayGest;
		variable["DayGestBasic"] = DayGestBasic;
		variable["DayLength"] = DayLength;
		variable["DaylengthP1"] = DaylengthP1;
		variable["DaylengthP2"] = DaylengthP2;
		variable["DaylightSavingShift"] = DaylightSavingShift;
		variable["DayMilk"] = DayMilk;
		variable["DayofYear"] = DayofYear;
		variable["DayTwLength"] = DayTwLength;
		variable["DayTwlengthP2"] = DayTwlengthP2;
		variable["dBCS"] = dBCS;
		variable["dBldUr"] = dBldUr;
		variable["DCadf"] = DCadf;
		variable["DCCe"] = DCCe;
		variable["dCd"] = dCd;
		variable["dCdKg"] = dCdKg;
		variable["DCDM"] = DCDM;
		variable["dCe"] = dCe;
		variable["dCH4g"] = dCH4g;
		variable["dCH4Kg"] = dCH4Kg;
		variable["DCHa"] = DCHa;
		variable["DCHb"] = DCHb;
		variable["DCLg"] = DCLg;
		variable["DCLipid"] = DCLipid;
		variable["DCndf"] = DCndf;
		variable["DCOm"] = DCOm;
		variable["DCPrt"] = DCPrt;
		variable["dCs"] = dCs;
		variable["dCsFvH"] = dCsFvH;
		variable["dCsHy"] = dCsHy;
		variable["dDCH4"] = dDCH4;
		variable["DE"] = DE;
		variable["dEating"] = dEating;
		variable["dEBW1"] = dEBW1;
		variable["DEI"] = DEI;
		variable["derivMilkingFrequencyLag"] = derivMilkingFrequencyLag;
		variable["dFa"] = dFa;
		variable["dFl"] = dFl;
		variable["dGestE"] = dGestE;
		variable["dGl"] = dGl;
		variable["dHa"] = dHa;
		variable["dHaMi"] = dHaMi;
		variable["dHb"] = dHb;
		variable["dHbMi"] = dHbMi;
		variable["dHc"] = dHc;
		variable["dHiM4"] = dHiM4;
		variable["dHiP4"] = dHiP4;
		variable["dHyEruct"] = dHyEruct;
		variable["dHyFlF"] = dHyFlF;
		variable["dHyMi"] = dHyMi;
		variable["dHyOther"] = dHyOther;
		variable["DilRate"] = DilRate;
		variable["dIndigFd"] = dIndigFd;
		variable["dKMilkInh"] = dKMilkInh;
		variable["dLHor"] = dLHor;
		variable["dLm"] = dLm;
		variable["dLmProd"] = dLmProd;
		variable["dLPart"] = dLPart;
		variable["dLwExclUterGutAndGrowth"] = dLwExclUterGutAndGrowth;
		variable["dMaint"] = dMaint;
		variable["dMamCellsA"] = dMamCellsA;
		variable["dMamCellsQ"] = dMamCellsQ;
		variable["dMamCellsS"] = dMamCellsS;
		variable["dMamLm"] = dMamLm;
		variable["dMamMilkAve"] = dMamMilkAve;
		variable["dMamPm"] = dMamPm;
		variable["dMamTm"] = dMamTm;
		variable["dMi"] = dMi;
		variable["DMilk"] = DMilk;
		variable["dMilkLm"] = dMilkLm;
		variable["dMilkPm"] = dMilkPm;
		variable["dMilkProd"] = dMilkProd;
		variable["dMilkTm"] = dMilkTm;
		variable["dMilkVol"] = dMilkVol;
		variable["DMP"] = DMP;
		variable["dMPart"] = dMPart;
		variable["dN"] = dN;
		variable["dNdfIn"] = dNdfIn;
		variable["dNep"] = dNep;
		variable["dNonUterEBW"] = dNonUterEBW;
		variable["dOthDna"] = dOthDna;
		variable["dOthE"] = dOthE;
		variable["dOx"] = dOx;
		variable["dPi"] = dPi;
		variable["dPm"] = dPm;
		variable["dPmProd"] = dPmProd;
		variable["dPOth"] = dPOth;
		variable["dPVis"] = dPVis;
		variable["dRest"] = dRest;
		variable["DrinkSW"] = DrinkSW;
		variable["DrnkWa"] = DrnkWa;
		variable["DrnkWaDiel"] = DrnkWaDiel;
		variable["DrnkWaTot"] = DrnkWaTot;
		variable["DrnkWaYest"] = DrnkWaYest;
		variable["dRumAa"] = dRumAa;
		variable["dRumAaHy"] = dRumAaHy;
		variable["dRumAc"] = dRumAc;
		variable["dRumBu"] = dRumBu;
		variable["dRumCd"] = dRumCd;
		variable["dRumLa"] = dRumLa;
		variable["dRumLiqVol"] = dRumLiqVol;
		variable["dRumntn"] = dRumntn;
		variable["dRumPr"] = dRumPr;
		variable["DryOffNow"] = DryOffNow;
		variable["DryOffNowVariable"] = DryOffNowVariable;
		variable["dSPart"] = dSPart;
		variable["dTCH4"] = dTCH4;
		variable["dTCH4E"] = dTCH4E;
		variable["dTHy"] = dTHy;
		variable["dTm"] = dTm;
		variable["dTmProd"] = dTmProd;
		variable["dTsAdip"] = dTsAdip;
		variable["dUrea"] = dUrea;
		variable["dVisDna"] = dVisDna;
		variable["dVisE"] = dVisE;
		variable["dVmAcTs"] = dVmAcTs;
		variable["dWtAdipNew"] = dWtAdipNew;
		variable["dWtGrvUter"] = dWtGrvUter;
		variable["dWtOth"] = dWtOth;
		variable["dWtPGrvUter"] = dWtPGrvUter;
		variable["dWtPUter"] = dWtPUter;
		variable["dWtTsAdip"] = dWtTsAdip;
		variable["dWtUter"] = dWtUter;
		variable["dWtVis"] = dWtVis;
		variable["Eating"] = Eating;
		variable["EatingSupplementsSW"] = EatingSupplementsSW;
		variable["EatSa"] = EatSa;
		variable["EatWa"] = EatWa;
		variable["EB"] = EB;
		variable["EBW"] = EBW;
		variable["EBW1"] = EBW1;
		variable["eerActivityAtp"] = eerActivityAtp;
		variable["EGrvUterCLF"] = EGrvUterCLF;
		variable["ELm"] = ELm;
		variable["EnergyCompensation"] = EnergyCompensation;
		variable["EnergyCompenstaion"] = EnergyCompenstaion;
		variable["EnergyForActivity"] = EnergyForActivity;
		variable["EnergyForGrowth"] = EnergyForGrowth;
		variable["EnergyForMaintenance"] = EnergyForMaintenance;
		variable["EnergyForMilk"] = EnergyForMilk;
		variable["EnergyForPregnancy"] = EnergyForPregnancy;
		variable["EPm"] = EPm;
		variable["ETm"] = ETm;
		variable["EUr"] = EUr;
		variable["EUrFd"] = EUrFd;
		variable["Fa"] = Fa;
		variable["FaCd"] = FaCd;
		variable["fAcFd"] = fAcFd;
		variable["fADFFd"] = fADFFd;
		variable["fAiFd"] = fAiFd;
		variable["fAiIndigFd"] = fAiIndigFd;
		variable["FaP"] = FaP;
		variable["fAsFd"] = fAsFd;
		variable["fAshFd"] = fAshFd;
		variable["FatAdd"] = FatAdd;
		variable["FaTmH7"] = FaTmH7;
		variable["FaTmH8"] = FaTmH8;
		variable["FaTmV1"] = FaTmV1;
		variable["FaTmVis"] = FaTmVis;
		variable["FaTsAdip"] = FaTsAdip;
		variable["FatSc"] = FatSc;
		variable["FaTsF1"] = FaTsF1;
		variable["fBCH4D"] = fBCH4D;
		variable["fBCH4E"] = fBCH4E;
		variable["fBCH4M"] = fBCH4M;
		variable["fBuFd"] = fBuFd;
		variable["fBuSilage"] = fBuSilage;
		variable["FCeCs"] = FCeCs;
		variable["fCeFd"] = fCeFd;
		variable["FCFatFd"] = FCFatFd;
		variable["fCH4DE"] = fCH4DE;
		variable["fCH4E"] = fCH4E;
		variable["fCH4ME"] = fCH4ME;
		variable["FCM3h"] = FCM3h;
		variable["FCM4Z"] = FCM4Z;
		variable["FCM4z1"] = FCM4z1;
		variable["fCPFd"] = fCPFd;
		variable["FCPsFd"] = FCPsFd;
		variable["fCsHa"] = fCsHa;
		variable["fCsHb"] = fCsHb;
		variable["fd1"] = fd1;
		variable["fd2"] = fd2;
		variable["fd3"] = fd3;
		variable["FdCapMolly"] = FdCapMolly;
		variable["FdDMIn"] = FdDMIn;
		variable["FdGEin"] = FdGEin;
		variable["fDMFd"] = fDMFd;
		variable["FdRat"] = FdRat;
		variable["FdRatDiel"] = FdRatDiel;
		variable["FecADF"] = FecADF;
		variable["FecAsh"] = FecAsh;
		variable["FecCe"] = FecCe;
		variable["FecCh"] = FecCh;
		variable["FecDM"] = FecDM;
		variable["FecENG"] = FecENG;
		variable["FecFa"] = FecFa;
		variable["FecFMPart"] = FecFMPart;
		variable["FecFSPart"] = FecFSPart;
		variable["FecHa"] = FecHa;
		variable["FecHaT"] = FecHaT;
		variable["FecHb"] = FecHb;
		variable["FecHC"] = FecHC;
		variable["FecLg"] = FecLg;
		variable["FecLipid"] = FecLipid;
		variable["FecMiHa"] = FecMiHa;
		variable["FecMiLi"] = FecMiLi;
		variable["FecMiNn"] = FecMiNn;
		variable["FecMiPi"] = FecMiPi;
		variable["FecMPart"] = FecMPart;
		variable["FecNDF"] = FecNDF;
		variable["FecOm"] = FecOm;
		variable["FecPi"] = FecPi;
		variable["FecPiT"] = FecPiT;
		variable["FecPiTN"] = FecPiTN;
		variable["FecSPart"] = FecSPart;
		variable["FeedInFlag"] = FeedInFlag;
		variable["fFatFd"] = fFatFd;
		variable["fFIM"] = fFIM;
		variable["FGAm"] = FGAm;
		variable["FGFa"] = FGFa;
		variable["fGlHyAdip"] = fGlHyAdip;
		variable["fGlHyVis"] = fGlHyVis;
		variable["fHcCs"] = fHcCs;
		variable["fHcFd"] = fHcFd;
		variable["fIndigFd"] = fIndigFd;
		variable["FirstEvent"] = FirstEvent;
		variable["FIXDpH"] = FIXDpH;
		variable["FixedLhorSW"] = FixedLhorSW;
		variable["Fl"] = Fl;
		variable["Fl1Fd"] = Fl1Fd;
		variable["FLaFd"] = FLaFd;
		variable["FlFd"] = FlFd;
		variable["fLgAdf"] = fLgAdf;
		variable["fLgFd"] = fLgFd;
		variable["fLgIndigFd"] = fLgIndigFd;
		variable["fLiFd"] = fLiFd;
		variable["FlMi"] = FlMi;
		variable["FlMiG"] = FlMiG;
		variable["fLPart"] = fLPart;
		variable["fLPartADF"] = fLPartADF;
		variable["fLPartAi"] = fLPartAi;
		variable["fLPartCe"] = fLPartCe;
		variable["fLPartHc"] = fLPartHc;
		variable["fLPartIndigFd"] = fLPartIndigFd;
		variable["fLPartLg"] = fLPartLg;
		variable["fLPartMPartSwal"] = fLPartMPartSwal;
		variable["fLPartNDF"] = fLPartNDF;
		variable["fLPartNDF_NDF"] = fLPartNDF_NDF;
		variable["fLPartNutIng"] = fLPartNutIng;
		variable["fLPartPi"] = fLPartPi;
		variable["fLPartplusMPart"] = fLPartplusMPart;
		variable["fLPartSt"] = fLPartSt;
		variable["fLPartSwal"] = fLPartSwal;
		variable["fMamCellsAQ"] = fMamCellsAQ;
		variable["fMamCellsAS"] = fMamCellsAS;
		variable["fMamCellsPA"] = fMamCellsPA;
		variable["fMamCellsQA"] = fMamCellsQA;
		variable["fMamCellsQS"] = fMamCellsQS;
		variable["fMamCellsUS"] = fMamCellsUS;
		variable["fMCH4D"] = fMCH4D;
		variable["fMCH4E"] = fMCH4E;
		variable["fMCH4M"] = fMCH4M;
		variable["fMPart"] = fMPart;
		variable["fMPart1"] = fMPart1;
		variable["fMPartNDF_NDF"] = fMPartNDF_NDF;
		variable["fMPartNutIng"] = fMPartNutIng;
		variable["fMPartSA"] = fMPartSA;
		variable["fMPartSwal"] = fMPartSwal;
		variable["fNDFFd"] = fNDFFd;
		variable["fNnCp"] = fNnCp;
		variable["fNnFd"] = fNnFd;
		variable["FNPNFd"] = FNPNFd;
		variable["fOaFd"] = fOaFd;
		variable["fOmFd"] = fOmFd;
		variable["FORSET"] = FORSET;
		variable["fParticulateFd"] = fParticulateFd;
		variable["fPartP"] = fPartP;
		variable["fPartSA"] = fPartSA;
		variable["fPeFd"] = fPeFd;
		variable["fPiCp"] = fPiCp;
		variable["fPiFd"] = fPiFd;
		variable["fPm"] = fPm;
		variable["fPsCp"] = fPsCp;
		variable["FPsFd"] = FPsFd;
		variable["fRoughageFd"] = fRoughageFd;
		variable["fRuAdfAdf"] = fRuAdfAdf;
		variable["fRuAdfFd"] = fRuAdfFd;
		variable["fRumDM"] = fRumDM;
		variable["fRupCp"] = fRupCp;
		variable["FRUPFd"] = FRUPFd;
		variable["fRUStFd"] = fRUStFd;
		variable["fRuStSt"] = fRuStSt;
		variable["fScCs"] = fScCs;
		variable["fScFd"] = fScFd;
		variable["fScTFd"] = fScTFd;
		variable["fSPart"] = fSPart;
		variable["fSPart1"] = fSPart1;
		variable["fSPartNDF_NDF"] = fSPartNDF_NDF;
		variable["fSPartNutIng"] = fSPartNutIng;
		variable["fSPartSA"] = fSPartSA;
		variable["fSPartSwal"] = fSPartSwal;
		variable["fStCs"] = fStCs;
		variable["fStFd"] = fStFd;
		variable["FStsFd"] = FStsFd;
		variable["fStSSt"] = fStSSt;
		variable["fTm"] = fTm;
		variable["fTm1"] = fTm1;
		variable["fUrCp"] = fUrCp;
		variable["FUrFd"] = FUrFd;
		variable["FvAcFd"] = FvAcFd;
		variable["FvBuFd"] = FvBuFd;
		variable["FvLaFd"] = FvLaFd;
		variable["G1"] = G1;
		variable["G2"] = G2;
		variable["GE"] = GE;
		variable["GE1Fd"] = GE1Fd;
		variable["GE2Fd"] = GE2Fd;
		variable["GE3Fd"] = GE3Fd;
		variable["GEFd"] = GEFd;
		variable["gGlLa"] = gGlLa;
		variable["Gl"] = Gl;
		variable["GlCd"] = GlCd;
		variable["GlCdT"] = GlCdT;
		variable["GlGyT"] = GlGyT;
		variable["GlHyAdip"] = GlHyAdip;
		variable["GlHyVis"] = GlHyVis;
		variable["GlLaB1"] = GlLaB1;
		variable["GlLaOth"] = GlLaOth;
		variable["GlLmHt"] = GlLmHt;
		variable["GLLmVis"] = GLLmVis;
		variable["GlMilk"] = GlMilk;
		variable["GLMilk1"] = GLMilk1;
		variable["GlMilk2"] = GlMilk2;
		variable["GlMilk3"] = GlMilk3;
		variable["GlTO"] = GlTO;
		variable["GlTpAdip"] = GlTpAdip;
		variable["GlTpF1"] = GlTpF1;
		variable["GlTpV1"] = GlTpV1;
		variable["GlTpVis"] = GlTpVis;
		variable["GrowthPerDay"] = GrowthPerDay;
		variable["GyGlV1"] = GyGlV1;
		variable["GyGlVis"] = GyGlVis;
		variable["Ha"] = Ha;
		variable["HaCs"] = HaCs;
		variable["HaMi"] = HaMi;
		variable["HaMiF"] = HaMiF;
		variable["HaMiG"] = HaMiG;
		variable["HaMiP"] = HaMiP;
		variable["HaMiRum"] = HaMiRum;
		variable["HaP"] = HaP;
		variable["HaPT"] = HaPT;
		variable["Hb"] = Hb;
		variable["Hbin"] = Hbin;
		variable["HbMi"] = HbMi;
		variable["HbMiF"] = HbMiF;
		variable["HbMiG"] = HbMiG;
		variable["HbMiP"] = HbMiP;
		variable["HbMiRum"] = HbMiRum;
		variable["HbP"] = HbP;
		variable["Hc"] = Hc;
		variable["HcAcAc"] = HcAcAc;
		variable["HcBuBu"] = HcBuBu;
		variable["HcCs"] = HcCs;
		variable["Hcin"] = Hcin;
		variable["HcombAa"] = HcombAa;
		variable["HcombAc"] = HcombAc;
		variable["HcombBu"] = HcombBu;
		variable["HcombCh"] = HcombCh;
		variable["HcombCH4"] = HcombCH4;
		variable["HcombCs"] = HcombCs;
		variable["HcombFa"] = HcombFa;
		variable["HcombFl"] = HcombFl;
		variable["HcombGl"] = HcombGl;
		variable["HcombGy"] = HcombGy;
		variable["HcombHc"] = HcombHc;
		variable["HcombLa"] = HcombLa;
		variable["HcombLg"] = HcombLg;
		variable["HcombLiFd"] = HcombLiFd;
		variable["HcombLm"] = HcombLm;
		variable["HcombMi"] = HcombMi;
		variable["HcombMiLi"] = HcombMiLi;
		variable["HcombNn"] = HcombNn;
		variable["HcombOa"] = HcombOa;
		variable["HcombPr"] = HcombPr;
		variable["HcombPs"] = HcombPs;
		variable["HcombTg"] = HcombTg;
		variable["HcombTp"] = HcombTp;
		variable["HcombUr"] = HcombUr;
		variable["HcP"] = HcP;
		variable["HcPrPr"] = HcPrPr;
		variable["Herd"] = Herd;
		variable["HFerm"] = HFerm;
		variable["HFermM"] = HFermM;
		variable["HiM"] = HiM;
		variable["HiM1"] = HiM1;
		variable["HiM2"] = HiM2;
		variable["HiM3"] = HiM3;
		variable["HiP"] = HiP;
		variable["HiP1"] = HiP1;
		variable["HiP2"] = HiP2;
		variable["HiP3"] = HiP3;
		variable["HiV8"] = HiV8;
		variable["HtF2"] = HtF2;
		variable["HtF3"] = HtF3;
		variable["HtV2"] = HtV2;
		variable["HtV3"] = HtV3;
		variable["HtV4"] = HtV4;
		variable["HtV5"] = HtV5;
		variable["HtV6"] = HtV6;
		variable["HtV7"] = HtV7;
		variable["i"] = i;
		variable["iAa"] = iAa;
		variable["iabsEAve"] = iabsEAve;
		variable["iAc"] = iAc;
		variable["iAm"] = iAm;
		variable["iAs"] = iAs;
		variable["iBCS"] = iBCS;
		variable["iBldUr"] = iBldUr;
		variable["iBW"] = iBW;
		variable["iCe"] = iCe;
		variable["iCs"] = iCs;
		variable["iDayGestDMI"] = iDayGestDMI;
		variable["iEBW"] = iEBW;
		variable["iFa"] = iFa;
		variable["iFdRat"] = iFdRat;
		variable["ifDWt"] = ifDWt;
		variable["iFl"] = iFl;
		variable["iGl"] = iGl;
		variable["iHa"] = iHa;
		variable["iHb"] = iHb;
		variable["iHbF"] = iHbF;
		variable["iHc"] = iHc;
		variable["iIndigFd"] = iIndigFd;
		variable["iLHor"] = iLHor;
		variable["ilogNewEvent"] = ilogNewEvent;
		variable["iLPart"] = iLPart;
		variable["iMamCells"] = iMamCells;
		variable["iMamCellsA"] = iMamCellsA;
		variable["iMamCellsQ"] = iMamCellsQ;
		variable["iMamCellsS"] = iMamCellsS;
		variable["iMamLm"] = iMamLm;
		variable["iMamMilkAve"] = iMamMilkAve;
		variable["iMamPm"] = iMamPm;
		variable["iMamTm"] = iMamTm;
		variable["iMi"] = iMi;
		variable["iMiHa"] = iMiHa;
		variable["iMiHb"] = iMiHb;
		variable["iMPart"] = iMPart;
		variable["iMPartF"] = iMPartF;
		variable["IncreasedUsDueToLowMf"] = IncreasedUsDueToLowMf;
		variable["IndigFd"] = IndigFd;
		variable["IndigFdFd"] = IndigFdFd;
		variable["IndigFdMiP"] = IndigFdMiP;
		variable["IndigFdP"] = IndigFdP;
		variable["InfAs"] = InfAs;
		variable["InfNaBicarb"] = InfNaBicarb;
		variable["InfNaCl"] = InfNaCl;
		variable["InfPrt"] = InfPrt;
		variable["InfRumPr"] = InfRumPr;
		variable["InMilk"] = InMilk;
		variable["iNonFatEBW"] = iNonFatEBW;
		variable["iNonFatNonUterEBW"] = iNonFatNonUterEBW;
		variable["iNonUterEBW"] = iNonUterEBW;
		variable["iNonUterEbwTarget"] = iNonUterEbwTarget;
		variable["IntakeDay"] = IntakeDay;
		variable["IntakeTotal"] = IntakeTotal;
		variable["IntakeYest"] = IntakeYest;
		variable["iotGutCont"] = iotGutCont;
		variable["iOthDna"] = iOthDna;
		variable["iotWtOth"] = iotWtOth;
		variable["iotWtVis"] = iotWtVis;
		variable["iPi"] = iPi;
		variable["iPOth"] = iPOth;
		variable["iPVis"] = iPVis;
		variable["iRumAa"] = iRumAa;
		variable["iRumAc"] = iRumAc;
		variable["iRumBu"] = iRumBu;
		variable["iRumLa"] = iRumLa;
		variable["iRumLiqVol"] = iRumLiqVol;
		variable["iRumPr"] = iRumPr;
		variable["iRumVol"] = iRumVol;
		variable["iSPart"] = iSPart;
		variable["iSPartF"] = iSPartF;
		variable["iStartDayGest"] = iStartDayGest;
		variable["iStartDIM"] = iStartDIM;
		variable["iTotMeals"] = iTotMeals;
		variable["iTsAdip"] = iTsAdip;
		variable["iVisDna"] = iVisDna;
		variable["iVmAcTs"] = iVmAcTs;
		variable["iVolAa"] = iVolAa;
		variable["iVolAc"] = iVolAc;
		variable["iVolBldUr"] = iVolBldUr;
		variable["iVolFa"] = iVolFa;
		variable["iVolGl"] = iVolGl;
		variable["iWtAdip"] = iWtAdip;
		variable["iWtCytAdip"] = iWtCytAdip;
		variable["iWtOth"] = iWtOth;
		variable["iWtTsAdip"] = iWtTsAdip;
		variable["iWtVis"] = iWtVis;
		variable["j"] = j;
		variable["kAcquisition"] = kAcquisition;
		variable["KCeCs"] = KCeCs;
		variable["KCeCs1"] = KCeCs1;
		variable["KHaCs"] = KHaCs;
		variable["KHcCs"] = KHcCs;
		variable["KHcCs1"] = KHcCs1;
		variable["kLHor"] = kLHor;
		variable["KLHorPP"] = KLHorPP;
		variable["kMamCellsQA"] = kMamCellsQA;
		variable["kMamCellsQaMfAdjustment"] = kMamCellsQaMfAdjustment;
		variable["KMilkI"] = KMilkI;
		variable["KMinh"] = KMinh;
		variable["KNaAtAdip"] = KNaAtAdip;
		variable["KNaAtOth"] = KNaAtOth;
		variable["KNaAtVis"] = KNaAtVis;
		variable["KPiAa"] = KPiAa;
		variable["LaCdAdip"] = LaCdAdip;
		variable["LaCdOth"] = LaCdOth;
		variable["LaGlAdip"] = LaGlAdip;
		variable["LaGlOth"] = LaGlOth;
		variable["LaGlV1"] = LaGlV1;
		variable["LaGlVis"] = LaGlVis;
		variable["LainOth"] = LainOth;
		variable["LaPrPr"] = LaPrPr;
		variable["LastEA"] = LastEA;
		variable["LastEv"] = LastEv;
		variable["LastEvent"] = LastEvent;
		variable["LastSu"] = LastSu;
		variable["LastSw"] = LastSw;
		variable["LginFd"] = LginFd;
		variable["LgP"] = LgP;
		variable["LgutAi"] = LgutAi;
		variable["LgutAs"] = LgutAs;
		variable["LgutCeAc"] = LgutCeAc;
		variable["LgutCeBu"] = LgutCeBu;
		variable["LgutCeFv"] = LgutCeFv;
		variable["LgutCePr"] = LgutCePr;
		variable["LgutFaDg"] = LgutFaDg;
		variable["LgutHaGl"] = LgutHaGl;
		variable["LgutHcAc"] = LgutHcAc;
		variable["LgutHcBu"] = LgutHcBu;
		variable["LgutHcFv"] = LgutHcFv;
		variable["LgutHcPr"] = LgutHcPr;
		variable["LgutPiAa"] = LgutPiAa;
		variable["LHor"] = LHor;
		variable["LHor1"] = LHor1;
		variable["LhorAa"] = LhorAa;
		variable["LhorAdip"] = LhorAdip;
		variable["LHorDeg"] = LHorDeg;
		variable["LhorGl"] = LhorGl;
		variable["LHorSyn"] = LHorSyn;
		variable["LHorSyn1"] = LHorSyn1;
		variable["LipidP"] = LipidP;
		variable["LiSc"] = LiSc;
		variable["logNewEvent"] = logNewEvent;
		variable["LowMfDecay"] = LowMfDecay;
		variable["LPart"] = LPart;
		variable["LPart1"] = LPart1;
		variable["LPartCeCe"] = LPartCeCe;
		variable["LPartHbHb"] = LPartHbHb;
		variable["LPartHcHc"] = LPartHcHc;
		variable["LPartIndigFdIndigFd"] = LPartIndigFdIndigFd;
		variable["LPartMi"] = LPartMi;
		variable["LPartMPart"] = LPartMPart;
		variable["LPartPiPi"] = LPartPiPi;
		variable["LPartplusMPart"] = LPartplusMPart;
		variable["LPartRed"] = LPartRed;
		variable["LPartSPart"] = LPartSPart;
		variable["LPartStHa"] = LPartStHa;
		variable["LPartSwal"] = LPartSwal;
		variable["LW"] = LW;
		variable["MamCells"] = MamCells;
		variable["MamCellsA"] = MamCellsA;
		variable["MamCellsQ"] = MamCellsQ;
		variable["MamCellsQaKickStartFactor"] = MamCellsQaKickStartFactor;
		variable["MamCellsQaPreToPostFactor"] = MamCellsQaPreToPostFactor;
		variable["MamCellsS"] = MamCellsS;
		variable["MamEnz"] = MamEnz;
		variable["MamLm"] = MamLm;
		variable["MamMilk"] = MamMilk;
		variable["MamMilkAve"] = MamMilkAve;
		variable["MamMilkAve2"] = MamMilkAve2;
		variable["MamPm"] = MamPm;
		variable["MamTm"] = MamTm;
		variable["MasticationSteps"] = MasticationSteps;
		variable["MaxEnergyForMilk"] = MaxEnergyForMilk;
		variable["MaxRumVol"] = MaxRumVol;
		variable["MCH4E"] = MCH4E;
		variable["MCH4kg"] = MCH4kg;
		variable["ME"] = ME;
		variable["ME1"] = ME1;
		variable["MealsDay"] = MealsDay;
		variable["MEAN1"] = MEAN1;
		variable["MeanParticleSize"] = MeanParticleSize;
		variable["MedianParticleSize"] = MedianParticleSize;
		variable["MEI"] = MEI;
		variable["MEinMJ"] = MEinMJ;
		variable["MEMBW"] = MEMBW;
		variable["MetabPP"] = MetabPP;
		variable["MHtAdip"] = MHtAdip;
		variable["MHtGest"] = MHtGest;
		variable["MHtGestGrth"] = MHtGestGrth;
		variable["MHtGestTO"] = MHtGestTO;
		variable["MHtOth"] = MHtOth;
		variable["MHtVis"] = MHtVis;
		variable["Mi"] = Mi;
		variable["MiAa"] = MiAa;
		variable["MiBu"] = MiBu;
		variable["MiCh"] = MiCh;
		variable["MiFa"] = MiFa;
		variable["MiG"] = MiG;
		variable["MiGl"] = MiGl;
		variable["MiHaMi"] = MiHaMi;
		variable["MiHbMi"] = MiHbMi;
		variable["MiLGl"] = MiLGl;
		variable["MiLiDg"] = MiLiDg;
		variable["MilkingFrequency"] = MilkingFrequency;
		variable["MilkingFrequencyAdjusted"] = MilkingFrequencyAdjusted;
		variable["MilkingFrequencyAgeAdjustment"] = MilkingFrequencyAgeAdjustment;
		variable["MilkingFrequencyBaseAdjustment"] = MilkingFrequencyBaseAdjustment;
		variable["MilkingFrequencyLag"] = MilkingFrequencyLag;
		variable["MilkingIndex"] = MilkingIndex;
		variable["MilkInhDeg"] = MilkInhDeg;
		variable["MilkInhSyn"] = MilkInhSyn;
		variable["MilkMax"] = MilkMax;
		variable["MilkProdDiel"] = MilkProdDiel;
		variable["MilkSolids270MfAdjusted"] = MilkSolids270MfAdjusted;
		variable["MilkSW"] = MilkSW;
		variable["MiMaAd"] = MiMaAd;
		variable["MinLPRumntn"] = MinLPRumntn;
		variable["MiNOm"] = MiNOm;
		variable["MiNOma"] = MiNOma;
		variable["MiNP"] = MiNP;
		variable["MiP"] = MiP;
		variable["MiPP"] = MiPP;
		variable["MiPr"] = MiPr;
		variable["MiPrOm"] = MiPrOm;
		variable["MirOma"] = MirOma;
		variable["MntHP"] = MntHP;
		variable["MPart"] = MPart;
		variable["MPartADFP"] = MPartADFP;
		variable["MPartDeg"] = MPartDeg;
		variable["MPartNDFP"] = MPartNDFP;
		variable["MPartP"] = MPartP;
		variable["MPartSA"] = MPartSA;
		variable["MPartSPart"] = MPartSPart;
		variable["MPartSwal"] = MPartSwal;
		variable["MPartVol"] = MPartVol;
		variable["MPcAc"] = MPcAc;
		variable["MPcBu"] = MPcBu;
		variable["MPcPr"] = MPcPr;
		variable["mult"] = mult;
		variable["MwtMiLi"] = MwtMiLi;
		variable["Nabs"] = Nabs;
		variable["Nan"] = Nan;
		variable["NANMNP"] = NANMNP;
		variable["NANP"] = NANP;
		variable["Nbal"] = Nbal;
		variable["NBody"] = NBody;
		variable["ndAt"] = ndAt;
		variable["NDFIn"] = NDFIn;
		variable["NdfinFd"] = NdfinFd;
		variable["NDFP"] = NDFP;
		variable["Ndiff"] = Ndiff;
		variable["Ndig"] = Ndig;
		variable["ndOx"] = ndOx;
		variable["NEP"] = NEP;
		variable["NetEff"] = NetEff;
		variable["netME"] = netME;
		variable["NextMilkingT"] = NextMilkingT;
		variable["NFec"] = NFec;
		variable["Nin"] = Nin;
		variable["Nintake"] = Nintake;
		variable["NitP"] = NitP;
		variable["NMilk"] = NMilk;
		variable["NnAmFd"] = NnAmFd;
		variable["NninFd"] = NninFd;
		variable["NonFatEBW"] = NonFatEBW;
		variable["NonFatNonUterEBW"] = NonFatNonUterEBW;
		variable["NonPreg"] = NonPreg;
		variable["NonUterEBW"] = NonUterEBW;
		variable["NonUterEbwTarget"] = NonUterEbwTarget;
		variable["Nout"] = Nout;
		variable["NpninFd"] = NpninFd;
		variable["Nret1"] = Nret1;
		variable["Nret2"] = Nret2;
		variable["NSal"] = NSal;
		variable["NUr"] = NUr;
		variable["NurConcentration"] = NurConcentration;
		variable["NurTotal"] = NurTotal;
		variable["OaSc"] = OaSc;
		variable["ObsCH4E"] = ObsCH4E;
		variable["ObsDEi"] = ObsDEi;
		variable["ObsMEi"] = ObsMEi;
		variable["ObsPredCH4"] = ObsPredCH4;
		variable["ObsPredDE"] = ObsPredDE;
		variable["ObsPredEUr"] = ObsPredEUr;
		variable["ObsPredME"] = ObsPredME;
		variable["OldBasalOth"] = OldBasalOth;
		variable["OminFd"] = OminFd;
		variable["OmPa"] = OmPa;
		variable["OmPt"] = OmPt;
		variable["OnceADay2YearsOldAdjustment1"] = OnceADay2YearsOldAdjustment1;
		variable["OsWa"] = OsWa;
		variable["OsWaInt"] = OsWaInt;
		variable["otGutCont"] = otGutCont;
		variable["OthDna"] = OthDna;
		variable["otWtOth"] = otWtOth;
		variable["otWtVis"] = otWtVis;
		variable["OXUP1"] = OXUP1;
		variable["Parity"] = Parity;
		variable["PartFd"] = PartFd;
		variable["PcLm"] = PcLm;
		variable["PcPm"] = PcPm;
		variable["PcSilage"] = PcSilage;
		variable["PcTm"] = PcTm;
		variable["PcTmFromScfa"] = PcTmFromScfa;
		variable["PeSc"] = PeSc;
		variable["Pi"] = Pi;
		variable["PiAa"] = PiAa;
		variable["PiMiP"] = PiMiP;
		variable["PiP"] = PiP;
		variable["PiPiFd"] = PiPiFd;
		variable["PostCalvingMamCells"] = PostCalvingMamCells;
		variable["POth"] = POth;
		variable["POthAaOth"] = POthAaOth;
		variable["POthfDr"] = POthfDr;
		variable["POthfSr"] = POthfSr;
		variable["PrCdVis"] = PrCdVis;
		variable["PreCalvingMamCells"] = PreCalvingMamCells;
		variable["Preg"] = Preg;
		variable["PreviousEvent"] = PreviousEvent;
		variable["PrGlHt"] = PrGlHt;
		variable["PrGlV1"] = PrGlV1;
		variable["PrGlVis"] = PrGlVis;
		variable["propLm"] = propLm;
		variable["PsAaFd"] = PsAaFd;
		variable["PsF"] = PsF;
		variable["PUter"] = PUter;
		variable["PUterAa"] = PUterAa;
		variable["PVis"] = PVis;
		variable["PVisAaVis"] = PVisAaVis;
		variable["PVisfDr"] = PVisfDr;
		variable["PVisfSr"] = PVisfSr;
		variable["RequiredEnergy"] = RequiredEnergy;
		variable["ResidMamMilk"] = ResidMamMilk;
		variable["Rest"] = Rest;
		variable["RestSa"] = RestSa;
		variable["RestWa"] = RestWa;
		variable["RQ1"] = RQ1;
		variable["rtAa1"] = rtAa1;
		variable["rtFa1"] = rtFa1;
		variable["rtGl1"] = rtGl1;
		variable["rtOx1"] = rtOx1;
		variable["rtOx2"] = rtOx2;
		variable["rtPO"] = rtPO;
		variable["rtPOx"] = rtPOx;
		variable["RuAdfinFd"] = RuAdfinFd;
		variable["RumAa"] = RumAa;
		variable["RumAaAc"] = RumAaAc;
		variable["RumAaBu"] = RumAaBu;
		variable["RumAaCd"] = RumAaCd;
		variable["RumAaFv"] = RumAaFv;
		variable["RumAaMi"] = RumAaMi;
		variable["RumAaP"] = RumAaP;
		variable["RumAaPr"] = RumAaPr;
		variable["RumAc"] = RumAc;
		variable["RumAc1"] = RumAc1;
		variable["RumAcP"] = RumAcP;
		variable["RumAcSynth"] = RumAcSynth;
		variable["RumADF"] = RumADF;
		variable["RumBU"] = RumBU;
		variable["RumBu1"] = RumBu1;
		variable["RumBuP"] = RumBuP;
		variable["RumBuSynth"] = RumBuSynth;
		variable["RumCein"] = RumCein;
		variable["RumCP"] = RumCP;
		variable["RumDCadf"] = RumDCadf;
		variable["RumDCCe"] = RumDCCe;
		variable["RumDCHa"] = RumDCHa;
		variable["RumDCHaT"] = RumDCHaT;
		variable["RumDCHb"] = RumDCHb;
		variable["RumDCHc"] = RumDCHc;
		variable["RumDCLiA"] = RumDCLiA;
		variable["RumDCLiT"] = RumDCLiT;
		variable["RumDCN"] = RumDCN;
		variable["RumDCndf"] = RumDCndf;
		variable["RumDCOm"] = RumDCOm;
		variable["RumDCOmA"] = RumDCOmA;
		variable["RumDCPrt"] = RumDCPrt;
		variable["RumDM"] = RumDM;
		variable["RumDPrta"] = RumDPrta;
		variable["RumHcin"] = RumHcin;
		variable["RumLa"] = RumLa;
		variable["RumLa1"] = RumLa1;
		variable["RumLaAc"] = RumLaAc;
		variable["RumLaFv"] = RumLaFv;
		variable["RumLaGl"] = RumLaGl;
		variable["RumLaP"] = RumLaP;
		variable["RumLaPr"] = RumLaPr;
		variable["RumLg"] = RumLg;
		variable["RumLiqVol"] = RumLiqVol;
		variable["RumLiqVolEQ"] = RumLiqVolEQ;
		variable["RumLPartSizeMean"] = RumLPartSizeMean;
		variable["RumLpMpSizeMean"] = RumLpMpSizeMean;
		variable["RumMPartSizeMean"] = RumMPartSizeMean;
		variable["RumNDF"] = RumNDF;
		variable["RumNit"] = RumNit;
		variable["Rumntn"] = Rumntn;
		variable["RUMNTNEQ"] = RUMNTNEQ;
		variable["RumntnSa"] = RumntnSa;
		variable["RumOM"] = RumOM;
		variable["RumOsMol"] = RumOsMol;
		variable["RumPartSizeInt"] = RumPartSizeInt;
		variable["RumPartSizeMean"] = RumPartSizeMean;
		variable["RumPartSizeSlp"] = RumPartSizeSlp;
		variable["RumpH"] = RumpH;
		variable["RumPr"] = RumPr;
		variable["RumPr1"] = RumPr1;
		variable["RumPrP"] = RumPrP;
		variable["RumPrSynth"] = RumPrSynth;
		variable["RumSPartSizeMean"] = RumSPartSizeMean;
		variable["RumVol"] = RumVol;
		variable["RUPinFd"] = RUPinFd;
		variable["RuStinFd"] = RuStinFd;
		variable["SaAs"] = SaAs;
		variable["SaIn"] = SaIn;
		variable["SaNnAm"] = SaNnAm;
		variable["SaNRumAm"] = SaNRumAm;
		variable["SaPsAa"] = SaPsAa;
		variable["ScAc"] = ScAc;
		variable["ScAcAc"] = ScAcAc;
		variable["ScBu"] = ScBu;
		variable["ScBuBu"] = ScBuBu;
		variable["ScinFd"] = ScinFd;
		variable["ScLa"] = ScLa;
		variable["ScLaLa"] = ScLaLa;
		variable["ScPr"] = ScPr;
		variable["ScPrPr"] = ScPrPr;
		variable["ScTCs"] = ScTCs;
		variable["SolDM"] = SolDM;
		variable["SolDMP"] = SolDMP;
		variable["SolOmP"] = SolOmP;
		variable["Somedummyvariable"] = Somedummyvariable;
		variable["SPart"] = SPart;
		variable["SPartADFP"] = SPartADFP;
		variable["SPartCeCs"] = SPartCeCs;
		variable["SPartDeg"] = SPartDeg;
		variable["SPartHaCs"] = SPartHaCs;
		variable["SPartHbCs"] = SPartHbCs;
		variable["SPartHcCs"] = SPartHcCs;
		variable["SPartMi"] = SPartMi;
		variable["SPartMiHa"] = SPartMiHa;
		variable["SPartMiHb"] = SPartMiHb;
		variable["SPartMiP"] = SPartMiP;
		variable["SPartMiPi"] = SPartMiPi;
		variable["SPartNDFP"] = SPartNDFP;
		variable["SPartP"] = SPartP;
		variable["SPartPiAa"] = SPartPiAa;
		variable["SPartSA"] = SPartSA;
		variable["SPartSwal"] = SPartSwal;
		variable["SPartVol"] = SPartVol;
		variable["SpeciesFactor"] = SpeciesFactor;
		variable["StAc"] = StAc;
		variable["StAcAc"] = StAcAc;
		variable["StandardMetabolicBw"] = StandardMetabolicBw;
		variable["StartDIM"] = StartDIM;
		variable["StBu"] = StBu;
		variable["StBuBu"] = StBuBu;
		variable["StCs"] = StCs;
		variable["StCsFd"] = StCsFd;
		variable["STFLAG"] = STFLAG;
		variable["StHaFd"] = StHaFd;
		variable["StinFd"] = StinFd;
		variable["StLa"] = StLa;
		variable["StLaLa"] = StLaLa;
		variable["StPr"] = StPr;
		variable["StPrPr"] = StPrPr;
		variable["StSol"] = StSol;
		variable["SumBinFd"] = SumBinFd;
		variable["Sunlight"] = Sunlight;
		variable["Sunrise"] = Sunrise;
		variable["SunriseToday"] = SunriseToday;
		variable["SunSet"] = SunSet;
		variable["SunsetToday"] = SunsetToday;
		variable["SunsetTodayTemp"] = SunsetTodayTemp;
		variable["SupplementOnOffer"] = SupplementOnOffer;
		variable["tAtCalving"] = tAtCalving;
		variable["tAtConception"] = tAtConception;
		variable["TBCH4"] = TBCH4;
		variable["TBCH41"] = TBCH41;
		variable["TCd"] = TCd;
		variable["TCH4"] = TCH4;
		variable["TCH4E"] = TCH4E;
		variable["TcHyAdip"] = TcHyAdip;
		variable["TcHyVis"] = TcHyVis;
		variable["TDE"] = TDE;
		variable["test"] = test;
		variable["TFCM4z"] = TFCM4z;
		variable["THP1"] = THP1;
		variable["THP2"] = THP2;
		variable["TIME"] = TIME;
		variable["TMCH42"] = TMCH42;
		variable["TMCH4E"] = TMCH4E;
		variable["TMilkLm"] = TMilkLm;
		variable["TMilkLmYest"] = TMilkLmYest;
		variable["TMilkPm"] = TMilkPm;
		variable["TMilkPmYest"] = TMilkPmYest;
		variable["TMilkTm"] = TMilkTm;
		variable["TMilkTmYest"] = TMilkTmYest;
		variable["TNdfIn"] = TNdfIn;
		variable["TNdfInYest"] = TNdfInYest;
		variable["tNep"] = tNep;
		variable["tNepYest"] = tNepYest;
		variable["TOmP"] = TOmP;
		variable["TotDMin"] = TotDMin;
		variable["TotEating"] = TotEating;
		variable["TotEatingYest"] = TotEatingYest;
		variable["totFd"] = totFd;
		variable["TotMeals"] = TotMeals;
		variable["TotMealsYest"] = TotMealsYest;
		variable["TotNurLast"] = TotNurLast;
		variable["TotRest"] = TotRest;
		variable["TotRestYest"] = TotRestYest;
		variable["TotRumntn"] = TotRumntn;
		variable["TotRumntnYest"] = TotRumntnYest;
		variable["TotWaConsumed"] = TotWaConsumed;
		variable["TotWaUrine"] = TotWaUrine;
		variable["TotWaUrineLast"] = TotWaUrineLast;
		variable["TpCdVis"] = TpCdVis;
		variable["TpinAdip"] = TpinAdip;
		variable["TpinVis"] = TpinVis;
		variable["TpLaAdip"] = TpLaAdip;
		variable["TPRTin"] = TPRTin;
		variable["TpTmVis"] = TpTmVis;
		variable["TpTsAdip"] = TpTsAdip;
		variable["TransitSW"] = TransitSW;
		variable["TsAdip"] = TsAdip;
		variable["TsFaAdip"] = TsFaAdip;
		variable["TsFaF1"] = TsFaF1;
		variable["TStin"] = TStin;
		variable["TTOmP"] = TTOmP;
		variable["TVFA"] = TVFA;
		variable["TVolMilk"] = TVolMilk;
		variable["TVolMilkVol"] = TVolMilkVol;
		variable["TVolMilkYest"] = TVolMilkYest;
		variable["upGl"] = upGl;
		variable["UrAmFd"] = UrAmFd;
		variable["UrinationCount"] = UrinationCount;
		variable["UrinationCountDiel"] = UrinationCountDiel;
		variable["UrinationCountYest"] = UrinationCountYest;
		variable["UrinationVol"] = UrinationVol;
		variable["UrinationVolDiel"] = UrinationVolDiel;
		variable["UrinationVolYest"] = UrinationVolYest;
		variable["UrNFd"] = UrNFd;
		variable["VisDna"] = VisDna;
		variable["VmAcTs"] = VmAcTs;
		variable["VmAcTs2"] = VmAcTs2;
		variable["VmGlLm1Vis"] = VmGlLm1Vis;
		variable["VmLHorSyn"] = VmLHorSyn;
		variable["VolAa"] = VolAa;
		variable["VolAc"] = VolAc;
		variable["VolFa"] = VolFa;
		variable["VolGl"] = VolGl;
		variable["WaConsumed"] = WaConsumed;
		variable["WaFeces"] = WaFeces;
		variable["WaIn"] = WaIn;
		variable["WaMi"] = WaMi;
		variable["WaMilk"] = WaMilk;
		variable["WaMiP"] = WaMiP;
		variable["WaOut"] = WaOut;
		variable["WaRespir"] = WaRespir;
		variable["WaSweat"] = WaSweat;
		variable["WaUrine"] = WaUrine;
		variable["WaUrineYest"] = WaUrineYest;
		variable["WtAcTm"] = WtAcTm;
		variable["WtAdip"] = WtAdip;
		variable["WtAdipNew"] = WtAdipNew;
		variable["WtAdipTarget"] = WtAdipTarget;
		variable["WtConc"] = WtConc;
		variable["WtConcAgeFactor"] = WtConcAgeFactor;
		variable["WtConcSyn"] = WtConcSyn;
		variable["WtCytAdip"] = WtCytAdip;
		variable["WtFaTm"] = WtFaTm;
		variable["WtGrvUter"] = WtGrvUter;
		variable["WtOth"] = WtOth;
		variable["WtPConc"] = WtPConc;
		variable["WtPConcSyn"] = WtPConcSyn;
		variable["WtPGrvUter"] = WtPGrvUter;
		variable["WtPGrvUterSyn"] = WtPGrvUterSyn;
		variable["WtPOth"] = WtPOth;
		variable["WtPUter"] = WtPUter;
		variable["WtPUterDeg"] = WtPUterDeg;
		variable["WtPUterSyn"] = WtPUterSyn;
		variable["WtPVis"] = WtPVis;
		variable["WtTsAdip"] = WtTsAdip;
		variable["WtUter"] = WtUter;
		variable["WtUterDeg"] = WtUterDeg;
		variable["WtUterPart"] = WtUterPart;
		variable["WtUterSyn"] = WtUterSyn;
		variable["WtVis"] = WtVis;
		variable["YAtp"] = YAtp;
		variable["YAtpAp"] = YAtpAp;
		variable["iTCH4"] = iTCH4;
		variable["iRumLaF"] = iRumLaF;
		variable["kConcSynDecay"] = kConcSynDecay;
		variable["kUterSynDecay"] = kUterSynDecay;
		variable["iAaF"] = iAaF;
		variable["kPConcSynDecay"] = kPConcSynDecay;
		variable["iGlF"] = iGlF;
		variable["KYAtAa"] = KYAtAa;
		variable["iOthDnaF"] = iOthDnaF;
		variable["iVisDnaF"] = iVisDnaF;
		variable["iRumAaF"] = iRumAaF;
		variable["KDnaVis"] = KDnaVis;
		variable["iMiHaF"] = iMiHaF;
		variable["kVmGlLmDeg"] = kVmGlLmDeg;
		variable["icFa"] = icFa;
		variable["iFaF"] = iFaF;
		variable["K1TsFa"] = K1TsFa;
		variable["KFaTmVis"] = KFaTmVis;
		variable["KFaTsAdip"] = KFaTsAdip;
		variable["iCsF"] = iCsF;
		variable["iMiHbF"] = iMiHbF;
		variable["KDnaOth"] = KDnaOth;
		variable["KFatPi"] = KFatPi;
		variable["iWtPConc"] = iWtPConc;
		variable["iFlF"] = iFlF;
		variable["BaseMamCellsTurnOver"] = BaseMamCellsTurnOver;
		variable["CINT"] = CINT;
		variable["K1AcTm"] = K1AcTm;
		variable["K1AcTs"] = K1AcTs;
		variable["VmFaTmVis"] = VmFaTmVis;
		variable["iAcF"] = iAcF;
		variable["iRumBuF"] = iRumBuF;
		variable["iPiF"] = iPiF;
		variable["K1FaTm"] = K1FaTm;
		variable["KGlLaOth"] = KGlLaOth;
		variable["KFGAm"] = KFGAm;
		variable["VmGlLmVisPart"] = VmGlLmVisPart;
		variable["iAsF"] = iAsF;
		variable["iLPartF"] = iLPartF;
		variable["iAmF"] = iAmF;
		variable["icAc"] = icAc;
		variable["KAcTmVis"] = KAcTmVis;
		variable["KAcTsAdip"] = KAcTsAdip;
		variable["cSaPs"] = cSaPs;
		variable["iIndigFdF"] = iIndigFdF;
		variable["K1FaTs"] = K1FaTs;
		variable["KAaLmVis"] = KAaLmVis;
		variable["KAcCd"] = KAcCd;
		variable["KAaPmVis"] = KAaPmVis;
		variable["KFaCd"] = KFaCd;
		variable["KAaPOthOth"] = KAaPOthOth;
		variable["KAaPVisVis"] = KAaPVisVis;
		variable["IntakeDeclineSlope"] = IntakeDeclineSlope;
		variable["lambdaMamCells"] = lambdaMamCells;
		variable["VmAaPmVis"] = VmAaPmVis;
		variable["iCeF"] = iCeF;
		variable["iHcF"] = iHcF;
		variable["iMiF"] = iMiF;
		variable["iHaF"] = iHaF;
		variable["icGl"] = icGl;
		variable["kAHor1Gl"] = kAHor1Gl;
		variable["kCHorGl"] = kCHorGl;
		variable["KGlLmVis"] = KGlLmVis;
		variable["KGlTpAdip"] = KGlTpAdip;
		variable["KGlTpVis"] = KGlTpVis;
		variable["KiAm"] = KiAm;
		variable["kMamCellsDeclineBase"] = kMamCellsDeclineBase;
		variable["MamCellsDecayRateOfSenescence"] = MamCellsDecayRateOfSenescence;
		variable["iRumPrF"] = iRumPrF;
		variable["cAaBase"] = cAaBase;
		variable["kAHorGl"] = kAHorGl;
		variable["kCHor1Gl"] = kCHor1Gl;
		variable["MAXT"] = MAXT;
		variable["KRumAaFv"] = KRumAaFv;
		variable["iBldUrF"] = iBldUrF;
		variable["KBldUrAm"] = KBldUrAm;
		variable["VmAcTmVis"] = VmAcTmVis;
		variable["fAcSilage"] = fAcSilage;
		variable["iRumAcF"] = iRumAcF;
		variable["fSaAs"] = fSaAs;
		variable["K1MamCells"] = K1MamCells;
		variable["KCsFv"] = KCsFv;
		variable["VmGlTpVis"] = VmGlTpVis;
		variable["cGlBase"] = cGlBase;
		variable["iMamPmF"] = iMamPmF;
		variable["iMamTmF"] = iMamTmF;
		variable["KAaGlVis"] = KAaGlVis;
		variable["kMilkAsh"] = kMilkAsh;
		variable["fAiFdBase"] = fAiFdBase;
		variable["iAtAdh"] = iAtAdh;
		variable["KGlCd"] = KGlCd;
		variable["kMilkingFrequencyLagUp"] = kMilkingFrequencyLagUp;
		variable["kUterSyn"] = kUterSyn;
		variable["kWaSweat"] = kWaSweat;
		variable["MwtAm"] = MwtAm;
		variable["MwtCH4"] = MwtCH4;
		variable["MwtN"] = MwtN;
		variable["RumYAtp"] = RumYAtp;
		variable["KHyEruct"] = KHyEruct;
		variable["kConcSyn"] = kConcSyn;
		variable["kMamCellsUsMfDecay"] = kMamCellsUsMfDecay;
		variable["fEndogLiFd"] = fEndogLiFd;
		variable["iVolAaF"] = iVolAaF;
		variable["iVolGlF"] = iVolGlF;
		variable["KFatFG"] = KFatFG;
		variable["KFatHb"] = KFatHb;
		variable["uTMamCells"] = uTMamCells;
		variable["EnergyForPregnancyFactor"] = EnergyForPregnancyFactor;
		variable["ifPm"] = ifPm;
		variable["ifTm"] = ifTm;
		variable["kComminuteOralMin"] = kComminuteOralMin;
		variable["KPOthAaOth"] = KPOthAaOth;
		variable["kWaRespir"] = kWaRespir;
		variable["MinLPRumntnF"] = MinLPRumntnF;
		variable["VmGlTpAdip"] = VmGlTpAdip;
		variable["AcTgTg"] = AcTgTg;
		variable["CappingForIntake"] = CappingForIntake;
		variable["fLm"] = fLm;
		variable["kMamCellsTransitionSteepness"] = kMamCellsTransitionSteepness;
		variable["KMiHbF"] = KMiHbF;
		variable["kVmGlLmSyn"] = kVmGlLmSyn;
		variable["TAveMilkMam"] = TAveMilkMam;
		variable["VmGlLaOth"] = VmGlLaOth;
		variable["KMPartP"] = KMPartP;
		variable["TAveabsE"] = TAveabsE;
		variable["kPConcSyn"] = kPConcSyn;
		variable["MEAN2"] = MEAN2;
		variable["VmBldUrAm"] = VmBldUrAm;
		variable["fLaSilage"] = fLaSilage;
		variable["MwtAc"] = MwtAc;
		variable["MwtUr"] = MwtUr;
		variable["VmTsFaAdip"] = VmTsFaAdip;
		variable["MwtPr"] = MwtPr;
		variable["MwtAs"] = MwtAs;
		variable["MwtBu"] = MwtBu;
		variable["OthDnaMx"] = OthDnaMx;
		variable["VisDnaMx"] = VisDnaMx;
		variable["FaScFd"] = FaScFd;
		variable["HrtWrk"] = HrtWrk;
		variable["KMiHaF"] = KMiHaF;
		variable["KPVisAaVis"] = KPVisAaVis;
		variable["MiNnNn"] = MiNnNn;
		variable["MwtGy"] = MwtGy;
		variable["MwtLa"] = MwtLa;
		variable["iMamLmF"] = iMamLmF;
		variable["KabsLa"] = KabsLa;
		variable["PartThick"] = PartThick;
		variable["PResidMamMilk"] = PResidMamMilk;
		variable["fPUter"] = fPUter;
		variable["AMP1FT"] = AMP1FT;
		variable["AMP2FT"] = AMP2FT;
		variable["EnergyForMilkFactor"] = EnergyForMilkFactor;
		variable["fOaPe"] = fOaPe;
		variable["KDayLength"] = KDayLength;
		variable["KHyOther"] = KHyOther;
		variable["kMamAQBase"] = kMamAQBase;
		variable["KNaAdip"] = KNaAdip;
		variable["KNaOth"] = KNaOth;
		variable["kVmGlLmDecay"] = kVmGlLmDecay;
		variable["LgutDCAi"] = LgutDCAi;
		variable["LgutDCHb"] = LgutDCHb;
		variable["LiChFd"] = LiChFd;
		variable["LiScSC"] = LiScSC;
		variable["MaxLossDueToLowMf"] = MaxLossDueToLowMf;
		variable["MiLiLI"] = MiLiLI;
		variable["MwtAa"] = MwtAa;
		variable["MwtCe"] = MwtCe;
		variable["MwtCh"] = MwtCh;
		variable["MwtCs"] = MwtCs;
		variable["MwtHc"] = MwtHc;
		variable["MwtOa"] = MwtOa;
		variable["MwtPe"] = MwtPe;
		variable["MwtPi"] = MwtPi;
		variable["MwtPOth"] = MwtPOth;
		variable["MwtPs"] = MwtPs;
		variable["MwtPVis"] = MwtPVis;
		variable["MwtRumAa"] = MwtRumAa;
		variable["MwtSc"] = MwtSc;
		variable["MwtSt"] = MwtSt;
		variable["MwtVa"] = MwtVa;
		variable["ObsEUr"] = ObsEUr;
		variable["TAveMilk"] = TAveMilk;
		variable["xOadIntakeTadIntake"] = xOadIntakeTadIntake;
		variable["BcsTargetDecay"] = BcsTargetDecay;
		variable["fBuAc"] = fBuAc;
		variable["KTsFaAdip"] = KTsFaAdip;
		variable["kUterDeg"] = kUterDeg;
		variable["MamCellsProliferationDecayRate"] = MamCellsProliferationDecayRate;
		variable["iWtUter"] = iWtUter;
		variable["AaFvBu"] = AaFvBu;
		variable["fDWt"] = fDWt;
		variable["fLaCdAdip"] = fLaCdAdip;
		variable["fLaCdOth"] = fLaCdOth;
		variable["GlGlHy"] = GlGlHy;
		variable["kMamCellsQAPostPeak"] = kMamCellsQAPostPeak;
		variable["KNaVis"] = KNaVis;
		variable["MiHaHA"] = MiHaHA;
		variable["MwtFa"] = MwtFa;
		variable["MwtFl"] = MwtFl;
		variable["ObsCH4"] = ObsCH4;
		variable["VmAaGlVis"] = VmAaGlVis;
		variable["xLHorSensAdip"] = xLHorSensAdip;
		variable["fHeifers"] = fHeifers;
		variable["PartWidth"] = PartWidth;
		variable["FaTgTg"] = FaTgTg;
		variable["ifMPartRum"] = ifMPartRum;
		variable["K2VAct"] = K2VAct;
		variable["KidWrk"] = KidWrk;
		variable["KSPartP"] = KSPartP;
		variable["MwtLm"] = MwtLm;
		variable["MwtNn"] = MwtNn;
		variable["ResWrk"] = ResWrk;
		variable["RumntnF"] = RumntnF;
		variable["VmAcTsAdip"] = VmAcTsAdip;
		variable["VmFaTsAdip"] = VmFaTsAdip;
		variable["wLHorSensAdip"] = wLHorSensAdip;
		variable["AaGlGl"] = AaGlGl;
		variable["iWtConc"] = iWtConc;
		variable["kMamCellsQAPrePeak"] = kMamCellsQAPrePeak;
		variable["KMiHb"] = KMiHb;
		variable["AmUrUr"] = AmUrUr;
		variable["GlLmLm"] = GlLmLm;
		variable["GyGlGl"] = GyGlGl;
		variable["KLaFv"] = KLaFv;
		variable["LaGlGl"] = LaGlGl;
		variable["MiLiBu"] = MiLiBu;
		variable["MiLiGl"] = MiLiGl;
		variable["MilkInt"] = MilkInt;
		variable["PrGlGl"] = PrGlGl;
		variable["xLHorSensGl"] = xLHorSensGl;
		variable["AaGlH"] = AaGlH;
		variable["CdMiG1"] = CdMiG1;
		variable["ifSPartRum"] = ifSPartRum;
		variable["kEnergyCompensation"] = kEnergyCompensation;
		variable["MiPiPI"] = MiPiPI;
		variable["AaGlAc"] = AaGlAc;
		variable["fGrvUterTO"] = fGrvUterTO;
		variable["pGlHyAdip"] = pGlHyAdip;
		variable["pGlHyVis"] = pGlHyVis;
		variable["AaFvAc"] = AaFvAc;
		variable["AaFvPr"] = AaFvPr;
		variable["AaGlUr"] = AaGlUr;
		variable["fGestEPrt"] = fGestEPrt;
		variable["iVolAcF"] = iVolAcF;
		variable["LgutDCFa"] = LgutDCFa;
		variable["LgutDCPi"] = LgutDCPi;
		variable["MwtLiFd"] = MwtLiFd;
		variable["OaScSC"] = OaScSC;
		variable["DCMiLi"] = DCMiLi;
		variable["EnergyForMilkingCowMaintenancePower"] = EnergyForMilkingCowMaintenancePower;
		variable["fPrGl"] = fPrGl;
		variable["DCMiPi"] = DCMiPi;
		variable["EnergyForDryCowMaintenancePower"] = EnergyForDryCowMaintenancePower;
		variable["KWaFeces"] = KWaFeces;
		variable["OnceADay2YearsOldAdjustment"] = OnceADay2YearsOldAdjustment;
		variable["refIngrKAdfDeg"] = refIngrKAdfDeg;
		variable["ikMilkInh"] = ikMilkInh;
		variable["MiLiCh"] = MiLiCh;
		variable["xLHorSensAa"] = xLHorSensAa;
		variable["LaAcAc"] = LaAcAc;
		variable["LgutDCAs"] = LgutDCAs;
		variable["LgutDCHa"] = LgutDCHa;
		variable["MwtFaFd"] = MwtFaFd;
		variable["MwtTm"] = MwtTm;
		variable["MwtTs"] = MwtTs;
		variable["OnceADayMilkingAdjustment"] = OnceADayMilkingAdjustment;
		variable["VmMiHa"] = VmMiHa;
		variable["VmMiHb"] = VmMiHb;
		variable["pLPartMPartComm"] = pLPartMPartComm;
		variable["AaFvAt"] = AaFvAt;
		variable["ExpV2"] = ExpV2;
		variable["FHcCs1"] = FHcCs1;
		variable["kComminuteOralMax"] = kComminuteOralMax;
		variable["LaScSc"] = LaScSc;
		variable["OsWaSlp"] = OsWaSlp;
		variable["PeScSC"] = PeScSC;
		variable["absAaAd"] = absAaAd;
		variable["absGlAd"] = absGlAd;
		variable["AcCor"] = AcCor;
		variable["ASCor"] = ASCor;
		variable["BldUrCor"] = BldUrCor;
		variable["BST"] = BST;
		variable["CeCor"] = CeCor;
		variable["CurrentEvent"] = CurrentEvent;
		variable["CurrHerbage"] = CurrHerbage;
		variable["CurrStrat"] = CurrStrat;
		variable["EatingSW"] = EatingSW;
		variable["EatSW5"] = EatSW5;
		variable["EPart"] = EPart;
		variable["F1"] = F1;
		variable["FaCor"] = FaCor;
		variable["FLCor"] = FLCor;
		variable["GlHyAD"] = GlHyAD;
		variable["GlHyTp"] = GlHyTp;
		variable["GlLmAd"] = GlLmAd;
		variable["HaCor"] = HaCor;
		variable["HbCor"] = HbCor;
		variable["HcCor"] = HcCor;
		variable["HMM"] = HMM;
		variable["iAnimal"] = iAnimal;
		variable["iHerd"] = iHerd;
		variable["iLHorF"] = iLHorF;
		variable["iMamMilkAveF"] = iMamMilkAveF;
		variable["IndigFdCor"] = IndigFdCor;
		variable["INS"] = INS;
		variable["iVolBldUrF"] = iVolBldUrF;
		variable["iVolFaF"] = iVolFaF;
		variable["KMilkInhDeg"] = KMilkInhDeg;
		variable["KumarMigEq"] = KumarMigEq;
		variable["LaFvAt"] = LaFvAt;
		variable["LPartCor"] = LPartCor;
		variable["MamMilkCor"] = MamMilkCor;
		variable["MICor"] = MICor;
		variable["MiHaCor"] = MiHaCor;
		variable["MIHbCor"] = MIHbCor;
		variable["MiLiPr"] = MiLiPr;
		variable["MilkProductionAgeAdjustment"] = MilkProductionAgeAdjustment;
		variable["MIXSET"] = MIXSET;
		variable["NSTP"] = NSTP;
		variable["OxPrGl"] = OxPrGl;
		variable["PICor"] = PICor;
		variable["RQEQ"] = RQEQ;
		variable["RumAcCor"] = RumAcCor;
		variable["RumBuCor"] = RumBuCor;
		variable["RumLaCor"] = RumLaCor;
		variable["RumpHCON"] = RumpHCON;
		variable["RumPrCor"] = RumPrCor;
		variable["slpKRuAdf"] = slpKRuAdf;
		variable["slpKRUP"] = slpKRUP;
		variable["slpKRUST"] = slpKRUST;
		variable["T3"] = T3;
		variable["TgGyGy"] = TgGyGy;
		variable["Theta3"] = Theta3;
		variable["TpTpTm"] = TpTpTm;
		variable["TpTpTs"] = TpTpTs;
		variable["WtConcBreedFactor"] = WtConcBreedFactor;
		variable["HcCeCe"] = HcCeCe;
		variable["MilkDen"] = MilkDen;
		variable["EnergyForDryCowMaintenanceFactor"] = EnergyForDryCowMaintenanceFactor;
		variable["MilkIntPowerForFMamCelsQA1"] = MilkIntPowerForFMamCelsQA1;
		variable["AaFvHy"] = AaFvHy;
		variable["AmMiG2"] = AmMiG2;
		variable["ExpOth2"] = ExpOth2;
		variable["KbasAdip"] = KbasAdip;
		variable["MiLiFA"] = MiLiFA;
		variable["MPartSize"] = MPartSize;
		variable["AaFvCd"] = AaFvCd;
		variable["kSpecies"] = kSpecies;
		variable["wLHorSensGl"] = wLHorSensGl;
		variable["AaFvAm"] = AaFvAm;
		variable["refIngrKHaCs"] = refIngrKHaCs;
		variable["kMastication"] = kMastication;
		variable["KMiHa"] = KMiHa;
		variable["kMilkingFrequencyLagDown"] = kMilkingFrequencyLagDown;
		variable["wLHorSensAa"] = wLHorSensAa;
		variable["EnergyForMilkingCowMaintenanceFactor"] = EnergyForMilkingCowMaintenanceFactor;
		variable["HyAcFa"] = HyAcFa;
		variable["iKPiAa"] = iKPiAa;
		variable["OsMolF"] = OsMolF;
		variable["LiFlFd"] = LiFlFd;
		variable["AcCdCd"] = AcCdCd;
		variable["EXP10"] = EXP10;
		variable["GlGyGY"] = GlGyGY;
		variable["GlLaAt"] = GlLaAt;
		variable["GlLaLa"] = GlLaLa;
		variable["GlTpAD"] = GlTpAD;
		variable["GlTpTp"] = GlTpTp;
		variable["GyGlAt"] = GyGlAt;
		variable["iParity"] = iParity;
		variable["kLHorSensAa"] = kLHorSensAa;
		variable["kLHorSensGl"] = kLHorSensGl;
		variable["kMamCellsQAKickStartDecay"] = kMamCellsQAKickStartDecay;
		variable["kMamCellsQAStart"] = kMamCellsQAStart;
		variable["MaxBladderVol"] = MaxBladderVol;
		variable["OxAcCd"] = OxAcCd;
		variable["P1"] = P1;
		variable["PrGlAd"] = PrGlAd;
		variable["TpLaAt"] = TpLaAt;
		variable["UrAmAm"] = UrAmAm;
		variable["BcsTargetNadir"] = BcsTargetNadir;
		variable["CsMiG2"] = CsMiG2;
		variable["KbasOth"] = KbasOth;
		variable["MamCellsPerKgMs270"] = MamCellsPerKgMs270;
		variable["ObsME"] = ObsME;
		variable["iME"] = iME;
		variable["HyMiG1"] = HyMiG1;
		variable["AcFaAd"] = AcFaAd;
		variable["FKRuP"] = FKRuP;
		variable["BCSBase"] = BCSBase;
		variable["EnergyForMilkPower"] = EnergyForMilkPower;
		variable["FaFlFd"] = FaFlFd;
		variable["GlHyCd"] = GlHyCd;
		variable["HyAtAt"] = HyAtAt;
		variable["LaCdCd"] = LaCdCd;
		variable["LaGlAd"] = LaGlAd;
		variable["ObsDE"] = ObsDE;
		variable["OxLaCd"] = OxLaCd;
		variable["OxTpCd"] = OxTpCd;
		variable["PcPeFd"] = PcPeFd;
		variable["PrCdCd"] = PrCdCd;
		variable["TgFaFa"] = TgFaFa;
		variable["TpCdCd"] = TpCdCd;
		variable["FKRuSt"] = FKRuSt;
		variable["MyPi"] = MyPi;
		variable["KWAP"] = KWAP;
		variable["OxPrCd"] = OxPrCd;
		variable["NnAmAM"] = NnAmAM;
		variable["refIngrKPiAa"] = refIngrKPiAa;
		variable["AtAmUr"] = AtAmUr;
		variable["BuCdCd"] = BuCdCd;
		variable["CsFvAt"] = CsFvAt;
		variable["IALG"] = IALG;
		variable["McalToMJ"] = McalToMJ;
		variable["Theta4"] = Theta4;
		variable["KMPartSPart"] = KMPartSPart;
		variable["kRetMilkI"] = kRetMilkI;
		variable["LPartSize"] = LPartSize;
		variable["AaMiG2"] = AaMiG2;
		variable["AaPxAD"] = AaPxAD;
		variable["iAgeInYears"] = iAgeInYears;
		variable["MastJawMoveBolus"] = MastJawMoveBolus;
		variable["OxBuCd"] = OxBuCd;
		variable["Theta1"] = Theta1;
		variable["numVersion"] = numVersion;
		variable["TcHyAd"] = TcHyAd;
		variable["FKRuAdf"] = FKRuAdf;
		variable["Theta2"] = Theta2;
		variable["GlCdCd"] = GlCdCd;
		variable["kAaGlGest"] = kAaGlGest;
		variable["OxGlCd"] = OxGlCd;
		variable["CsMiG1"] = CsMiG1;
		variable["iKHcCs"] = iKHcCs;
		variable["KabsAc"] = KabsAc;
		variable["RumpHBase"] = RumpHBase;
		variable["K1VAct"] = K1VAct;
		variable["AmMiG1"] = AmMiG1;
		variable["CeMeanRRT"] = CeMeanRRT;
		variable["HaMeanRRT"] = HaMeanRRT;
		variable["PiMeanRRT"] = PiMeanRRT;
		variable["IntakeVersion"] = IntakeVersion;
		variable["KbasVis"] = KbasVis;
		variable["iKCeCs"] = iKCeCs;
		variable["version"] = version;
		variable["TpTgAD"] = TpTgAD;
		variable["AaCor"] = AaCor;
		variable["AcCdAt"] = AcCdAt;
		variable["AcquisitionJawMovesCurrent"] = AcquisitionJawMovesCurrent;
		variable["AmCor"] = AmCor;
		variable["CsCor"] = CsCor;
		variable["GlCor"] = GlCor;
		variable["iMilkAve"] = iMilkAve;
		variable["RumAaCor"] = RumAaCor;
		variable["THETA5"] = THETA5;
		variable["KabsBu"] = KabsBu;
		variable["NumberOfFeeds"] = NumberOfFeeds;
		variable["iKHaCs"] = iKHaCs;
		variable["KabsPr"] = KabsPr;
		variable["PMamEnzCell"] = PMamEnzCell;
		variable["vfaeff"] = vfaeff;
		variable["FaCdCd"] = FaCdCd;
		variable["kMamCellsTransitionDim"] = kMamCellsTransitionDim;
		variable["LaCdAt"] = LaCdAt;
		variable["PrCdAt"] = PrCdAt;
		variable["FdRatWFM"] = FdRatWFM;
		variable["iDMilk"] = iDMilk;
		variable["LHorBase"] = LHorBase;
		variable["LhorTurnoverDays"] = LhorTurnoverDays;
		variable["SmoothingPeriodDays"] = SmoothingPeriodDays;
		variable["TpCdAt"] = TpCdAt;
		variable["KLPartRed"] = KLPartRed;
		variable["OxFaCd"] = OxFaCd;
		variable["KAmabs"] = KAmabs;
		variable["AcAcTs"] = AcAcTs;
		variable["BuCdAt"] = BuCdAt;
		variable["kSurfaceArea"] = kSurfaceArea;
		variable["CountDownDays"] = CountDownDays;
		variable["GlCdAt"] = GlCdAt;
		variable["Latitude"] = Latitude;
		variable["MwtCd"] = MwtCd;
		variable["iFCM4Z"] = iFCM4Z;
		variable["KAsabs"] = KAsabs;
		variable["iMEi"] = iMEi;
		variable["iDEi"] = iDEi;
		variable["PeakIntakeDay"] = PeakIntakeDay;
		variable["VsizF"] = VsizF;
		variable["DaysDry"] = DaysDry;
		variable["DaysOpen"] = DaysOpen;
		variable["kInitRumVol"] = kInitRumVol;
		variable["KMilk"] = KMilk;
		variable["FaCdAt"] = FaCdAt;
		variable["iOxup"] = iOxup;
		variable["BolusWeightTotalCurrent"] = BolusWeightTotalCurrent;
		variable["MamCellsPerKgMsAdjustment"] = MamCellsPerKgMsAdjustment;
		variable["MaxBolusWeight"] = MaxBolusWeight;
		variable["VmAaPVisVis"] = VmAaPVisVis;
		variable["VmAaPOthOth"] = VmAaPOthOth;
		variable["StartDayGest"] = StartDayGest;
		variable["iDayOfYear"] = iDayOfYear;
		variable["GestLength"] = GestLength;
		variable["WaPool"] = WaPool;
		variable["WaPoolTarget"] = WaPoolTarget;
		variable["TSTP"] = TSTP;
		variable["VmRumAaFv"] = VmRumAaFv;
		variable["KgMilkSolidsExpectedIn270Days"] = KgMilkSolidsExpectedIn270Days;
		variable["StandardBw"] = StandardBw;
		variable["MatBW"] = MatBW;
		variable["MamCellsPart"] = MamCellsPart;
		variable["VmCsFv"] = VmCsFv;
		variable["KBldUrU"] = KBldUrU;
		variable["SecondsPerDay"] = SecondsPerDay;
	
	} // end pull_variables_from_model
	
	void push_variables_to_model ( ) {
		
		// push system time
		t = variable["t"];
		
		// push model variables
		Expt = variable["Expt"];
		GlobalDMIEqn = variable["GlobalDMIEqn"];
		jj = variable["jj"];
		logCriteria1 = variable["logCriteria1"];
		Old = variable["Old"];
		Silage = variable["Silage"];
		HyMiG2 = variable["HyMiG2"];
		AA = variable["AA"];
		AA1 = variable["AA1"];
		AaAcV1 = variable["AaAcV1"];
		AaAm = variable["AaAm"];
		AaFvFat = variable["AaFvFat"];
		AaGlGest = variable["AaGlGest"];
		AaGlHt = variable["AaGlHt"];
		AaGlV1 = variable["AaGlV1"];
		AaGlVis = variable["AaGlVis"];
		AaPConc = variable["AaPConc"];
		AaPGest = variable["AaPGest"];
		AaPmHt = variable["AaPmHt"];
		AaPmVis = variable["AaPmVis"];
		AaPOthHt = variable["AaPOthHt"];
		AaPOthOth = variable["AaPOthOth"];
		AaPUter = variable["AaPUter"];
		AaPVisVis = variable["AaPVisVis"];
		AaTO = variable["AaTO"];
		AaUrGest = variable["AaUrGest"];
		AaUrVis = variable["AaUrVis"];
		AbortPregNow = variable["AbortPregNow"];
		AbortPregNowVariable = variable["AbortPregNowVariable"];
		AbsAa = variable["AbsAa"];
		absAaE = variable["absAaE"];
		absAaHt = variable["absAaHt"];
		absAc = variable["absAc"];
		AbsAcE = variable["AbsAcE"];
		AbsAm = variable["AbsAm"];
		absAs = variable["absAs"];
		absBu = variable["absBu"];
		absBuE = variable["absBuE"];
		AbsE = variable["AbsE"];
		absEAve = variable["absEAve"];
		absEF = variable["absEF"];
		absFa = variable["absFa"];
		absFaE = variable["absFaE"];
		absGl = variable["absGl"];
		absGlE = variable["absGlE"];
		absGlHt = variable["absGlHt"];
		absLa = variable["absLa"];
		absLaE = variable["absLaE"];
		absPr = variable["absPr"];
		absPrE = variable["absPrE"];
		absRumAc = variable["absRumAc"];
		absRumAm = variable["absRumAm"];
		absRumAs = variable["absRumAs"];
		absRumBu = variable["absRumBu"];
		absRumLa = variable["absRumLa"];
		absRumPr = variable["absRumPr"];
		Ac = variable["Ac"];
		AcCd = variable["AcCd"];
		AccDEi = variable["AccDEi"];
		AccGEi = variable["AccGEi"];
		AccMEi = variable["AccMEi"];
		AcTmH1 = variable["AcTmH1"];
		AcTmH2 = variable["AcTmH2"];
		AcTmH3 = variable["AcTmH3"];
		AcTmH4 = variable["AcTmH4"];
		AcTmV1 = variable["AcTmV1"];
		AcTmVis = variable["AcTmVis"];
		AcTsAdip = variable["AcTsAdip"];
		AcTsF1 = variable["AcTsF1"];
		AcTsH1 = variable["AcTsH1"];
		AcTsH2 = variable["AcTsH2"];
		AcTsH3 = variable["AcTsH3"];
		AcTsH4 = variable["AcTsH4"];
		AcTsH5 = variable["AcTsH5"];
		AcTsH6 = variable["AcTsH6"];
		AcTsH7 = variable["AcTsH7"];
		AcTsHt = variable["AcTsHt"];
		AdAt = variable["AdAt"];
		AdAtAdip = variable["AdAtAdip"];
		AdAtB1 = variable["AdAtB1"];
		AdAtB2 = variable["AdAtB2"];
		AdAtF1 = variable["AdAtF1"];
		AdAtF2 = variable["AdAtF2"];
		AdAtOth = variable["AdAtOth"];
		AdAtV1 = variable["AdAtV1"];
		AdAtV2 = variable["AdAtV2"];
		AdAtV3 = variable["AdAtV3"];
		AdAtV4 = variable["AdAtV4"];
		AdAtV5 = variable["AdAtV5"];
		AdAtVis = variable["AdAtVis"];
		ADFIn = variable["ADFIn"];
		AdfinFd = variable["AdfinFd"];
		ADFP = variable["ADFP"];
		afAiFd = variable["afAiFd"];
		AFTmH5 = variable["AFTmH5"];
		AFTmH6 = variable["AFTmH6"];
		AfTmH9 = variable["AfTmH9"];
		AgeInYears = variable["AgeInYears"];
		AHor = variable["AHor"];
		AHor1 = variable["AHor1"];
		AiP = variable["AiP"];
		AM = variable["AM"];
		Am1 = variable["Am1"];
		AM2 = variable["AM2"];
		AmMi = variable["AmMi"];
		AmUr = variable["AmUr"];
		Animal = variable["Animal"];
		aPAGlH = variable["aPAGlH"];
		appDE = variable["appDE"];
		As = variable["As"];
		AsAsFd = variable["AsAsFd"];
		AshinFd = variable["AshinFd"];
		AshP = variable["AshP"];
		AsP = variable["AsP"];
		AtAd = variable["AtAd"];
		AtAd10 = variable["AtAd10"];
		AtAd11 = variable["AtAd11"];
		AtAd12 = variable["AtAd12"];
		AtAd13 = variable["AtAd13"];
		AtAd14 = variable["AtAd14"];
		ATAd15 = variable["ATAd15"];
		AtAdAdip = variable["AtAdAdip"];
		AtAdB1 = variable["AtAdB1"];
		AtAdF1 = variable["AtAdF1"];
		AtAdF2 = variable["AtAdF2"];
		AtAdF3 = variable["AtAdF3"];
		AtAdF4 = variable["AtAdF4"];
		AtAdGest = variable["AtAdGest"];
		AtAdGestGrth = variable["AtAdGestGrth"];
		AtAdGestTO = variable["AtAdGestTO"];
		AtAdH1 = variable["AtAdH1"];
		AtAdHT = variable["AtAdHT"];
		AtAdOth = variable["AtAdOth"];
		AtAdV1 = variable["AtAdV1"];
		AtAdV2 = variable["AtAdV2"];
		AtAdV3 = variable["AtAdV3"];
		AtAdV4 = variable["AtAdV4"];
		AtAdV5 = variable["AtAdV5"];
		AtAdV6 = variable["AtAdV6"];
		AtAdV7 = variable["AtAdV7"];
		AtAdV8 = variable["AtAdV8"];
		AtAdV9 = variable["AtAdV9"];
		AtAdVis = variable["AtAdVis"];
		AtHt = variable["AtHt"];
		AtHt1 = variable["AtHt1"];
		AtHt2 = variable["AtHt2"];
		AtHt3 = variable["AtHt3"];
		AtHt4 = variable["AtHt4"];
		AtHt5 = variable["AtHt5"];
		AtHt6 = variable["AtHt6"];
		AtHt7 = variable["AtHt7"];
		AtHt8 = variable["AtHt8"];
		AtpC = variable["AtpC"];
		AtpF = variable["AtpF"];
		AtpG = variable["AtpG"];
		AtpM = variable["AtpM"];
		basalAdip = variable["basalAdip"];
		basalOth = variable["basalOth"];
		basalVis = variable["basalVis"];
		basHtAdip = variable["basHtAdip"];
		basHtOth = variable["basHtOth"];
		basHtVis = variable["basHtVis"];
		BCH4 = variable["BCH4"];
		BCH4Fd = variable["BCH4Fd"];
		BCS = variable["BCS"];
		BCS_NZ = variable["BCS_NZ"];
		BcsTarget = variable["BcsTarget"];
		BcsTargetFactor = variable["BcsTargetFactor"];
		BladderVol = variable["BladderVol"];
		BldUr = variable["BldUr"];
		BldUr1 = variable["BldUr1"];
		BldUrAm = variable["BldUrAm"];
		BldUrMUN = variable["BldUrMUN"];
		BldUrRumAm = variable["BldUrRumAm"];
		BldUrVol = variable["BldUrVol"];
		BuCdVis = variable["BuCdVis"];
		BW = variable["BW"];
		BW1 = variable["BW1"];
		BwCorrected = variable["BwCorrected"];
		BwCorrection = variable["BwCorrection"];
		BWF = variable["BWF"];
		cAa = variable["cAa"];
		cAc = variable["cAc"];
		cAm = variable["cAm"];
		cAs = variable["cAs"];
		cBldUr = variable["cBldUr"];
		cCs = variable["cCs"];
		CdMi = variable["CdMi"];
		CdMiG2 = variable["CdMiG2"];
		Ce = variable["Ce"];
		CeAcAc = variable["CeAcAc"];
		CeBuBu = variable["CeBuBu"];
		CeCs = variable["CeCs"];
		Cein = variable["Cein"];
		CeP = variable["CeP"];
		CePrPr = variable["CePrPr"];
		cFa = variable["cFa"];
		CFatinFd = variable["CFatinFd"];
		cGl = variable["cGl"];
		cGlTarget = variable["cGlTarget"];
		CH4DEi = variable["CH4DEi"];
		CH4E = variable["CH4E"];
		CH4EFd = variable["CH4EFd"];
		CH4GEi = variable["CH4GEi"];
		CH4KGY = variable["CH4KGY"];
		CH4MEi = variable["CH4MEi"];
		CH4Milk = variable["CH4Milk"];
		ChChFd = variable["ChChFd"];
		CHor = variable["CHor"];
		CHor1 = variable["CHor1"];
		cMiHa = variable["cMiHa"];
		cMiHb = variable["cMiHb"];
		cMiSPart = variable["cMiSPart"];
		cMiWa = variable["cMiWa"];
		cMun = variable["cMun"];
		ConceiveNow = variable["ConceiveNow"];
		ConceiveNowVariable = variable["ConceiveNowVariable"];
		CONSET = variable["CONSET"];
		CorME = variable["CorME"];
		CorMEi = variable["CorMEi"];
		CorNEP = variable["CorNEP"];
		CorrectedBW = variable["CorrectedBW"];
		CPinFd = variable["CPinFd"];
		cPOth = variable["cPOth"];
		CpP = variable["CpP"];
		CPsinFd = variable["CPsinFd"];
		cPun = variable["cPun"];
		cPVis = variable["cPVis"];
		cRumAa = variable["cRumAa"];
		cRumAc = variable["cRumAc"];
		cRumBu = variable["cRumBu"];
		cRumLa = variable["cRumLa"];
		cRumPr = variable["cRumPr"];
		Cs = variable["Cs"];
		CsAc = variable["CsAc"];
		CsBu = variable["CsBu"];
		CsCd = variable["CsCd"];
		CsFv = variable["CsFv"];
		CsFvAc = variable["CsFvAc"];
		CsFvBu = variable["CsFvBu"];
		CsFvCd = variable["CsFvCd"];
		CsFvLa = variable["CsFvLa"];
		CsFvPr = variable["CsFvPr"];
		Csin = variable["Csin"];
		CsLa = variable["CsLa"];
		CsMi = variable["CsMi"];
		CsMiG = variable["CsMiG"];
		CsP = variable["CsP"];
		CsPr = variable["CsPr"];
		cTs = variable["cTs"];
		CumulativeLowMfDays = variable["CumulativeLowMfDays"];
		CurrentFeed = variable["CurrentFeed"];
		CurrentSupplement = variable["CurrentSupplement"];
		cVFA = variable["cVFA"];
		CWC = variable["CWC"];
		CWCF = variable["CWCF"];
		dAa = variable["dAa"];
		dabsEAve = variable["dabsEAve"];
		dAc = variable["dAc"];
		dAdipE = variable["dAdipE"];
		DailyDMin = variable["DailyDMin"];
		DailyMfDiff = variable["DailyMfDiff"];
		dAm = variable["dAm"];
		dAs = variable["dAs"];
		DAY = variable["DAY"];
		DayGest = variable["DayGest"];
		DayGestBasic = variable["DayGestBasic"];
		DayLength = variable["DayLength"];
		DaylengthP1 = variable["DaylengthP1"];
		DaylengthP2 = variable["DaylengthP2"];
		DaylightSavingShift = variable["DaylightSavingShift"];
		DayMilk = variable["DayMilk"];
		DayofYear = variable["DayofYear"];
		DayTwLength = variable["DayTwLength"];
		DayTwlengthP2 = variable["DayTwlengthP2"];
		dBCS = variable["dBCS"];
		dBldUr = variable["dBldUr"];
		DCadf = variable["DCadf"];
		DCCe = variable["DCCe"];
		dCd = variable["dCd"];
		dCdKg = variable["dCdKg"];
		DCDM = variable["DCDM"];
		dCe = variable["dCe"];
		dCH4g = variable["dCH4g"];
		dCH4Kg = variable["dCH4Kg"];
		DCHa = variable["DCHa"];
		DCHb = variable["DCHb"];
		DCLg = variable["DCLg"];
		DCLipid = variable["DCLipid"];
		DCndf = variable["DCndf"];
		DCOm = variable["DCOm"];
		DCPrt = variable["DCPrt"];
		dCs = variable["dCs"];
		dCsFvH = variable["dCsFvH"];
		dCsHy = variable["dCsHy"];
		dDCH4 = variable["dDCH4"];
		DE = variable["DE"];
		dEating = variable["dEating"];
		dEBW1 = variable["dEBW1"];
		DEI = variable["DEI"];
		derivMilkingFrequencyLag = variable["derivMilkingFrequencyLag"];
		dFa = variable["dFa"];
		dFl = variable["dFl"];
		dGestE = variable["dGestE"];
		dGl = variable["dGl"];
		dHa = variable["dHa"];
		dHaMi = variable["dHaMi"];
		dHb = variable["dHb"];
		dHbMi = variable["dHbMi"];
		dHc = variable["dHc"];
		dHiM4 = variable["dHiM4"];
		dHiP4 = variable["dHiP4"];
		dHyEruct = variable["dHyEruct"];
		dHyFlF = variable["dHyFlF"];
		dHyMi = variable["dHyMi"];
		dHyOther = variable["dHyOther"];
		DilRate = variable["DilRate"];
		dIndigFd = variable["dIndigFd"];
		dKMilkInh = variable["dKMilkInh"];
		dLHor = variable["dLHor"];
		dLm = variable["dLm"];
		dLmProd = variable["dLmProd"];
		dLPart = variable["dLPart"];
		dLwExclUterGutAndGrowth = variable["dLwExclUterGutAndGrowth"];
		dMaint = variable["dMaint"];
		dMamCellsA = variable["dMamCellsA"];
		dMamCellsQ = variable["dMamCellsQ"];
		dMamCellsS = variable["dMamCellsS"];
		dMamLm = variable["dMamLm"];
		dMamMilkAve = variable["dMamMilkAve"];
		dMamPm = variable["dMamPm"];
		dMamTm = variable["dMamTm"];
		dMi = variable["dMi"];
		DMilk = variable["DMilk"];
		dMilkLm = variable["dMilkLm"];
		dMilkPm = variable["dMilkPm"];
		dMilkProd = variable["dMilkProd"];
		dMilkTm = variable["dMilkTm"];
		dMilkVol = variable["dMilkVol"];
		DMP = variable["DMP"];
		dMPart = variable["dMPart"];
		dN = variable["dN"];
		dNdfIn = variable["dNdfIn"];
		dNep = variable["dNep"];
		dNonUterEBW = variable["dNonUterEBW"];
		dOthDna = variable["dOthDna"];
		dOthE = variable["dOthE"];
		dOx = variable["dOx"];
		dPi = variable["dPi"];
		dPm = variable["dPm"];
		dPmProd = variable["dPmProd"];
		dPOth = variable["dPOth"];
		dPVis = variable["dPVis"];
		dRest = variable["dRest"];
		DrinkSW = variable["DrinkSW"];
		DrnkWa = variable["DrnkWa"];
		DrnkWaDiel = variable["DrnkWaDiel"];
		DrnkWaTot = variable["DrnkWaTot"];
		DrnkWaYest = variable["DrnkWaYest"];
		dRumAa = variable["dRumAa"];
		dRumAaHy = variable["dRumAaHy"];
		dRumAc = variable["dRumAc"];
		dRumBu = variable["dRumBu"];
		dRumCd = variable["dRumCd"];
		dRumLa = variable["dRumLa"];
		dRumLiqVol = variable["dRumLiqVol"];
		dRumntn = variable["dRumntn"];
		dRumPr = variable["dRumPr"];
		DryOffNow = variable["DryOffNow"];
		DryOffNowVariable = variable["DryOffNowVariable"];
		dSPart = variable["dSPart"];
		dTCH4 = variable["dTCH4"];
		dTCH4E = variable["dTCH4E"];
		dTHy = variable["dTHy"];
		dTm = variable["dTm"];
		dTmProd = variable["dTmProd"];
		dTsAdip = variable["dTsAdip"];
		dUrea = variable["dUrea"];
		dVisDna = variable["dVisDna"];
		dVisE = variable["dVisE"];
		dVmAcTs = variable["dVmAcTs"];
		dWtAdipNew = variable["dWtAdipNew"];
		dWtGrvUter = variable["dWtGrvUter"];
		dWtOth = variable["dWtOth"];
		dWtPGrvUter = variable["dWtPGrvUter"];
		dWtPUter = variable["dWtPUter"];
		dWtTsAdip = variable["dWtTsAdip"];
		dWtUter = variable["dWtUter"];
		dWtVis = variable["dWtVis"];
		Eating = variable["Eating"];
		EatingSupplementsSW = variable["EatingSupplementsSW"];
		EatSa = variable["EatSa"];
		EatWa = variable["EatWa"];
		EB = variable["EB"];
		EBW = variable["EBW"];
		EBW1 = variable["EBW1"];
		eerActivityAtp = variable["eerActivityAtp"];
		EGrvUterCLF = variable["EGrvUterCLF"];
		ELm = variable["ELm"];
		EnergyCompensation = variable["EnergyCompensation"];
		EnergyCompenstaion = variable["EnergyCompenstaion"];
		EnergyForActivity = variable["EnergyForActivity"];
		EnergyForGrowth = variable["EnergyForGrowth"];
		EnergyForMaintenance = variable["EnergyForMaintenance"];
		EnergyForMilk = variable["EnergyForMilk"];
		EnergyForPregnancy = variable["EnergyForPregnancy"];
		EPm = variable["EPm"];
		ETm = variable["ETm"];
		EUr = variable["EUr"];
		EUrFd = variable["EUrFd"];
		Fa = variable["Fa"];
		FaCd = variable["FaCd"];
		fAcFd = variable["fAcFd"];
		fADFFd = variable["fADFFd"];
		fAiFd = variable["fAiFd"];
		fAiIndigFd = variable["fAiIndigFd"];
		FaP = variable["FaP"];
		fAsFd = variable["fAsFd"];
		fAshFd = variable["fAshFd"];
		FatAdd = variable["FatAdd"];
		FaTmH7 = variable["FaTmH7"];
		FaTmH8 = variable["FaTmH8"];
		FaTmV1 = variable["FaTmV1"];
		FaTmVis = variable["FaTmVis"];
		FaTsAdip = variable["FaTsAdip"];
		FatSc = variable["FatSc"];
		FaTsF1 = variable["FaTsF1"];
		fBCH4D = variable["fBCH4D"];
		fBCH4E = variable["fBCH4E"];
		fBCH4M = variable["fBCH4M"];
		fBuFd = variable["fBuFd"];
		fBuSilage = variable["fBuSilage"];
		FCeCs = variable["FCeCs"];
		fCeFd = variable["fCeFd"];
		FCFatFd = variable["FCFatFd"];
		fCH4DE = variable["fCH4DE"];
		fCH4E = variable["fCH4E"];
		fCH4ME = variable["fCH4ME"];
		FCM3h = variable["FCM3h"];
		FCM4Z = variable["FCM4Z"];
		FCM4z1 = variable["FCM4z1"];
		fCPFd = variable["fCPFd"];
		FCPsFd = variable["FCPsFd"];
		fCsHa = variable["fCsHa"];
		fCsHb = variable["fCsHb"];
		fd1 = variable["fd1"];
		fd2 = variable["fd2"];
		fd3 = variable["fd3"];
		FdCapMolly = variable["FdCapMolly"];
		FdDMIn = variable["FdDMIn"];
		FdGEin = variable["FdGEin"];
		fDMFd = variable["fDMFd"];
		FdRat = variable["FdRat"];
		FdRatDiel = variable["FdRatDiel"];
		FecADF = variable["FecADF"];
		FecAsh = variable["FecAsh"];
		FecCe = variable["FecCe"];
		FecCh = variable["FecCh"];
		FecDM = variable["FecDM"];
		FecENG = variable["FecENG"];
		FecFa = variable["FecFa"];
		FecFMPart = variable["FecFMPart"];
		FecFSPart = variable["FecFSPart"];
		FecHa = variable["FecHa"];
		FecHaT = variable["FecHaT"];
		FecHb = variable["FecHb"];
		FecHC = variable["FecHC"];
		FecLg = variable["FecLg"];
		FecLipid = variable["FecLipid"];
		FecMiHa = variable["FecMiHa"];
		FecMiLi = variable["FecMiLi"];
		FecMiNn = variable["FecMiNn"];
		FecMiPi = variable["FecMiPi"];
		FecMPart = variable["FecMPart"];
		FecNDF = variable["FecNDF"];
		FecOm = variable["FecOm"];
		FecPi = variable["FecPi"];
		FecPiT = variable["FecPiT"];
		FecPiTN = variable["FecPiTN"];
		FecSPart = variable["FecSPart"];
		FeedInFlag = variable["FeedInFlag"];
		fFatFd = variable["fFatFd"];
		fFIM = variable["fFIM"];
		FGAm = variable["FGAm"];
		FGFa = variable["FGFa"];
		fGlHyAdip = variable["fGlHyAdip"];
		fGlHyVis = variable["fGlHyVis"];
		fHcCs = variable["fHcCs"];
		fHcFd = variable["fHcFd"];
		fIndigFd = variable["fIndigFd"];
		FirstEvent = variable["FirstEvent"];
		FIXDpH = variable["FIXDpH"];
		FixedLhorSW = variable["FixedLhorSW"];
		Fl = variable["Fl"];
		Fl1Fd = variable["Fl1Fd"];
		FLaFd = variable["FLaFd"];
		FlFd = variable["FlFd"];
		fLgAdf = variable["fLgAdf"];
		fLgFd = variable["fLgFd"];
		fLgIndigFd = variable["fLgIndigFd"];
		fLiFd = variable["fLiFd"];
		FlMi = variable["FlMi"];
		FlMiG = variable["FlMiG"];
		fLPart = variable["fLPart"];
		fLPartADF = variable["fLPartADF"];
		fLPartAi = variable["fLPartAi"];
		fLPartCe = variable["fLPartCe"];
		fLPartHc = variable["fLPartHc"];
		fLPartIndigFd = variable["fLPartIndigFd"];
		fLPartLg = variable["fLPartLg"];
		fLPartMPartSwal = variable["fLPartMPartSwal"];
		fLPartNDF = variable["fLPartNDF"];
		fLPartNDF_NDF = variable["fLPartNDF_NDF"];
		fLPartNutIng = variable["fLPartNutIng"];
		fLPartPi = variable["fLPartPi"];
		fLPartplusMPart = variable["fLPartplusMPart"];
		fLPartSt = variable["fLPartSt"];
		fLPartSwal = variable["fLPartSwal"];
		fMamCellsAQ = variable["fMamCellsAQ"];
		fMamCellsAS = variable["fMamCellsAS"];
		fMamCellsPA = variable["fMamCellsPA"];
		fMamCellsQA = variable["fMamCellsQA"];
		fMamCellsQS = variable["fMamCellsQS"];
		fMamCellsUS = variable["fMamCellsUS"];
		fMCH4D = variable["fMCH4D"];
		fMCH4E = variable["fMCH4E"];
		fMCH4M = variable["fMCH4M"];
		fMPart = variable["fMPart"];
		fMPart1 = variable["fMPart1"];
		fMPartNDF_NDF = variable["fMPartNDF_NDF"];
		fMPartNutIng = variable["fMPartNutIng"];
		fMPartSA = variable["fMPartSA"];
		fMPartSwal = variable["fMPartSwal"];
		fNDFFd = variable["fNDFFd"];
		fNnCp = variable["fNnCp"];
		fNnFd = variable["fNnFd"];
		FNPNFd = variable["FNPNFd"];
		fOaFd = variable["fOaFd"];
		fOmFd = variable["fOmFd"];
		FORSET = variable["FORSET"];
		fParticulateFd = variable["fParticulateFd"];
		fPartP = variable["fPartP"];
		fPartSA = variable["fPartSA"];
		fPeFd = variable["fPeFd"];
		fPiCp = variable["fPiCp"];
		fPiFd = variable["fPiFd"];
		fPm = variable["fPm"];
		fPsCp = variable["fPsCp"];
		FPsFd = variable["FPsFd"];
		fRoughageFd = variable["fRoughageFd"];
		fRuAdfAdf = variable["fRuAdfAdf"];
		fRuAdfFd = variable["fRuAdfFd"];
		fRumDM = variable["fRumDM"];
		fRupCp = variable["fRupCp"];
		FRUPFd = variable["FRUPFd"];
		fRUStFd = variable["fRUStFd"];
		fRuStSt = variable["fRuStSt"];
		fScCs = variable["fScCs"];
		fScFd = variable["fScFd"];
		fScTFd = variable["fScTFd"];
		fSPart = variable["fSPart"];
		fSPart1 = variable["fSPart1"];
		fSPartNDF_NDF = variable["fSPartNDF_NDF"];
		fSPartNutIng = variable["fSPartNutIng"];
		fSPartSA = variable["fSPartSA"];
		fSPartSwal = variable["fSPartSwal"];
		fStCs = variable["fStCs"];
		fStFd = variable["fStFd"];
		FStsFd = variable["FStsFd"];
		fStSSt = variable["fStSSt"];
		fTm = variable["fTm"];
		fTm1 = variable["fTm1"];
		fUrCp = variable["fUrCp"];
		FUrFd = variable["FUrFd"];
		FvAcFd = variable["FvAcFd"];
		FvBuFd = variable["FvBuFd"];
		FvLaFd = variable["FvLaFd"];
		G1 = variable["G1"];
		G2 = variable["G2"];
		GE = variable["GE"];
		GE1Fd = variable["GE1Fd"];
		GE2Fd = variable["GE2Fd"];
		GE3Fd = variable["GE3Fd"];
		GEFd = variable["GEFd"];
		gGlLa = variable["gGlLa"];
		Gl = variable["Gl"];
		GlCd = variable["GlCd"];
		GlCdT = variable["GlCdT"];
		GlGyT = variable["GlGyT"];
		GlHyAdip = variable["GlHyAdip"];
		GlHyVis = variable["GlHyVis"];
		GlLaB1 = variable["GlLaB1"];
		GlLaOth = variable["GlLaOth"];
		GlLmHt = variable["GlLmHt"];
		GLLmVis = variable["GLLmVis"];
		GlMilk = variable["GlMilk"];
		GLMilk1 = variable["GLMilk1"];
		GlMilk2 = variable["GlMilk2"];
		GlMilk3 = variable["GlMilk3"];
		GlTO = variable["GlTO"];
		GlTpAdip = variable["GlTpAdip"];
		GlTpF1 = variable["GlTpF1"];
		GlTpV1 = variable["GlTpV1"];
		GlTpVis = variable["GlTpVis"];
		GrowthPerDay = variable["GrowthPerDay"];
		GyGlV1 = variable["GyGlV1"];
		GyGlVis = variable["GyGlVis"];
		Ha = variable["Ha"];
		HaCs = variable["HaCs"];
		HaMi = variable["HaMi"];
		HaMiF = variable["HaMiF"];
		HaMiG = variable["HaMiG"];
		HaMiP = variable["HaMiP"];
		HaMiRum = variable["HaMiRum"];
		HaP = variable["HaP"];
		HaPT = variable["HaPT"];
		Hb = variable["Hb"];
		Hbin = variable["Hbin"];
		HbMi = variable["HbMi"];
		HbMiF = variable["HbMiF"];
		HbMiG = variable["HbMiG"];
		HbMiP = variable["HbMiP"];
		HbMiRum = variable["HbMiRum"];
		HbP = variable["HbP"];
		Hc = variable["Hc"];
		HcAcAc = variable["HcAcAc"];
		HcBuBu = variable["HcBuBu"];
		HcCs = variable["HcCs"];
		Hcin = variable["Hcin"];
		HcombAa = variable["HcombAa"];
		HcombAc = variable["HcombAc"];
		HcombBu = variable["HcombBu"];
		HcombCh = variable["HcombCh"];
		HcombCH4 = variable["HcombCH4"];
		HcombCs = variable["HcombCs"];
		HcombFa = variable["HcombFa"];
		HcombFl = variable["HcombFl"];
		HcombGl = variable["HcombGl"];
		HcombGy = variable["HcombGy"];
		HcombHc = variable["HcombHc"];
		HcombLa = variable["HcombLa"];
		HcombLg = variable["HcombLg"];
		HcombLiFd = variable["HcombLiFd"];
		HcombLm = variable["HcombLm"];
		HcombMi = variable["HcombMi"];
		HcombMiLi = variable["HcombMiLi"];
		HcombNn = variable["HcombNn"];
		HcombOa = variable["HcombOa"];
		HcombPr = variable["HcombPr"];
		HcombPs = variable["HcombPs"];
		HcombTg = variable["HcombTg"];
		HcombTp = variable["HcombTp"];
		HcombUr = variable["HcombUr"];
		HcP = variable["HcP"];
		HcPrPr = variable["HcPrPr"];
		Herd = variable["Herd"];
		HFerm = variable["HFerm"];
		HFermM = variable["HFermM"];
		HiM = variable["HiM"];
		HiM1 = variable["HiM1"];
		HiM2 = variable["HiM2"];
		HiM3 = variable["HiM3"];
		HiP = variable["HiP"];
		HiP1 = variable["HiP1"];
		HiP2 = variable["HiP2"];
		HiP3 = variable["HiP3"];
		HiV8 = variable["HiV8"];
		HtF2 = variable["HtF2"];
		HtF3 = variable["HtF3"];
		HtV2 = variable["HtV2"];
		HtV3 = variable["HtV3"];
		HtV4 = variable["HtV4"];
		HtV5 = variable["HtV5"];
		HtV6 = variable["HtV6"];
		HtV7 = variable["HtV7"];
		i = variable["i"];
		iAa = variable["iAa"];
		iabsEAve = variable["iabsEAve"];
		iAc = variable["iAc"];
		iAm = variable["iAm"];
		iAs = variable["iAs"];
		iBCS = variable["iBCS"];
		iBldUr = variable["iBldUr"];
		iBW = variable["iBW"];
		iCe = variable["iCe"];
		iCs = variable["iCs"];
		iDayGestDMI = variable["iDayGestDMI"];
		iEBW = variable["iEBW"];
		iFa = variable["iFa"];
		iFdRat = variable["iFdRat"];
		ifDWt = variable["ifDWt"];
		iFl = variable["iFl"];
		iGl = variable["iGl"];
		iHa = variable["iHa"];
		iHb = variable["iHb"];
		iHbF = variable["iHbF"];
		iHc = variable["iHc"];
		iIndigFd = variable["iIndigFd"];
		iLHor = variable["iLHor"];
		ilogNewEvent = variable["ilogNewEvent"];
		iLPart = variable["iLPart"];
		iMamCells = variable["iMamCells"];
		iMamCellsA = variable["iMamCellsA"];
		iMamCellsQ = variable["iMamCellsQ"];
		iMamCellsS = variable["iMamCellsS"];
		iMamLm = variable["iMamLm"];
		iMamMilkAve = variable["iMamMilkAve"];
		iMamPm = variable["iMamPm"];
		iMamTm = variable["iMamTm"];
		iMi = variable["iMi"];
		iMiHa = variable["iMiHa"];
		iMiHb = variable["iMiHb"];
		iMPart = variable["iMPart"];
		iMPartF = variable["iMPartF"];
		IncreasedUsDueToLowMf = variable["IncreasedUsDueToLowMf"];
		IndigFd = variable["IndigFd"];
		IndigFdFd = variable["IndigFdFd"];
		IndigFdMiP = variable["IndigFdMiP"];
		IndigFdP = variable["IndigFdP"];
		InfAs = variable["InfAs"];
		InfNaBicarb = variable["InfNaBicarb"];
		InfNaCl = variable["InfNaCl"];
		InfPrt = variable["InfPrt"];
		InfRumPr = variable["InfRumPr"];
		InMilk = variable["InMilk"];
		iNonFatEBW = variable["iNonFatEBW"];
		iNonFatNonUterEBW = variable["iNonFatNonUterEBW"];
		iNonUterEBW = variable["iNonUterEBW"];
		iNonUterEbwTarget = variable["iNonUterEbwTarget"];
		IntakeDay = variable["IntakeDay"];
		IntakeTotal = variable["IntakeTotal"];
		IntakeYest = variable["IntakeYest"];
		iotGutCont = variable["iotGutCont"];
		iOthDna = variable["iOthDna"];
		iotWtOth = variable["iotWtOth"];
		iotWtVis = variable["iotWtVis"];
		iPi = variable["iPi"];
		iPOth = variable["iPOth"];
		iPVis = variable["iPVis"];
		iRumAa = variable["iRumAa"];
		iRumAc = variable["iRumAc"];
		iRumBu = variable["iRumBu"];
		iRumLa = variable["iRumLa"];
		iRumLiqVol = variable["iRumLiqVol"];
		iRumPr = variable["iRumPr"];
		iRumVol = variable["iRumVol"];
		iSPart = variable["iSPart"];
		iSPartF = variable["iSPartF"];
		iStartDayGest = variable["iStartDayGest"];
		iStartDIM = variable["iStartDIM"];
		iTotMeals = variable["iTotMeals"];
		iTsAdip = variable["iTsAdip"];
		iVisDna = variable["iVisDna"];
		iVmAcTs = variable["iVmAcTs"];
		iVolAa = variable["iVolAa"];
		iVolAc = variable["iVolAc"];
		iVolBldUr = variable["iVolBldUr"];
		iVolFa = variable["iVolFa"];
		iVolGl = variable["iVolGl"];
		iWtAdip = variable["iWtAdip"];
		iWtCytAdip = variable["iWtCytAdip"];
		iWtOth = variable["iWtOth"];
		iWtTsAdip = variable["iWtTsAdip"];
		iWtVis = variable["iWtVis"];
		j = variable["j"];
		kAcquisition = variable["kAcquisition"];
		KCeCs = variable["KCeCs"];
		KCeCs1 = variable["KCeCs1"];
		KHaCs = variable["KHaCs"];
		KHcCs = variable["KHcCs"];
		KHcCs1 = variable["KHcCs1"];
		kLHor = variable["kLHor"];
		KLHorPP = variable["KLHorPP"];
		kMamCellsQA = variable["kMamCellsQA"];
		kMamCellsQaMfAdjustment = variable["kMamCellsQaMfAdjustment"];
		KMilkI = variable["KMilkI"];
		KMinh = variable["KMinh"];
		KNaAtAdip = variable["KNaAtAdip"];
		KNaAtOth = variable["KNaAtOth"];
		KNaAtVis = variable["KNaAtVis"];
		KPiAa = variable["KPiAa"];
		LaCdAdip = variable["LaCdAdip"];
		LaCdOth = variable["LaCdOth"];
		LaGlAdip = variable["LaGlAdip"];
		LaGlOth = variable["LaGlOth"];
		LaGlV1 = variable["LaGlV1"];
		LaGlVis = variable["LaGlVis"];
		LainOth = variable["LainOth"];
		LaPrPr = variable["LaPrPr"];
		LastEA = variable["LastEA"];
		LastEv = variable["LastEv"];
		LastEvent = variable["LastEvent"];
		LastSu = variable["LastSu"];
		LastSw = variable["LastSw"];
		LginFd = variable["LginFd"];
		LgP = variable["LgP"];
		LgutAi = variable["LgutAi"];
		LgutAs = variable["LgutAs"];
		LgutCeAc = variable["LgutCeAc"];
		LgutCeBu = variable["LgutCeBu"];
		LgutCeFv = variable["LgutCeFv"];
		LgutCePr = variable["LgutCePr"];
		LgutFaDg = variable["LgutFaDg"];
		LgutHaGl = variable["LgutHaGl"];
		LgutHcAc = variable["LgutHcAc"];
		LgutHcBu = variable["LgutHcBu"];
		LgutHcFv = variable["LgutHcFv"];
		LgutHcPr = variable["LgutHcPr"];
		LgutPiAa = variable["LgutPiAa"];
		LHor = variable["LHor"];
		LHor1 = variable["LHor1"];
		LhorAa = variable["LhorAa"];
		LhorAdip = variable["LhorAdip"];
		LHorDeg = variable["LHorDeg"];
		LhorGl = variable["LhorGl"];
		LHorSyn = variable["LHorSyn"];
		LHorSyn1 = variable["LHorSyn1"];
		LipidP = variable["LipidP"];
		LiSc = variable["LiSc"];
		logNewEvent = variable["logNewEvent"];
		LowMfDecay = variable["LowMfDecay"];
		LPart = variable["LPart"];
		LPart1 = variable["LPart1"];
		LPartCeCe = variable["LPartCeCe"];
		LPartHbHb = variable["LPartHbHb"];
		LPartHcHc = variable["LPartHcHc"];
		LPartIndigFdIndigFd = variable["LPartIndigFdIndigFd"];
		LPartMi = variable["LPartMi"];
		LPartMPart = variable["LPartMPart"];
		LPartPiPi = variable["LPartPiPi"];
		LPartplusMPart = variable["LPartplusMPart"];
		LPartRed = variable["LPartRed"];
		LPartSPart = variable["LPartSPart"];
		LPartStHa = variable["LPartStHa"];
		LPartSwal = variable["LPartSwal"];
		LW = variable["LW"];
		MamCells = variable["MamCells"];
		MamCellsA = variable["MamCellsA"];
		MamCellsQ = variable["MamCellsQ"];
		MamCellsQaKickStartFactor = variable["MamCellsQaKickStartFactor"];
		MamCellsQaPreToPostFactor = variable["MamCellsQaPreToPostFactor"];
		MamCellsS = variable["MamCellsS"];
		MamEnz = variable["MamEnz"];
		MamLm = variable["MamLm"];
		MamMilk = variable["MamMilk"];
		MamMilkAve = variable["MamMilkAve"];
		MamMilkAve2 = variable["MamMilkAve2"];
		MamPm = variable["MamPm"];
		MamTm = variable["MamTm"];
		MasticationSteps = variable["MasticationSteps"];
		MaxEnergyForMilk = variable["MaxEnergyForMilk"];
		MaxRumVol = variable["MaxRumVol"];
		MCH4E = variable["MCH4E"];
		MCH4kg = variable["MCH4kg"];
		ME = variable["ME"];
		ME1 = variable["ME1"];
		MealsDay = variable["MealsDay"];
		MEAN1 = variable["MEAN1"];
		MeanParticleSize = variable["MeanParticleSize"];
		MedianParticleSize = variable["MedianParticleSize"];
		MEI = variable["MEI"];
		MEinMJ = variable["MEinMJ"];
		MEMBW = variable["MEMBW"];
		MetabPP = variable["MetabPP"];
		MHtAdip = variable["MHtAdip"];
		MHtGest = variable["MHtGest"];
		MHtGestGrth = variable["MHtGestGrth"];
		MHtGestTO = variable["MHtGestTO"];
		MHtOth = variable["MHtOth"];
		MHtVis = variable["MHtVis"];
		Mi = variable["Mi"];
		MiAa = variable["MiAa"];
		MiBu = variable["MiBu"];
		MiCh = variable["MiCh"];
		MiFa = variable["MiFa"];
		MiG = variable["MiG"];
		MiGl = variable["MiGl"];
		MiHaMi = variable["MiHaMi"];
		MiHbMi = variable["MiHbMi"];
		MiLGl = variable["MiLGl"];
		MiLiDg = variable["MiLiDg"];
		MilkingFrequency = variable["MilkingFrequency"];
		MilkingFrequencyAdjusted = variable["MilkingFrequencyAdjusted"];
		MilkingFrequencyAgeAdjustment = variable["MilkingFrequencyAgeAdjustment"];
		MilkingFrequencyBaseAdjustment = variable["MilkingFrequencyBaseAdjustment"];
		MilkingFrequencyLag = variable["MilkingFrequencyLag"];
		MilkingIndex = variable["MilkingIndex"];
		MilkInhDeg = variable["MilkInhDeg"];
		MilkInhSyn = variable["MilkInhSyn"];
		MilkMax = variable["MilkMax"];
		MilkProdDiel = variable["MilkProdDiel"];
		MilkSolids270MfAdjusted = variable["MilkSolids270MfAdjusted"];
		MilkSW = variable["MilkSW"];
		MiMaAd = variable["MiMaAd"];
		MinLPRumntn = variable["MinLPRumntn"];
		MiNOm = variable["MiNOm"];
		MiNOma = variable["MiNOma"];
		MiNP = variable["MiNP"];
		MiP = variable["MiP"];
		MiPP = variable["MiPP"];
		MiPr = variable["MiPr"];
		MiPrOm = variable["MiPrOm"];
		MirOma = variable["MirOma"];
		MntHP = variable["MntHP"];
		MPart = variable["MPart"];
		MPartADFP = variable["MPartADFP"];
		MPartDeg = variable["MPartDeg"];
		MPartNDFP = variable["MPartNDFP"];
		MPartP = variable["MPartP"];
		MPartSA = variable["MPartSA"];
		MPartSPart = variable["MPartSPart"];
		MPartSwal = variable["MPartSwal"];
		MPartVol = variable["MPartVol"];
		MPcAc = variable["MPcAc"];
		MPcBu = variable["MPcBu"];
		MPcPr = variable["MPcPr"];
		mult = variable["mult"];
		MwtMiLi = variable["MwtMiLi"];
		Nabs = variable["Nabs"];
		Nan = variable["Nan"];
		NANMNP = variable["NANMNP"];
		NANP = variable["NANP"];
		Nbal = variable["Nbal"];
		NBody = variable["NBody"];
		ndAt = variable["ndAt"];
		NDFIn = variable["NDFIn"];
		NdfinFd = variable["NdfinFd"];
		NDFP = variable["NDFP"];
		Ndiff = variable["Ndiff"];
		Ndig = variable["Ndig"];
		ndOx = variable["ndOx"];
		NEP = variable["NEP"];
		NetEff = variable["NetEff"];
		netME = variable["netME"];
		NextMilkingT = variable["NextMilkingT"];
		NFec = variable["NFec"];
		Nin = variable["Nin"];
		Nintake = variable["Nintake"];
		NitP = variable["NitP"];
		NMilk = variable["NMilk"];
		NnAmFd = variable["NnAmFd"];
		NninFd = variable["NninFd"];
		NonFatEBW = variable["NonFatEBW"];
		NonFatNonUterEBW = variable["NonFatNonUterEBW"];
		NonPreg = variable["NonPreg"];
		NonUterEBW = variable["NonUterEBW"];
		NonUterEbwTarget = variable["NonUterEbwTarget"];
		Nout = variable["Nout"];
		NpninFd = variable["NpninFd"];
		Nret1 = variable["Nret1"];
		Nret2 = variable["Nret2"];
		NSal = variable["NSal"];
		NUr = variable["NUr"];
		NurConcentration = variable["NurConcentration"];
		NurTotal = variable["NurTotal"];
		OaSc = variable["OaSc"];
		ObsCH4E = variable["ObsCH4E"];
		ObsDEi = variable["ObsDEi"];
		ObsMEi = variable["ObsMEi"];
		ObsPredCH4 = variable["ObsPredCH4"];
		ObsPredDE = variable["ObsPredDE"];
		ObsPredEUr = variable["ObsPredEUr"];
		ObsPredME = variable["ObsPredME"];
		OldBasalOth = variable["OldBasalOth"];
		OminFd = variable["OminFd"];
		OmPa = variable["OmPa"];
		OmPt = variable["OmPt"];
		OnceADay2YearsOldAdjustment1 = variable["OnceADay2YearsOldAdjustment1"];
		OsWa = variable["OsWa"];
		OsWaInt = variable["OsWaInt"];
		otGutCont = variable["otGutCont"];
		OthDna = variable["OthDna"];
		otWtOth = variable["otWtOth"];
		otWtVis = variable["otWtVis"];
		OXUP1 = variable["OXUP1"];
		Parity = variable["Parity"];
		PartFd = variable["PartFd"];
		PcLm = variable["PcLm"];
		PcPm = variable["PcPm"];
		PcSilage = variable["PcSilage"];
		PcTm = variable["PcTm"];
		PcTmFromScfa = variable["PcTmFromScfa"];
		PeSc = variable["PeSc"];
		Pi = variable["Pi"];
		PiAa = variable["PiAa"];
		PiMiP = variable["PiMiP"];
		PiP = variable["PiP"];
		PiPiFd = variable["PiPiFd"];
		PostCalvingMamCells = variable["PostCalvingMamCells"];
		POth = variable["POth"];
		POthAaOth = variable["POthAaOth"];
		POthfDr = variable["POthfDr"];
		POthfSr = variable["POthfSr"];
		PrCdVis = variable["PrCdVis"];
		PreCalvingMamCells = variable["PreCalvingMamCells"];
		Preg = variable["Preg"];
		PreviousEvent = variable["PreviousEvent"];
		PrGlHt = variable["PrGlHt"];
		PrGlV1 = variable["PrGlV1"];
		PrGlVis = variable["PrGlVis"];
		propLm = variable["propLm"];
		PsAaFd = variable["PsAaFd"];
		PsF = variable["PsF"];
		PUter = variable["PUter"];
		PUterAa = variable["PUterAa"];
		PVis = variable["PVis"];
		PVisAaVis = variable["PVisAaVis"];
		PVisfDr = variable["PVisfDr"];
		PVisfSr = variable["PVisfSr"];
		RequiredEnergy = variable["RequiredEnergy"];
		ResidMamMilk = variable["ResidMamMilk"];
		Rest = variable["Rest"];
		RestSa = variable["RestSa"];
		RestWa = variable["RestWa"];
		RQ1 = variable["RQ1"];
		rtAa1 = variable["rtAa1"];
		rtFa1 = variable["rtFa1"];
		rtGl1 = variable["rtGl1"];
		rtOx1 = variable["rtOx1"];
		rtOx2 = variable["rtOx2"];
		rtPO = variable["rtPO"];
		rtPOx = variable["rtPOx"];
		RuAdfinFd = variable["RuAdfinFd"];
		RumAa = variable["RumAa"];
		RumAaAc = variable["RumAaAc"];
		RumAaBu = variable["RumAaBu"];
		RumAaCd = variable["RumAaCd"];
		RumAaFv = variable["RumAaFv"];
		RumAaMi = variable["RumAaMi"];
		RumAaP = variable["RumAaP"];
		RumAaPr = variable["RumAaPr"];
		RumAc = variable["RumAc"];
		RumAc1 = variable["RumAc1"];
		RumAcP = variable["RumAcP"];
		RumAcSynth = variable["RumAcSynth"];
		RumADF = variable["RumADF"];
		RumBU = variable["RumBU"];
		RumBu1 = variable["RumBu1"];
		RumBuP = variable["RumBuP"];
		RumBuSynth = variable["RumBuSynth"];
		RumCein = variable["RumCein"];
		RumCP = variable["RumCP"];
		RumDCadf = variable["RumDCadf"];
		RumDCCe = variable["RumDCCe"];
		RumDCHa = variable["RumDCHa"];
		RumDCHaT = variable["RumDCHaT"];
		RumDCHb = variable["RumDCHb"];
		RumDCHc = variable["RumDCHc"];
		RumDCLiA = variable["RumDCLiA"];
		RumDCLiT = variable["RumDCLiT"];
		RumDCN = variable["RumDCN"];
		RumDCndf = variable["RumDCndf"];
		RumDCOm = variable["RumDCOm"];
		RumDCOmA = variable["RumDCOmA"];
		RumDCPrt = variable["RumDCPrt"];
		RumDM = variable["RumDM"];
		RumDPrta = variable["RumDPrta"];
		RumHcin = variable["RumHcin"];
		RumLa = variable["RumLa"];
		RumLa1 = variable["RumLa1"];
		RumLaAc = variable["RumLaAc"];
		RumLaFv = variable["RumLaFv"];
		RumLaGl = variable["RumLaGl"];
		RumLaP = variable["RumLaP"];
		RumLaPr = variable["RumLaPr"];
		RumLg = variable["RumLg"];
		RumLiqVol = variable["RumLiqVol"];
		RumLiqVolEQ = variable["RumLiqVolEQ"];
		RumLPartSizeMean = variable["RumLPartSizeMean"];
		RumLpMpSizeMean = variable["RumLpMpSizeMean"];
		RumMPartSizeMean = variable["RumMPartSizeMean"];
		RumNDF = variable["RumNDF"];
		RumNit = variable["RumNit"];
		Rumntn = variable["Rumntn"];
		RUMNTNEQ = variable["RUMNTNEQ"];
		RumntnSa = variable["RumntnSa"];
		RumOM = variable["RumOM"];
		RumOsMol = variable["RumOsMol"];
		RumPartSizeInt = variable["RumPartSizeInt"];
		RumPartSizeMean = variable["RumPartSizeMean"];
		RumPartSizeSlp = variable["RumPartSizeSlp"];
		RumpH = variable["RumpH"];
		RumPr = variable["RumPr"];
		RumPr1 = variable["RumPr1"];
		RumPrP = variable["RumPrP"];
		RumPrSynth = variable["RumPrSynth"];
		RumSPartSizeMean = variable["RumSPartSizeMean"];
		RumVol = variable["RumVol"];
		RUPinFd = variable["RUPinFd"];
		RuStinFd = variable["RuStinFd"];
		SaAs = variable["SaAs"];
		SaIn = variable["SaIn"];
		SaNnAm = variable["SaNnAm"];
		SaNRumAm = variable["SaNRumAm"];
		SaPsAa = variable["SaPsAa"];
		ScAc = variable["ScAc"];
		ScAcAc = variable["ScAcAc"];
		ScBu = variable["ScBu"];
		ScBuBu = variable["ScBuBu"];
		ScinFd = variable["ScinFd"];
		ScLa = variable["ScLa"];
		ScLaLa = variable["ScLaLa"];
		ScPr = variable["ScPr"];
		ScPrPr = variable["ScPrPr"];
		ScTCs = variable["ScTCs"];
		SolDM = variable["SolDM"];
		SolDMP = variable["SolDMP"];
		SolOmP = variable["SolOmP"];
		Somedummyvariable = variable["Somedummyvariable"];
		SPart = variable["SPart"];
		SPartADFP = variable["SPartADFP"];
		SPartCeCs = variable["SPartCeCs"];
		SPartDeg = variable["SPartDeg"];
		SPartHaCs = variable["SPartHaCs"];
		SPartHbCs = variable["SPartHbCs"];
		SPartHcCs = variable["SPartHcCs"];
		SPartMi = variable["SPartMi"];
		SPartMiHa = variable["SPartMiHa"];
		SPartMiHb = variable["SPartMiHb"];
		SPartMiP = variable["SPartMiP"];
		SPartMiPi = variable["SPartMiPi"];
		SPartNDFP = variable["SPartNDFP"];
		SPartP = variable["SPartP"];
		SPartPiAa = variable["SPartPiAa"];
		SPartSA = variable["SPartSA"];
		SPartSwal = variable["SPartSwal"];
		SPartVol = variable["SPartVol"];
		SpeciesFactor = variable["SpeciesFactor"];
		StAc = variable["StAc"];
		StAcAc = variable["StAcAc"];
		StandardMetabolicBw = variable["StandardMetabolicBw"];
		StartDIM = variable["StartDIM"];
		StBu = variable["StBu"];
		StBuBu = variable["StBuBu"];
		StCs = variable["StCs"];
		StCsFd = variable["StCsFd"];
		STFLAG = variable["STFLAG"];
		StHaFd = variable["StHaFd"];
		StinFd = variable["StinFd"];
		StLa = variable["StLa"];
		StLaLa = variable["StLaLa"];
		StPr = variable["StPr"];
		StPrPr = variable["StPrPr"];
		StSol = variable["StSol"];
		SumBinFd = variable["SumBinFd"];
		Sunlight = variable["Sunlight"];
		Sunrise = variable["Sunrise"];
		SunriseToday = variable["SunriseToday"];
		SunSet = variable["SunSet"];
		SunsetToday = variable["SunsetToday"];
		SunsetTodayTemp = variable["SunsetTodayTemp"];
		SupplementOnOffer = variable["SupplementOnOffer"];
		tAtCalving = variable["tAtCalving"];
		tAtConception = variable["tAtConception"];
		TBCH4 = variable["TBCH4"];
		TBCH41 = variable["TBCH41"];
		TCd = variable["TCd"];
		TCH4 = variable["TCH4"];
		TCH4E = variable["TCH4E"];
		TcHyAdip = variable["TcHyAdip"];
		TcHyVis = variable["TcHyVis"];
		TDE = variable["TDE"];
		test = variable["test"];
		TFCM4z = variable["TFCM4z"];
		THP1 = variable["THP1"];
		THP2 = variable["THP2"];
		TIME = variable["TIME"];
		TMCH42 = variable["TMCH42"];
		TMCH4E = variable["TMCH4E"];
		TMilkLm = variable["TMilkLm"];
		TMilkLmYest = variable["TMilkLmYest"];
		TMilkPm = variable["TMilkPm"];
		TMilkPmYest = variable["TMilkPmYest"];
		TMilkTm = variable["TMilkTm"];
		TMilkTmYest = variable["TMilkTmYest"];
		TNdfIn = variable["TNdfIn"];
		TNdfInYest = variable["TNdfInYest"];
		tNep = variable["tNep"];
		tNepYest = variable["tNepYest"];
		TOmP = variable["TOmP"];
		TotDMin = variable["TotDMin"];
		TotEating = variable["TotEating"];
		TotEatingYest = variable["TotEatingYest"];
		totFd = variable["totFd"];
		TotMeals = variable["TotMeals"];
		TotMealsYest = variable["TotMealsYest"];
		TotNurLast = variable["TotNurLast"];
		TotRest = variable["TotRest"];
		TotRestYest = variable["TotRestYest"];
		TotRumntn = variable["TotRumntn"];
		TotRumntnYest = variable["TotRumntnYest"];
		TotWaConsumed = variable["TotWaConsumed"];
		TotWaUrine = variable["TotWaUrine"];
		TotWaUrineLast = variable["TotWaUrineLast"];
		TpCdVis = variable["TpCdVis"];
		TpinAdip = variable["TpinAdip"];
		TpinVis = variable["TpinVis"];
		TpLaAdip = variable["TpLaAdip"];
		TPRTin = variable["TPRTin"];
		TpTmVis = variable["TpTmVis"];
		TpTsAdip = variable["TpTsAdip"];
		TransitSW = variable["TransitSW"];
		TsAdip = variable["TsAdip"];
		TsFaAdip = variable["TsFaAdip"];
		TsFaF1 = variable["TsFaF1"];
		TStin = variable["TStin"];
		TTOmP = variable["TTOmP"];
		TVFA = variable["TVFA"];
		TVolMilk = variable["TVolMilk"];
		TVolMilkVol = variable["TVolMilkVol"];
		TVolMilkYest = variable["TVolMilkYest"];
		upGl = variable["upGl"];
		UrAmFd = variable["UrAmFd"];
		UrinationCount = variable["UrinationCount"];
		UrinationCountDiel = variable["UrinationCountDiel"];
		UrinationCountYest = variable["UrinationCountYest"];
		UrinationVol = variable["UrinationVol"];
		UrinationVolDiel = variable["UrinationVolDiel"];
		UrinationVolYest = variable["UrinationVolYest"];
		UrNFd = variable["UrNFd"];
		VisDna = variable["VisDna"];
		VmAcTs = variable["VmAcTs"];
		VmAcTs2 = variable["VmAcTs2"];
		VmGlLm1Vis = variable["VmGlLm1Vis"];
		VmLHorSyn = variable["VmLHorSyn"];
		VolAa = variable["VolAa"];
		VolAc = variable["VolAc"];
		VolFa = variable["VolFa"];
		VolGl = variable["VolGl"];
		WaConsumed = variable["WaConsumed"];
		WaFeces = variable["WaFeces"];
		WaIn = variable["WaIn"];
		WaMi = variable["WaMi"];
		WaMilk = variable["WaMilk"];
		WaMiP = variable["WaMiP"];
		WaOut = variable["WaOut"];
		WaRespir = variable["WaRespir"];
		WaSweat = variable["WaSweat"];
		WaUrine = variable["WaUrine"];
		WaUrineYest = variable["WaUrineYest"];
		WtAcTm = variable["WtAcTm"];
		WtAdip = variable["WtAdip"];
		WtAdipNew = variable["WtAdipNew"];
		WtAdipTarget = variable["WtAdipTarget"];
		WtConc = variable["WtConc"];
		WtConcAgeFactor = variable["WtConcAgeFactor"];
		WtConcSyn = variable["WtConcSyn"];
		WtCytAdip = variable["WtCytAdip"];
		WtFaTm = variable["WtFaTm"];
		WtGrvUter = variable["WtGrvUter"];
		WtOth = variable["WtOth"];
		WtPConc = variable["WtPConc"];
		WtPConcSyn = variable["WtPConcSyn"];
		WtPGrvUter = variable["WtPGrvUter"];
		WtPGrvUterSyn = variable["WtPGrvUterSyn"];
		WtPOth = variable["WtPOth"];
		WtPUter = variable["WtPUter"];
		WtPUterDeg = variable["WtPUterDeg"];
		WtPUterSyn = variable["WtPUterSyn"];
		WtPVis = variable["WtPVis"];
		WtTsAdip = variable["WtTsAdip"];
		WtUter = variable["WtUter"];
		WtUterDeg = variable["WtUterDeg"];
		WtUterPart = variable["WtUterPart"];
		WtUterSyn = variable["WtUterSyn"];
		WtVis = variable["WtVis"];
		YAtp = variable["YAtp"];
		YAtpAp = variable["YAtpAp"];
		iTCH4 = variable["iTCH4"];
		iRumLaF = variable["iRumLaF"];
		kConcSynDecay = variable["kConcSynDecay"];
		kUterSynDecay = variable["kUterSynDecay"];
		iAaF = variable["iAaF"];
		kPConcSynDecay = variable["kPConcSynDecay"];
		iGlF = variable["iGlF"];
		KYAtAa = variable["KYAtAa"];
		iOthDnaF = variable["iOthDnaF"];
		iVisDnaF = variable["iVisDnaF"];
		iRumAaF = variable["iRumAaF"];
		KDnaVis = variable["KDnaVis"];
		iMiHaF = variable["iMiHaF"];
		kVmGlLmDeg = variable["kVmGlLmDeg"];
		icFa = variable["icFa"];
		iFaF = variable["iFaF"];
		K1TsFa = variable["K1TsFa"];
		KFaTmVis = variable["KFaTmVis"];
		KFaTsAdip = variable["KFaTsAdip"];
		iCsF = variable["iCsF"];
		iMiHbF = variable["iMiHbF"];
		KDnaOth = variable["KDnaOth"];
		KFatPi = variable["KFatPi"];
		iWtPConc = variable["iWtPConc"];
		iFlF = variable["iFlF"];
		BaseMamCellsTurnOver = variable["BaseMamCellsTurnOver"];
		CINT = variable["CINT"];
		K1AcTm = variable["K1AcTm"];
		K1AcTs = variable["K1AcTs"];
		VmFaTmVis = variable["VmFaTmVis"];
		iAcF = variable["iAcF"];
		iRumBuF = variable["iRumBuF"];
		iPiF = variable["iPiF"];
		K1FaTm = variable["K1FaTm"];
		KGlLaOth = variable["KGlLaOth"];
		KFGAm = variable["KFGAm"];
		VmGlLmVisPart = variable["VmGlLmVisPart"];
		iAsF = variable["iAsF"];
		iLPartF = variable["iLPartF"];
		iAmF = variable["iAmF"];
		icAc = variable["icAc"];
		KAcTmVis = variable["KAcTmVis"];
		KAcTsAdip = variable["KAcTsAdip"];
		cSaPs = variable["cSaPs"];
		iIndigFdF = variable["iIndigFdF"];
		K1FaTs = variable["K1FaTs"];
		KAaLmVis = variable["KAaLmVis"];
		KAcCd = variable["KAcCd"];
		KAaPmVis = variable["KAaPmVis"];
		KFaCd = variable["KFaCd"];
		KAaPOthOth = variable["KAaPOthOth"];
		KAaPVisVis = variable["KAaPVisVis"];
		IntakeDeclineSlope = variable["IntakeDeclineSlope"];
		lambdaMamCells = variable["lambdaMamCells"];
		VmAaPmVis = variable["VmAaPmVis"];
		iCeF = variable["iCeF"];
		iHcF = variable["iHcF"];
		iMiF = variable["iMiF"];
		iHaF = variable["iHaF"];
		icGl = variable["icGl"];
		kAHor1Gl = variable["kAHor1Gl"];
		kCHorGl = variable["kCHorGl"];
		KGlLmVis = variable["KGlLmVis"];
		KGlTpAdip = variable["KGlTpAdip"];
		KGlTpVis = variable["KGlTpVis"];
		KiAm = variable["KiAm"];
		kMamCellsDeclineBase = variable["kMamCellsDeclineBase"];
		MamCellsDecayRateOfSenescence = variable["MamCellsDecayRateOfSenescence"];
		iRumPrF = variable["iRumPrF"];
		cAaBase = variable["cAaBase"];
		kAHorGl = variable["kAHorGl"];
		kCHor1Gl = variable["kCHor1Gl"];
		MAXT = variable["MAXT"];
		KRumAaFv = variable["KRumAaFv"];
		iBldUrF = variable["iBldUrF"];
		KBldUrAm = variable["KBldUrAm"];
		VmAcTmVis = variable["VmAcTmVis"];
		fAcSilage = variable["fAcSilage"];
		iRumAcF = variable["iRumAcF"];
		fSaAs = variable["fSaAs"];
		K1MamCells = variable["K1MamCells"];
		KCsFv = variable["KCsFv"];
		VmGlTpVis = variable["VmGlTpVis"];
		cGlBase = variable["cGlBase"];
		iMamPmF = variable["iMamPmF"];
		iMamTmF = variable["iMamTmF"];
		KAaGlVis = variable["KAaGlVis"];
		kMilkAsh = variable["kMilkAsh"];
		fAiFdBase = variable["fAiFdBase"];
		iAtAdh = variable["iAtAdh"];
		KGlCd = variable["KGlCd"];
		kMilkingFrequencyLagUp = variable["kMilkingFrequencyLagUp"];
		kUterSyn = variable["kUterSyn"];
		kWaSweat = variable["kWaSweat"];
		MwtAm = variable["MwtAm"];
		MwtCH4 = variable["MwtCH4"];
		MwtN = variable["MwtN"];
		RumYAtp = variable["RumYAtp"];
		KHyEruct = variable["KHyEruct"];
		kConcSyn = variable["kConcSyn"];
		kMamCellsUsMfDecay = variable["kMamCellsUsMfDecay"];
		fEndogLiFd = variable["fEndogLiFd"];
		iVolAaF = variable["iVolAaF"];
		iVolGlF = variable["iVolGlF"];
		KFatFG = variable["KFatFG"];
		KFatHb = variable["KFatHb"];
		uTMamCells = variable["uTMamCells"];
		EnergyForPregnancyFactor = variable["EnergyForPregnancyFactor"];
		ifPm = variable["ifPm"];
		ifTm = variable["ifTm"];
		kComminuteOralMin = variable["kComminuteOralMin"];
		KPOthAaOth = variable["KPOthAaOth"];
		kWaRespir = variable["kWaRespir"];
		MinLPRumntnF = variable["MinLPRumntnF"];
		VmGlTpAdip = variable["VmGlTpAdip"];
		AcTgTg = variable["AcTgTg"];
		CappingForIntake = variable["CappingForIntake"];
		fLm = variable["fLm"];
		kMamCellsTransitionSteepness = variable["kMamCellsTransitionSteepness"];
		KMiHbF = variable["KMiHbF"];
		kVmGlLmSyn = variable["kVmGlLmSyn"];
		TAveMilkMam = variable["TAveMilkMam"];
		VmGlLaOth = variable["VmGlLaOth"];
		KMPartP = variable["KMPartP"];
		TAveabsE = variable["TAveabsE"];
		kPConcSyn = variable["kPConcSyn"];
		MEAN2 = variable["MEAN2"];
		VmBldUrAm = variable["VmBldUrAm"];
		fLaSilage = variable["fLaSilage"];
		MwtAc = variable["MwtAc"];
		MwtUr = variable["MwtUr"];
		VmTsFaAdip = variable["VmTsFaAdip"];
		MwtPr = variable["MwtPr"];
		MwtAs = variable["MwtAs"];
		MwtBu = variable["MwtBu"];
		OthDnaMx = variable["OthDnaMx"];
		VisDnaMx = variable["VisDnaMx"];
		FaScFd = variable["FaScFd"];
		HrtWrk = variable["HrtWrk"];
		KMiHaF = variable["KMiHaF"];
		KPVisAaVis = variable["KPVisAaVis"];
		MiNnNn = variable["MiNnNn"];
		MwtGy = variable["MwtGy"];
		MwtLa = variable["MwtLa"];
		iMamLmF = variable["iMamLmF"];
		KabsLa = variable["KabsLa"];
		PartThick = variable["PartThick"];
		PResidMamMilk = variable["PResidMamMilk"];
		fPUter = variable["fPUter"];
		AMP1FT = variable["AMP1FT"];
		AMP2FT = variable["AMP2FT"];
		EnergyForMilkFactor = variable["EnergyForMilkFactor"];
		fOaPe = variable["fOaPe"];
		KDayLength = variable["KDayLength"];
		KHyOther = variable["KHyOther"];
		kMamAQBase = variable["kMamAQBase"];
		KNaAdip = variable["KNaAdip"];
		KNaOth = variable["KNaOth"];
		kVmGlLmDecay = variable["kVmGlLmDecay"];
		LgutDCAi = variable["LgutDCAi"];
		LgutDCHb = variable["LgutDCHb"];
		LiChFd = variable["LiChFd"];
		LiScSC = variable["LiScSC"];
		MaxLossDueToLowMf = variable["MaxLossDueToLowMf"];
		MiLiLI = variable["MiLiLI"];
		MwtAa = variable["MwtAa"];
		MwtCe = variable["MwtCe"];
		MwtCh = variable["MwtCh"];
		MwtCs = variable["MwtCs"];
		MwtHc = variable["MwtHc"];
		MwtOa = variable["MwtOa"];
		MwtPe = variable["MwtPe"];
		MwtPi = variable["MwtPi"];
		MwtPOth = variable["MwtPOth"];
		MwtPs = variable["MwtPs"];
		MwtPVis = variable["MwtPVis"];
		MwtRumAa = variable["MwtRumAa"];
		MwtSc = variable["MwtSc"];
		MwtSt = variable["MwtSt"];
		MwtVa = variable["MwtVa"];
		ObsEUr = variable["ObsEUr"];
		TAveMilk = variable["TAveMilk"];
		xOadIntakeTadIntake = variable["xOadIntakeTadIntake"];
		BcsTargetDecay = variable["BcsTargetDecay"];
		fBuAc = variable["fBuAc"];
		KTsFaAdip = variable["KTsFaAdip"];
		kUterDeg = variable["kUterDeg"];
		MamCellsProliferationDecayRate = variable["MamCellsProliferationDecayRate"];
		iWtUter = variable["iWtUter"];
		AaFvBu = variable["AaFvBu"];
		fDWt = variable["fDWt"];
		fLaCdAdip = variable["fLaCdAdip"];
		fLaCdOth = variable["fLaCdOth"];
		GlGlHy = variable["GlGlHy"];
		kMamCellsQAPostPeak = variable["kMamCellsQAPostPeak"];
		KNaVis = variable["KNaVis"];
		MiHaHA = variable["MiHaHA"];
		MwtFa = variable["MwtFa"];
		MwtFl = variable["MwtFl"];
		ObsCH4 = variable["ObsCH4"];
		VmAaGlVis = variable["VmAaGlVis"];
		xLHorSensAdip = variable["xLHorSensAdip"];
		fHeifers = variable["fHeifers"];
		PartWidth = variable["PartWidth"];
		FaTgTg = variable["FaTgTg"];
		ifMPartRum = variable["ifMPartRum"];
		K2VAct = variable["K2VAct"];
		KidWrk = variable["KidWrk"];
		KSPartP = variable["KSPartP"];
		MwtLm = variable["MwtLm"];
		MwtNn = variable["MwtNn"];
		ResWrk = variable["ResWrk"];
		RumntnF = variable["RumntnF"];
		VmAcTsAdip = variable["VmAcTsAdip"];
		VmFaTsAdip = variable["VmFaTsAdip"];
		wLHorSensAdip = variable["wLHorSensAdip"];
		AaGlGl = variable["AaGlGl"];
		iWtConc = variable["iWtConc"];
		kMamCellsQAPrePeak = variable["kMamCellsQAPrePeak"];
		KMiHb = variable["KMiHb"];
		AmUrUr = variable["AmUrUr"];
		GlLmLm = variable["GlLmLm"];
		GyGlGl = variable["GyGlGl"];
		KLaFv = variable["KLaFv"];
		LaGlGl = variable["LaGlGl"];
		MiLiBu = variable["MiLiBu"];
		MiLiGl = variable["MiLiGl"];
		MilkInt = variable["MilkInt"];
		PrGlGl = variable["PrGlGl"];
		xLHorSensGl = variable["xLHorSensGl"];
		AaGlH = variable["AaGlH"];
		CdMiG1 = variable["CdMiG1"];
		ifSPartRum = variable["ifSPartRum"];
		kEnergyCompensation = variable["kEnergyCompensation"];
		MiPiPI = variable["MiPiPI"];
		AaGlAc = variable["AaGlAc"];
		fGrvUterTO = variable["fGrvUterTO"];
		pGlHyAdip = variable["pGlHyAdip"];
		pGlHyVis = variable["pGlHyVis"];
		AaFvAc = variable["AaFvAc"];
		AaFvPr = variable["AaFvPr"];
		AaGlUr = variable["AaGlUr"];
		fGestEPrt = variable["fGestEPrt"];
		iVolAcF = variable["iVolAcF"];
		LgutDCFa = variable["LgutDCFa"];
		LgutDCPi = variable["LgutDCPi"];
		MwtLiFd = variable["MwtLiFd"];
		OaScSC = variable["OaScSC"];
		DCMiLi = variable["DCMiLi"];
		EnergyForMilkingCowMaintenancePower = variable["EnergyForMilkingCowMaintenancePower"];
		fPrGl = variable["fPrGl"];
		DCMiPi = variable["DCMiPi"];
		EnergyForDryCowMaintenancePower = variable["EnergyForDryCowMaintenancePower"];
		KWaFeces = variable["KWaFeces"];
		OnceADay2YearsOldAdjustment = variable["OnceADay2YearsOldAdjustment"];
		refIngrKAdfDeg = variable["refIngrKAdfDeg"];
		ikMilkInh = variable["ikMilkInh"];
		MiLiCh = variable["MiLiCh"];
		xLHorSensAa = variable["xLHorSensAa"];
		LaAcAc = variable["LaAcAc"];
		LgutDCAs = variable["LgutDCAs"];
		LgutDCHa = variable["LgutDCHa"];
		MwtFaFd = variable["MwtFaFd"];
		MwtTm = variable["MwtTm"];
		MwtTs = variable["MwtTs"];
		OnceADayMilkingAdjustment = variable["OnceADayMilkingAdjustment"];
		VmMiHa = variable["VmMiHa"];
		VmMiHb = variable["VmMiHb"];
		pLPartMPartComm = variable["pLPartMPartComm"];
		AaFvAt = variable["AaFvAt"];
		ExpV2 = variable["ExpV2"];
		FHcCs1 = variable["FHcCs1"];
		kComminuteOralMax = variable["kComminuteOralMax"];
		LaScSc = variable["LaScSc"];
		OsWaSlp = variable["OsWaSlp"];
		PeScSC = variable["PeScSC"];
		absAaAd = variable["absAaAd"];
		absGlAd = variable["absGlAd"];
		AcCor = variable["AcCor"];
		ASCor = variable["ASCor"];
		BldUrCor = variable["BldUrCor"];
		BST = variable["BST"];
		CeCor = variable["CeCor"];
		CurrentEvent = variable["CurrentEvent"];
		CurrHerbage = variable["CurrHerbage"];
		CurrStrat = variable["CurrStrat"];
		EatingSW = variable["EatingSW"];
		EatSW5 = variable["EatSW5"];
		EPart = variable["EPart"];
		F1 = variable["F1"];
		FaCor = variable["FaCor"];
		FLCor = variable["FLCor"];
		GlHyAD = variable["GlHyAD"];
		GlHyTp = variable["GlHyTp"];
		GlLmAd = variable["GlLmAd"];
		HaCor = variable["HaCor"];
		HbCor = variable["HbCor"];
		HcCor = variable["HcCor"];
		HMM = variable["HMM"];
		iAnimal = variable["iAnimal"];
		iHerd = variable["iHerd"];
		iLHorF = variable["iLHorF"];
		iMamMilkAveF = variable["iMamMilkAveF"];
		IndigFdCor = variable["IndigFdCor"];
		INS = variable["INS"];
		iVolBldUrF = variable["iVolBldUrF"];
		iVolFaF = variable["iVolFaF"];
		KMilkInhDeg = variable["KMilkInhDeg"];
		KumarMigEq = variable["KumarMigEq"];
		LaFvAt = variable["LaFvAt"];
		LPartCor = variable["LPartCor"];
		MamMilkCor = variable["MamMilkCor"];
		MICor = variable["MICor"];
		MiHaCor = variable["MiHaCor"];
		MIHbCor = variable["MIHbCor"];
		MiLiPr = variable["MiLiPr"];
		MilkProductionAgeAdjustment = variable["MilkProductionAgeAdjustment"];
		MIXSET = variable["MIXSET"];
		NSTP = variable["NSTP"];
		OxPrGl = variable["OxPrGl"];
		PICor = variable["PICor"];
		RQEQ = variable["RQEQ"];
		RumAcCor = variable["RumAcCor"];
		RumBuCor = variable["RumBuCor"];
		RumLaCor = variable["RumLaCor"];
		RumpHCON = variable["RumpHCON"];
		RumPrCor = variable["RumPrCor"];
		slpKRuAdf = variable["slpKRuAdf"];
		slpKRUP = variable["slpKRUP"];
		slpKRUST = variable["slpKRUST"];
		T3 = variable["T3"];
		TgGyGy = variable["TgGyGy"];
		Theta3 = variable["Theta3"];
		TpTpTm = variable["TpTpTm"];
		TpTpTs = variable["TpTpTs"];
		WtConcBreedFactor = variable["WtConcBreedFactor"];
		HcCeCe = variable["HcCeCe"];
		MilkDen = variable["MilkDen"];
		EnergyForDryCowMaintenanceFactor = variable["EnergyForDryCowMaintenanceFactor"];
		MilkIntPowerForFMamCelsQA1 = variable["MilkIntPowerForFMamCelsQA1"];
		AaFvHy = variable["AaFvHy"];
		AmMiG2 = variable["AmMiG2"];
		ExpOth2 = variable["ExpOth2"];
		KbasAdip = variable["KbasAdip"];
		MiLiFA = variable["MiLiFA"];
		MPartSize = variable["MPartSize"];
		AaFvCd = variable["AaFvCd"];
		kSpecies = variable["kSpecies"];
		wLHorSensGl = variable["wLHorSensGl"];
		AaFvAm = variable["AaFvAm"];
		refIngrKHaCs = variable["refIngrKHaCs"];
		kMastication = variable["kMastication"];
		KMiHa = variable["KMiHa"];
		kMilkingFrequencyLagDown = variable["kMilkingFrequencyLagDown"];
		wLHorSensAa = variable["wLHorSensAa"];
		EnergyForMilkingCowMaintenanceFactor = variable["EnergyForMilkingCowMaintenanceFactor"];
		HyAcFa = variable["HyAcFa"];
		iKPiAa = variable["iKPiAa"];
		OsMolF = variable["OsMolF"];
		LiFlFd = variable["LiFlFd"];
		AcCdCd = variable["AcCdCd"];
		EXP10 = variable["EXP10"];
		GlGyGY = variable["GlGyGY"];
		GlLaAt = variable["GlLaAt"];
		GlLaLa = variable["GlLaLa"];
		GlTpAD = variable["GlTpAD"];
		GlTpTp = variable["GlTpTp"];
		GyGlAt = variable["GyGlAt"];
		iParity = variable["iParity"];
		kLHorSensAa = variable["kLHorSensAa"];
		kLHorSensGl = variable["kLHorSensGl"];
		kMamCellsQAKickStartDecay = variable["kMamCellsQAKickStartDecay"];
		kMamCellsQAStart = variable["kMamCellsQAStart"];
		MaxBladderVol = variable["MaxBladderVol"];
		OxAcCd = variable["OxAcCd"];
		P1 = variable["P1"];
		PrGlAd = variable["PrGlAd"];
		TpLaAt = variable["TpLaAt"];
		UrAmAm = variable["UrAmAm"];
		BcsTargetNadir = variable["BcsTargetNadir"];
		CsMiG2 = variable["CsMiG2"];
		KbasOth = variable["KbasOth"];
		MamCellsPerKgMs270 = variable["MamCellsPerKgMs270"];
		ObsME = variable["ObsME"];
		iME = variable["iME"];
		HyMiG1 = variable["HyMiG1"];
		AcFaAd = variable["AcFaAd"];
		FKRuP = variable["FKRuP"];
		BCSBase = variable["BCSBase"];
		EnergyForMilkPower = variable["EnergyForMilkPower"];
		FaFlFd = variable["FaFlFd"];
		GlHyCd = variable["GlHyCd"];
		HyAtAt = variable["HyAtAt"];
		LaCdCd = variable["LaCdCd"];
		LaGlAd = variable["LaGlAd"];
		ObsDE = variable["ObsDE"];
		OxLaCd = variable["OxLaCd"];
		OxTpCd = variable["OxTpCd"];
		PcPeFd = variable["PcPeFd"];
		PrCdCd = variable["PrCdCd"];
		TgFaFa = variable["TgFaFa"];
		TpCdCd = variable["TpCdCd"];
		FKRuSt = variable["FKRuSt"];
		MyPi = variable["MyPi"];
		KWAP = variable["KWAP"];
		OxPrCd = variable["OxPrCd"];
		NnAmAM = variable["NnAmAM"];
		refIngrKPiAa = variable["refIngrKPiAa"];
		AtAmUr = variable["AtAmUr"];
		BuCdCd = variable["BuCdCd"];
		CsFvAt = variable["CsFvAt"];
		IALG = variable["IALG"];
		McalToMJ = variable["McalToMJ"];
		Theta4 = variable["Theta4"];
		KMPartSPart = variable["KMPartSPart"];
		kRetMilkI = variable["kRetMilkI"];
		LPartSize = variable["LPartSize"];
		AaMiG2 = variable["AaMiG2"];
		AaPxAD = variable["AaPxAD"];
		iAgeInYears = variable["iAgeInYears"];
		MastJawMoveBolus = variable["MastJawMoveBolus"];
		OxBuCd = variable["OxBuCd"];
		Theta1 = variable["Theta1"];
		numVersion = variable["numVersion"];
		TcHyAd = variable["TcHyAd"];
		FKRuAdf = variable["FKRuAdf"];
		Theta2 = variable["Theta2"];
		GlCdCd = variable["GlCdCd"];
		kAaGlGest = variable["kAaGlGest"];
		OxGlCd = variable["OxGlCd"];
		CsMiG1 = variable["CsMiG1"];
		iKHcCs = variable["iKHcCs"];
		KabsAc = variable["KabsAc"];
		RumpHBase = variable["RumpHBase"];
		K1VAct = variable["K1VAct"];
		AmMiG1 = variable["AmMiG1"];
		CeMeanRRT = variable["CeMeanRRT"];
		HaMeanRRT = variable["HaMeanRRT"];
		PiMeanRRT = variable["PiMeanRRT"];
		IntakeVersion = variable["IntakeVersion"];
		KbasVis = variable["KbasVis"];
		iKCeCs = variable["iKCeCs"];
		version = variable["version"];
		TpTgAD = variable["TpTgAD"];
		AaCor = variable["AaCor"];
		AcCdAt = variable["AcCdAt"];
		AcquisitionJawMovesCurrent = variable["AcquisitionJawMovesCurrent"];
		AmCor = variable["AmCor"];
		CsCor = variable["CsCor"];
		GlCor = variable["GlCor"];
		iMilkAve = variable["iMilkAve"];
		RumAaCor = variable["RumAaCor"];
		THETA5 = variable["THETA5"];
		KabsBu = variable["KabsBu"];
		NumberOfFeeds = variable["NumberOfFeeds"];
		iKHaCs = variable["iKHaCs"];
		KabsPr = variable["KabsPr"];
		PMamEnzCell = variable["PMamEnzCell"];
		vfaeff = variable["vfaeff"];
		FaCdCd = variable["FaCdCd"];
		kMamCellsTransitionDim = variable["kMamCellsTransitionDim"];
		LaCdAt = variable["LaCdAt"];
		PrCdAt = variable["PrCdAt"];
		FdRatWFM = variable["FdRatWFM"];
		iDMilk = variable["iDMilk"];
		LHorBase = variable["LHorBase"];
		LhorTurnoverDays = variable["LhorTurnoverDays"];
		SmoothingPeriodDays = variable["SmoothingPeriodDays"];
		TpCdAt = variable["TpCdAt"];
		KLPartRed = variable["KLPartRed"];
		OxFaCd = variable["OxFaCd"];
		KAmabs = variable["KAmabs"];
		AcAcTs = variable["AcAcTs"];
		BuCdAt = variable["BuCdAt"];
		kSurfaceArea = variable["kSurfaceArea"];
		CountDownDays = variable["CountDownDays"];
		GlCdAt = variable["GlCdAt"];
		Latitude = variable["Latitude"];
		MwtCd = variable["MwtCd"];
		iFCM4Z = variable["iFCM4Z"];
		KAsabs = variable["KAsabs"];
		iMEi = variable["iMEi"];
		iDEi = variable["iDEi"];
		PeakIntakeDay = variable["PeakIntakeDay"];
		VsizF = variable["VsizF"];
		DaysDry = variable["DaysDry"];
		DaysOpen = variable["DaysOpen"];
		kInitRumVol = variable["kInitRumVol"];
		KMilk = variable["KMilk"];
		FaCdAt = variable["FaCdAt"];
		iOxup = variable["iOxup"];
		BolusWeightTotalCurrent = variable["BolusWeightTotalCurrent"];
		MamCellsPerKgMsAdjustment = variable["MamCellsPerKgMsAdjustment"];
		MaxBolusWeight = variable["MaxBolusWeight"];
		VmAaPVisVis = variable["VmAaPVisVis"];
		VmAaPOthOth = variable["VmAaPOthOth"];
		StartDayGest = variable["StartDayGest"];
		iDayOfYear = variable["iDayOfYear"];
		GestLength = variable["GestLength"];
		WaPool = variable["WaPool"];
		WaPoolTarget = variable["WaPoolTarget"];
		TSTP = variable["TSTP"];
		VmRumAaFv = variable["VmRumAaFv"];
		KgMilkSolidsExpectedIn270Days = variable["KgMilkSolidsExpectedIn270Days"];
		StandardBw = variable["StandardBw"];
		MatBW = variable["MatBW"];
		MamCellsPart = variable["MamCellsPart"];
		VmCsFv = variable["VmCsFv"];
		KBldUrU = variable["KBldUrU"];
		SecondsPerDay = variable["SecondsPerDay"];
	
	} // end push_variables_to_model
	
	void do_event ( string next_event ) {
	
		// find event
		if ( next_event == "MilkOut" ) { // discrete MilkOut // Schedules Milking
			// Initiate milking according to the new MilkingArray
			// early, the last milking of each day should complete before the daily summary.
			
			ResidMamMilk = MamMilk * PResidMamMilk ;
			MilkingIndex = MilkingIndex +1 ;
			NextMilkingT = floor ( t ) + MilkingHours [ MilkingIndex + 1 - 1 ] / 24.0 ;
			if ( MilkingIndex < MilkingFrequency ) {
				schedule ( t + NextMilkingT , "MilkOut" ) ; // Schedule the next milking within that day
			}
			MilkSW = 1.0 ;
		} // end of discrete // Discrete MilkOut
		if ( next_event == "NewDay" ) { // discrete NewDay // things to do just AFTER midnight
			MilkingIndex = 0 ;
			NextMilkingT = floor ( t ) + MilkingHours [ 1 - 1 ] / 24.0 ;
			if ( DayMilk >= 0 ) { // SCHEDULE must be at start of line
				schedule ( t + NextMilkingT , "MilkOut" ) ;
			}
			schedule ( t + ( floor ( t ) + 1.01 ) , "NewDay" ) ; // Schedule the next daily summary, just before midnight, so WFM "midnight" samplings would always occure AFTER the summary
		} // end of discrete
		if ( next_event == "DailySummary" ) { // discrete DailySummary // Summarize items at the end of the day
			schedule ( t + ( floor ( t + 1.1 ) - 2 * MAXT ) , "DailySummary" ) ; // Schedule the next daily summary, just before midnight, so WFM "midnight" samplings would always occure AFTER the summary
			
			IntakeDay = IntakeTotal - IntakeYest ;
			IntakeYest = IntakeTotal ;
			TNdfInYest = TNdfIn ;
			UrinationVolYest = UrinationVol ;
			UrinationCountYest = UrinationCount ;
			DrnkWaYest = DrnkWaTot ;
			
			// Summarizing daily meals, eating time, resting time, and rumination time.
			MealsDay = TotMeals - TotMealsYest ;
			TotMealsYest = TotMeals ;
			TotEatingYest = TotEating ;
			TotRumntnYest = TotRumntn ;
			TotRestYest = TotRest ;
			
			tNepYest = tNep ;
			
			// Determine what time the sun will set today.  Used in Mindy Inake submodel
			SunriseToday = floor ( t +0.1 ) + Sunrise ;
			SunsetToday = floor ( t +0.1 ) + SunSet ;
			
			dMilkProd = TVolMilk - TVolMilkYest ;
			dLmProd = TMilkLm - TMilkLmYest ;
			dPmProd = TMilkPm - TMilkPmYest ;
			dTmProd = TMilkTm - TMilkTmYest ;
			
			// Calculate daily milk and milk compenent yields and percentages
			TVolMilkYest = TVolMilk ;
			TMilkLmYest = TMilkLm ;
			TMilkPmYest = TMilkPm ;
			TMilkTmYest = TMilkTm ;
			
		} // end of discrete
		if ( next_event == "breakpoint4debug" ) { // discrete breakpoint4debug // ---
			schedule ( t + 1.0 , "breakpoint4debug" ) ; // --- this is the interval that this discrete section will be executed.
			Somedummyvariable = t ; // --- use this dummy assignment statement to set a debug breakpoint. Can equal any variable or number.
		} // end of discrete
		
	} // end do_event
	
	double derivt( double dx0, double x ) {
	
		return( x / t ) ;
	
	}
	
	void calculate_rate ( ) {
		
		// derivative calculations
		// derivative 
		
		TIME = t ;
		
		// END  INCLUDE 'FdRat_Deriv.csl'
		// BEGIN  INCLUDE '..\Molly_ProximateExpand_In_Deriv.csl'  ! Take from the shared parent folder as all current projects use ths one
		// ** Include Proximate Nutrient inputs in the Derivative Section
		// 4-27-10, added an if statement to trap fStFd .le. 0 before fStsFd calc, mdh
		// 28-Feb-11 Added fBuAc and fBuAc instead of the hard wired 0.2 and 0.2   GL
		// 11-7-10, made PsF a function of fRoughageFd, mdh
		
		// ***********Calculate nutrient inputs when the Event array is used****************
		// Event=DM CP NPN Urea Fat Starch NDF ADF Ash Roughage
		
		// procedural ( PartFd , fDMFd , fCPFd , FCPsFd , FUrFd , FNPNFd , FRUPFd , FPsFd , fPiFd , fNnFd , 
			// FCFatFd , fLiFd , fFatFd , fAshFd , fAsFd , fStFd , FStsFd , fNDFFd , fADFFd , 
			// fLgFd , fHcFd , fCeFd , fOmFd , fScFd , StSol = CurrentEvent , CurrentSupplement , EatingSupplementsSW , EatSW5 ) 
		
		if ( CurrentSupplement == 0 ) {
			CurrentFeed = CurrHerbage ;
		} else {
			CurrentFeed = CurrentSupplement ;
		}
		
		if ( LastEv != CurrentEvent || LastSu != CurrentSupplement || LastSw != EatingSupplementsSW || LastEA != EatSW5 ) { // Avoid unnecessary recomutation. We want it recomputed ONLY if one of these 3 have changed, yet Acsl is re evaluating all tyhe time for some reason
			LastEv = CurrentEvent ;
			LastSu = CurrentSupplement ;
			LastSw = EatingSupplementsSW ;
			LastEA = EatSW5 ;
			
			// Kg/Kg Corn Silage DM
			
			// Gil sept 2014, reducing all nutrients, to leave enough room for SC!! (Delagargde for instance has 921 g/kg for CP+SC+NDF which leaves only 8% for ST PE OA ASH!!!!)
			
			// Oa = Pe * .2 is an Unsupported Estimate mdh. reduced from 0.2 to 0.1555 - Gil
			
			fAcFd = PcSilage / 100 * fAcSilage ;
			FLaFd = PcSilage / 100 * fLaSilage ;
			fBuSilage = fAcSilage * fBuAc ;
			fBuFd = PcSilage / 100 * fBuSilage ;
			fPeFd = PcPeFd / 100 ;
			fOaFd = fPeFd * fOaPe ; // Oa = Pe*.2 is an Unsupported Estimate
			
			fRoughageFd = Event [ CurrentEvent - 1 ][ IngrFor - 1 ] / 100 ;
			
			fDMFd = Event [ CurrentEvent - 1 ][ IngrDM - 1 ] / 100 ;
			
			if ( SupplementOnOffer <= NumberOfFeeds ) {
				// BEGIN  Include '../Molly_ProximateExpand_Conversion.csl'
				
				// This is the body of the procedural from proximateExpand in deriv
				
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				
				fAiFd = fAiFdBase ; // average insoluble Ash contents Gil Sep 2014 this is no longer a contsant
				
				//  Feed Nitrogen
				fCPFd = Event [ CurrentFeed - 1 ][ IngrCP - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
				// fCPFd=Event(IngrCP,CurrentFeed)/100;	!Original equation not corrected for bias in the input data
				FCPsFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrCPs - 1 ] / 100 ; // includes NPN sources
				FUrFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrUr - 1 ] / 100 / 2.8555 ; // IngrUr is in CP equivalents. Changed to urea mass here
				FNPNFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrNPN - 1 ] / 100 ; // CP equivalents including that from urea
				FRUPFd = fCPFd * IngrProtComp [ CurrentFeed - 1 ][ IngrRUP - 1 ] / 100 ;
				FPsFd = FCPsFd - FNPNFd ;
				fPiFd = fCPFd - FCPsFd ;
				if ( fPiFd <= 0 ) { // This should not happen, but just in case a bad number gets entered
					FCPsFd = FCPsFd + fPiFd ;
					fPiFd = 1e-12 ;
				}
				fNnFd = FNPNFd - ( FUrFd * 2.8555 ) ; // Nn mass, % of DM, assumes a C to N ratio the same as CP which is correct for nucleic acids
				if ( fNnFd < 0 ) fNnFd = 0 ;
				
				// Feed Lipid
				FCFatFd = Event [ CurrentFeed - 1 ][ IngrFat - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
				// 	fCFatFd=Event(IngrFat,CurrentFeed) / 100
				
				fLiFd = fEndogLiFd ;
				fFatFd = FCFatFd - fEndogLiFd ;
				
				// Feed Ash
				fAshFd = Event [ CurrentFeed - 1 ][ IngrAsh - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
				// 	fAshFd=Event(IngrAsh,CurrentFeed)/100
				fAsFd = fAshFd - fAiFd ;
				
				// Feed Carbohydrate
				fNDFFd = Event [ CurrentFeed - 1 ][ IngrNDF - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
				fADFFd = Event [ CurrentFeed - 1 ][ IngrADF - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
				// 	fNdfFd=Event(IngrNDF,CurrentFeed)/100
				// 	fAdfFd=Event(IngrADF,CurrentFeed)/100
				if ( fADFFd == 0 ) fADFFd = 1e-12 ;
				fRuAdfFd = fADFFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrRUAdf - 1 ] / 100 ;
				if ( fRuAdfFd > fADFFd ) fRuAdfFd = fADFFd -1e-12 ;
				
				fLgFd = Event [ CurrentFeed - 1 ][ IngrLg - 1 ] / 100 ;
				fHcFd = fNDFFd - fADFFd ;
				fCeFd = fADFFd - fLgFd - fAiFd ;
				fOmFd = 1.0 - fAshFd ;
				
				fStFd = Event [ CurrentFeed - 1 ][ IngrSt - 1 ] / 100 ; // These are currently only defined for the Bate NRC data set
				// 	fStFd=Event(IngrSt,CurrentFeed) / 100
				if ( fStFd <= 0 ) fStFd = 1.0e-12 ;
				FStsFd = fStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrStS - 1 ] / 100 ;
				if ( FStsFd <= 0 ) FStsFd = 0.0 ;
				fRUStFd = fStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrRUSt - 1 ] / 100 ;
				if ( fRUStFd > fStFd ) fRUStFd = fStFd -1e-12 ;
				
				if ( fFatFd < 0 ) { // trap negative values for fFatFd
					fFatFd = 0 ;
					fLiFd = ( Event [ CurrentFeed - 1 ][ IngrFat - 1 ] / 100 ) ;
				}
				
				PartFd = fPiFd + FPsFd + fNnFd + FUrFd + fAcFd + FLaFd + fBuFd + fPeFd + fOaFd +
					fHcFd + fCeFd + fLgFd + fLiFd + fFatFd + fAsFd + fAiFd ;
				
				// Calculates fSCFd and fStFd and traps negative added fat values
				fScFd = 1 - PartFd - fStFd ;
				if ( fScFd < 0 ) {
					if ( fStFd > - fScFd ) {
						fStFd = fStFd + fScFd ;
						FStsFd = fStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrStS - 1 ] / 100 ;
						fRUStFd = fRUStFd * IngrCHOComp [ CurrentFeed - 1 ][ IngrRUSt - 1 ] / 100 ;
						fScFd = 0 ;
					} else {
						fStFd = 1.0e-12 ;
						FStsFd = 0.0 ;
						fRUStFd = 1.0e-13 ;
						fScFd = 0 ;
					}
				}
				StSol = FStsFd / fStFd ;
				SpeciesFactor = Event [ CurrentFeed - 1 ][ IngrEaseOfBreakdown - 1 ] ;
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// END  Include '../Molly_ProximateExpand_Conversion.csl'
			}
		}
		// end of procedural 
		
		// Rate constants
		// procedural ( KPiAa , KHaCs , KCeCs1 , KHcCs1 , fRupCp , fPiCp , fPsCp , fNnCp , fUrCp = FRUPFd , 
			// fPiFd , fRUStFd , fStFd , FStsFd , fRuAdfFd , fADFFd ) 
		
		// The following are scalars that are used to adjust in situ degradation values
		// such that they are consistent with previously determined model values
		// Multiplicitive - located in the divisor so a large number results in minimal slope
		// Used to calculate kPiAa from RUP and CP
		// Used to calculate kHaCs from RUSt and St
		// Used to calculate kCeCs from RuADF and ADF
		// Additive intercept adjustmentsT
		
		// Scale KHcCs1 to KCeCs1 based on previously derived estimates for ikHcCs and ikCeCs
		// added slopes to each of the K calculations to keep the
		// rate constants sensitive to input in situ rates. MDH 5-17-14
		// The large intercept for RuADF removed all sensitivity.
		
		KPiAa = ( - log ( FRUPFd / fPiFd ) / PiMeanRRT * 24 ) * slpKRUP + FKRuP ; // 24h Degradation Rates for protein
		// kPiAa=ikPiAa; !for testing static kPiAa versus the above equation
		KHaCs = ( - log ( fRUStFd / ( fStFd - FStsFd ) ) / HaMeanRRT * 24 ) * slpKRUST + FKRuSt ; // Starch
		KCeCs1 = ( - log ( fRuAdfFd / fADFFd ) / CeMeanRRT * 24 ) * slpKRuAdf + FKRuAdf ; // Cellulose, Need RuAdf estimates to fully use.
		KHcCs1 = FHcCs1 * KCeCs1 ; // Scale rate from Ce to achieve appropriate Hc rate
		
		fRupCp = FRUPFd / fCPFd ; // Rup as a fraction of CP
		fPiCp = fPiFd / fCPFd ; // SolP as a fraction of CP
		fPsCp = FPsFd / fCPFd ; // SolP as a fraction of CP
		fNnCp = fNnFd / fCPFd ; // Nn as a fraction of CP
		fUrCp = FUrFd / fCPFd ; // Urea as a fraction of CP
		fRuStSt = fRUStFd / fStFd ; // RuST as a fraction of St
		fStSSt = FStsFd / fStFd ; // Soluble St as a fraction of St
		fRuAdfAdf = fRuAdfFd / fADFFd ; // RuAdf as a fraction of Adf
		fLgAdf = fLgFd / fADFFd ; // Lignin as a fraction of Adf
		// end of procedural 
		// Water functions based on 4.7L/Kg DM, with 70% being consumed
		// during eating, and only 75% of water consumed entering the rumen
		
		if ( t <= 1.0 ) { // This if was merged from Molly86 Gil July 2012
			otGutCont = iotGutCont ; // Use initial guess until the first days intake has been determined
		} else {
			otGutCont = 1.0 * IntakeDay ; // Changed to a daily summary value to accomodate within day eating patterns. MDH 5-23-11
		}
		WaRespir = BWF * kWaRespir ;
		WaSweat = BWF * kWaSweat ;
		AgeInYears = iAgeInYears + ( t / 365.0 ) ;
		
		// procedural ( DayGest , DryOffNowVariable , ConceiveNowVariable = t ) 
		// Minimum days that DayMilk should count down to next calving
		
		if ( DryOffNow >= 1 && DayMilk > 0 ) DryOffNowVariable = 1.0 ; // DryOffNow may be manipulated from the outside thru m file or WFM
		if ( ConceiveNow >= 1 && DayGest <= 0 ) ConceiveNowVariable = 1.0 ; // ConceiveNow may be manipulated from the outside thru m file or WFM
		if ( AbortPregNow >= 1 && DayGest > 0 ) AbortPregNowVariable = 1.0 ; // AbortPregNow may be manipulated from the outside thru m file or WFM
		
		if ( DayGest >= GestLength - CountDownDays ) { // Start Count Down - move tAtCalving from past calving to the coming future one
			tAtCalving = tAtConception + GestLength ;
		}
		if ( DayGest > GestLength ) { // Calving occured, clear conception timers
			tAtConception = 2999.0 ;
			DayGestBasic = -2999.0 ;
		}
		
		// Advance DayGest; DayMilk
		DayGestBasic = t - tAtConception ;
		DayGest = max ( DayGestBasic , -300. ) ; // This prevents undeflows in some preg equations while she is ompty.
		DayMilk = t - tAtCalving ;
		if ( DayMilk < -21 ) DayMilk = -21 ;
		
		// Conceive. ConceiveNow is a hook for external (WFM / m file). Conceiving will also work the traditional way <DaysOpen> days after last calving
		// If used in m file or WFM, it needs to be reset to zero sometime before this preg ends or she'll conceive again right after calving
		if ( ConceiveNowVariable >= 1.0 || ( tAtConception > t && ( t > tAtCalving + DaysOpen ) ) ) {
			tAtConception = t ;
			ConceiveNowVariable = 0.0 ;
		}
		
		// Dry off.  DryOffNow is a hook for external (WFM / m file). Dryoff will also work the traditional way <DaysDry> days before next calving
		// If used in m file or WFM, it needs to be reset to zero sometime before she calves again and start a new lactaion, or she'll dry off immediately after
		if ( DryOffNowVariable >= 1.0 || t > ( tAtConception + GestLength - DaysDry ) ) {
			tAtCalving = tAtConception + GestLength ;
			DryOffNowVariable = 0.0 ;
		}
		
		// Abort Pregnancy: hook for external (WFM / m file) command. Need to reset it to zero before next preg starts
		if ( AbortPregNowVariable >= 1.0 ) {
			tAtConception = 2999.0 ;
			AbortPregNowVariable = 0.0 ;
		}
		// end of procedural 
		
		// ! Apply the grinding effect of mastication to get the particle size distribution of the swallowed ingesta
		// ! Based predominantly on the original feed particle size distribution and the number of mastication jaw movements.
		// ! As decribed in 2.2.1 Gregorini 2014.
		// procedural ( fLPartSwal , fMPartSwal , fSPartSwal = MastJawMoveBolus , AcquisitionJawMovesCurrent , kComminuteOralMin , kComminuteOralMax ) 
		SumBinFd = 0 ;
		MeanParticleSize = 0 ;
		MedianParticleSize = 0 ;
		
		if ( EatingSW == 1 ) {
			MasticationSteps = MastJawMoveBolus * kMastication + AcquisitionJawMovesCurrent * kAcquisition ; // Arbitrary unit to feed into the iterative model below
			MasticationSteps = MasticationSteps / ( BolusWeightTotalCurrent / MaxBolusWeight ) ; // Simulate increased ease of breakdown if the bolus is smaller
			MasticationSteps = min ( 100. , floor ( MasticationSteps / SpeciesFactor *pow(1,1)* kSpecies ) ) ; // Simulate increased ease of breakdown as more step into the model for now.
			
			fSPartSwal = 0 ;
			fMPartSwal = 0 ;
			fLPartSwal = 0 ;
			for ( int i = 1 ; i <= MaxFdScreens ; ++ i ) { // do PartSwalLoop1 i = 1, MaxFdScreens
				fBinFd1 [ i - 1 ] = fBinFd [ i - 1 ] ;
			} // PartSwalLoop1: continue
			// ! Calculate the various kComminuteOral interpolating bewteen kComminuteOralMin and kComminuteOralMax.
			// ! That is, the proprtions that pass from each bin to the next-small upon each mastication jaw movement
			// ! As described in Gregorini 2014 section 2.2.3
			
			// ! Apply chewing movements in a loop. Each chewing movement moves some from each bin to the next bin with smaller size.
			if ( t > 0 ) {
				for ( int j = 1 ; j <= MasticationSteps ; ++ j ) { // do PartSwalLoop2 j = 1, MasticationSteps ! do for each mastication step
					for ( int k = 2 ; k <= MaxFdScreens ; ++ k ) { // do PartSwalLoop3 k = 2, MaxFdScreens ! do for each bin: Transfer some from the each bin up to the next (smaller particles) bin
						fBinFd1 [ k -1 - 1 ] = fBinFd1 [ k -1 - 1 ] + kComminuteOral [ k - 1 ] * fBinFd1 [ k - 1 ] ; // put some in the next-smaller bin
						fBinFd1 [ k - 1 ] = fBinFd1 [ k - 1 ] - kComminuteOral [ k - 1 ] * fBinFd1 [ k - 1 ] ; // taking it from the current bin. Order may look weird (taking after putting) but it would require a temporary variable for fBinFd1(k) if the order is to be changed!
						if ( fBinFd1 [ k - 1 ] < 1e-8 ) fBinFd1 [ k - 1 ] = 0 ;
					} // PartSwalLoop3: Continue
				} // PartSwalLoop2: Continue
			}
			
			// ! Collate all the many bins into the 3 pools (fSPartSwal,fMPartSwal,fLPartSwal)
			// ! Mark you can add calculation of mean particle size within each of the three pools here, instead of assuming Pond
			for ( int i = 1 ; i <= MaxFdScreens ; ++ i ) { // do PartSwalLoop4 i = 1, MaxFdScreens
				if ( FdBinMeshSize [ i - 1 ] < MPartSize ) { // Note: we assume that MPartSize and LPartSize are aligned with/ exist in fdBinMeshSize()
					fSPartSwal = fSPartSwal + fBinFd1 [ i - 1 ] ; // Collate all bins with particles smaller than MPartSize
				} else if ( FdBinMeshSize [ i - 1 ] < LPartSize ) {
					fMPartSwal = fMPartSwal + fBinFd1 [ i - 1 ] ; // Collate all other bins with particles smaller than LPartSize
				} else {
					fLPartSwal = fLPartSwal + fBinFd1 [ i - 1 ] ; // Collate all bins larger than LPartSize
				}
				MeanParticleSize = MeanParticleSize + FdBinMeshSize [ i +1 - 1 ] * 0.7555 * fBinFd1 [ i - 1 ] ; // 0.7555 * mesh is the middle point of the size range below e.g. 0.7555 * 4.8 = 3.6 = middle of the range of (2.4 to 4.8)
			} // PartSwalLoop4: Continue
			
			fSPartSwal = max ( fSPartSwal , 1e-4 ) ; // Prevent 0.0, as it causes a crash
			fMPartSwal = max ( fMPartSwal , 1e-4 ) ; // Prevent 0.0, as it causes a crash
			fLPartSwal = 1 - fMPartSwal - fSPartSwal ;
			
			for ( int i = 1 ; i <= MaxFdScreens ; ++ i ) { // do PartSwalLoop5 i = 1, MaxFdScreens
				if ( ( SumBinFd < 0.5 ) && ( SumBinFd + fBinFd1 [ i - 1 ] >= 0.5 ) ) {
					MedianParticleSize = ( FdBinMeshSize [ i +1 - 1 ] * ( 0.5 - SumBinFd ) + FdBinMeshSize [ i - 1 ] * ( SumBinFd + fBinFd1 [ i - 1 ] - 0.5 ) ) / fBinFd1 [ i - 1 ] ;
				}
				SumBinFd = SumBinFd + fBinFd1 [ i - 1 ] ;
			} // PartSwalLoop5: Continue
		} else {
			MasticationSteps = 0.0 ;
			fSPartSwal = 0.3555 ;
			fMPartSwal = 0.3555 ;
			fLPartSwal = 0.3555 ;
		}
		fLPartMPartSwal = fLPartSwal + fMPartSwal ;
		// end of procedural 
		
		// ***** End of Nutrient Inputs Include Section ***************
		
		// ***************************************************************
		// This section is a PROCEDURAL which initiates values for
		// non-state variables and transfers new values which
		// were calculated algebraically based upon current status of
		// system from one integration step to the next.
		
		// procedural ( FCM4Z , OXUP1 , MamMilkAve2 , rtPOx , VolGl , VolAc , VolFa , 
			// VolAa , BldUrVol , ME , fTm , AtAdHT , EBW = t , iEBW ) 
		if ( t <= 1.0E-6 ) goto label_P ;
		rtPOx = rtPO ;
		fTm = fTm1 ;
		FCM4Z = FCM4z1 ;
		AtAdHT = AtAdH1 ;
		ME = ME1 ;
		OXUP1 = dOx ;
		// MamMilkAve=MamMilkAve changed to MamMilkAve2=MamMilkAve
		MamMilkAve2 = MamMilkAve ;
		EBW = EBW1 ;
		BW = BW1 ;
		VolGl = iVolGlF * BwCorrected * GlCor ; // Next4: Changed to non consider adipose Gil 2015.
		//        Changed to reflect conceptus space outside of maternal space, MDH or CCP 4-14-07
		VolFa = iVolFaF * BwCorrected * FaCor ;
		VolAc = iVolAcF * BwCorrected * AcCor ;
		VolAa = iVolAaF * BwCorrected * AaCor ;
		BldUrVol = iVolBldUrF * NonUterEBW * BldUrCor ;
		goto label_Q ;
	label_P: rtPOx = 5.3555 ;
		OXUP1 = iOxup ;
		// MamMilkAve=iMamMilkAve changed back to MamMilkAve2=iMamMilkAve
		MamMilkAve2 = iMamMilkAve ;
		//       MilkAve2=idMilk
		DMilk = iDMilk ;
		// THIS SECTION INITIATES VALUES FOR NON-STATE VARIABLES
		VolGl = iVolGl ;
		VolAc = iVolAc ;
		VolFa = iVolFa ;
		VolAa = iVolAa ;
		BldUrVol = iVolBldUr ;
		FCM4Z = iFCM4Z ;
		// TFCM4z = integ ( FCM4Z , iFCM4Z ) 
		ME = iME ;
		EBW = iEBW ;
		BW = iBW ;
		WtCytAdip = iWtCytAdip ;
		otWtOth = iotWtOth ;
		otWtVis = iotWtVis ;
		fPm = ifPm ;
		fTm = ifTm ;
		AtAdHT = iAtAdh ;
	label_Q:
		// end of procedural 
		// OF PROCEDURAL
		
		// *****************************************************
		// DYNAMIC ELEMENTS OF RUMEN SUBMODEL
		// ****************************************************
		
		// FEED COMPOSITION AND PHYSICAL PROPERTIES
		
		// The reference diet is 50% concentrate/50% chopped hay
		// fed at 15 Kg/day on a continuous basis (0.6555 kg/hr).
		// This is equivalent to an intake of 3% of BW (calculated
		// at EBW=500 kg). This keeps the reference cow in energy
		// balance.
		
		// The prefix f---Fd indicates the fraction in feed of each nutrient
		// in Kg/Kg dry matter.Codes are soluble carbohydrate(Sc),organic
		// acids(Oa),pectin(Pe),lactate(La),lipid(Li),starch
		// (St),hemicellulose(Hc),cellulose(Ce),soluble protein(Ps),insoluble
		// protein(Pi),non-protein nitrogen(Nn),lignin(Lg),soluble ash(As),
		// insoluble ash(Ai),volatile fatty acids as in silage(Ac,Bu),and
		// organic matter(Om).StSol is the fraction of starch which is
		// soluble and must be input as it is a physical characteristic of
		// feed.Similarly,PsF (1-fLPartSwal) is the proportion of feed which enters the
		// small plus medium particle pools directly and is a property of feed.
		
		// Allows fat to be added from control language
		//  without recalculating proportions of other feed compenents.
		// Thus, fFatFd is specified by setting FatAdd to proportions
		//  of diet.
		
		// InfPrt=ABOMASAL INFUSION OF CASEIN
		//  EXPRESSED IN KG/DAY
		CWC = fHcFd + fCeFd + fLgFd ;
		GE1Fd = fLiFd * 8.4555 + fStFd * 4.1555 + fHcFd * 4.2555 + fCeFd * 4.1555 + fNnFd * 5.7 ;
		GE2Fd = ( FPsFd + fPiFd ) * 5.7 + fLgFd * 8.3 + fAcFd * 3.4555 + fBuFd * 5.9555 ;
		GE3Fd = fScFd * 3.9555 + fFatFd * 9.6555 ;
		GEFd = ( fOaFd * 2.5555 + fPeFd * 3.6 + FLaFd * 3.7555 + GE1Fd + GE2Fd + GE3Fd ) * F1 ;
		
		// These factors convert Kg of nutrients metabolized via a common
		// pool to a common molecular weight basis.They were calculated as
		// follows:(171g Sc/mole Sc)*(0.5mole Sc/moleOa)/(134gOa/moleOa)=
		// (85.5gSc/moleOa)/(134gOa/moleOa)=0.6555gSc/gOa.
		
		// LARGE PARTICLE FRACTION AND COMPOSITION
		fLPartNutIng = ( fStFd - FStsFd + fCeFd + fHcFd + fLgFd + fAiFd + fPiFd ) * fLPartSwal ; // Proportion of LPartSwal
		fLPartSt = ( fStFd - FStsFd ) * fLPartSwal / fLPartNutIng ; // proportion of starch, added to LPart 5-21-13, MDH
		fLPartHc = fHcFd * fLPartSwal / fLPartNutIng ; // proportion of hemicellulose
		fLPartCe = fCeFd * fLPartSwal / fLPartNutIng ; // proportion of cellulose
		fLPartPi = fPiFd * fLPartSwal / fLPartNutIng ; // proportion of insoluble protein
		fLPartLg = fLgFd * fLPartSwal / fLPartNutIng ; // proportion of lignin
		fLPartAi = fAiFd * fLPartSwal / fLPartNutIng ; // proportion of insoluble ash
		fIndigFd = fLgFd + fAiFd ; // Combines lignin and silicates as Indigestible Feed (IndigFd)
		fLPartIndigFd = fLPartLg + fLPartAi ; // proportion of IndigFd
		fLgIndigFd = fLgFd / fIndigFd ;
		fAiIndigFd = fAiFd / fIndigFd ;
		fLPartADF = fLPartCe + fLPartIndigFd ;
		fLPartNDF = fLPartHc + fLPartADF ;
		
		// Medium PARTICLE INPUT COMPOSITION
		fMPartNutIng = ( fStFd - FStsFd + fCeFd + fHcFd + fLgFd + fAiFd + fPiFd ) * fMPartSwal ;
		
		// SMALL PARTICLE INPUT COMPOSITION
		fSPartNutIng = ( fStFd - FStsFd + fCeFd + fHcFd + fLgFd + fAiFd + fPiFd ) * fSPartSwal ;
		
		// TOTAL SOLUBLE SUGAR EQUIVALENT IN FEED
		// Combines Oa,Pe,lipid glycerol,La,and Sc as one fraction
		OaSc = fOaFd * OaScSC ;
		PeSc = fPeFd * PeScSC ;
		LiSc = fLiFd * LiScSC ;
		// GLYCEROL FROM FatFd ADDED
		FatSc = fFatFd * FaScFd ;
		// La moved to a separate pool
		fScTFd = fScFd + OaSc + PeSc + LiSc + FatSc ;
		
		// RUMINATION!
		// ***********
		// Next block (up to !STOICHIOMETRIC) was merged From Molly86 for Mindy. Gil July 2012
		// Rumntn is a switch that is on provided the animal is not eating and there is adequate material
		// in the LP pool to stimulate rumination. The following code does not currenlty support continuous
		// Rumntn is Proportion of time spent ruminating per unit time.
		// There are separate equations for twice daily feeding and
		// continuous or multiple feedings.  Rumination is shut off during
		// feeding when the animal is fed twice daily.  In continuous
		// feeding, the animal ruminates during feeding because rumination
		// is needed for large particles to be broken down into small
		// particles.  Since the default is continuous feeding, RumntnEQ is
		// set to zero and ruminating rate (RumntnF=Rumntn) to 0.3555. When
		// discrete feeding periods are used (FdInt does not = FdTm), one
		// should implement the rumination equation of Murphy, et al
		// (JDS___) by setting RumntnF=0.0 and RumntnEQ=1.0. This also enables
		// resting salivation and drinking for the water balance equations
		// below.
		
		DAY = 1.0 ;
		CWCF = 1.0 * ( 0.1555 +0.5555 * CWC ) ;
		
		// TotRumntn = integ ( Rumntn , 0.0 ) 
		// TotEating = integ ( Eating , 0.0 ) 
		// TotRest = integ ( Rest , 0.0 ) 
		
		// STOICHIOMETRIC COEFFICIENTS FOR FERMENTATION
		// Three sets of values are included. One is for, largely,forage
		// diets(FORSET),one is for 50:50 forage:concentrate diets(MIXSET)
		// The third set is for,largely,concentrate diets(CONSET).FORSET and
		// CONSET are from Murphy et al,JAS 55:411-421,(1982).MIXSET is an
		// average of the other two since these apply to less than 50% and
		// more than 50% concentrate, respectfully. Valerate was separated
		// to 1/2Bu+1Pr to maintain carbon and hydrogen balance. Values are
		// in moles of VFA produced per mole of glucose fermented. 1 mole of
		// glucose gives 2 moles of Ac or Pr but only one mole of butyrate.
		// Amino acid fermentation stoichiomietry adjusted 08/26/91 to
		// Murphy model to reflect C, H and O balance.  Heavy propionate
		// reflects Pr+1/2 Bu from Bcfa.  H2 (or CH4) is VERY HIGHLY!
		// DEPENDENT on proportion of Bcfa produced.  Expressed as moles
		// VFA produced per mole Aa fermented with an average C/Aa=5.08.
		
		LaPrPr = 0.1555 ;
		// for Simusolv fitting
		
		// procedural ( ScAcAc , StAcAc , HcAcAc , CeAcAc = FORSET , MIXSET , CONSET ) 
		if ( FORSET == 1.0 ) goto label_10 ;
		if ( MIXSET == 1.0 ) goto label_11 ;
		if ( CONSET == 1.0 ) goto label_12 ;
		
		// FORSET
	label_10: ScAcAc = 1.3555 ;
		ScPrPr = 0.4555 ;
		ScBuBu = 0.1555 ;
		ScLaLa = 1.0E-8 ;
		StAcAc = 1.2555 ;
		StPrPr = 0.3555 ;
		StBuBu = 0.2555 ;
		StLaLa = 1.0E-8 ;
		HcAcAc = 1.1555 ;
		HcPrPr = 0.4555 ;
		HcBuBu = 0.2555 ;
		CeAcAc = 1.3555 ;
		CePrPr = 0.2555 ;
		CeBuBu = 0.2555 ;
		goto label_13 ;
		
		// MIXSET
	label_11: ScAcAc = 1.1555 ;
		ScPrPr = 0.4555 ;
		ScBuBu = 0.2555 ;
		ScLaLa = 1.0E-8 ;
		StAcAc = 1.00 ;
		StPrPr = 0.5555 ;
		StBuBu = 0.2555 ;
		StLaLa = 1.0E-8 ;
		HcAcAc = 1.1555 ;
		HcPrPr = 0.4555 ;
		HcBuBu = 0.1555 ;
		CeAcAc = 1.4555 ;
		CePrPr = 0.2555 ;
		CeBuBu = 0.1555 ;
		goto label_13 ;
		
		// CONSET
	label_12: ScAcAc = 0.9555 ;
		ScPrPr = 0.4555 ;
		ScBuBu = 0.3555 ;
		ScLaLa = 1.0E-8 ;
		StAcAc = 0.8555 ;
		StPrPr = 0.7555 ;
		StBuBu = 0.2555 ;
		StLaLa = 1.0E-8 ;
		HcAcAc = 1.1555 ;
		HcPrPr = 0.5555 ;
		HcBuBu = 0.1555 ;
		CeAcAc = 1.5555 ;
		CePrPr = 0.2555 ;
		CeBuBu = 0.1555 ;
		goto label_13 ;
		
	label_13:
		// end of procedural 
		TVFA = RumAc / RumAcCor + RumPr / RumPrCor + RumBU / RumBuCor ;
		// SPart = integ ( dSPart , iSPart ) 
		
		// Fractional particle pool sizes
		fLPart = ( LPart / LPartCor ) / ( LPart / LPartCor + MPart + SPart ) ;
		fMPart = MPart / ( LPart / LPartCor + MPart + SPart ) ;
		fSPart = SPart / ( LPart / LPartCor + MPart + SPart ) ;
		fMPart1 = MPart / ( MPart + SPart ) ;
		fSPart1 = SPart / ( MPart + SPart ) ;
		
		// Calculate the Particle size distribution from the LP and MP pools from Fisher et al., 1988
		// Slope is calculated from only 2 points.  If particle size reduction and degradation is not
		// 	appropriately represented in the model, this slope and intercept could become invalid. MDH 5-9-13
		RumPartSizeSlp = ( log ( ( fLPart + fMPart ) * 100 ) - log ( 100.0 ) - log ( fLPart * 100 ) + log ( 100.0 ) ) / ( MPartSize - LPartSize ) ;
		RumPartSizeInt = log ( ( fMPart + fLPart ) * 100 ) - log ( 100.0 ) - RumPartSizeSlp * MPartSize ;
		if ( RumPartSizeSlp == 0 ) RumPartSizeSlp = 1e-9 ;
		
		// Calculate the mean particle size and surface area in each pool for use in degradation equations
		// mm from Pierre Beukes
		
		// 27.6555mm2/mm3=mean SA per unit Vol for Shaver et al. 1988 data
		
		RumPartSizeMean = 1 / - RumPartSizeSlp + RumPartSizeInt / - RumPartSizeSlp ;
		RumLpMpSizeMean = 1 / - RumPartSizeSlp + MPartSize ;
		RumLPartSizeMean = 1 / - RumPartSizeSlp + LPartSize ;
		RumMPartSizeMean = RumLPartSizeMean - RumLpMpSizeMean ;
		RumSPartSizeMean = RumLpMpSizeMean - RumPartSizeMean ;
		MPartSA = RumMPartSizeMean * PartWidth * 2 + RumMPartSizeMean * PartThick * 2 + PartWidth * PartThick * 2 ;
		MPartVol = RumMPartSizeMean * PartWidth * PartThick ;
		SPartSA = RumSPartSizeMean * PartWidth * 2 + RumSPartSizeMean * PartThick * 2 + PartWidth * PartThick * 2 ;
		SPartVol = RumSPartSizeMean * PartWidth * PartThick ;
		fSPartSA = SPartSA / SPartVol / kSurfaceArea ; // Scale SPartSA per Vol to that for a standard SP pool
		fMPartSA = MPartSA / MPartVol / kSurfaceArea ; // Scale MPartSA per Vol to that for a standard SP pool
		fPartSA = ( fMPart1 * fMPartSA + fSPart1 * fSPartSA ) ;
		fPartP = ( fMPart1 * KMPartP + fSPart1 * KSPartP ) ;
		
		cMiHa = ( HaMi / MiHaCor ) / ( Ha / HaCor ) ; // Concentration (kg/kg) of
		cMiHb = ( HbMi / MIHbCor ) / ( Hb / HbCor ) ; // microbes associated with Ha and Hb
		
		HaMiF = VmMiHa / ( 1.0 + KMiHaF / ( ( Ha / HaCor ) / ( SPart + MPart ) ) ) ; // Fraction of potentially released microbes actually
		HbMiF = VmMiHb / ( 1.0 + KMiHbF / ( ( Hb / HbCor ) / ( SPart + MPart ) ) ) ; // dependent on fractions of Ha and Hb in SPart where
		SPartHaCs = KHaCs * Ha / HaCor * cMiHa * fPartSA ;
		HaCs = SPartHaCs / MwtSt ; // CS
		CsP = KWAP * Cs / CsCor ;
		RumAaP = KWAP * RumAa / RumAaCor ; // AMINO ACIDS-RumAa
		// AM2 = integ ( dAm , iAm ) 
		AM = max ( 1e-9 , AM2 ) ; // Prevent a crash when AM goes negative. I don't have time to find the source
		InfAs = InfNaCl + InfNaBicarb ;
		AsP = As / ASCor * KWAP ;
		// FATTY ACIDS
		FaP = KWAP * Fl / FLCor ;
		absRumAc = KabsAc * RumAc / RumAcCor ;
		RumAcP = ( RumAc / RumAcCor ) * KWAP ;
		RumPrP = ( RumPr / RumPrCor ) * KWAP ;
		absRumPr = KabsPr * RumPr / RumPrCor ;
		absRumBu = KabsBu * RumBU / RumBuCor ;
		RumBuP = ( RumBU / RumBuCor ) * KWAP ;
		RumLaP = ( RumLa / RumLaCor ) * KWAP ;
		absRumLa = RumLa * KabsLa / RumLaCor ;
		
		// MICROBIAL FUNCTIONS (Mi in Kg)
		// ********************************
		
		// MICROBIAL GROWTH PARAMETRS
		
		// ATP generation and microbial growth and composition
		// CsFvAt is set at 4.0 molesATP/mole Cs but is really
		// a variable.
		
		// Growth without amino acids (G1)
		// Moles nutrient/Kg microbe formed
		// Recalculated 14 Feb,1984 to conform to elemental composition
		// and paths of Reichl and Baldwin(JDS 58:879(1975)) with diet
		// lipid as the source of long chain fatty acids(1.2 moles
		// /mole Li)
		
		FlMiG = 0.2555 ;
		
		// Microbe composition (Kg/Kg)
		// NOTE:Organic Matter only
		
		// Lipid composition in mole/mole
		
		MwtMiLi = 0.6555 ;
		FGFa = 1 + ( fFatFd / fLiFd * KFatFG ) ;
		SPartMi = ( ( SPart + MPart ) / ( RumDM - Mi / MICor ) ) * ( Mi / MICor ) ;
		WaMi = Mi / MICor * SolDM / ( RumDM - Mi / MICor ) ;
		cMiSPart = SPartMi / ( SPart + MPart ) ;
		cMiWa = WaMi / SolDM ;
		// Mi = integ ( dMi , iMi ) 
		
		// Total Ruminal DM, SolDM, N and CP, kg
		RumNit = ( RumAa / RumAaCor * MwtPs + Pi / PICor + Mi * MiPiPI + Mi * MiNnNn + fLPartPi * LPart / LPartCor ) * .16 + AM * .14 ;
		RumNDF = Hc / HcCor + Ce / CeCor + IndigFd / IndigFdCor + LPart / LPartCor * fLPartNDF ;
		
		// BEGIN  INCLUDE 'EXPERIMENTAL_BIAS_Vectors_in_deriv.csl'
		
		// Not needed in this project
		// END  INCLUDE 'EXPERIMENTAL_BIAS_Vectors_in_deriv.csl'
		
		// LOWER GUT (Lgut) FUNCTIONS and DIGESTION COEFFICIENTS (DC)
		// ************************
		
		otGutCont = 1 * IntakeDay ; // Changed to a daily summary value to accomodate within day eating patterns. MDH 5-23-11
		LgutFaDg = LgutDCFa * FaP ;
		LgutAs = LgutDCAs * AsP ;
		FecFa = FaP * ( 1 - LgutDCFa ) * MwtFl ;
		absLa = absRumLa + RumLaP ;
		absLaE = absLa * HcombLa ;
		
		// **********************************************************
		// DYNAMIC ELEMENTS OF ANIMAL SUBMODEL
		// ********************************************************
		
		// Daylength to calculate intake and photoperiod effects on lactation
		// Positive for N Hemi, Neg for S Hemisphere
		DayofYear = iDayOfYear + t ;
		// Unit: hours. Set to 1 if the management hours (milking hours, feeding times in Mindy) are expressed in daylight saving time with 1 hour shift.
		
		// Daylength calculated using the model of Forsythe et al, 1995 Ecol Mod 80:87-95
		// This equation set works much better than my sin wave and provides for twighlight
		// Expressed as hours of daylight
		// This was a bad to express in hours.  It should be changed to days to be consistent with the rest of the model.
		// procedural ( DayLength , DayTwLength = DayofYear , Latitude ) 
		DaylengthP1 = asin ( 0.3555 * cos ( 0.2555 +2 * atan ( 0.9555 *
			tan ( 0.008555 * ( DayofYear -186 ) ) ) ) ) ;
		// Daylength including twilight used for within day intake equation
		DayTwlengthP2 = ( sin ( 6 * 3.1555 / 180 ) + sin ( Latitude * 3.1555 / 180 ) *
			sin ( DaylengthP1 ) ) / ( cos ( Latitude * 3.1555 / 180 ) * cos ( DaylengthP1 ) ) ;
		if ( DayTwlengthP2 < -1 ) DayTwlengthP2 = -1.0 ;
		if ( DayTwlengthP2 > 1 ) DayTwlengthP2 = 1.0 ;
		DayTwLength = 24 - ( 24 / 3.1555 ) * acos ( DayTwlengthP2 ) ;
		// Daylength without twilight used for lactation photoperiod
		DaylengthP2 = ( sin ( 0.8555 * 3.1555 / 180 ) + sin ( Latitude * 3.1555 / 180 ) *
			sin ( DaylengthP1 ) ) / ( cos ( Latitude * 3.1555 / 180 ) * cos ( DaylengthP1 ) ) ;
		if ( DaylengthP2 < -1 ) DaylengthP2 = -1.0 ;
		if ( DaylengthP2 > 1 ) DaylengthP2 = 1.0 ;
		DayLength = 24 - ( 24 / 3.1555 ) * acos ( DaylengthP2 ) ;
		// Determine if the sun is above the horizon. Next 5 lines were Merged from Mark's Molly86. Gil July 2012
		Sunlight = sin ( ( t - DaylightSavingShift / 24 - 0.2555 ) * 3.1555 * 2 ) - ( 0.5 - DayLength / 24 ) * 3 ; // positive if the sun is up and negative it is below the horizon
		Sunrise = 0.5 + DaylightSavingShift / 24 - DayLength / 24 / 2 ;
		SunSet = 0.5 + DaylightSavingShift / 24 + DayLength / 24 / 2 ;
		if ( t == 0 ) {
			SunriseToday = Sunrise ;
			SunsetToday = SunSet ;
			SunsetTodayTemp = SunSet ;
		}
		// end of procedural 
		
		// *************** Hormones ***********************
		// ANABOLIC AND CATABOLIC
		// procedural ( AHor , AHor1 , CHor , CHor1 = ) 
		
		if ( t == 0 ) cGl = icGl ; // trap bad Glc value at time 0
		AHor = ( cGl / kAHorGl ) *pow(1,1)* Theta2 ; // Stimulates FaTsAdip, AaPOthOth, AaPVisVis, rtOx1 (GlOx)
		AHor1 = ( cGl / kAHor1Gl ) *pow(1,1)* Theta3 ; // Stimulates AcTsAdip
		CHor = ( kCHorGl / cGl ) *pow(1,1)* Theta4 ; // Not Used
		CHor1 = ( kCHor1Gl / cGl ) *pow(1,1)* Theta4 ; // Stimulates TsFaAdip
		// end of procedural 
		
		// Gil May 2012 code that was here removed, because had to introduce include file intermittent_Eating_deriv.csl
		
		// *************************** Gestation ***********************************
		// Pregnancy switches
		// procedural ( NonPreg , Preg = DayGest ) 
		// Gestation Switches
		Preg = 0 ;
		NonPreg = 1 ;
		if ( DayGest > 0 ) {
			Preg = 1 ;
			NonPreg = 0 ;
		}
		// end of procedural 
		
		// ****************** Uterine Tissue and Protein Mass ********************
		WtUterPart = iWtUter * exp ( ( kUterSyn - kUterSynDecay * GestLength ) * GestLength ) ;
		
		// ********************* Conceptus plus Fluid Mass ***********************
		WtConcAgeFactor = 1.0 - 0.02555 * ( 4.001 - min ( 4.0 , AgeInYears ) ) *pow(1,1)* 2.3555 ; // GL. This yields 0.8555 for 2 years old; 0.9555 for 3 & 1.0 for 4+ Based on DairyNZ data
		WtConc = WtConcAgeFactor * WtConcBreedFactor *
			iWtConc * exp ( ( kConcSyn - kConcSynDecay * DayGest ) * DayGest ) ;
		WtConcSyn = WtConc * ( kConcSyn -2 * kConcSynDecay * DayGest ) * Preg ;
		WtPConc = iWtPConc * exp ( ( kPConcSyn - kPConcSynDecay * DayGest ) * DayGest ) ;
		WtPConcSyn = WtPConc * ( kPConcSyn -2 * kPConcSynDecay * DayGest ) * Preg ;
		AaPConc = WtPConcSyn * Preg / MwtPVis ;
		// OF PROCEDURAL
		
		// BEGIN  INCLUDE 'MamCells_deriv.csl'
		
		// ****** Modified lactaion sub model GL October 2011 *******************************************
		
		// Next 3 are NOT USED IN ACSL BUT DO NOT DELETE, we need it in the automaticaly generated caowParameters file to be use optimiseable. It is used in WFM / Smalltlak
		
		// Also Key Determiner of required intake
		// WFM sets it to 2.5555 * KgMilkSolidsExpectedIn270Days - 200
		// Decay rate for cell proliferation precalving from Dijkstra goat DNA
		// Specific Cell proliferation rate at parturition from Dijkstra
		// Mam Cell Death rate from Dijkstra Mexican cow (now used only precalving GL)
		// From Vetharaniam et al 2003
		// Determines the base (2x) post peak decline of MamCells (Note: LHOR affcets Milk Production decline as well)
		// Very sensitive! determines the timing of the peak. Decraese delays
		// The ratio between this one and kMamCellsAQBase drives the proprtion of the A pool in its relatively stable period (post peak)
		// Speeds up the first week's A pool growth => 1st week milk yield
		// Speeds up the first week's A pool growth => 1st week milk yield
		// Affects mid lactation milk curve. Controls the centre DIM of the change from "pre" tp "post" kMamCellsQA
		// Affects mid lactation milk curve. Controls the speed of transition from "pre" to "post"kMamCellsQA. 0.1 gives 90% of the change with the 60 days around kMamCellsTransitionDim. With 0.2 the 60 goes down to 30.
		// This determines the mild curvature (convex) of the  post peak mam cells decline, by slowing the absolute senescence rate gardually down to base turn over rate. Note that a typical near-constant persistnecy is exponantial decay by nature (e.g. 98% weekly persistency means every week is 98% of the previous => exponential decay.
		// Base rate of proliferation and senecence, has no real effect because it does not affect the net change of MamCells. Only purpose is to maintain biological correctness of 100% turnover by around 250 DIM (Capuco / Ruminant phisiology)
		// This determines Peak mam cells. Not much to play with here as it is commonly accepted that day 10 in milk is the peak
		// Rate of the increased senecence while on low milking frequency. (works on a continuous scale between MF 1 and 2)
		// Changes the Q to A flux as MF changes. When decreased, difference between 1x,2x and 3x will grow.
		// This sets the maximum proportion of MamCellPart that would be permanently lost due to long term low milking frequency. The loss is fast to start with and slows down the more days on low MF the cow goes through
		// **** MamCells ************************************************************************************
		// Direct Dijkstra prediction precalving.
		// After calving - fluxes of proliferation and senescence, that create dijkstra pattern
		// when integreated for 2x cows, yet on low milking frequency senescence is increased
		
		PreCalvingMamCells = MamCellsPart * exp ( - uTMamCells * ( 1 - exp ( K1MamCells * DayMilk ) ) / K1MamCells + lambdaMamCells * DayMilk ) ; // Oroginal Dijkstra
		PostCalvingMamCells = MamCellsA + MamCellsQ ;
		fMamCellsAQ = MamCellsA * kMamAQBase ; // GL: removed KMINH as there are alternative explicit MF drivers
		MamCellsQaPreToPostFactor = 1 / ( 1 + exp ( kMamCellsTransitionSteepness * ( kMamCellsTransitionDim - DayMilk ) ) ) ;
		// CumulativeLowMfDays = integ ( DailyMfDiff , 0 ) // Keep track of the cummulative number of 1x eqivalent days on low liking freq, e.g 20 days on 1.5x or 10 days on 1x both grow cumulativeLowMfDiff by 10.
		LowMfDecay = exp ( - kMamCellsUsMfDecay * CumulativeLowMfDays ) ; // The longer on low MF the slower the senescence
		LHor = FixedLhorSW * LHorBase + ( 1 - FixedLhorSW ) * LHor1 ;
		VmLHorSyn = wLHorSensAa + wLHorSensGl + wLHorSensAdip ; // Denominator of LhorSyn. Set so that the division gives 1.0 when all drivers are on base line
		
		kLHor = LHorBase / LhorTurnoverDays ; // Base degredadion and synthesis rate. Unit: LHOR units per day
		
		KLHorPP = ( 12 / DayLength - 1 ) * KDayLength ; // CCP 12-11-06
		BcsTargetFactor = min ( 1.0 , max ( exp ( - BcsTargetDecay * ( DayMilk - 1.0 ) ) , ( DayMilk -50.0 ) / ( 365.0 -50.0 ) ) ) ; // Changes beween 1 (calving) through 0 (~day 40) than up to 1 after 365 days, to create the BcsTarget
		CorrectedBW = NonUterEbwTarget + iRumVol ; // Using iRumVol rather than RumVol, to avoid fluctuation
		
		// **** Mammary ENZYMES ***************************************************************************
		// This should be a fn of DayMilk
		MamEnz = MamCellsA * PMamEnzCell * ( LHor / LHorBase ) * BST ;
		MilkingFrequencyAdjusted = MilkingFrequencyLag * MilkingFrequencyAgeAdjustment ; // Young cows do not perform as well as mature cows on OAD. Observed relationship incorporated, and we capture that in both reduced intake(FdRat_DairyNZ.csl), and here, by artificially reducing their milking frequency as if they are miked even less than once a day, to magnify the OAD effect (slow return from Q to A pools)
		
		// procedural ( InMilk = DayMilk ) 
		if ( DayMilk < 0 ) {
			InMilk = 0.0 ;
		} else {
			InMilk = 1.0 ;
		}
		// end of procedural 
		
		// Spike smoothing example - keep
		// AaSmoothed  = Integ(dAaSmoothed,AaBase)
		// dAaSmoothed = 0.1 * (Aa - AaSmoothed)
		// END  INCLUDE 'MamCells_deriv.csl'
		
		MilkingFrequency = 1.0 / MilkInt ; // Gil May 2012 added to make the merged code compile
		MilkInhDeg = KMinh * KMilkInhDeg ;
		WtTsAdip = MwtTs * TsAdip ;
		// Adds injected INS as effector of Gl uptake
		cFa = Fa / VolFa ;
		// Adds injected INS as long term effector
		// Changed VmAcTs to VmAcTs2 - kc 3/2/95
		// VmAcTs2 = integ ( dVmAcTs , iVmAcTs ) 
		VmAcTs = VmAcTs2 ;
		cAc = Ac / VolAc ;
		
		dOthDna = ( KDnaOth *pow(1,1)* ExpOth2 ) * ( ( OthDnaMx - OthDna ) / OthDnaMx ) ;
		dVisDna = ( KDnaVis *pow(1,1)* ExpV2 ) * ( ( VisDnaMx - VisDna ) / VisDnaMx ) ;
		POthAaOth = KPOthAaOth * POth ;
		PVisAaVis = KPVisAaVis * PVis ;
		WtPOth = POth * MwtPOth ;
		WtPVis = PVis * MwtPVis ;
		WtOth = WtPOth / fDWt + otWtOth ;
		WtVis = WtPVis / fDWt + otWtVis ;
		// PVis = integ ( dPVis , iPVis ) 
		// POth = integ ( dPOth , iPOth ) 
		// Fixed by Gil: instead of Aa =INTEG(dAa ,iAa) use the following 2 lines
		// AA1 = integ ( dAa , iAa ) 
		AA = max ( 0.1 , AA1 ) ;
		// BldUr1 = integ ( dBldUr , iBldUr ) 
		BldUr = max ( 1e-9 , BldUr1 ) ; // Prevent a crash when AM goes negative. I don't have time to find the source
		// The following adjustment to VmGlLm was added to allow for greater fat and protein content
		// in early and late lactation. MDH 3-6-06
		
		VmGlLm1Vis = VmGlLmVisPart * exp ( kVmGlLmSyn * ( 1 - exp ( - kVmGlLmDecay * DayMilk ) ) /
			kVmGlLmDecay - kVmGlLmDeg * DayMilk ) ; // Dijkstras equation to represent Lm synth capacity
		fGlHyAdip = pGlHyAdip ;
		fGlHyVis = pGlHyVis ;
		// Gl = integ ( dGl , iGl ) 
		cGl = Gl / VolGl ;
		// MamLm = integ ( dMamLm , iMamLm ) // Mammary LACTOSE
		// TMilkLm = integ ( dMilkLm , 1.0E-8 ) // AND TOTAL YIELD
		TVolMilk = TMilkLm / fLm ;
		MamMilk = MamLm / fLm ;
		dMamMilkAve = TAveMilkMam * ( MamMilk - MamMilkAve2 ) ;
		// absEAve = integ ( dabsEAve , iabsEAve ) 
		absEF = absEAve / ( EBW *pow(1,1)* 0.7555 ) ;
		KNaAtOth = 0.8 + KNaOth * T3 * absEF ;
		KNaAtAdip = 0.3 + KNaAdip * T3 * absEF ;
		AtAd12 = KidWrk * EBW *pow(1,1)* 0.7555 ;
		AtAd13 = HrtWrk * OXUP1 ;
		AtAd14 = ResWrk * OXUP1 ;
		HtV5 = AtAd12 * AtAdHT ; // kidney work
		HtV6 = AtAd13 * AtAdHT ; // heart work
		HtV7 = AtAd14 * AtAdHT ; // respiration
		rtOx1 = cGl * ( cAc + KAcCd ) / ( cAc * ( cGl + KGlCd / AHor ) ) ;
		rtOx2 = cFa * ( cAc + KAcCd ) / ( cAc * ( cFa + KFaCd ) ) ;
		// tNep = integ ( NEP , 0 ) 
		// DAILY MILK COMPOSITION
		propLm = dLmProd / ( dMilkProd +1.0e-9 ) ;
		fPm = dPmProd / ( dMilkProd +1.0e-9 ) ;
		fTm1 = dTmProd / ( dMilkProd +1.0e-9 ) ;
		
		// TCH4 = integ ( dTCH4 , iTCH4 ) // TCH4 is in moles
		
		CH4KGY = TCH4 * MwtCH4 ; // total kg methane
		TCH4E = TCH4 * HcombCH4 ; // total kcal methane
		
		SPartMiHa = KMiHa * Ha / HaCor * cMiSPart ; // Microbes attached
		SPartMiHb = KMiHb * Hb / HbCor * cMiSPart ;
		
		HaMiRum = cMiHa * SPartHaCs ; // Microbes already associated with SPart potentially
		SPartPiAa = Pi / PICor * KPiAa * cMiSPart * fPartSA * ( 1 - ( fFatFd / fLiFd * KFatPi ) ) ;
		// BldUr transport accross rumen wall inhibited by Am.
		absRumAm = KAmabs * AM / AmCor ;
		cBldUr = ( BldUr / BldUrCor ) / BldUrVol ;
		RumLaFv = KLaFv * cMiWa * RumLa / RumLaCor ;
		WaMiP = KWAP * WaMi ;
		WtUter = WtConcAgeFactor * WtConcBreedFactor *
			( ( iWtUter * exp ( ( kUterSyn - kUterSynDecay * DayGest ) * DayGest ) ) * Preg
			+ ( ( WtUterPart - iWtUter ) * exp ( - kUterDeg * fmod ( DayMilk +3000.0 , 3000.0 ) ) + iWtUter ) * NonPreg ) ;
		// Ox and CO2 are not accounted for, MDH 4-14-07
		
		// ************** MILKING *****************************
		// MilkingTm controls the amount of time required for milk out (d).  Set to 1 for continuous milking.
		// MilkInt controls the time interval for milking (d). Set to .5 for 2x/d, .33 for 3x/d, etc.
		
		// procedural ( MilkSW = ResidMamMilk , MamMilk , DayMilk ) 
		if ( DayMilk <= 0 ) MilkSW = 0 ; // Turn off milking when dry
		if ( MamMilk <= ResidMamMilk ) MilkSW = 0.0 ; // Turn off milking when udder empty
		// end of procedural 
		MamCells = ( InMilk * PostCalvingMamCells ) + ( ( 1 - InMilk ) * PreCalvingMamCells ) ;
		fMamCellsPA = InMilk * MamCellsPart * ( BaseMamCellsTurnOver + uTMamCells * exp ( - MamCellsProliferationDecayRate * DayMilk ) ) ; // Approximation of the growth rate derived from Dijkstra's MamCells eq.
		
		// MamCellsA = integ ( dMamCellsA , iMamCellsA ) 
		// MamCellsQ = integ ( dMamCellsQ , iMamCellsQ ) 
		// MamCellsS = integ ( dMamCellsS , iMamCellsS ) // We don't really need this pool, only to verify that there is approx 100% turnover over 250 days of lactation
		
		// ****** kMamCellsQa ***************************************************************************
		// To Delay the peak without tempering with MamCells peaking at 10 DIM, a variable QA rate function
		// was needed. Rather than being constant, kMamCellsQA is now very fast during first week of lactaion,
		// low afterwards for few weeks and than gradually stabilises on the final value for post peak lactation.
		// This final value is the major driver of the steady state proportion of active cells (pMamCellsA)
		
		MamCellsQaKickStartFactor = InMilk * ( exp ( - kMamCellsQAKickStartDecay * DayMilk ) ) ;
		
		// ****** Low Milking Frequency adjustements ******************************************************
		// While on low milking frequency we have increased senescence (long term carry over)
		// on top of the short term reduction (larger Q pool)
		// currently senescence is capped and reaches the cap gradually in a slowing manner
		// depending on total days in low milking frequency (not necessarily consequtive)
		kMamCellsQaMfAdjustment = ( MilkingFrequencyAdjusted / 2 ) *pow(1,1)* MilkIntPowerForFMamCelsQA1 ; // Stimulus adjustment, more cells go from Q to A as the milking frequency goes higher. Note that it gives 1 = no change for twice a day milking (MilkInt = 0.5).
		DailyMfDiff = max ( 0. , ( 2 - MilkingFrequency ) * InMilk ) ; // DailyMfDiff would be 1 while on once a day; 0.5 while on 1.5 a day etc.
		LHorDeg = kLHor * ( 1 + KLHorPP ) * ( LHor / LHorBase ) ; // CCP + GL (made turnover adjustable). This equation empties (1 + KLhorPP)/turnoverDays of LHOR pool
		
		// **** Target BCS *******************************************************************************************
		// Target BCS around peak lactaion. Calving, Calving + 365 days => BcsTarget = BcsBase; DIM 10 to 70:  BCS = BcsTargetNadir;  oOther DIM: linear - connect the dots.
		// Determines the exponential down curve of TargetBcs from calving to nadir
		
		BcsTarget = BcsTargetNadir + ( BCSBase - BcsTargetNadir ) * BcsTargetFactor ; // This goes from BcsBase down to BcsTargetNadir around peak and recovering to base after 365 days. This reflectes the findin of JR et al JDS 2007, around this VARYING target BCS the sesnitivity (milk response) to adipose size drastically changes from high (e.g. under that BCS: 17% yield for 1 BCS NZ point at peak) to very low (4% yield for BCS point around peak)
		WtAdipTarget = 0.2555 * CorrectedBW + 36 * BcsTarget - 122.1 ; // MDH Target Adipose weight defended by the animal. GL made TegetBcs variable to refelect findings of BCS article, JR et al JDS 2007
		
		// **** Misc **************************************************************************************
		
		// MilkingFrequencyLag = integ ( derivMilkingFrequencyLag , 2 ) // Lagging Mf moves down fast but slow to catch up after MF changed upwards.
		if ( MilkingFrequency < MilkingFrequencyLag ) {
			derivMilkingFrequencyLag = kMilkingFrequencyLagDown * ( MilkingFrequency - MilkingFrequencyLag ) ; // fast change from higher MilkingFrequency to a lower MilkingFrequency
		} else {
			derivMilkingFrequencyLag = kMilkingFrequencyLagUp * ( MilkingFrequency - MilkingFrequencyLag ) ; // slow recovery from low MilkingFrequency to higher MilkingFrequency
		}
		
		// procedural ( MilkSolids270MfAdjusted = MilkingFrequency , InMilk ) 
		
		MilkingFrequencyBaseAdjustment = ( 2.0 - min ( 2.0 , max ( 1.0 , MilkingFrequency ) ) ) * OnceADayMilkingAdjustment +
			( min ( 2.0 , max ( 1.0 , MilkingFrequency ) ) - 1.0 ) * 1.0 ; // Interpolate according to current Milking Frequency. Yields 1.0 for twice a day or more milking.
		
		OnceADay2YearsOldAdjustment1 = ( 2.0 - min ( 2.0 , max ( 1.0 , MilkingFrequency ) ) ) * OnceADay2YearsOldAdjustment +
			( min ( 2.0 , max ( 1.0 , MilkingFrequency ) ) - 1.0 ) * 1.0 ; // Interpolate according to current Milking Frequency. Yields 1.0 for twice a day or more milking.
		
		MilkingFrequencyAgeAdjustment = ( ( 4 - min ( 4. , max ( 2.0 , AgeInYears ) ) ) * OnceADay2YearsOldAdjustment1 +
			( min ( 4. , max ( 2.0 , AgeInYears ) ) - 2.0 ) * 1.0 ) / 2.0 ; // Interpolate according to current age. Yields 1.0 for mature cows.
		
		MilkSolids270MfAdjusted = KgMilkSolidsExpectedIn270Days * MilkingFrequencyAgeAdjustment * MilkingFrequencyBaseAdjustment ; // Expected yield on actual milking frequency
		// end of procedural 
		MilkInhSyn = ( 1 * MamMilk ) / ( KMilkI + MamMilk ) ; // Changed to prevent negative values, Apr 23, 2008 MDH
		// TRIGLYCERIDE-TsAdip
		WtAdip = WtCytAdip + WtTsAdip ;
		FaTsAdip = VmFaTsAdip * ( EBW *pow(1,1)* 0.7555 ) / ( 1.0 + KFaTsAdip / cFa + K1FaTs / ( AHor * INS * cGl ) ) ;
		FaTmVis = ( VmFaTmVis * MamEnz ) * ( INS *pow(1,1)* P1 ) / ( 1.0 + KFaTmVis / cFa + K1FaTm / cGl ) ;
		// Adds injected INS as effector of VMAx
		// Fa = integ ( dFa , iFa ) 
		
		// ACETATE METABOLISM(Ac)
		// **************************
		// Inputs are from absorbtion (absAc=65.9) and
		// gluconeogenesis(AaGlVis*AaAcAc=4*.6=2.4).
		// Output is oxidation (AcCd=35.5) and lipogenesis in adipose
		// and mammary(AcTsAdip=16.,AcTmVis=16.8).
		// AaAcAa set at 0.6 to replace 0.2555 KB on an equal ~P basis.Must
		// correct final output of Cd for excess produced by this compromize.
		// Respective Ka values for Ac and Gl in adipose and mammary
		// are similar at 1.5-2.0 mM and 0.8-1.0 mM. Thus set at 1.8E-3 and
		// 1.0E-3 for both tissues.
		
		// Regulation of adipose lipogenic capacity (VmAcTsAdip)
		
		// constants K1 and K2 revised 7/92 (kc)
		
		dVmAcTs = K1VAct * AHor1 * INS - K2VAct * VmAcTs ;
		AcTsAdip = VmAcTs / ( 1.0 + KAcTsAdip / cAc + K1AcTs / ( AHor * cGl ) ) ;
		AcTmVis = VmAcTmVis * MamEnz * INS *pow(1,1)* P1 / ( 1.0 + KAcTmVis / cAc + K1AcTm / cGl ) ;
		// Added Gl based on residual errors observed for ExtLact and Aston simulations
		cAa = AA / VolAa ;
		GLLmVis = VmGlLm1Vis * MamEnz / ( 1.0 + KGlLmVis / cGl + KAaLmVis / cAa ) ; // cAa should affect the Vm not Ks
		GlHyAdip = AcTsAdip * fGlHyAdip * GlGlHy ;
		GlHyVis = AcTmVis * fGlHyVis * GlGlHy ;
		GlTpAdip = VmGlTpAdip * ( EBW *pow(1,1)* 0.7555 ) / ( 1.0 + KGlTpAdip / cGl ) ;
		GlTpVis = VmGlTpVis * ( EBW *pow(1,1)* 0.7555 ) / ( 1.0 + KGlTpVis / cGl ) ;
		GlLaOth = VmGlLaOth * ( EBW *pow(1,1)* 0.7555 ) / ( 1.0 + KGlLaOth / cGl ) ;
		basalOth = eerActivityAtp / 3 + ( KbasOth + KNaAtOth ) * WtOth *pow(1,1)* 0.7555 ; // GL Apr 2014: KBasalOth reduced from ~2.2 to XXX and eerActivityATP introduced
		basalAdip = ( KbasAdip + KNaAtAdip ) * WtAdip *pow(1,1)* 0.7555 ;
		PiAa = SPartPiAa / MwtPs ;
		// input to cow model.
		AbsAm = absRumAm ;
		
		WtPUter = WtUter * fPUter ; // kg protein in uterus
		WtUterSyn = WtUter * ( kUterSyn -2 * kUterSynDecay * DayGest ) * Preg ;
		WtUterDeg = ( WtUter - iWtUter ) * kUterDeg * NonPreg ;
		
		// *********************** Gravid Uterus *********************************
		// Yields approximately 2x use for Oxid as for prt syn per Bell 1995
		WtGrvUter = WtConc + WtUter ;
		kMamCellsQA = ( kMamCellsQAPrePeak + // Baseline value
			MamCellsQaKickStartFactor * ( kMamCellsQAStart - kMamCellsQAPrePeak ) + // Adds the kickstart boost which decays to nearly nothing after dew days
			MamCellsQaPreToPostFactor * ( kMamCellsQAPostPeak - kMamCellsQAPrePeak ) ) ; // Adds the trasnsition portion towards the final post peak value
		IncreasedUsDueToLowMf = -1 * MamCells * MaxLossDueToLowMf * derivt ( 0 , LowMfDecay ) ;
		dMilkTm = MamTm * KMilk * MilkSW ; // Mammary MILK FAT
		dUrea = KBldUrU * cBldUr ;
		
		// MUN calculations, CCP 3-28-07
		cPun = cBldUr * 2 * 14 * 1000 / 10 ; // Converts cBldUr from mol Urea/l to mg N/dl.
		dMilkPm = MamPm * KMilk * MilkSW ;
		GlTpF1 = GlTpAdip * GlTpTp ;
		GlTpV1 = GlTpVis * GlTpTp ;
		GlLaB1 = GlLaOth * GlLaLa ;
		AcTmV1 = AcTmVis * AcTgTg ;
		FaTmV1 = FaTmVis * FaTgTg ;
		
		// ********************** Milk Yield ********************************
		// Lactose in mammary gland(MamLm)and secreted (dMilkLm,TMilkLm) plus
		// TOTAL MILK YIELD
		
		dLm = GLLmVis * GlLmLm * MwtLm ; // Kg/d produced
		dMilkLm = MamLm * KMilk * MilkSW ;
		// WtGrvUter added Apr 5, 2007 MDH to refect gestation status.
		EBW1 = WtOth + WtAdip + WtVis + WtGrvUter + WaPool - WaPoolTarget ;
		NonFatEBW = EBW1 - WtAdip ;
		AdAtB1 = GlLaOth * GlLaAt ;
		
		// Heat production in body
		
		basHtOth = basalOth * AtAdHT ; // Basal
		AtAdF1 = GlHyAdip * GlHyAD ;
		AtAdF2 = GlTpAdip * GlTpAD ;
		// ADIPOSE
		AtAdF4 = AcTsAdip * ( TcHyAd * ( 1.0 - fGlHyAdip ) + AcFaAd ) ;
		
		// Heat Production in adipose
		basHtAdip = basalAdip * AtAdHT ; // Basal
		AtAdV2 = AcTmVis * ( TcHyAd * ( 1.0 - fGlHyVis ) + AcFaAd ) ;
		AtAdV5 = GLLmVis * GlLmAd ;
		AtAdV6 = GlHyVis * GlHyAD ;
		AtAdV7 = GlTpVis * GlTpAD ;
		TcHyAdip = AcTsAdip * HyAcFa * ( 1.0 - fGlHyAdip ) ; // correction for NADH
		TcHyVis = AcTmVis * HyAcFa * ( 1.0 - fGlHyVis ) ; // from ICD in TCA
		RumLaAc = RumLaFv * LaAcAc ;
		RumLaPr = RumLaFv * LaPrPr ;
		AaGlGest = WtGrvUter * kAaGlGest * cAa ; // I think I have accounted for heat and ATP correctly
		
		// **** Active, Quiescent and Senecenced cell dynamics  *********************************************
		// Pools are: A(Active); Q(Quiescent); S(Senecenced); U(Udder=A+Q); P (Proliferated = endless stand by pool).
		// U, P, S are conceptual, not explicitly implemented
		// Fluxes are: PA; AQ; QA; AS; QS; US = AS+QS
		
		fMamCellsQA = InMilk * MamCellsQ * kMamCellsQA * kMamCellsQaMfAdjustment ;
		// WtAdipNew = max ( 0. , integ ( dWtAdipNew , iWtAdip ) ) 
		// NonUterEbwTarget = integ ( GrowthPerDay , iNonUterEbwTarget ) 
		
		// **** LHOR *******************************************************************************************
		// Roughly the number days it would take LHOR to change to the full extent once the drivers state changed
		// Linear slope component in the equation
		// Linear slope component in the equation
		// Relative importance of blood amino acids
		// Relative importance of adipose size.
		// Relative importance of blood glocose.
		// Curvature component of the sensitivity
		// Curvature component of the sensitivity
		// Curvature component of the sensitivity
		// Baseline of cAA.  When cAA = cAaBase => nil change to LHOR due to amino acids level. 10-2015 GL changed from 0.06555 to 0.005
		// Baseline of CGL. When cGL = cGlBase => nil change to LHOR due to glucose level
		// CCP Scalar for PP effect on LHor degradation
		// Unit: 0 or 1. set to 1 to bypass the Lhor equation and use Lhor set to its baseline instead.
		
		cGlTarget = cGlBase * ( BcsTarget / BCSBase ) ; // Gl Has the same strong dip as BCS, so we want only deviation from the pattern t count topwards LHor syntheis.
		
		// Create a sigmoid / saturating  sensitivity to high levels of nutrients (AA GL)to avoid runaway to extremes.
		// Example: If AA are at -40%; -20%; 0%; 20%; 40% above base level (i.e.  cAa/cAaBase = 0.6; 0.8; 1; 1.2; 1.4).
		// Now lets say xLhorAaSens = 0.5 then  LhorAa (contribution of Aa to lactation hormone synthesis) will be:
		// 1 - ((1-0.6)^0.5); 1 - ((1-0.8)^0.5); 1; 1 + (1.2 - 1)^0.5; 1 + (1.4 - 1)^0.5
		// = -63%; -44%; 0%; +44%; +63% of base contribution to LhorSyn,
		if ( cAa < cAaBase ) {
			LhorAa = 1 - kLHorSensAa * ( 1 - max ( cAa , 0.000001 ) / cAaBase ) *pow(1,1)* xLHorSensAa ;
		} else {
			LhorAa = 1 + kLHorSensAa * ( ( cAa / cAaBase ) - 1 ) *pow(1,1)* xLHorSensAa ;
		}
		
		LhorAdip = ( WtAdipNew / WtAdipTarget ) *pow(1,1)* xLHorSensAdip ; // Contribution of Adippose to LhorSyn (not linear)
		
		// RETAINED MILK EFFECTS, Altered Apr 23, 2008 to smooth effects on milk production
		
		dKMilkInh = MilkInhSyn - MilkInhDeg ;
		// Adds cFa as feedback term and T3 as effector of VMAX
		cTs = TsAdip / WtAdip ;
		FaTsF1 = FaTsAdip * FaTgTg ;
		// BODY FAT OR STORAGE
		AcTsF1 = AcTsAdip * AcTgTg ;
		AaPOthOth = VmAaPOthOth * OthDna / ( 1.0 + KAaPOthOth / ( AHor * cAa ) ) ;
		AaPVisVis = VmAaPVisVis * VisDna / ( 1.0 + KAaPVisVis / ( AHor * cAa ) ) ;
		AaGlVis = VmAaGlVis * ( EBW *pow(1,1)* 0.7555 ) / ( 1.0 + KAaGlVis / cAa ) ; // GLUCONEOGENESIS
		
		// MILK PROTEIN SYNTHESIS
		AaPmVis = VmAaPmVis * MamEnz / ( 1.0 + KAaPmVis / cAa + KGlLmVis / cGl ) ;
		WtPUterSyn = WtUterSyn * fPUter ;
		WtPUterDeg = WtUterDeg * fPUter ;
		WtPGrvUter = WtPConc + WtPUter ;
		AmUr = AbsAm * AmUrUr ;
		cMun = 0.8555 * cPun -1.9555 ; // From Kauffman & St-Pierre, 2001, JDS, 84: 2284-2294 (mg/dl)
		DMilk = dLm / fLm ; // This now has significant within day variation due to udder fill. MDH. Changed from dMamLm to dLm, 23-1-2014
		NonUterEBW = EBW1 - WtGrvUter ;
		HtF2 = AtAdF2 * AtAdHT ; // Gl to Tp
		HtV3 = AtAdV7 * AtAdHT ; // Gl to Tp
		// AbsAa & AbsAm in moles
		NUr = dUrea * UrAmAm * MwtN ;
		
		// Milk yield and composition
		// MILK ENERGY
		ELm = dLm * 2.0 * HcombGl / MwtLm ;
		EUr = dUrea * HcombUr ; // METABOLIZABLE
		dWtUter = WtUterSyn - WtUterDeg ;
		fMamCellsUS = InMilk * MamCellsPart * ( ( kMamCellsDeclineBase * exp ( - MamCellsDecayRateOfSenescence * DayMilk ) + BaseMamCellsTurnOver ) ) + IncreasedUsDueToLowMf ; // Approximation of the senescence rate derived from Dijkstra's MamCells eq, PLUS dymaically calculated increase loss due to low milking frequency
		
		if ( cGl < cGlTarget ) {
			LhorGl = 1 - kLHorSensGl * ( 1 - ( cGl / cGlTarget ) ) *pow(1,1)* xLHorSensGl ;
		} else {
			LhorGl = 1 + kLHorSensGl * ( ( cGl / cGlTarget ) - 1 ) *pow(1,1)* xLHorSensGl ;
		}
		// Adds INS as effector
		// Ac = integ ( dAc , iAc ) 
		
		// Fat in Mammary gland (MamTm) and secreted (dMilkTm,TMilkTm)
		// Grams milk fat from de novo (WtAcTm) and pre-formed (WtFaTm)
		// calculations added 11/99. NES
		WtAcTm = AcTmV1 * MwtTm ;
		WtFaTm = FaTmV1 * MwtTm ;
		TpinAdip = GlHyAdip * GlHyTp + GlTpF1 ;
		TpinVis = GlHyVis * GlHyTp + GlTpV1 ;
		LainOth = GlLaB1 ;
		TpTmVis = ( FaTmV1 + AcTmV1 ) * TpTpTm ;
		dMamLm = dLm - dMilkLm ;
		AaUrVis = AaGlVis * AaGlUr ;
		AaUrGest = AaGlGest * AaGlUr ;
		
		// Protein in Mammary gland (MamPm) and secreted (dMilkPm,TMilkPm)
		
		dPm = AaPmVis * MwtAa ; // Kg/d produced
		AaGlV1 = AaGlVis * AaGlGl ;
		TpTsAdip = ( FaTsF1 + AcTsF1 ) * TpTpTs ;
		AtAdB1 = AaPOthOth * AaPxAD ;
		AtAdV3 = AaPVisVis * AaPxAD ;
		AtAdV4 = AaPmVis * AaPxAD ;
		
		// procedural ( HiM , HiP = AtAdV5 , AtAdHT , AcTsAdip , AcTmVis , GLLmVis , AaPmVis ) 
		if ( EPart == 0.0 ) goto label_81 ;
		// Calculation of energy costs of specific processes
		
		GlLmHt = AtAdV5 * AtAdHT ; // Lactose synthesis
		AaPmHt = AtAdV4 * AtAdHT ; // milk protein synthesis
		
		// costs of Ts synthesis from Ac in adipose
		
		AcTsH1 = AcTsAdip * HcombAc ; // Acetate energy input
		AcTsH2 = GlHyAdip * ( HcombGl - HcombTp ) ; // Gl energy loss in PC
		AcTsH3 = AtAdF1 * AtAdHT ; // activation of Gl for PC
		AcTsH4 = AcTsF1 * TpTgAD * AtAdHT ; // Tp reduction and Fa esterification
		AcTsH5 = AtAdF4 * AtAdHT ; // NADPH from TCA + AtAd in Fa synthesis
		AcTsH6 = AcTsF1 * HcombTp ; // Tp energy incorporated
		AcTsH7 = AcTsF1 * HcombTg ; // Energy in Ts
		AcTsHt = AcTsH1 + AcTsH2 + AcTsH3 + AcTsH4 + AcTsH5 + AcTsH6 - AcTsH7 ;
		
		// Cost of milk fat synthesis (HIP1)
		
		AcTmH1 = AcTmVis * HcombAc ; // Acetate energy input
		AcTmH2 = GlHyVis * ( HcombGl - HcombTp ) ; // Gl energy loss in PC
		AcTmH3 = AtAdV6 * AtAdHT ; // Activation of Gl for PC
		AcTmH4 = AtAdV2 * AtAdHT ; // NADPH from TCA and AtAd in FAS
		AFTmH5 = AtAdV1 * AtAdHT ; // Tp reduction and Fa esterification
		AFTmH6 = TpTmVis * HcombTp ; // Tp energy input
		FaTmH7 = FaTmVis * HcombFa ; // Fa energy input
		FaTmH8 = AcTsHt * rtFa1 ; // Cost of Fa synthesis in adipose
		rtFa1 = AcTsF1 / ( TsFaAdip - FaTsF1 + absFa ) ; // proportion of Fa entering central pool formed from Ac
		// and corrected for adipose TO
		AfTmH9 = AcTmV1 * HcombGy ; // Glycerol energy recovered
		HiP1 = AcTmH1 + AcTmH2 + AcTmH3 + AcTmH4 + AFTmH5 + AFTmH6 +
			FaTmH7 + FaTmH8 - AfTmH9 - dMilkTm * HcombTg / MwtTm ;
		HiM1 = AcTsHt - FaTmH8 ;
		
		// Partition of heat losses in glucose absorbtion and gluconeogenesis
		
		PrGlHt = PrGlVis * ( HcombPr + PrGlAd * AtAdHT ) - HcombGl * PrGlV1 ;
		AaGlHt = ( AaGlVis + AaGlGest ) * AaGlH ;
		absGlHt = AtAd10 * AtAdHT ; // Cost of absorbtion
		aPAGlH = PrGlHt + AaGlHt + absGlHt ; // Total Hi of Gl
		
		GLMilk1 = GlHyAdip * 0.5 * rtFa1 ; // Gl to PC in adipose
		GlMilk2 = GlHyVis * 0.5 ; // Gl to PC in mammary
		GlMilk3 = TpTmVis * 0.5 ; // Gl to Tm and Gy in viscera
		GlMilk = GLLmVis + GLMilk1 + GlMilk2 + GlMilk3 ;
		
		rtGl1 = GlMilk / GlTO ; // Proportion of Gl entry to milk
		HiP2 = aPAGlH * rtGl1 ; // Hi of Gl assigned to production
		HiM2 = aPAGlH - HiP2 ; // Hi of Gl assigned to maintenance
		
		// Partition of cost of amino acid absorbtion
		
		AaTO = AbsAa + PVisAaVis ; // Note that only visceral protein TO contributes to Aa TO
		// in this calculation
		absAaHt = AtAd11 * AtAdHT ;
		
		// cost of Aa absorbtion
		rtAa1 = AaPmVis / AaTO ;
		HiP3 = absAaHt * rtAa1 ;
		HiM3 = absAaHt - HiP3 ;
		
		// Costs of milk synthesis in animal(an)
		dHiP4 = GlLmHt + AaPmHt + HiP1 + HiP2 + HiP3 ;
		
		// partition of HFerm between HiM and HiP
		
		HFermM = HFerm * ( dMaint + dHiM4 ) / ( dMaint + dHiM4 + NEP + dHiP4 ) ;
		HiM = dHiM4 + HFermM ;
		HiP = dHiP4 + ( HFerm - HFermM ) ;
		
		goto label_81 ;
	label_81:
		// end of procedural 
		LHorSyn1 = kLHor * ( wLHorSensAa * LhorAa + wLHorSensGl * LhorGl + wLHorSensAdip * LhorAdip ) / VmLHorSyn ; // The expression in the brackets yields 1 when all drivers are on their baseline.
		// TsFaAdip=(VmTsFaAdip**CHor1/(1.0+(KTsFaAdip/cTs)**Theta1))
		// Replaced 12/87
		TsFaAdip = VmTsFaAdip * ( EBW *pow(1,1)* 0.7555 ) * CHor1 * T3 / ( 1.0 + ( cFa / K1TsFa ) *pow(1,1)* EXP10
			+ ( KTsFaAdip / cTs ) *pow(1,1)* Theta1 ) ;
		AaAcV1 = AaGlVis * AaGlAc ;
		dPOth = AaPOthOth - POthAaOth ;
		dPVis = AaPVisVis - PVisAaVis ;
		AaPUter = WtPUterSyn / MwtPVis ;
		PUterAa = WtPUterDeg / MwtPVis ;
		AtAdGestTO = WtPGrvUter / MwtPVis * fGrvUterTO * AaPxAD ; // Protein T/O estimate
		
		// procedural ( RequiredEnergy , FdCapMolly = DayMilk , NonUterEbwTarget , NonUterEBW ) 
		
		EnergyForActivity = 0 ; // (ActEnergyReq / MjoulesToATPConv / MEinMJ) / (0.02 * (17.0 * (MEinMJ/3.6/4.1555) - 2.0) + 0.5)
		EnergyForPregnancy = EnergyForPregnancyFactor * 11.5 * WtGrvUter ; //
		EnergyForGrowth = GrowthPerDay * 50.0 ; // Assuming 50MJ required for 1kg LW gain
		EnergyCompensation = ( NonUterEbwTarget / NonUterEBW ) *pow(1,1)* kEnergyCompensation ; // Any extra/deficit in condition would decrease/increase intake, trying to simulate the extra hunger of low condition cows, and lower insentive to eat for fat cows
		// ** 0.5 gives approx 1% more food for every 2% LW deficit in the non extreme range, and vice versa. ** 1 would give 1:1 relationship.
		if ( DayMilk <= 0 ) { // Dry Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForDryCowMaintenancePower * EnergyForDryCowMaintenanceFactor ;
			EnergyForMilk = 0 ;
		} else { // Milking Cow
			EnergyForMaintenance = NonUterEbwTarget *pow(1,1)* EnergyForMilkingCowMaintenancePower * EnergyForMilkingCowMaintenanceFactor ;
			if ( DayMilk <= PeakIntakeDay ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - ( 1.0 - DayMilk / PeakIntakeDay ) *pow(1,1)* EnergyForMilkPower ) ;
			} else if ( DayMilk <= ( PeakIntakeDay + SmoothingPeriodDays ) ) {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 ) * ( ( DayMilk - PeakIntakeDay ) / SmoothingPeriodDays ) *pow(1,1)* 2.0 ) ;
			} else {
				EnergyForMilk = MaxEnergyForMilk * ( 1.0 - IntakeDeclineSlope * ( SmoothingPeriodDays / 2.0 + ( DayMilk - ( PeakIntakeDay + SmoothingPeriodDays ) ) ) ) ;
			}
		}
		
		RequiredEnergy = ( EnergyForMaintenance // Molly's required intake, in MJ
			+ EnergyForMilk //
			+ EnergyForPregnancy //
			+ EnergyForGrowth //
			+ EnergyForActivity ) // Currently this one zero, avarage activity assumed and bundled in rEnergeyForMaintenance
			* EnergyCompensation ; // Smaller than one for fat cows, largerthan 1 for skinny, on the long run brings cows that differ only in condition, to a similar state.
		//
		FdCapMolly = NonUterEbwTarget * CappingForIntake ; // in kgDM actual, WFM will not feed her more forage (pasture & silage) then this amount, but may feed things like grains on top of that
		//
		// end of procedural 
		dWtPUter = WtPUterSyn - WtPUterDeg ;
		// IncreasedUsDueToLowMfPublished = MamCells * MaxLossDueToLowMf * kMamCellsUsMfDecay * EXP(-kMamCellsUsMfDecay * CumulativeLowMfDays) ! This one is published because it is simple enough, and is identical to the IncreasedUsDueToLowMf BUT ONLY for a single period of non-2x-frequency. The above IncreasedUsDueToLowMf handles multiple periods of non-2x milking frequency elegantly but harder to describe in publication.
		
		// ***** AdiposeNew = estimation for LHOR sensitivity only ***********************************************
		// Back Calculated Adipose for LHOR sensitivity. WtAdip not reliable
		// calculates WtAdipNew as following NonUterEBW changes, excl preg, gut,and growth of
		// young cows. For average condition cows most of net LW loss will come from fat.
		// The lower the adipose is the less fat and more muscle will be lost / gained.
		// The higher the adipose is the more fat and less muscle will be lost / gained.
		
		dNonUterEBW = derivt ( iNonUterEBW , NonUterEBW ) ;
		BldUrMUN = DMilk * ( cMun * 10 / 14 / 1000 / 2 ) ; // (moles urea/day transferred from Bld to milk)
		dWtGrvUter = WtConcSyn + dWtUter ;
		fMamCellsAS = fMamCellsUS * MamCellsA / ( MamCellsQ + MamCellsA ) ; // Partition the total death rate between the Q and A pools
		fMamCellsQS = fMamCellsUS * MamCellsQ / ( MamCellsQ + MamCellsA ) ; // Partition the total death rate between the Q and A pools
		dTm = WtAcTm + WtFaTm ; // Kg/d Produced
		LaCdOth = LainOth * fLaCdOth ;
		LaGlOth = LainOth - LaCdOth ;
		AtAdV1 = TpTmVis * TpTgAD ;
		LHorSyn = InMilk * LHorSyn1 + ( 1 - InMilk ) * LHorDeg ; // Maintain base level while dry, the real game starts when she calves, otherwise we may have too much sesnitivity to the dry period situation of the cow
		TpCdVis = TpinVis - TpTmVis ;
		AtAdOth = basalOth + AtAdB1 ;
		AaPOthHt = AtAdB1 * AtAdHT ; // Protein TO
		AtAdF3 = TpTsAdip * TpTgAD ;
		ATAd15 = ( AaUrVis + AaUrGest + AmUr ) * AtAmUr ;
		HtV2 = AtAdV3 * AtAdHT ; // Protein TO
		EPm = dPm * HcombAa / MwtAa ; // Changed from dMilkPm (milkout) to dPm, dTm, and dLm
		WaMilk = DMilk - dLm - dPm - dTm - DMilk * kMilkAsh ;
		dMamPm = dPm - dMilkPm ;
		// Gut GLucose metabolism to Lactate
		GyGlVis = ( TsFaAdip + FaTmV1 ) * TgGyGy ;
		TpLaAdip = TpinAdip - TpTsAdip ;
		// PLASMA LIPIDS-Fa
		TsFaF1 = TsFaAdip * TgFaFa ;
		dWtOth = dPOth * MwtPOth / fDWt ;
		dWtVis = dPVis * MwtPVis / fDWt ;
		// ENERGY BALANCE
		// Body energy balance
		// Derived from (AaPGest*HcombAa)/EGrvUterCLF
		dOthE = dPOth * HcombAa ;
		dVisE = dPVis * HcombAa ;
		// KMinh = integ ( dKMilkInh , ikMilkInh ) 
		
		// Gil May 2012 MamEnz code moved to MamCells_MDH_in_deriv.csl / MamCells_DairyNZ_in_deriv.csl becsue to retain difference
		
		// ************************** LIPID METABOLISM(Fa,Ts) **********************
		// Inputs to storage triacylglyceride pool (Ts) are fatty acid
		// esterification(FaTsAdip*FaTsTs=6*0.3555=2.0)and lipogenesis from acetate
		// (AcTs*AcTsTs=16*.042=0.6555). Output is lipolysis (TsFaAdip = 2.6555).
		// Inputs to fatty acid plus triacylglyceride pool(Fa) are
		// absorption (absFa = 1.0) from gut and lipolysis(TsFaAdip*TsFaFa
		// =2.6555*3 = 8.0). Outputs are fatty acid (re)esterification
		// (FaTsAdip=6.0),incorporation into milk fat(FaTmVis=1.8) and oxidation
		// FaCd = 1.2). Rationale in setting VmTsFaAdip, KTsFaAdip and theta1 was
		// that, in later versions VmTsFaAdip may be a function of catabolic
		// hormone rather than having Estate as a general effector as is current.
		// Also, Ts should not become rate limiting until cTs is less than 0.2.
		// With KTsFaAdip set at .5 and theta1 at 5,cTs is not limiting until .2
		// and then becomes very limiting. cFa is 0.5E-3 and cGl is 3.0E-3.
		// KFaTsAdip was set at 1.6555E-4 to make cFa close to saturating and K1FaTs
		// at 2.0E-3 to make the reaction responsive to Gl changes. Aggregation
		// confounds KFaTmVis and VmFaTmVis so these were set to produce 1/2 Vmax
		// in reference state.
		
		// revised constants VmFaTmVis and VmTsFaAdip 7/92 (kc)
		dTsAdip = FaTsF1 + AcTsF1 - TsFaAdip ;
		AaPGest = AaPConc + AaPUter - PUterAa ;
		
		// Heat production in the Gravid Uterus
		// All oxidation in GrvUter driven by protein T/O
		// It would be better to calculate the exact need as Ferrell did, but his equation
		// is for beef cows not dairy cows.  The shortcut is to drive energy use by protein T/O
		// in the GrvUter which likely accounts for a lot of the energy use.  But oxidation of AA
		// is also considered separately so total gestation energy use must consider that.
		// A value of 0.6 (60% T/O per day) for fGrvUterTO yields about the correct total energy
		// usage per Ferrell. Need to fit this to observed data.
		
		AtAdGestGrth = ( AaPUter + AaPConc ) * AaPxAD ; // Protein deposition cost
		AtAdGest = AtAdGestGrth + AtAdGestTO ;
		
		// END  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		if ( FeedInFlag == 1.0 ) { //
			FdRat = FdRatWFM ; // Allocated feed - m files or WFM
		} else { //
			FdRat = RequiredEnergy / 11.5 ; // Fully fed automatically (standalone)
		}
		dLwExclUterGutAndGrowth = dNonUterEBW - GrowthPerDay ;
		NMilk = dMilkPm * 0.1555 + BldUrMUN ;
		dMamCellsS = fMamCellsAS + fMamCellsQS ;
		AdAtB2 = LaCdOth * LaCdAt ;
		MHtOth = basHtOth + AaPOthHt ;
		ETm = dTm * HcombTg / MwtTm ; // MDH, 5-6-14
		
		dMamCellsA = InMilk * ( fMamCellsQA - fMamCellsAQ + fMamCellsPA - fMamCellsAS ) - 100 * ( 1 - InMilk ) * ( MamCellsA - 1e-12 ) ; // Second part empties the A pool withing few hours after dry off
		dMamCellsQ = InMilk * ( fMamCellsAQ - fMamCellsQA - fMamCellsQS ) + 100 * ( 1 - InMilk ) * ( MamCells - MamCellsQ ) ; // Second part brings Q pool to have all MamCells when dry
		dMamTm = dTm - dMilkTm ;
		AdAtV2 = TpCdVis * TpCdAt ;
		AtHt2 = TpCdVis * HcombTp ;
		// LHor1 = integ ( dLHor , iLHor ) 
		dLHor = LHorSyn - LHorDeg ;
		HtF3 = AtAdF3 * AtAdHT - AcTsH4 ; // Ts TO
		HiV8 = ATAd15 * AtAdHT ; // urea synthesis
		NEP = ELm + EPm + ETm ; // this is NEl, NEp would include growth and pregnancy
		GyGlV1 = GyGlVis * GyGlGl ;
		LaCdAdip = TpLaAdip * fLaCdAdip ;
		LaGlAdip = TpLaAdip - LaCdAdip ;
		AdAtF1 = TpLaAdip * TpLaAt ;
		AdAtV1 = GyGlVis * GyGlAt ;
		
		// Oxidative metabolism in adipose (Adip)
		
		// Energy expenditures (AtAdAdip=169.2) are basalAdip(41.0), GlHyAdip (2.8),
		// GlTpAdip (4.0),TpTsAdip*TpTsAD (2.6555*9(6 ATP to form acyl CoA plus
		// 3 for NADH2 to reduce Tp)=24, and AcTsAdip*(tcHyAD*(1.0 - fGlHyAdip)
		// =16*(5.2555*(1 - 0.6)+2.8555)=16*(2.1+2.8555)=79.6;tcHyAD is cost
		// of NADPH2 from TCA cycle expressed in ATP per acetate(1.7555*3=5.2555)
		// and ATP cFa is cost in ATP / Ac converted to Fa((8Ac-8AcCoA(=16AtAd)
		// +7AcCoA-7Mal-CoA(=7AtAd))/8=2.8555).Oxidation of Tp yields 18.6555
		// ATP and 6.4 ATP are generated in TpLaAdip.
		// Reduction in oxygen uptake due to NADPH2 generation
		// in Tc is HyAcFa*(1.0-fGlHyAdip)=1.7555*.4=0.7 in this version.
		
		AtAdAdip = basalAdip + AtAdF1 + AtAdF2 + AtAdF3 + AtAdF4 ;
		dWtTsAdip = dTsAdip * MwtTs ;
		dAdipE = dTsAdip * HcombTg ;
		MHtGest = AtAdGest * AtAdHT ; // Maintence Heat of Gestation
		dGestE = AaPGest / fGestEPrt * HcombAa ; // Matched Ferrell's estimate which will be slightly
		
		// FEEDING MANAGEMENT
		
		// FdRat, FdDMin, and DailyDMin are all Kg dry matter
		// consumed per day (FdRat is specified input from animal
		// database(ie Event 2)).
		// FDINT (feeding interval) and FDTM (time spent eating in
		// each feeding interval0 are used in rumination, salivation
		// and water dynamics functions so are left in at this time
		// but may not function properly.  NES 8/99
		
		// Next block (up to RUMINATION) was merged From Molly86 for Mindy. Gil July 2012, including deletion of the Feeding Starategies section!
		// Daily Feed and Nutrient Intakes
		FdDMIn = FdRat ;
		dWtAdipNew = dLwExclUterGutAndGrowth * ( 1 - exp ( -8 * WtAdipNew / ( WtAdipNew + NonUterEBW ) ) ) ; // The smaller the adipose the smaller the propertion of fat is from any weight gain / loss
		// BODY(lean body mass--Oth)
		AdAtOth = AdAtB1 + AdAtB2 ;
		AdAtF2 = LaCdAdip * LaCdAt ;
		MHtAdip = basHtAdip + HtF2 + HtF3 ;
		
		// Calculation of heat equivalent of ATP .Note that HcombLa
		// and HcombTp being very close to 1/2 glucose are set exactly to
		// that so energy changes in glycolysis are not represented.
		
		AtHt1 = ( LaCdOth + LaCdAdip ) * HcombLa ;
		AtHt3 = GyGlV1 * ( GlGyGY * HcombGy - HcombGl ) ; // gluconeogenesis from Gy
		AdAtAdip = AdAtF1 + AdAtF2 ;
		EB = dOthE + dAdipE + dVisE + dGestE ;
		DailyDMin = FdDMIn ;
		// TotDMin = integ ( DailyDMin , 1.0E-9 ) 
		// LpinFd=fLp*FdDMin
		OminFd = fOmFd * FdDMIn ;
		NdfinFd = fNDFFd * FdDMIn ;
		
		// procedural ( Rumntn , Eating , Rest = RUMNTNEQ , CWCF , DAY , TIME , 
			// AMP1FT , MEAN1 , RumntnF , FdDMIn ) 
		if ( RUMNTNEQ == 0.0 ) {
			Eating = RumntnF ;
			Rumntn = RumntnF ;
			Rest = 1 - Rumntn ; // - Eating
		} else {
			// Determine whether the animal is eating, ruminating, or resting with Mindy Intake model, 4-25-2011, MDH
			Rest = 0.0 ;
			Rumntn = 0.0 ;
			Eating = 0.0 ;
			if ( FdDMIn > 0.1 ) {
				Eating = 1.0 ;
			} else if ( LPart > MinLPRumntn ) { // Ruminate until LP becomes less than or equal to the minimum LP size to stimulate rumination.
				Rumntn = 1.0 ;
			} else {
				Rest = 1.0 ;
			}
		}
		// end of procedural 
		LPartSwal = FdDMIn * fLPartNutIng ; // LARGE PARTICLES
		MPartSwal = FdDMIn * fMPartNutIng ;
		SPartSwal = FdDMIn * fSPartNutIng ;
		StinFd = fStFd * FdDMIn ;
		StCsFd = FStsFd * FdDMIn ;
		RumHcin = fHcFd * FdDMIn ;
		RumCein = fCeFd * FdDMIn ;
		PiPiFd = fPiFd * FdDMIn * ( 1 - fLPartSwal ) ;
		TPRTin = ( FPsFd + fPiFd + fNnFd ) * FdDMIn ;
		IndigFdFd = FdDMIn * fIndigFd * ( 1 - fLPartSwal ) ;
		ScTCs = fScTFd * FdDMIn / MwtSc ;
		PsAaFd = FPsFd * FdDMIn / MwtPs ;
		UrAmFd = FUrFd * FdDMIn * UrAmAm / MwtUr ;
		NnAmFd = fNnFd * FdDMIn * NnAmAM / MwtNn ; // RUMEN AMMONNIA-Am
		AsAsFd = fAsFd * FdDMIn ;
		FlFd = fLiFd * FdDMIn / ( MwtLiFd ) * LiFlFd ;
		Fl1Fd = fFatFd * FdDMIn * FaFlFd / MwtFaFd ;
		FvAcFd = fAcFd * FdDMIn / MwtAc ;
		FvBuFd = fBuFd * FdDMIn / MwtBu ;
		// assumes no Hc,Ce,Aa, GOTO La
		FvLaFd = FdDMIn * FLaFd / MwtLa ;
		ChChFd = fLiFd * FdDMIn / MwtLiFd * LiChFd * MwtCh ;
		TStin = FdDMIn * fStFd ;
		
		// Computation of digestion coefficients for energy and energy terms.
		FdGEin = GEFd * FdDMIn + InfPrt * 5.7 ;
		Nintake = FdDMIn * ( FPsFd + fPiFd + FUrFd + fNnFd ) * 1000 * 0.1555 ;
		
		// When NDFFd is given as an input, methane (MCH4) is also
		// calulated using the equation described by Moe and
		// Tyrrell, 1979
		// Corrected so NDFFd not necessary as input jk 06/19/91
		
		// changed MCH4 to MCH4E - 2/2/95 kc!
		// Units for MCH4E is Mcal - 2/2/95 kc!
		
		MCH4E = ( 3.4555 + 0.5555 * ( FdDMIn * ( fStFd + fScFd + fOaFd + fPeFd ) )
			+ 1.7555 * ( FdDMIn * fHcFd ) + 2.6555 * ( FdDMIn * fCeFd ) ) * F1 / 4.1555 ;
		// (2002; JDS:85, 1176-1182) and Cassida and Stokes (1986; JDS:69,1282-1292)
		RestWa = 1.4 * FdDMIn * 0.7555 ;
		// 85 L/DAY with Rumntn=0.3555 at 500 kg EBW
		// Only active during rumination
		
		EatSa = 3.2 * FdDMIn ; // L/Kg FdDMin. Redefined by Mindy to be: EatSa=2.6555*(EBW**0.7555)*Eating
		EatWa = 3.3555 * FdDMIn * 0.7555 ;
		EUrFd = EUr / FdDMIn ;
		LPartRed = KLPartRed * LPart / LPartCor * Rumntn ;
		MPartSPart = MPart * KMPartSPart * Rumntn ;
		StHaFd = ( StinFd - StCsFd ) * ( 1 - fLPartSwal ) ; // error as fLPart was applied to StinFd previously, MDH 5-27-13
		Cein = RumCein * ( 1 - fLPartSwal ) ;
		StCs = StCsFd / MwtSt ; // SOLUBLE CARBOHYDRATES
		KNaAtVis = 3.2 + KNaVis * T3 * OminFd ;
		UrNFd = UrAmFd * MwtN ;
		dHyFlF = FlFd * 2.0 ;
		// END  INCLUDE '..\Molly_ProximateExpand_In_Deriv.csl'  ! Take from the shared parent folder as all current projects use ths one
		// BEGIN  INCLUDE 'Intermittent_Eating_deriv.csl'
		
		// No intermittent eating for basic Molly
		
		// END  INCLUDE 'Intermittent_Eating_deriv.csl'
		// BEGIN  INCLUDE 'Mindy_Dynamic.csl'                      ! This and the next statement must come be in this order and after the above input include statements.
		// *************************************************
		// COMPUTATION OF WATER DYNAMICS: different in MIndy
		// *************************************************
		// Salivation, drinking, water flow through rumen wall, rumen soluble,
		// particulate and total rumen dry matter (RumDM) and rumen volume.
		// Rumen volume can be calculated based upon RumDM/0.1555 which is the
		// default or based upon water dynamics and osmolality when the rumen
		// liquid volume equation(RumLiqVolEQ)is set to 1.0. The empirical
		// equation for OSWa is not generally applicable and should not be used
		// for continuous feeding and unusual diets e.g. high salt, NaHCO3, and
		// thus should be closely monitored when RumLiqVolEQ is set to 1.0.
		
		RestSa = 0.8555 * ( EBW *pow(1,1)* 0.7555 ) * Rest ; // At 8 h resting, this equates to 70 ml/min which is below the 114 ml/min according to Maekawa et al.
		// RestSa=6.0*(FdDMin/FdInt)*Rest*RestCor
		// 60 L/DAY with Rumntn=0.3555
		// Only runs when not feeding, corrected down for ruminating
		RumntnSa = 2.4555 * ( EBW *pow(1,1)* 0.7555 ) * Rumntn ;
		
		// PASSAGE RATE CONSTANTS
		// **********************
		// KSPartP is now a fraction of KWAP, MDH 5-25-13
		// KSPartP=2.6555*(FdDMin/ebw**0.7555)+1.00
		// KWaP=1/((0.02555*(ebw**0.7555)/FdDMin)+0.1555)
		// KSPartP & KWaP are in TURNOVERS PER DAY
		// Equations derived by regression from Robinson 1985 Can.JAS 65:347
		// and Evans 1981 Can.JAS 61:97
		
		// Inputs to the particle pools was altered to utilize seiving data, default seive is the Penn State Particle
		// 	separator, but any sieve size can be used.  The feed seiving data is modified using a set of equations
		// 	to reflect the effects of mastication during ingestion, and thus particle entry into the rumen reflects,
		// 	the original particle size reduced by mastication.  A normal Penn State Shaker distribution is used as
		// 	as a default.
		// Initial rumen particle pool sizes were the mean of the seiving observations of Shaver et al., 1988. MDH
		
		// LARGE PARTICLE POOL(LPart in Kg); Retained on a 4.8 mm screen
		// This is a real pool that is intended to represent the floating raft.  Starch was added to the pool, 5-21,2013
		// 	to reflect entry of larger starch particles.  Assuming starch particle size reflects total diet
		// 	particle size is likely an overestimate of starch entry into the LPart pool, but failing to include
		// 	it as in the original model is clearly an underestimate. MDH
		
		// This pool was reduced in size and more dietary material is diverted through it
		// necessitating a large increase in the rate constant, MDH
		// Should be function of physical properties of feed!
		// Should KLPartRed be a function of fermentation rate?
		// Should entry be lagged for hydration??
		
		dLPart = LPartSwal - LPartRed ;
		Hcin = RumHcin * ( 1 - fLPartSwal ) ;
		SolOmP = CsP * MwtCs + FaP * MwtFl + RumAaP * MwtRumAa + ChChFd ; // Added ChChFd to be consistent with DMP, MDH. Mar 31, 2014
		SaIn = EatSa + RestSa + RumntnSa ;
		// RestWa=1.4555*(FdDMin/FdInt)*0.7555*RumntnCor*NOFEED
		DrnkWa = EatWa + RestWa ;
		LPartMPart = LPartRed * pLPartMPartComm ;
		LPartSPart = LPartRed * ( 1 - pLPartMPartComm ) ;
		LPartStHa = LPartRed * fLPartSt ;
		LPartHcHc = LPartRed * fLPartHc ; // HOLOCELLULOSE-Hb
		LPartCeCe = LPartRed * fLPartCe ; // HOLOCELLULOSE-Hb
		LPartPiPi = LPartRed * fLPartPi ;
		LPartIndigFdIndigFd = LPartRed * fLPartIndigFd ; // INDIGESTIBLE FEED
		basalVis = ( KbasVis + KNaAtVis ) * WtVis *pow(1,1)* 0.7555 ;
		Hbin = Cein + Hcin ;
		SaNnAm = cBldUr * SaIn * UrAmAm ;
		
		// procedural ( RumLiqVol , fRumDM = RumLiqVolEQ , SaIn , DrnkWa , LPart , MPart , SPart ) 
		RumOsMol = ( Cs / CsCor + Fl / FLCor + AM / AmCor + RumAc / RumAcCor +
			RumPr / RumPrCor + RumBU / RumBuCor + RumLa / RumLaCor
			+ RumAa / RumAaCor + As / MwtAs / ASCor * OsMolF ) / RumLiqVol ;
		// Rumen Fluid OsMolality. As (soluble ash) multiflied by a factor
		// to give moles of ions.  MwtAs=0.08555 was picked from NaHCO3
		// Not clear why OsMolF is applied to AA.  This would appear to be a mistake and thus removed. MDH 2-19-14
		// As plays a very minor role in driving Osmol.  VFA are the drivers.
		// OsWa=0.7555*((RumOsMol-0.2555)*1000)-41.0   	  !Dobson , the negative intercept in this emprical equation causes mOsmol to run at 300, MDH 2-19-14
		// Set the slope a little higher to keep osmolality down, MDH 2-19-14
		OsWa = OsWaSlp * ( ( RumOsMol -0.2555 ) * 1000 ) + OsWaInt ; // Set the intercept to 0 to achieve a center around 280 mOsmol
		fRumDM = 0.1555 ;
		// 14.7% DM of rumen contents based on lactating cow data from
		// VanVuuren 1999,JDS 82:143,Johnson 1991,JDS 74:933,Woodford 1988
		// JDS 71:674,Shaver 1985,18th Rum Func Conf p45, DCM90(Purina),
		// and Hartnell 1979,JAS 48:381
		RumVol = RumDM / fRumDM ;
		RumLiqVol = RumVol - RumDM ;
		WaOut = RumLiqVol * KWAP ;
		dRumLiqVol = WaIn - WaOut + OsWa ;
		// end of procedural // OF PROCEDURAL
		SaPsAa = cSaPs * SaIn ;
		SaAs = fSaAs * SaIn ;
		
		Nin = ( FPsFd + fPiFd + fNnFd ) * FdDMIn * 0.1555 + UrNFd ;
		LPartHbHb = LPartCeCe + LPartHcHc ;
		// Heat production in viscera
		basHtVis = basalVis * AtAdHT ; // basal
		SaNRumAm = SaNnAm * AmUrUr ;
		cVFA = TVFA / RumLiqVol ; // cVFA in Moles/liter
		
		HaMiP = ( HaMi / MiHaCor ) / RumLiqVol * WaOut * fPartP ;
		HbMiP = ( HbMi / MIHbCor ) / RumLiqVol * WaOut * fPartP ;
		HaP = Ha / HaCor / RumLiqVol * WaOut * fPartP ;
		HcP = Hc / HcCor / RumLiqVol * WaOut * fPartP ;
		CeP = Ce / CeCor / RumLiqVol * WaOut * fPartP ;
		PiP = Pi / PICor / RumLiqVol * WaOut * fPartP ;
		IndigFdP = IndigFd / IndigFdCor / RumLiqVol * WaOut * fPartP ;
		cCs = ( Cs / CsCor ) / RumLiqVol ;
		cRumAa = ( RumAa / RumAaCor ) / RumLiqVol ;
		cAm = ( AM / AmCor ) / RumLiqVol ;
		cAs = ( As / ASCor ) / RumLiqVol ;
		// LACTATE
		cRumLa = ( RumLa / RumLaCor ) / RumLiqVol ;
		// Pi = integ ( dPi , iPi ) 
		
		// LIGNIN AND INSOLUBLE ASH (IndigFd in Kg)
		dIndigFd = IndigFdFd + LPartIndigFdIndigFd - IndigFdP ;
		// Hb = integ ( dHb , iHb ) 
		
		// INSOLUBLE PROTEIN (Pi in Kg) METABOLISM
		// Sept. 20, 2004 solution against Bate5o2 data.
		// Effect of added dietary fat on protein degradability added 12/90
		// again this is an effect which is poorly supported.
		dPi = PiPiFd + LPartPiPi - SPartPiAa - PiP ;
		HbP = HcP + CeP ;
		LgP = IndigFdP * fLgIndigFd ; // To Fit Duodenal Data Kg/d
		AiP = IndigFdP * fAiIndigFd ;
		FGAm = 1.0 / ( 1.0 + KFGAm / cAm ) ;
		YAtp = 0.01555 + RumYAtp / ( 1.0 + KYAtAa / cRumAa ) ;
		G2 = 0.5 / ( 1.0 + KYAtAa / cRumAa ) ;
		
		// ****************************************************************
		// 	Passage Rates from the Rumen
		ADFP = CeP + IndigFdP ;
		// Total Duodenal Dry Matter Flow, Kg/d
		
		LgutHaGl = HaP * LgutDCHa / MwtSt ;
		LgutHcFv = HcP * LgutDCHb / MwtHc * 0.8555 ;
		LgutCeFv = CeP * LgutDCHb / MwtCe ;
		LgutPiAa = PiP * LgutDCPi / MwtPi ;
		LgutAi = LgutDCAi * IndigFdP * fAiFd / fIndigFd ;
		
		// FECES (Fec)
		
		FecHa = HaP * ( 1.0 - LgutDCHa ) ;
		FecHC = HcP * ( 1.0 - LgutDCHb ) ;
		FecCe = CeP * ( 1.0 - LgutDCHb ) ;
		FecADF = FecCe + IndigFdP ;
		FecLg = IndigFdP * fLgFd / fIndigFd ;
		FecPi = PiP * ( 1.0 - LgutDCPi ) ;
		FecAsh = AsP * ( 1.0 - LgutDCAs ) +
			IndigFdP * fAiFd / fIndigFd * ( 1.0 - LgutDCAi ) ;
		
		// RUMEN pH CRumpH
		// Rumen pH influences stoichiometry of fermentation (above) and
		// hydrolytic rate constants for cellulose and hemicellulose.
		// This is most relevant when meal rather than continuous feeding
		// is implemented.  However, in the default it is left on
		// (RumpHCON=1.0,FIXDPH=0.0).  (RumpHCON) RumpH control allows the
		// RumpH equation to be shut off so a fixed pH (FIXDpH) can be used
		
		// pH predicted from Briggs et al., 1957 as used in Argyle and Baldwin, 1988
		// RumpH=(7.2555-(10.0*cVFA+1.5*cRumLa))
		RumpH = ( RumpHBase - ( vfaeff * cVFA +1.5 * cRumLa ) ) * RumpHCON + FIXDpH ;
		MPartP = ( HaP + HcP + CeP + PiP + IndigFdP ) / fPartP * ( fMPart1 * KMPartP ) ;
		SPartP = ( HaP + HcP + CeP + PiP + IndigFdP ) / fPartP * ( fSPart1 * KSPartP ) ;
		IndigFdMiP = IndigFdP * cMiSPart ; // Passage of microbes in
		PiMiP = PiP * cMiSPart ; // association with SPart
		CsFv = VmCsFv * WaMi / ( 1.0 + KCsFv / cCs ) ;
		RumAaFv = VmRumAaFv * WaMi / ( 1.0 + KRumAaFv / cRumAa ) ;
		absRumAs = KAsabs * cAs ;
		TOmP = SolOmP + HaP + HbP + PiP + LgP ;
		BldUrAm = ( VmBldUrAm * ( EBW *pow(1,1)* 0.7555 ) / ( 1.0 + KBldUrAm / cBldUr + cAm / KiAm ) ) * UrAmAm ;
		// HaMi = integ ( dHaMi , iMiHa ) 
		// HbMi = integ ( dHbMi , iMiHb ) 
		
		// STARCH (St in Kg) OR ALPHA-HEXOSE (Ha in Kg) METABOLISM
		dHa = StHaFd + LPartStHa - HaP - SPartHaCs ;
		RumAaAc = AaFvAc * ( RumAaFv + ( 0.7555 * NnAmFd ) ) ;
		RumAaPr = AaFvPr * ( RumAaFv + ( 0.7555 * NnAmFd ) ) ;
		RumAaBu = AaFvBu * ( RumAaFv + ( 0.7555 * NnAmFd ) ) ;
		AtpF = CsFv * CsFvAt + RumAaFv * AaFvAt +0.7555 * NnAmFd * AaFvAt + RumLaFv * LaFvAt ;
		AshP = AsP + AiP ; // Total Duodenal Ash, Kg/d
		// Corrects kg hemicellulose
		LgutHcAc = LgutHcFv * HcAcAc ;
		// to moles hexose equivalents
		LgutHcPr = LgutHcFv * HcPrPr ;
		LgutHcBu = LgutHcFv * HcBuBu ;
		LgutCeAc = LgutCeFv * CeAcAc ;
		LgutCePr = LgutCeFv * CePrPr ;
		LgutCeBu = LgutCeFv * CeBuBu ;
		FecHb = HbP * ( 1.0 - LgutDCHb ) ;
		G1 = 1.0 - G2 ;
		SPartMiP = cMiSPart * ( SPartP + MPartP ) ;
		// Ha = integ ( dHa , iHa ) 
		
		// HOLOCELLULOSE(Hc+Ce in Kg) OR BETA-HEXOSES(Hc and Ce in Kg)
		// METABOLISM
		
		// The Hb equation should probably be sigmoid from pH 7.0 on down
		// to pH 5.5 with the steapest decrease below 6.2 to 5.5, but there
		// are not enough data to create that form.
		
		// procedural ( KHcCs , KCeCs = RumpH , KHcCs1 , KCeCs1 ) 
		KHcCs = KHcCs1 ;
		KCeCs = KCeCs1 ;
		if ( RumpH >= 6.2 ) goto label_22 ;
		KHcCs = KHcCs - ( KHcCs * 1.8555 * ( 6.2 - RumpH ) ) ;
		KHcCs = max ( KHcCs , 0.0 ) ;
		KCeCs = KCeCs - ( KCeCs * 1.8555 * ( 6.2 - RumpH ) ) ;
		KCeCs = max ( KCeCs , 0.0 ) ;
	label_22:
		// end of procedural // OF PROCEDURAL
		AaAm = RumAaFv * AaFvAm ;
		
		// Effect of added dietary fat on microbial yield (MiG) 12/14/90 jk
		
		// procedural ( MiMaAd = RumpH ) 
		// Effect of pH on microbe maintenence requirement.
		MiMaAd = 20 ;
		// MOLES/KG/DAY
		if ( RumpH >= 6.2 ) goto label_26 ;
		if ( RumpH <= 5.4 ) goto label_25 ;
		MiMaAd = MiMaAd + ( MiMaAd * ( ( 0.8 - ( RumpH -5.4 ) ) / 0.8 ) ) ;
		goto label_26 ;
	label_25: MiMaAd = 40 ;
	label_26:
		// end of procedural 
		
		// Need to calculate MPart and SPart in feces from ruminal outflow.  Subtract non NDF digested
		// nutrients from the ruminal particle outlfow values to get feces, MDH Feb 11, 2014
		FecMPart = MPartP ; // these do not account for intestinal digestion of MPart and SPart. Need to fix per above.
		FecSPart = SPartP ;
		dRumAaHy = RumAaFv * AaFvHy ;
		// OF PROCEDURAL
		
		// ADJUSTMENT OF STOICHIOMETRIC COEFFICIENTS FOR RumpH
		// Stiochiometric coefficients have pH effects embedded in them.
		// These equations should probably be sigmoidal.
		// Switch to pure lactate at pH 5.4 may not be correct, it is
		// probably a logarithmic increase starting at 5.5.  These changes
		// in stoichiometric coefficients parallel those suggested by
		// the equation which corrects for systematic errors in CONSET
		// due to increasing proportions of concentrate in the ration
		// which was formulated by Murphy et al (1982).
		
		// procedural ( ScAc , ScPr , ScBu , ScLa , StAc , StPr , StBu , StLa = RumpH , 
			// ScAcAc , ScPrPr , ScBuBu , ScLaLa , StAcAc , StPrPr , StBuBu , StLaLa ) 
		ScAc = ScAcAc ;
		ScPr = ScPrPr ;
		ScBu = ScBuBu ;
		ScLa = ScLaLa ;
		StAc = StAcAc ;
		StPr = StPrPr ;
		StBu = StBuBu ;
		StLa = StLaLa ;
		if ( RumpH >= 6.2 ) goto label_24 ;
		if ( RumpH <= 5.4 ) goto label_23 ;
		ScAc = 0.7555 + ( ( RumpH -5.4 ) / 0.8 ) * ( ScAcAc -0.7555 ) ;
		ScPr = 0.5555 + ( ( RumpH -5.4 ) / 0.8 ) * ( ScPrPr -0.5555 ) ;
		ScBu = 0.4555 + ( ( RumpH -5.4 ) / 0.8 ) * ( ScBuBu -0.4555 ) ;
		StAc = 0.6555 + ( ( RumpH -5.4 ) / 0.8 ) * ( StAcAc -0.6555 ) ;
		StPr = 0.8555 + ( ( RumpH -5.4 ) / 0.8 ) * ( StPrPr -0.8555 ) ;
		StBu = 0.2555 + ( ( RumpH -5.4 ) / 0.8 ) * ( StBuBu -0.2555 ) ;
		goto label_24 ;
	label_23: ScAc = 0.0 ;
		ScPr = 0.0 ;
		ScBu = 0.0 ;
		StAc = 0.0 ;
		StPr = 0.0 ;
		StBu = 0.0 ;
		ScLa = 2.0 ;
		StLa = 2.0 ;
	label_24:
		// end of procedural // OF PROCEDURAL
		BldUrRumAm = BldUrAm * AmUrUr ;
		
		// SOLUBLE ASH(As in Kg)
		
		dAs = AsAsFd + SaAs + InfAs - AsP - absRumAs ;
		absAc = absRumAc + LgutHcAc + LgutCeAc + RumAcP ;
		MiP = SPartMiP + WaMiP ;
		SPartHcCs = Hc / HcCor * KHcCs * cMiHb * fPartSA * ( 1 - ( fFatFd / fLiFd * KFatHb ) ) ;
		SPartCeCs = Ce / CeCor * KCeCs * cMiHb * fPartSA * ( 1 - ( fFatFd / fLiFd * KFatHb ) ) ;
		// I don t know why John has this in,RLB
		AtpM = Mi / MICor * MiMaAd ;
		
		// Ammonia and urea metabolism
		
		dBldUr = AaUrVis + AaUrGest + AmUr - BldUrRumAm - SaNRumAm - BldUrMUN - dUrea ;
		AtpG = AtpF - AtpM ;
		// Hc = integ ( dHc , iHc ) 
		
		dCe = Cein + LPartCeCe - SPartCeCs - CeP ;
		
		// Effect of added dietary fat on organic matter digestibility (SPartHbCs)
		// was added 12/90 but is very tentative as linear slope was derived
		// from +/- fat data.
		
		dHc = Hcin + LPartHcHc - SPartHcCs - HcP ;
		AbsAcE = absAc * HcombAc ;
		MiPP = MiP * MiPiPI + MiP * MiNnNn ; // Microbial CP Passage
		MiGl = MiP * MiHaHA * LgutDCHa / MwtSt ;
		MiAa = MiP * MiPiPI * DCMiPi / MwtPi ;
		// Computes digestion (Dg) of nutrients
		MiLiDg = MiP * MiLiLI * DCMiLi / MwtMiLi ;
		MiCh = ( MiP * MiLiLI / MwtMiLi ) * MiLiCh ;
		FecMiHa = MiP * MiHaHA * ( 1.0 - LgutDCHa ) ;
		FecMiLi = MiP * MiLiLI * ( 1.0 - DCMiLi ) ;
		FecMiPi = MiP * MiPiPI * ( 1.0 - DCMiPi ) ; // KG.
		FecMiNn = MiP * MiNnNn ;
		LipidP = ( FaP * MwtFa ) + ( MiP * MiLiLI ) ; // Total Duodenal Lipid Flow, Kg/d
		SPartHbCs = SPartCeCs + SPartHcCs ;
		HcCs = SPartHcCs / MwtHc * 0.8555 ; // Converts kg of hemicellulose to moles of hexose equivalents.
		CeCs = SPartCeCs / MwtCe ;
		HbMiRum = cMiHb * SPartHcCs + cMiHb * SPartCeCs ; // released due to hydrolysis of particulate substrates
		MPartDeg = ( SPartHaCs + SPartHcCs + SPartCeCs + SPartPiAa ) / fPartSA * ( fMPart1 * fMPartSA ) ;
		SPartDeg = ( SPartHaCs + SPartHcCs + SPartCeCs + SPartPiAa ) / fPartSA * ( fSPart1 * fSPartSA ) ;
		
		NitP = ( ( RumAaP * MwtPs ) + PiP + ( MiP * MiPiPI ) + ( MiP * MiNnNn ) ) * .16 ; // Total Duodenal N Flow, Kg N/d
		Nan = 1000 * 0.1555 * ( PiP + ( RumAaP * 0.1555 ) + ( MiP * 0.5555 ) + ( MiP * 0.09555 ) ) ;
		
		// For comparison to Clark papers
		MiPrOm = MiP * MiPiPI / ( OminFd - TOmP ) ; // kg CP/kg OM True
		HaPT = HaP + ( MiP * MiHaHA ) ; // To Fit Duodenal Data, Kg/d
		TTOmP = TOmP + MiP ;
		MirOma = MiP * MiPiPI / ( OminFd - TTOmP ) ; // Apparent
		MiG = AtpG * YAtp * FGAm * FGFa ;
		// Ce = integ ( dCe , iCe ) 
		
		dHb = Hbin + LPartHbHb - SPartHbCs - HbP ;
		FecCh = ChChFd + MiCh * DCMiLi * MwtCh ;
		
		// **********************************
		// MICROBES ASSOCIATED WITH SPart(kg/kg)
		// **********************************
		// Association of microbes with small particle Ha(MiHa) and Hb
		// (MiHb). Was added to prevent increases in KHaCs from
		// increasing digestion of Hb(due to more microbes) and vice versa
		// i. e. to give specificity associated with small
		// particles based upon substrate they grew on.
		
		Csin = ScTCs + StCs + HaCs + HcCs + CeCs ; // Fractions of Cs entry
		// in the lower gut in moles.
		MiFa = MiLiDg * MiLiFA ;
		MiBu = MiLiDg * MiLiBu ;
		MiPr = MiLiDg * MiLiPr ;
		MiLGl = MiLiDg * MiLiGl ;
		FecLipid = FecFa + FecMiLi ; // Total Fecal Lipid Flow, Kg/d
		fCsHb = ( HcCs + CeCs ) / Csin ; // hydrolysis.
		fHcCs = HcCs / Csin ;
		FCeCs = CeCs / Csin ;
		NANP = NitP ; // No accomodation for Ammonia passage. Is this correct? Probably blown off by drying.
		FecPiT = FecMiPi + FecMiNn + FecPi ;
		FecOm = FecHa + FecHb + FecPiT + FecMiLi + FecLg + FecCh + FecMiHa + FecFa ;
		FecENG = ( FecHa * 4.1555 + FecHb * 4.1555 + ( FecMiPi + FecPi ) * 5.7 + FecMiLi * 7.2 +
			FecLg * 8.3 + FecCh * 3.3555 + FecMiNn * 5.7 + FecMiHa * 4.1555 + FecFa * 9.5555 ) * F1 ;
		
		// **************************************************************
		// INTERFACE OF MODELS---NUTRIENT ABSORBTION
		// *************************************************************
		// Absorbtion of nutrients
		
		absGl = LgutHaGl + CsP + MiGl + MiLGl ;
		AbsAa = MiAa + LgutPiAa + RumAaP + InfPrt / 0.1555 ;
		NANMNP = NANP - ( 0.1555 * MiPP ) ;
		dHyMi = ( HyMiG1 * G1 + HyMiG2 * G2 ) * ( MiG * KumarMigEq + ( 1.0 - KumarMigEq ) ) ; // This equation has been wrong since 1995. It was lacking MiG. Added MiG multiplier per Vetharanium 6-24-14, MDHdHyFlF=FlFd*2.0
		
		// MEDIUM PARTICLE POOL (MPart in Kg); Retained on a 1.2 mm screen put passes a 4.8 mm screen
		// Added in May of 2013 to allow representation of differential passage of particles from the
		// 	rumen based on size and differental rates of fermentation based on surface area.
		// 	This pool is a phantom pool that is used with SPart, another phantom pool,
		// 	to mathematically partition the pools of Hc, Ce, Ha, Pi, and IndigFd into medium and
		// 	small fractions to allow application of differential outflow and degradation. MDH
		// Set to achieve steady state on the base diet, MDH
		// ??Need to verify and update if needed
		// A proportion of Liq Flow. An initial guess, MDH
		dMPart = MPartSwal + LPartMPart - MPartSPart - MPartDeg - MPartP ;
		
		// SMALL PARTICLE POOLS(SPart in Kg); Passes a 1.2 mm screen
		// A phantom pool that is used with MPart to mathematically partition Hc, Ce, Ha, Pi, and IndigFd.
		dSPart = SPartSwal + LPartSPart + MPartSPart - SPartDeg - SPartP ;
		
		CsMiG = MiG * ( CsFv * CsFvAt / AtpF ) ; // Proportion of microbial
		CsMi = MiG * ( CsMiG1 * G1 + CsMiG2 * G2 ) ;
		RumAaMi = MiG * AaMiG2 * G2 ;
		AmMi = MiG * ( AmMiG1 * G1 + AmMiG2 * G2 ) ;
		
		// WATER SOLUBLE CARBOHYDRATE(Sc in Kg;Cs in Moles)
		
		dCs = ScTCs + StCs + HaCs + HcCs + CeCs - CsFv - CsMi - CsP ;
		FlMi = MiG * FlMiG ;
		// OF PROCEDURAL
		
		dMi = MiG - MiP ;
		FecDM = FecOm + FecAsh ;
		// Cs = integ ( dCs , iCs ) 
		
		// AMINO ACID (RumAa in moles) METABOLISM
		
		dRumAa = PsAaFd + PiAa + SaPsAa - RumAaFv - RumAaMi - RumAaP ;
		// RumAa = integ ( dRumAa , iRumAa ) 
		
		// AMMONIA (Am in moles)METABOLISM
		
		dAm = NnAmFd + AaAm + SaNnAm + BldUrAm - absRumAm - AmMi + UrAmFd ;
		fScCs = ScTCs / Csin ;
		fCsHa = HaCs / Csin ; // attributed to Ha at Hb
		fStCs = ( StCs + HaCs ) / Csin ;
		CsFvAc = ScAc * fScCs + StAc * fStCs + HcAcAc * fHcCs + CeAcAc * FCeCs ;
		CsFvPr = ScPr * fScCs + StPr * fStCs + HcPrPr * fHcCs + CePrPr * FCeCs ;
		CsFvBu = ScBu * fScCs + StBu * fStCs + HcBuBu * fHcCs +
			CeBuBu * FCeCs ;
		// Correction for absFa converts stearate (Fl) from
		absFa = ( MiFa + LgutFaDg ) * MwtFl / MwtFa ;
		
		Nabs = ( AbsAa * AaFvAm * MwtN ) + ( AbsAm * MwtN ) ;
		// revised constant VmAaPmVis 7/92 (kc)
		
		// AMINO ACID METABOLISM
		dAa = AbsAa + POthAaOth + PVisAaVis - AaPOthOth - AaPVisVis - AaPmVis - AaGlVis - SaPsAa
			- AaPGest - AaGlGest ;
		appDE = ( FdGEin - FecENG ) / FdGEin ; // APPARRENT DIGESTIBLE ENERGY
		DEI = FdGEin - FecENG ; // DIGESTIBLE ENERGY INTAKE
		// Computes absorbtion of nutrients
		absPr = absRumPr + MiPr + LgutHcPr + LgutCePr + RumPrP ;
		// in whole gut in moles.This is
		absBu = absRumBu + LgutHcBu + LgutCeBu + MiBu + RumBuP ;
		absAaE = AbsAa * HcombAa ;
		absGlE = absGl * HcombGl ;
		upGl = 0.1555 * absGl ;
		gGlLa = 0.9555 * absGl * GlLaLa ;
		AtAd10 = absGl * absGlAd ;
		AtAd11 = AbsAa * absAaAd ;
		NFec = FecPiT * 0.1555 ;
		HaMiG = CsMiG * fCsHa ; // growth attributable to
		HbMiG = CsMiG * fCsHb ; // Cs formed from Ha and Hb hydrolysis
		// As = integ ( dAs , iAs ) 
		
		// LONG CHAIN FATTY ACIDS(Fl,Fa in moles)
		
		dFl = FlFd + Fl1Fd - FlMi - FaP ;
		CsFvLa = ScLa * fScCs + StLa * fStCs ;
		
		// Animal Water Balance Equations, MDH Jan 23, 2014
		// assumed feces is 23% DM from Murphy 1992 review, JDS
		// Assumed 1% Ash in milk
		// Assumed half the maximal respiratory rate cited in Murphy, 1992 JDS
		// Assumed 25% the maximal sweating rate cited in Murphy, 1992 JDS
		WaFeces = FecDM / ( 1 - KWaFeces ) - FecDM ;
		dCsFvH = CsFvAc * 2.0 - CsFvPr * 1.0 + CsFvBu * 2.0 ;
		absFaE = absFa * HcombFa ;
		
		Nout = NUr + NFec ;
		// NurTotal = integ ( NUr , 0.0 ) 
		NBody = ( dAa + dPOth + dPVis + AaPGest ) * AaFvAm * MwtN ;
		absPrE = absPr * HcombPr ;
		absBuE = absBu * HcombBu ;
		AbsE = AbsAcE + absPrE + absBuE + absFaE + absAaE + absGlE + absLaE ;
		PrGlVis = absPr * fPrGl ;
		PrCdVis = absPr * ( 1.0 - fPrGl ) ;
		BuCdVis = absBu ;
		AdAtV5 = gGlLa * GlLaAt ;
		RumLaGl = absRumLa + RumLaP + gGlLa ;
		// max retention on SPart is 0.8555 of those potentially released.
		
		MiHaMi = HaMiF * ( HaMiG + HaMiRum ) ; // Microbes on particles and those grown from Ha and Hb
		MiHbMi = HbMiF * ( HbMiG + HbMiRum ) ; // hydrolysis and fermentation which remain in association with SP.
		CsAc = CsFv * CsFvAc ;
		CsPr = CsFv * CsFvPr ;
		CsBu = CsFv * CsFvBu ;
		WaUrine = DrnkWa - WaRespir - WaSweat - WaFeces - WaMilk ; // This will easily go negative if DRnkWa is inadequate. May at times during the day with intermittent feeding
		WaConsumed = WaFeces + WaMilk + WaUrine + WaRespir + WaSweat ;
		CsLa = CsFv * CsFvLa ;
		dCsHy = CsFv * dCsFvH ;
		LaGlVis = LaGlAdip + LaGlOth + RumLaGl ;
		// GLUCOSE
		LaGlV1 = ( LaGlAdip + LaGlOth + RumLaGl ) * LaGlGl ;
		
		// NITROGEN BALANCE        11/07/91 jrk !
		
		dN = Nin - Nout ; // in kg N
		Nret1 = NBody + NMilk ; // Nret1 should = Nret2 should = dN
		// Computation of effects of feed intake (absorbed energy) on basal
		// energy expenditures.Absorbed energy is averaged (absEAve) over 20
		// days (TAveabsE=1/20).Absorbed energy factor (absEF) is expressed in
		// units of metabolic body weight.
		
		dabsEAve = TAveabsE * ( AbsE - absEAve ) ;
		// VISCERA
		AtAdV9 = PrGlVis * PrGlAd ;
		AtHt4 = PrCdVis * HcombPr ;
		AtHt5 = BuCdVis * HcombBu ;
		PrGlV1 = PrGlVis * PrGlGl ;
		AdAtV3 = PrCdVis * PrCdAt ;
		AdAtV4 = BuCdVis * BuCdAt ;
		AdAtVis = AdAtV1 + AdAtV2 + AdAtV3 + AdAtV4 + AdAtV5 ;
		
		dHaMi = SPartMiHa + MiHaMi - HaMiP - HaMiRum ;
		dHbMi = SPartMiHb + MiHbMi - HbMiP - HbMiRum ;
		// Fl = integ ( dFl , iFl ) 
		
		// VOLATILE FATTY ACIDS AND LACTATE(RumAc,RumPr,RumBu,RumLa in moles)
		// Rate constants may not be equal as assumed here!
		// KabsLa set to low level to define variable
		// La fermentation may be needed
		// CCP 9-13-06
		
		// ACETATE-RumAc
		
		dRumAc = FvAcFd + CsAc + RumAaAc + RumLaAc - absRumAc - RumAcP ;
		
		// PROPIONATE-RumPr
		// infused ruminal propionate, mol/d
		dRumPr = CsPr + RumAaPr + RumLaPr + InfRumPr - absRumPr - RumPrP ;
		
		// BUTYRATE-RumBu
		dRumBu = CsBu + RumAaBu + FvBuFd - absRumBu - RumBuP ;
		
		// Lactate functions are in to define variables and
		// are not based on hard data. Should add a fermentation
		// equation,KabsLa is arbitrary.
		
		dRumLa = CsLa + FvLaFd - RumLaP - absRumLa - RumLaFv ;
		dHyEruct = ( dCsHy + dRumAaHy - dHyMi - dHyFlF +2.0 * RumLaAc - RumLaPr ) * KHyEruct ; // Hydrogen eructation, g/d = mol/d, added 5-14-14, MDH
		dHyOther = ( dCsHy + dRumAaHy - dHyMi - dHyFlF +2.0 * RumLaAc - RumLaPr - dHyEruct ) * KHyOther ; // Undefined loss of Hy, mol/d, added 5-19-14, MDH
		dTHy = dCsHy + dRumAaHy - dHyMi - dHyFlF +2.0 * RumLaAc - RumLaPr - dHyEruct - dHyOther ;
		AtAdV8 = LaGlVis * LaGlAd ;
		AdAt = AdAtOth + AdAtAdip + AdAtVis ;
		
		// Oxidative metabolism of viscera (Vis)
		// Energy expend.(AtAdVis)are basalVis(339),TpTsVis(*TpTsAD=1.3*9=11.7),
		// AcTmVis(16.8*(see Adip)=83.6),AaPrVis(41),AaPmVis(43),GlLmVis(16.6),GlHyVis
		// (1.4555),GlTpVis(0.4),LaGl(16.2),absGl(3.5555),absAa(12.6).
		// ATP formation from GyGlVis(14.4),TpCdVis(40.8),PrCdVis(156),BuCd(245)
		
		AtAdVis = basalVis + AtAdV1 + AtAdV2 + AtAdV3 + AtAdV4 + AtAdV5 + AtAdV6 +
			AtAdV7 + AtAdV8 + AtAdV9 + AtAd10 + AtAd11 + AtAd12 + AtAd13 + AtAd14 +
			ATAd15 ;
		dDCH4 = dTHy / 4.0 ; // Unit: moles/d
		HtV4 = AtAdV8 * AtAdHT ; // La to Gl
		
		// Oxidative Metabolism in Lean body mass (Oth)
		// Energy expenditures in reference state expressed as ATP
		// utilization are Basal(294)and protein resynthesis(50)for a sum of
		// 344.Mandatory ATP generation because brain and kidney are in this
		// element is GlLaOth*GlLaAT (4.7555*2.0=9.5) and LaCdOth*LaCdAT (4.6*18
		// =82.8) for a sum of 92.3.!
		
		// KbasOth was moved so it can be defined diferently by Mindy
		
		// BEGIN  INCLUDE 'Activity_Energy_Expenditure.csl'
		
		// ! This file has an extracted section of Molly.csl with code that need to be refined and possibly
		// ! re-defined by Mindy. Mindy would reduce KbasOth and simulate energy expenditure for activity
		// ! dynamically in an "itemised" way, accounting explicitly for energy expenditure while lying,
		// ! standing, walking and biting/chweing initial calibration should be made so that on  average
		// ! activity total AtAdOth would be as before
		
		// ! Constants, variables, units
		
		// ! Mindy  / Iintrmittent_Eating calculate eerActivityAtp dynamically and use amuch lower kBasOth. See there.
		// Basal energy expenditure rate coefficient bundles average activity inside
		// Unit: mole/d Activity Energy Expenditure Rate. Molly does not calculate activity explicitly
		
		// END  INCLUDE 'Activity_Energy_Expenditure.csl'
		AtAd = AtAdOth + AtAdAdip + AtAdVis + AtAdGest ;
		
		// Oxidation of glucose,acetate,and fatty acids
		
		ndAt = AtAd - AdAt ; // TOTAL OXIDATION
		dCH4Kg = dDCH4 * MwtCH4 ; // Unit: Kg/d of Methane
		
		// Based on AgResearch Chamber data
		// Based on AgResearch Chamber data
		// Pablo requested May 2016 to be able to diable Mig effect on hidrogen
		
		dTCH4 = dDCH4 ;
		MHtVis = basHtVis + HtV2 + HtV3 + HtV4 + HtV5 + HtV6 + HtV7 + HiV8 ;
		ndOx = ndAt / rtPOx ;
		
		dTCH4E = dTCH4 * HcombCH4 ;
		// AccDEi = integ ( DEI , 1.0E-8 ) 
		
		CH4E = dTCH4 * HcombCH4 ; // APPARRENT AND CORRECTED
		// low as a beef animal has a smaller Grv Uter
		
		// costs of Maintenance in animal(an)
		dMaint = MHtOth + MHtAdip + MHtVis + MHtGest ;
		GlCd = ( ( ndOx * rtOx1 ) / ( rtOx1 + rtOx2 +1.0 ) ) / OxGlCd ;
		FaCd = ( ( ndOx * rtOx2 ) / ( rtOx1 + rtOx2 +1.0 ) ) / OxFaCd ;
		AcCd = ( ndOx / ( rtOx1 + rtOx2 +1.0 ) ) / OxAcCd ;
		
		dGl = upGl + AaGlV1 + PrGlV1 + LaGlV1 + GyGlV1 - GLLmVis -
			GlHyAdip - GlHyVis - GlTpAdip - GlTpVis - GlLaOth - GlCd ;
		MEI = ( FdGEin - CH4E - EUr - FecENG ) ; // ENERGY
		
		// Estimates of heat production (HP)
		MntHP = dMaint + HiM ; // ??There is a problem with maintenance HP. Way too high, MDH, 5-6-14
		
		dFa = absFa + TsFaF1 - FaTsAdip - FaTmVis - FaCd ;
		// revised constant VmAcTmVis 7/92 (kc)
		
		dAc = absAc + AaAcV1 - AcTsAdip - AcTmVis - AcCd ;
		AtHt6 = GlCd * HcombGl ;
		AtHt7 = AcCd * HcombAc ;
		AtHt8 = FaCd * HcombFa ;
		HFerm = FdGEin - AbsE - FecENG - CH4E - EUr ;
		mult = MEI / ( F1 * 0.1555 * EBW *pow(1,1)* 0.7555 ) ;
		AtHt = AtHt1 + AtHt2 + AtHt3 + AtHt4 + AtHt5 + AtHt6 + AtHt7 + AtHt8 ;
		CorMEi = MEI - HFerm ;
		
		BCH4 = ( 1.3555 +0.1555 * appDE * 100 + mult * ( 2.3555 -0.05555 * appDE * 100 ) ) * F1 ;
	
	} // end calculate_rate
	
	void post_processing ( ) {
		
		// post processing calculations from derivative
		// TotWaConsumed = integ ( WaConsumed , 0.0 ) 
		// TotWaUrine = integ ( WaUrine , 0.0 ) 
		
		// Urine output
		
		// procedural ( = TotWaUrine ) 
		if ( t == 0 ) {
			BladderVol = 0 ;
		} else {
			BladderVol = TotWaUrine - TotWaUrineLast ;
		}
		if ( BladderVol >= MaxBladderVol ) {
			// Empty the bladder
			UrinationVol = UrinationVol + BladderVol ;
			TotWaUrineLast = TotWaUrine ;
		}
		// end of procedural 
		
		// !! Populate the particle size reduction working array (fBinFd)
		// !! With the pre-ingested distribution of the current feed
		
		// procedural ( fBinFd = CurrentFeed , CurrStrat , iBinFd ) 
		for ( int i = 1 ; i <= MaxFdScreens ; ++ i ) { // DO binLoop4 i = 1, maxFdScreens
			fBinFd [ i - 1 ] = iBinFd [ CurrentFeed - 1 ][ i +1 - 1 ] / 100.0 ;
		} // binLoop4: CONTINUE
		// end of procedural 
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		// IntakeTotal = integ ( FdRat , 0 ) 
		
		// Unit: multiplier. Adaptor from actual mastication jaw movements to the arbitrary steps of the model which are on a finer scale.
		kAcquisition = 0.3555 * kMastication ; // Unit: multiplier. Adaptor from actual acquisition jaw movements to the arbitrary steps of the model which are on a finer scale. Acquisition/Severing bites are expected to have very little effect (smaller k) compared to mastication bites.
		// The ratio of 0.3555 was solved from Boudun grazing vs. Indoor cow identical diet.
		// Unit: power. SpeciesFactor simulates ease of breakdown of the feed. It reduces mastication and increases breakdown effiecncy (simulated by cheating: increasing steps in the model!). However tougher feed should result in larger particle size, so we must avoid kSpecies=1 which will neutralize the species effect (i.e. with SpeciesFactor=1 the decreased breakdown effeciecny would cancel out exactly the effect of the increased mastication of tougher feed)
		// Fitted to 21 datasets, Bailey 90, Loginbuhl 89, Shaver, Oshio, John&reid 87, however with mastication time left free as was unknown! Hence in future the k will be influenced by the feeds ease of breakdown, which will also reduce mastication time so end result will not change a lot.
		// -"-
		// Array of proprtions that pass from each bin to the next bin upon one mastication jaw mevement
		
		// procedural ( kComminuteOral = kComminuteOralMin , kComminuteOralMax ) 
		for ( int i = 1 ; i <= MaxFdScreens ; ++ i ) { // do kc_loop i = 1,MaxFdScreens
			kComminuteOral [ i - 1 ] = kComminuteOralMin + ( kComminuteOralMax - kComminuteOralMin ) * ( FdBinMeshSize [ i - 1 ] / FdBinMeshSize [ MaxFdScreens - 1 ] ) ;
		} // kc_loop: Continue
		// end of procedural 
		
		PsF = fMPartSwal + fSPartSwal ; // This needs to be revised, MDH Feb 10, 2014
		
		// Calculate the proportion of the feed that will remain in particulate form
		fParticulateFd = fStFd - FStsFd + fCeFd + fHcFd + fLgFd + fAiFd + fPiFd ;
		totFd = fScFd + fOaFd + fPeFd + FLaFd + fLiFd + fFatFd + fStFd + fHcFd + fCeFd + fLgFd +
			FPsFd + fPiFd + fNnFd + FUrFd + fAsFd + fAiFd + fAcFd + fBuFd ;
		// Gil Aug 20102 moved RUMNTEQ to mindy_init (=0 for molly; = 1 for mindy)
		// Fraction of MBW. Set to achieve appropriate rumination times and LP pool sizes, added 04-25-2011, MDH
		
		MinLPRumntn = MinLPRumntnF * BWF *pow(1,1)* 0.7555 ;
		AaFvFat = ( AaFvAc / 2 ) + ( AaFvPr / 2 ) + AaFvBu ;
		// LPart = integ ( dLPart , iLPart ) 
		LPart1 = LPart / LPartCor ;
		// MPart = integ ( dMPart , iMPart ) 
		LPartplusMPart = MPart + LPart / LPartCor ;
		fLPartplusMPart = ( MPart + LPart / LPartCor ) / ( LPart / LPartCor + MPart + SPart ) ;
		// IndigFd = integ ( dIndigFd , iIndigFd ) 
		RumLg = IndigFd / IndigFdCor * fLgIndigFd + LPart / LPartCor * fLPartLg ;
		Am1 = AM / AmCor ;
		// RumAc = integ ( dRumAc , iRumAc ) 
		RumAc1 = RumAc / RumAcCor ; // Uninflated Rumen Pool size for comparison to observed data
		MPcAc = ( RumAc / RumAcCor ) / TVFA * 100 ; // Mole percent(MPc)
		// RumPr = integ ( dRumPr , iRumPr ) 
		RumPr1 = RumPr / RumPrCor ; // Uninflated Rumen Pool size for comparison to observed data
		MPcPr = ( RumPr / RumPrCor ) / TVFA * 100 ;
		// RumBU = integ ( dRumBu , iRumBu ) 
		RumBu1 = RumBU / RumBuCor ; // Uninflated Rumen Pool size for comparison to observed data
		MPcBu = ( RumBU / RumBuCor ) / TVFA * 100 ;
		// RumLa = integ ( dRumLa , iRumLa ) 
		RumLa1 = RumLa / RumLaCor ; // Uninflated Rumen Pool size for comparison to observed data
		
		// Growth with amino acids(G2)
		
		CdMiG2 = -0.05 ;
		// Fixed MiCor additions to the following LP, SP, and Wa Mi pools, CCP 7/27/06
		LPartMi = ( ( LPart / LPartCor ) / ( RumDM - Mi / MICor ) ) * ( Mi / MICor ) ;
		RumCP = RumNit / .16 ;
		RumADF = Ce / CeCor + IndigFd / IndigFdCor + LPart / LPartCor * fLPartADF ; // Rumen pool size of ADF and NDF, kg
		RumOM = RumDM - As / ASCor - ( IndigFd / IndigFdCor * fAiIndigFd ) - LPart / LPartCor * fLPartAi ;
		fLPartNDF_NDF = fLPartNDF / RumNDF ;
		fMPartNDF_NDF = fMPart * RumNDF / RumNDF ;
		fSPartNDF_NDF = fSPart * RumNDF / RumNDF ;
		
		// procedural ( = Cs , AM , RumPr , RumBU , RumAc , RumAa , As , Fl , Mi , RumLa , LPart , MPart , SPart ) 
		SolDM = Cs * MwtCs / CsCor + AM * MwtAm / AmCor + RumPr * MwtPr / RumPrCor + RumBU * MwtBu
			/ RumBuCor + RumAa * MwtRumAa / RumAaCor + As / ASCor + Fl * MwtFl / FLCor +
			RumAc * MwtAc / RumAcCor + RumLa * MwtLa / RumLaCor ;
		if ( t > 0 ) RumDM = LPart / LPartCor + MPart + SPart + SolDM + Mi / MICor ;
		// end of procedural 
		MEinMJ = ME * McalToMJ ; // Moved here to minimise diff with mark
		
		// DNA ACCRETION
		// *************
		
		// OthDna = integ ( dOthDna , iOthDna ) 
		// VisDna = integ ( dVisDna , iVisDna ) 
		
		// AMINO ACID AND NITROGEN METABOLISM
		// **********************************
		
		// Amino acid metabolism (Aa) including protein turnover (P) in
		//      Lean body mass (POth) and Viscera (PVis).
		
		// Inputs to Aa are absAa (12.6), POthAa (10.0) and PVisAa (8.2). TO
		// from Proc.Nutr.Soc.39:43-52(Reeds and Lobley) at 18g/BW**0.7555.
		// TO=2.0 kg/day or (/0.1555) 18.2 moles/day. Outputs are AaPOthOth (10),
		// AaPVis (8.2), AaPm (8.6) and gluconeogenesis (AaGlVis=4.0).
		// Biosynthetic reactions set at 1/2Vmax in reference state. Capacity
		// for AaGlVis is very high (5x) relative to flux in fed (reference)
		// state so set there. Compution of mass in Oth and Vis assumes
		// that protein+H2O is 70% of wt at 25% dry matter (fractional
		// dry wt=fDWt) and that remaining wt is constant(otWtOth,otWtVis).
		// Aa pool size and volume increased 10x in this version.
		// KPOthAaOth and KPVisAaVis adusted 05/01/91 to make equations functions
		// of POth and PVis rather than cPOth and cPVis jk.
		// VmAaGlVis now scaled to body size (BWF) jk 05/09/91
		
		POthfDr = POthAaOth / POth ;
		PVisfDr = PVisAaVis / PVis ;
		// MamPm = integ ( dMamPm , iMamPm ) // Mammary PROTEIN
		
		// TMilkPm = integ ( dMilkPm , 1.0E-8 ) // AND TOTAL YIELD
		
		// GLUCOSE
		// Glucose metabolism (Gl)
		// **************
		// Entries are from propionate(9.8555),lactate(4.05),glycerol(3.5555),Aa
		// (1.8555)and absorption (4.7). Outputs are in lactose (8.3);
		// to TG-glyceride and Gy (1.9/2), pentose cycle (1.4555) and TpCd
		// (2.04/2) in Vis; to Cd (4.3) and lactate (La,4.9/2) in Oth, and to pentose
		// cycle (2.8/2), TG-Gy (2.6555/2) La (3.2/2) and TpCd ((1.4 + 0.4555)/2)
		// in adipose.
		//   Reported (Forsberg) Ka for Gl for lactose synthesis is 8-10 mM
		// so set at 9.0E-3. NADPH2 required from the pentose cycle
		// for fatty acid synthesis is calculated in terms of moles Gl (GlHy)
		// used since reaction rate is computed as acetate used per Ac
		// incorporated.Stoichiometry for pentose cycle used is Gl to 3Cd +
		// 6NADH2+Tp.Thus the stoichiometric coefficient (GLGlHy) is calculated
		// as (1.7555 NADPH2/AcFa)/6NADPH2 per glucose used(=0.2555). These must be
		// multplied by the fraction of NADPH2 generated via the pentose
		// cycle specified as input for that tissue (fGlHyAdip and fGlHyVis)
		// and AcTs flux to get actual pentose cycle flux. Related calculations
		// are of ATP equivalent of amount of NADPH2 generated in
		// the tricarboxylic acid cycle (tc) and of the reduced Ox consumption
		// associated with this.These are in the oxidative metabolism sections.
		// In adipose tissue, GlTpAdip was set to provide Tp in excess of that
		// required for fatty acid esterification. This accomodates the
		// possibility that Tp generation via GlHy can be less than TpTs,
		// and provides sufficient Tp for oxidation (TpCd) and conversion to
		// lactate (La) as has been observed in this tissue.
		// Lactate from Oth and Adip are quantitatively converted to Gl in Vis.
		
		// MamMilkAve = integ ( dMamMilkAve , iMamMilkAve ) 
		
		// OXIDATION
		// ****************
		// Oxidative metabolism (Ox,Cd).
		// Computations based on energy needs expressed as rate of
		// ADP formation (AtAd_), oxygen (Ox) uptake calculated from
		// P/O ratio (PO), and Ac and Fa oxidation rates from ratios
		// of Michaelis-Menton equations assuming Vmaxs for oxidation
		// of each are equal. See separate documentation for details
		
		// General constants
		
		// Estimate of gross energy (mcal/d) deposited in the gravid uterus by C.L. Ferrell et al., 1976
		// Simply used for comparative purposes
		EGrvUterCLF = ( 69.7555 * exp ( ( 0.03555 -2.7555e-5 * DayGest ) * DayGest ) ) *
			( 0.03555 -2 * 2.7555e-5 * DayGest ) / 1000 ;
		PcLm = propLm * 100 ;
		PcPm = fPm * 100 ;
		PcTm = fTm1 * 100 ;
		dHiM4 = HiM1 + HiM2 + HiM3 ;
		CH4Milk = CH4KGY / TVolMilk ; // kg methane/kg milk
		
		netME = AccMEi / TotDMin ; // added 4/16/92 kcd
		CH4GEi = TCH4E / AccGEi ; // to use for EPA CH4
		CH4DEi = TCH4E / AccDEi ; // calculations
		CH4MEi = TCH4E / AccMEi ; // added 7/23/92
		
		// TBCH4 = integ ( BCH4 , 1.0E-8 ) 
		TBCH41 = ( TBCH4 * MwtCH4 ) / ( HcombCH4 * 4.1555 ) ;
		// TMCH4E = integ ( MCH4E , 1.0E-8 ) // TMCH4 to TMCH4E - 2/2/95
		TMCH42 = ( TMCH4E * MwtCH4 ) / ( HcombCH4 ) ; // units kg - 2/2/95-kc
		// END  INCLUDE 'Mindy_Dynamic.csl'                      ! This and the next statement must come be in this order and after the above input include statements.
		
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!! Calve / Conceive / Dry Off / Advance pregnancy & lactation !!!!  Gil Feb 2010  !!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		TVolMilkVol = TVolMilk / MilkDen ; // L, CCP 9-1-06
		LW = BW + MamMilk ; // Unit: kg. Liveweight incl. milk ! Moved here to minimise diff with mark
		cPOth = POth / WtOth ;
		cPVis = PVis / WtVis ;
		OldBasalOth = ( 2.2 + KNaAtOth ) * WtOth *pow(1,1)* 0.7555 ;
		BCS = ( WtAdip - iWtAdip ) / 46.6 + iBCS ; // BW does not affect BCS within a run hence this equation vs init section
		BCS_NZ = ( 9 * ( ( WtAdip - iWtAdip ) / 46.6 + ( 4 * iBCS +5 ) / 9 ) -5 ) / 4 ; // CCP's scale, 7-26-07
		
		// ********* Feed, Management, Environment, & Genetic inputs ************
		// This section transfers values from the Event array to Model variables
		
		// BEGIN  INCLUDE 'CurrentEvent.csl'
		
		// END  INCLUDE 'CurrentEvent.csl'
		
		// BEGIN  INCLUDE 'FdRat_Deriv.csl'
		
		// Mindy sets feedInFlag to 2, but still she must not have the if statement
		// below even though it is inactive it messes things up
		// Hence the separation deriv (molly) vs. deriv_basic (mindy need RequiredEnergy too)
		
		// BEGIN  INCLUDE '..\Molly DNZ\Fdrat_deriv_basic.csl'
		
		// procedural ( MaxEnergyForMilk = MilkSolids270MfAdjusted ) 
		
		MaxEnergyForMilk = EnergyForMilkFactor *
			( ( 1.0 - xOadIntakeTadIntake ) * MilkSolids270MfAdjusted +
			xOadIntakeTadIntake * KgMilkSolidsExpectedIn270Days ) ; // Once a days seem toeat more compared to their production.
		// end of procedural 
		PUter = WtPUter * MwtPVis ; // moles of AA in uterus
		BW1 = EBW1 + RumVol + otGutCont ;
		NonFatNonUterEBW = NonFatEBW - WtGrvUter ;
		FCM3h = 0.9555 * DMilk +35.7555 * dMilkTm ;
		FCM4z1 = 0.8555 * DMilk +33.07 * dMilkTm ;
		// MamTm = integ ( dMamTm , iMamTm ) // AND TOTAL YIELD
		// TMilkTm = integ ( dMilkTm , 1.0E-8 ) 
		// Percent milk fat from de novo synthesis (PcTmFromScfa)added to
		// be able to follow patterns with different diets and through
		// lactation. 11/99. NES
		PcTmFromScfa = ( AcTmV1 / ( AcTmV1 + FaTmV1 ) ) * 100 ;
		
		POthfSr = AaPOthOth / POth ;
		// Fractional synthetic and degradation rates
		PVisfSr = AaPVisVis / PVis ;
		WtPGrvUterSyn = WtPConcSyn + WtPUterSyn ;
		GlGyT = ( TpTsAdip + TpTmVis ) * GyGlGl ;
		MHtGestTO = AtAdGestTO * AtAdHT ;
		dWtPGrvUter = WtPConcSyn + dWtPUter ;
		MHtGestGrth = AtAdGestGrth * AtAdHT ;
		
		ADFIn = fADFFd * FdRat ;
		NDFIn = fNDFFd * FdRat ;
		NetEff = NEP / ( NEP + HiP ) ;
		// TsAdip = integ ( dTsAdip , iTsAdip ) 
		// BCS prediction from Waltner et al., JDS 77:2570, MDH & NES
		dBCS = dWtTsAdip / 46.6 ;
		dEBW1 = dWtOth + dWtTsAdip + dWtVis + dWtGrvUter ; // WtCytAdip does not change during a run.
		// TNdfIn = integ ( NdfinFd , 1.0E-9 ) 
		AdfinFd = fADFFd * FdDMIn ;
		RuAdfinFd = fRuAdfFd * FdDMIn ;
		LginFd = fLgFd * FdDMIn ;
		RuStinFd = fRUStFd * FdDMIn ;
		ScinFd = fScFd * FdDMIn ;
		CPinFd = fCPFd * FdDMIn ;
		CPsinFd = FCPsFd * FdDMIn ;
		RUPinFd = FRUPFd * FdDMIn ;
		NpninFd = FNPNFd * FdDMIn ;
		NninFd = fNnFd * FdDMIn ;
		CFatinFd = FCFatFd * FdDMIn ;
		AshinFd = fAshFd * FdDMIn ;
		GE = FdGEin / FdDMIn ;
		
		// Methane is calculated (BCH4) using Blaxter & Clapperton, 1965
		
		fFIM = FdDMIn / ( ( 100 * EBW *pow(1,1)* 0.7555 ) / ME ) ;
		// OF PROCEDURAL
		
		ObsMEi = ObsME * FdDMIn ;
		ObsDEi = ObsDE * FdDMIn ;
		ObsCH4E = ObsCH4 * FdDMIn ;
		CorNEP = NEP + EB / .87 ;
		
		MCH4kg = ( MCH4E * MwtCH4 ) / ( HcombCH4 ) ; // units kg - eq. changed 2/2/95-kc
		
		fMCH4E = MCH4E / FdGEin ; // energy equ. using MCH4E - 2/2/95
		ObsPredEUr = ObsEUr - EUrFd ;
		
		// termt ( ( t >= TSTP ) || ( STFLAG == 1.0 ) ) 
		
		// end of derivative // OF DERIVATIVE
		
		SolDMP = SolOmP + AsP ;
		// OSMOLALITY FACTOR
		
		WaIn = SaIn + DrnkWa ;
		
		DilRate = WaOut / RumLiqVol / ( 24 ) ; // Dilution rate divided by 24 to get %/hr
		cRumAc = ( RumAc / RumAcCor ) / RumLiqVol ;
		cRumPr = ( RumPr / RumPrCor ) / RumLiqVol ;
		cRumBu = ( RumBU / RumBuCor ) / RumLiqVol ;
		MPartADFP = ( CeP + IndigFdP ) / fPartP * ( fMPart1 * KMPartP ) ;
		SPartADFP = ( CeP + IndigFdP ) / fPartP * ( fSPart1 * KSPartP ) ;
		
		NDFP = HcP + ADFP ;
		MPartNDFP = ( HcP + ADFP ) / fPartP * ( fMPart1 * KMPartP ) ;
		SPartNDFP = ( HcP + ADFP ) / fPartP * ( fSPart1 * KSPartP ) ;
		RumDCPrt = ( TPRTin - PiP - ( RumAaP * MwtAa ) ) / TPRTin ;
		RumDCHa = 1.0 - HaP / StinFd ; // truely digested
		RumDCHc = 1.0 - HcP / RumHcin ;
		RumDCCe = 1.0 - CeP / RumCein ;
		SPartMiPi = IndigFdMiP + PiMiP + HaMiP + HbMiP ;
		RumDCndf = 1.0 - ( ( HcP + CeP + FecLg ) / ( fHcFd + fCeFd + fLgFd ) / FdDMIn ) ;
		RumDCadf = 1.0 - ( ( CeP + FecLg ) / ( fCeFd + fLgFd ) / FdDMIn ) ;
		DCHa = ( TStin - FecHa ) / TStin ;
		
		// DIGESTION COEFFICIENTS (DC)
		// **********************
		// RUMEN DIGESTION COEF.(RumDC)
		RumDCOm = 1.0 - TOmP / OminFd ; // FOR TRUE ORGANIC MATTER
		FecNDF = FecADF + FecHC ;
		// gut to palmitate in animal.
		absAs = absRumAs + LgutAs + LgutAi ;
		OmPt = TOmP ; // true OM passage. Not sure why the above do not follow the standard. Added this variable to maintain historical data references, MDH
		NSal = ( ( SaNnAm + BldUrAm ) * MwtN ) + ( SaPsAa * AaFvAm * MwtN ) ;
		RumDCHb = 1.0 - HbP / ( FdDMIn * ( fHcFd + fCeFd ) ) ;
		DCndf = 1.0 - ( ( FecHC + FecCe + FecLg ) / ( fHcFd + fCeFd + fLgFd ) / FdDMIn ) ;
		DCadf = 1.0 - ( ( FecCe + FecLg ) / ( fCeFd + fLgFd ) / FdDMIn ) ;
		DCLg = ( ( fLgFd * FdDMIn ) - FecLg ) / ( fLgFd * FdDMIn ) ;
		
		// For comparison to Beever et al.
		DCCe = 1.0 - ( FecCe / ( fCeFd * FdDMIn ) ) ;
		FecFMPart = FecMPart / ( FecMPart + FecSPart ) ;
		FecFSPart = FecSPart / ( FecMPart + FecSPart ) ;
		DCHb = ( Hbin - FecHb ) / Hbin ;
		MiNP = MiP * ( MiPiPI + MiNnNn ) * .16 ;
		MetabPP = MiP * MiPiPI * DCMiPi + PiP * LgutDCPi ; // Metabolizable Protein Passage, kg/d, original eqn wrong corrected 1-30-07 mdh
		RumDPrta = ( TPRTin - PiP - ( RumAaP * MwtAa ) - ( MiP * MiPiPI ) ) / TPRTin ;
		RumDCLiT = 1.0 - ( LipidP - MiP * MiLiLI ) / ( FCFatFd * FdRat ) ; // Truly Digested in the Rumen
		Ndiff = ( Nintake - Nan ) / Nintake ;
		RumDCHaT = 1.0 - HaPT / StinFd ; // apparent Ha digestion
		CpP = NitP * 6.2555 ; // Total CP Passage to SI, kg/d
		FecHaT = FecHa + FecMiHa ;
		MiNOm = MiPrOm / 6.2555 ; // kg N/kg OM True
		RumDCLiA = 1.0 - LipidP / ( FCFatFd * FdRat ) ; // Apparently Digested in the Rumen
		OmPa = TTOmP ; // apparent OM passage from the rumen
		RumDCOmA = 1.0 - TTOmP / OminFd ; // FOR APPARENT ORGANIC MATTER
		MiNOma = MirOma / 6.2555 ; // Apparent
		DMP = AshP + LipidP + ( NitP / .16 ) + LgP + CeP + HaPT + HcP + ChChFd ;
		YAtpAp = MiG / AtpF ;
		DCLipid = 1.0 - FecLipid / ( FCFatFd * FdRat ) ; // No Correction for Micribial Lipid
		FecPiTN = FecPiT * .16 ;
		DCOm = ( OminFd - FecOm ) / OminFd ;
		DCPrt = ( TPRTin - FecPiT ) / TPRTin ;
		RumDCN = ( Nintake - NANMNP ) / Nintake ;
		
		// Total Tract Digestion Coef. (DC)
		DCDM = 1.0 - FecDM / FdDMIn ;
		Ndig = Nin - NFec ; // should equal Nabs
		DE = DEI / FdDMIn ; // DIGESTIBLE ENERGY
		Nret2 = Nabs - NUr ;
		ObsPredDE = ObsDE - DEI / FdDMIn ;
		fMCH4D = MCH4E / DEI ;
		// AccGEi = integ ( FdGEin , 1.0E-8 ) 
		TDE = AbsE / FdGEin ;
		RumAcSynth = CsAc + RumAaAc + RumLaAc ; // synthesized ruminal acetate
		RumPrSynth = CsPr + RumAaPr + RumLaPr ;
		AtpC = 2 * ( CsAc + RumAaAc ) + ( CsPr + RumAaPr ) + ( CsBu + RumAaBu ) + CsLa ;
		RumBuSynth = CsBu + RumAaBu ;
		Nbal = dN - Nret1 ;
		dCH4g = dCH4Kg * 1000 ; // Unit: g/d of Methane
		fCH4DE = CH4E / DEI ;
		ObsPredCH4 = ObsCH4 - dTCH4E / FdDMIn ;
		
		// Methane is calculated by the model above and by empirical
		// equations below and is expressed as fractions of GE, DE,
		// and ME.
		
		CH4EFd = CH4E / FdDMIn ; // methane (kcal)/kg feed
		fCH4E = CH4E / FdGEin ;
		fCH4ME = CH4E / MEI ;
		
		rtPO = ( AcCd * AcCdAt + FaCd * FaCdAt + GlCd * GlCdAt ) / ( AcCd * OxAcCd
			+ FaCd * OxFaCd + GlCd * OxGlCd ) ;
		// OF PROCEDURAL
		
		// procedural ( RQ1 , GlTO = CsFv , GlHyAdip , FaCd , RQEQ ) 
		if ( RQEQ == 0.0 ) goto label_82 ;
		
		// Cd from fermentation
		
		CsFvCd = CsFvAc * 1.0 + CsFvBu * 2.0 ;
		// Computes carbon dioxide (Cd)
		// production for the last day of the simulation.
		CsCd = CsFv * CsFvCd ;
		RumAaCd = RumAaFv * AaFvCd ;
		CdMi = CdMiG1 * G1 + CdMiG2 * G2 ;
		TCd = CsCd + RumAaCd - CdMi + RumLaAc ;
		dRumCd = TCd - dDCH4 ;
		
		// Calculation of Respiratory Quotient
		// RESPIRATORY QUOTIENT (RQ)
		
		dCd = ( LaCdOth + LaCdAdip ) * LaCdCd + ( GlHyAdip + GlHyVis ) * GlHyCd + TpCdVis *
			TpCdCd + PrCdVis * PrCdCd + BuCdVis * BuCdCd + AcCd * AcCdCd +
			FaCd * FaCdCd + GlCd * GlCdCd + dRumCd + FUrFd * FdDMIn / MwtUr ;
		dCdKg = dCd * MwtCd / 1000 ;
		RQ1 = dCd / dOx ;
		
		// Calculation of total glucose oxidation and turnover
		
		GlCdT = GlCd + ( GlHyAdip + GlHyVis + LaCdOth + LaCdAdip + TpCdVis ) * 0.5 ;
		GlTO = GlCdT + GlGyT + LaGlV1 + GLLmVis ;
		
		goto label_82 ;
	label_82:
		// end of procedural 
		dOx = ( LaCdOth + LaCdAdip ) * OxLaCd + TpCdVis * OxTpCd + PrCdVis * OxPrCd +
			BuCdVis * OxBuCd + AcCd * OxAcCd + FaCd * OxFaCd + GlCd * OxGlCd
			- TcHyAdip - TcHyVis ;
		THP2 = MEI - EB - NEP ;
		// AccMEi = integ ( MEI , 1.0E-8 ) 
		ME1 = MEI / FdDMIn ;
		fMCH4M = MCH4E / MEI ;
		// added 4/28/92 kcd
		
		// ObsPred=observed minus predicted;Mcal/kg feed
		
		ObsPredME = ObsME - MEI / FdDMIn ;
		// HEAT PRODUCTION AND BODY
		THP1 = MntHP + HiP ;
		MEMBW = MntHP / ( EBW *pow(1,1)* 0.7555 ) ;
		AtAdH1 = AtHt / AtAd ;
		CorME = CorMEi / FdDMIn ;
		BCH4Fd = BCH4 / FdDMIn ;
		
		fBCH4E = BCH4 / FdGEin ;
		fBCH4D = BCH4 / DEI ;
		fBCH4M = BCH4 / MEI ;
		
		// post processing calculations from dynamic
		// dynamic 
		// Determine if the calcualted slope for RumPartSize is positive and write a message
		// to the screen if it is.
		
		if ( RumPartSizeSlp > 0.0 ) {
			// CALL DISSTR(bufferslp); ! write the message
			// CALL DISSTR(' '); ! write a blank line
			// else if(RumPartSizeInt.gt.0.5) then
			// CALL DISSTR(bufferint); ! write the message
			// else
			// 	Continue
		}
		
		dNdfIn = TNdfIn - TNdfInYest ;
		dRumntn = TotRumntn - TotRumntnYest ;
		dRest = TotRest - TotRestYest ;
		dEating = TotEating - TotEatingYest ;
		dNep = tNep - tNepYest ;
		MilkProdDiel = TVolMilk - TVolMilkYest ; // Unit: kg. Cummulative milk produced since last midnight. See also dMilkProd
		dMilkVol = dMilkProd / MilkDen ;
		
		// !! Maintain some DAILY cummulatives (that is, like integ, but reset to zero every midnight).
		// !! Best for fitting intermittent potentially short term flows like FdRat, DrnkWa and WeUrine,
		// !! As fitting the momentary value may lose accuracy significantly. Fitting the "Total" integ
		// !! on the other hand has the prioblem of not knowing what the carry over of the pre-run is.
		FdRatDiel = IntakeTotal - IntakeYest ; // Unit: kgDM. Daily cummlative DM intake. Being reset to zero at midnight
		UrinationVolDiel = UrinationVol - UrinationVolYest ; // Unit: litre. Daily cummlative urination volume. Being reset to zero at midnight
		UrinationCountDiel = UrinationCount - UrinationCountYest ; // Unit: urinationEvents. Daily cummlative count. Being reset to zero at midnight
		DrnkWaDiel = DrnkWaTot - DrnkWaYest ; // Unit litre. Daily cummlative drinking volume. Being reset to zero at midnight
		
		// Include code to allow intermittent feeding of a supplement
		// BEGIN  Include 'Intermittent_Eating_Discrete.csl'	! Summarize meals
		
		// No intermittent eating for basic Molly
		// END  Include 'Intermittent_Eating_Discrete.csl'	! Summarize meals
		// BEGIN  Include 'Mindy_Discrete.csl'                ! Discrete include section for changing swards when Mindy intake submodel is used. MDH 6/1/2011
		
		// No Mindy code in this project
		// END  Include 'Mindy_Discrete.csl'                ! Discrete include section for changing swards when Mindy intake submodel is used. MDH 6/1/2011
		
		// end of dynamic // OF DYNAMIC
	
	} // end post_processing
	
	// called by boost::odeint::integrate()
	void operator()( const state_type &a_state , state_type &a_rate, double a_time ){
	
		// calculate rate
		t = a_time;
		set_state( a_state );
		calculate_rate();
		
		// return rate
		a_rate[0]= WaConsumed;
		a_rate[1]= WaUrine;
		a_rate[2]= FdRat;
		a_rate[3]= FCM4Z;
		a_rate[4]= DailyDMin;
		a_rate[5]= NdfinFd;
		a_rate[6]= Rumntn;
		a_rate[7]= Eating;
		a_rate[8]= Rest;
		a_rate[9]= dLPart;
		a_rate[10]= dMPart;
		a_rate[11]= dSPart;
		a_rate[12]= dHaMi;
		a_rate[13]= dHbMi;
		a_rate[14]= dHa;
		a_rate[15]= dHc;
		a_rate[16]= dCe;
		a_rate[17]= dHb;
		a_rate[18]= dPi;
		a_rate[19]= dIndigFd;
		a_rate[20]= dCs;
		a_rate[21]= dRumAa;
		a_rate[22]= dAm;
		a_rate[23]= dAs;
		a_rate[24]= dFl;
		a_rate[25]= dRumAc;
		a_rate[26]= dRumPr;
		a_rate[27]= dRumBu;
		a_rate[28]= dRumLa;
		a_rate[29]= dMi;
		a_rate[30]= FdGEin;
		a_rate[31]= DEI;
		a_rate[32]= MEI;
		a_rate[33]= dMamCellsA;
		a_rate[34]= dMamCellsQ;
		a_rate[35]= dMamCellsS;
		a_rate[36]= DailyMfDiff;
		a_rate[37]= max ( 0. , dWtAdipNew );
		a_rate[38]= GrowthPerDay;
		a_rate[39]= dLHor;
		a_rate[40]= derivMilkingFrequencyLag;
		a_rate[41]= dKMilkInh;
		a_rate[42]= dTsAdip;
		a_rate[43]= dFa;
		a_rate[44]= dVmAcTs;
		a_rate[45]= dAc;
		a_rate[46]= dMamTm;
		a_rate[47]= dMilkTm;
		a_rate[48]= dOthDna;
		a_rate[49]= dVisDna;
		a_rate[50]= dPVis;
		a_rate[51]= dPOth;
		a_rate[52]= dAa;
		a_rate[53]= dBldUr;
		a_rate[54]= dMamPm;
		a_rate[55]= dMilkPm;
		a_rate[56]= dGl;
		a_rate[57]= dMamLm;
		a_rate[58]= dMilkLm;
		a_rate[59]= dMamMilkAve;
		a_rate[60]= dabsEAve;
		a_rate[61]= NUr;
		a_rate[62]= NEP;
		a_rate[63]= dTCH4;
		a_rate[64]= BCH4;
		a_rate[65]= MCH4E;
	
	} // end operator
	
	int advance_model ( double end_time , double time_step ) {
	
		double next_time;
		static constexpr double eps = 0.00001;
		state_type a_state;
		double a_time;
		int nsteps = 0;
		
		while ( t < end_time ) {
		
			// do current events
			do {
				if ( event_list.begin() == event_list.end() ){
					// no events
					next_time = end_time + 1;
				} else if ( event_list.begin()->first < t - eps ) {
					// missed event
					event_list.erase( event_list.begin() );
					next_time = t - 1;
				} else if ( event_list.begin()->first < t + eps ) {
					do_event( event_list.begin()->second );
					event_list.erase( event_list.begin() );
					next_time = t - 1;
				} else {
					// next event
					next_time = event_list.begin()->first;
				}
			} while ( next_time < t + eps ) ;
		
			// advance model to next event or end_time
			a_state = get_state();
			a_time = t;
			next_time = std::min( end_time, next_time );
			// https://stackoverflow.com/questions/10976078/using-boostnumericodeint-inside-the-class
			nsteps += boost::numeric::odeint::integrate_const( stepper , *this , a_state, a_time , next_time , time_step );
			set_state( a_state );
			t = next_time;
		
		}
		
		calculate_rate();
		post_processing();
		return( nsteps );
	
	} // end advance_model

}; // end class


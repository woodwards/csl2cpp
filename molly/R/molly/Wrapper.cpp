// Wrapper.cpp : Implementation of CWrapper
#include "stdafx.h"
#include "Molly.h"
#include "Wrapper.h"

/////////////////////////////////////////////////////////////////////////////
// CWrapper

int gil = 0;

STDMETHODIMP CWrapper::Init()
{
	if (gil++ == 0) 
		AcslInitialize();
	simHandle = AcslCreate();

	return S_OK;
}


STDMETHODIMP CWrapper::Start()
{
	AcslStartBlocking(simHandle);
	return S_OK;
}



STDMETHODIMP CWrapper::Continue()
{
	AcslContinueBlocking(simHandle);
	return S_OK;
}

STDMETHODIMP CWrapper::ContinueNonBlocking()
{
	AcslContinue(simHandle);

	return S_OK;
}

STDMETHODIMP CWrapper::get_theValue(double *pVal)
{
	// Answer in pVal the value of the last variable requested 
	// (the one whose index is stored in <theVariableIndex>)
	// MUST FOLLOW setting the property theName!!!

	*pVal = theValue;
	return S_OK;
}

STDMETHODIMP CWrapper::put_theValue(double valueToSet)
{
	// Set the value of the last variable requested (the one whose 
	// index is stored in <theVariableIndex>) to <newVal>
	// MUST FOLLOW setting the property theName!!!

	AcslSetDouble(simHandle, theVariableIndex, valueToSet);
	return S_OK;
}


STDMETHODIMP CWrapper::put_theName(BSTR variableName)
{
	// Prepare for a get / set of a molly variable operation. Do it by
	// storing the index of the requested variable in theVariableIndex

	char theVar[100];
	for(int i=0; i<100 && variableName[i]!='\0'; i++)
      theVar[i] = (char) variableName[i];			  //wchar_t to char
	theVar[i] = '\0';								  //append the null terminator
	theVariableIndex = AcslGetSymbolIndex(theVar);

	theValue = AcslGetDouble(simHandle, theVariableIndex);
	theValueLong = AcslGetLong(simHandle, theVariableIndex);
	AcslGetSymbolInfo (theVariableIndex, &acslSymbolInfo); 
	if (isCurrentVariableA1DArray())
	{
		theValue = AcslGetDouble1 (simHandle, theVariableIndex, the1DarrayIndex);
	};
	return S_OK;									 //Sadly smalltalk always gets s_OK no matter what is being returned here
}

STDMETHODIMP CWrapper::get_theVariableIndex(long *pVal)
{
	// Currently used merely to see if the last patrameter requested exists
	// MUST BE USED AFTER setting theName to the name of the parameter
	// exists. (It exists if theVariableIndex is greater than zero)
	*pVal = theVariableIndex;
	return S_OK;
}


STDMETHODIMP CWrapper::get_TwoDimArrayElement(int i, int j, double *pVal)
{
	if (isCurrentVariableA2DArray())
	{
		*pVal = AcslGetDouble2 (simHandle, theVariableIndex, i, j);
		return S_OK;
	}
	else return S_FALSE;
}

STDMETHODIMP CWrapper::put_TwoDimArrayElement(int i, int j, double newVal)
{
	if (isCurrentVariableA2DArray())
	{
		AcslSetDouble2 (simHandle,theVariableIndex, newVal, i, j);
		return S_OK;
	}
	else return S_FALSE;
}



STDMETHODIMP CWrapper::setValueToName(BSTR theName, double theValue)
{
	// Prepare for a get / set of a molly variable operation. Do it by
	// storing the index of the requested variable in theVariableIndex

	char theVar[100];
	for(int i=0; i<100 && theName[i]!='\0'; i++)
      theVar[i] = (char) theName[i];			  //wchar_t to char
	theVar[i] = '\0';								  //append the null terminator
	theVariableIndex = AcslGetSymbolIndex(theVar);
	//theValue = AcslGetDouble(simHandle, theVariableIndex);
	AcslSetDouble(simHandle, theVariableIndex, theValue);

	return S_OK;
}

STDMETHODIMP CWrapper::get_getValueLong(long *pVal)
{
	*pVal = theValueLong;

	return S_OK;
}

STDMETHODIMP CWrapper::put_getValueLong(long newVal)
{
	theValueLong = newVal;

	return S_OK;
}

STDMETHODIMP CWrapper::get_OneDimArrayElement(int i, double *pVal)
{
//	if (isCurrentVariableA1DArray())
//	{
		*pVal = 9.99; // AcslGetDouble1 (simHandle, theVariableIndex, i);
		return S_OK;
//	}
//	else return S_FALSE;
}

STDMETHODIMP CWrapper::put_OneDimArrayElement(int i, double newVal) // Not working for some reason, done differently...
{
//	if (isCurrentVariableA1DArray())
//	{
		AcslSetDouble1 (simHandle,theVariableIndex, newVal, i);
		return S_OK;
//	}
//	else return S_FALSE;
}

STDMETHODIMP CWrapper::get_theIndex(long *pVal)
{
	*pVal = the1DarrayIndex;

	return S_OK;
}

STDMETHODIMP CWrapper::put_theIndex(long newVal)
{
	the1DarrayIndex = newVal;

	return S_OK;
}

STDMETHODIMP CWrapper::get_numDims(long *pVal)
{

	*pVal = acslSymbolInfo.numdims;
	return S_OK;
}

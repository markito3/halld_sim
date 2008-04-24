// $Id$
//
//    File: DFCALHit.h
// Created: Thu Jun  9 10:29:52 EDT 2005
// Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
//

#ifndef _DFCALHit_
#define _DFCALHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

class DFCALHit:public JObject{
	
public:
	
	JOBJECT_PUBLIC(DFCALHit);
	
	DFCALHit(){}
    
    float x;
    float y;
    float E;
    float t;

		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "x(cm)", "%3.1f", x);
			AddString(items, "y(cm)", "%3.1f", y);
			AddString(items, "E(MeV)", "%2.3f", E*1000.0);
			AddString(items, "t(ns)", "%2.3f", t);
		}
};

#endif // _DFCALHit_


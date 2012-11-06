// $Id$
//
//    File: Df250StreamingRawData.h
// Created: Tue Aug  7 15:25:15 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _Df250StreamingRawData_
#define _Df250StreamingRawData_

#include <JANA/jerror.h>
#include <JANA/JFactory.h>

class Df250StreamingRawData:public jana::JObject{
	
	/// Holds streaming raw data for one event in one
	/// channel of a single f250 Flash ADC module.
	/// This should represent a dump of the entire
	/// memory for the channel (~17 milliseconds worth
	/// of data) It will only be used for debugging.
	
	public:
		JOBJECT_PUBLIC(Df250StreamingRawData);
		
		uint32_t rocid;             // from EVIO header (crate number)
		uint32_t slot;              // from Block Header
		uint32_t channel;           // from Streaming Raw Data word
		vector<uint16_t> samples;   // from Streaming Raw Data words 2-N (each word contains 2 samples)
		bool invalid_samples;       // true if any sample's "not valid" bit set
		bool overflow;              // true if any sample's "overflow" bit set

		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "rocid", "%d", rocid);
			AddString(items, "slot", "%d", slot);
			AddString(items, "channel", "%d", channel);
			AddString(items, "Nsamples", "%d", samples.size());
			AddString(items, "invalid_samples", "%d", invalid_samples);
			AddString(items, "overflow", "%d", overflow);
		}

};

#endif // _Df250StreamingRawData_


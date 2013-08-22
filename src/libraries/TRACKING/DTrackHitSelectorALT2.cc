// $Id$
//
//    File: DTrackHitSelectorALT2.cc
// Created: Fri Feb  6 08:22:58 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#include <TROOT.h>

#include <TRACKING/DReferenceTrajectory.h>

#include "DTrackHitSelectorALT2.h"

#ifndef ansi_escape
#define ansi_escape			((char)0x1b)
#define ansi_bold 			ansi_escape<<"[1m"
#define ansi_normal			ansi_escape<<"[0m"
#define ansi_red			ansi_escape<<"[31m"
#define ansi_green			ansi_escape<<"[32m"
#define ansi_blue			ansi_escape<<"[34m"
#endif // ansi_escape

#define ONE_OVER_SQRT12  0.288675
#define ONE_OVER_12 0.08333333333333
#define EPS 1e-6

bool static DTrackHitSelector_cdchit_cmp(pair<double,const DCDCTrackHit *>a,
				      pair<double,const DCDCTrackHit *>b){
  if (a.second->wire->ring!=b.second->wire->ring) 
    return (a.second->wire->ring>b.second->wire->ring);
  return (a.first>b.first);
}
bool static DTrackHitSelector_fdchit_cmp(pair<double,const DFDCPseudo *>a,
				      pair<double,const DFDCPseudo *>b){
  if (a.second->wire->layer!=b.second->wire->layer) 
    return (a.second->wire->layer>b.second->wire->layer);
  return (a.first>b.first);
}


//---------------------------------
// DTrackHitSelectorALT2    (Constructor)
//---------------------------------
DTrackHitSelectorALT2::DTrackHitSelectorALT2(jana::JEventLoop *loop):DTrackHitSelector(loop)
{
	HS_DEBUG_LEVEL = 0;
	MAKE_DEBUG_TREES = false;
	MIN_HIT_PROB_CDC = 0.01;
	MIN_HIT_PROB_FDC = 0.01;
	MIN_FDC_SIGMA_ANODE_CANDIDATE = 0.1000;
	MIN_FDC_SIGMA_CATHODE_CANDIDATE = 0.1000;
	MIN_FDC_SIGMA_ANODE_WIREBASED = 0.0100;
	MIN_FDC_SIGMA_CATHODE_WIREBASED = 0.0100;
	MAX_DOCA=2.5;

	gPARMS->SetDefaultParameter("TRKFIT:MAX_DOCA",MAX_DOCA,"Maximum doca for associating hit with track");

	gPARMS->SetDefaultParameter("TRKFIT:HS_DEBUG_LEVEL", HS_DEBUG_LEVEL, "Debug verbosity level for hit selector used in track fitting (0=no debug messages)");
	gPARMS->SetDefaultParameter("TRKFIT:MAKE_DEBUG_TREES", MAKE_DEBUG_TREES, "Create a TTree with debugging info on hit selection for the FDC and CDC");
	gPARMS->SetDefaultParameter("TRKFIT:MIN_HIT_PROB_CDC", MIN_HIT_PROB_CDC, "Minimum probability a CDC hit may have to be associated with a track to be included in list passed to fitter");
	gPARMS->SetDefaultParameter("TRKFIT:MIN_HIT_PROB_FDC", MIN_HIT_PROB_FDC, "Minimum probability a FDC hit may have to be associated with a track to be included in list passed to fitter");
	gPARMS->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_ANODE_CANDIDATE", MIN_FDC_SIGMA_ANODE_CANDIDATE, "Minimum sigma used for FDC anode hits on track candidates");
	gPARMS->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_CATHODE_CANDIDATE", MIN_FDC_SIGMA_CATHODE_CANDIDATE, "Minimum sigma used for FDC cathode hits on track candidates");
	gPARMS->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_ANODE_WIREBASED", MIN_FDC_SIGMA_ANODE_WIREBASED, "Minimum sigma used for FDC anode hits on wire-based tracks");
	gPARMS->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_CATHODE_WIREBASED", MIN_FDC_SIGMA_CATHODE_WIREBASED, "Minimum sigma used for FDC cathode hits on wire-based tracks");
	
	cdchitsel = NULL;
	fdchitsel = NULL;
	if(MAKE_DEBUG_TREES){
		loop->GetJApplication()->Lock();
		
		cdchitsel= (TTree*)gROOT->FindObject("cdchitsel");
		if(!cdchitsel){
			cdchitsel = new TTree("cdchitsel", "CDC Hit Selector");
			cdchitsel->Branch("H", &cdchitdbg, "fit_type/I:p/F:theta:mass:sigma:mom_factor:x:y:z:s:s_factor:itheta02:itheta02s:itheta02s2:dist:doca:resi:sigma_total:chisq:prob:sig_phi:sig_lambda:sig_pt");
		}else{
			_DBG__;
			jerr<<" !!! WARNING !!!"<<endl;
			jerr<<"It appears that the cdchitsel TTree is already defined."<<endl;
			jerr<<"This is probably means you are running with multiple threads."<<endl;
			jerr<<"To avoid complication, filling of the hit selector debug"<<endl;
			jerr<<"trees will be disabled for this thread."<<endl;
			_DBG__;
			cdchitsel = NULL;
		}

		fdchitsel= (TTree*)gROOT->FindObject("fdchitsel");
		if(!fdchitsel){
			fdchitsel = new TTree("fdchitsel", "FDC Hit Selector");
			fdchitsel->Branch("H", &fdchitdbg, "fit_type/I:hit_cdc_endplate:p/F:theta:mass:sigma_anode:sigma_cathode:mom_factor_anode:mom_factor_cathode:x:y:z:s:s_factor_anode:s_factor_cathode:itheta02:itheta02s:itheta02s2:dist:doca:resi:u:u_cathodes:resic:sigma_anode_total:sigma_cathode_total:chisq:prob:prob_anode:prob_cathode:pull_anode:pull_cathode:sig_phi:sig_lambda:sig_pt");
		}else{
			_DBG__;
			jerr<<" !!! WARNING !!!"<<endl;
			jerr<<"It appears that the fdchitsel TTree is already defined."<<endl;
			jerr<<"This is probably means you are running with multiple threads."<<endl;
			jerr<<"To avoid complication, filling of the hit selector debug"<<endl;
			jerr<<"trees will be disabled for this thread."<<endl;
			_DBG__;
			fdchitsel = NULL;
		}

		loop->GetJApplication()->Unlock();
	}
	
	// Calibration constants
	correction_parms_t &cp0 = correction_parms[0]; // miss endplate, helical
	correction_parms_t &cp1 = correction_parms[1]; // miss endplate, wire-based
	correction_parms_t &cp2 = correction_parms[2]; // hit endplate, helical
	correction_parms_t &cp3 = correction_parms[3]; // hit endplate, wire-based
	
	// These values come from macros fitting single track data.
	// The macros have names like "s_factor_candidates_anodes.C"
	// This is temporary and these will eventually need to be moved
	// to the CCDB one the technique is proven.
	// 1/31/2011 DL
	cp0.s1_anode=1.61482;    cp0.s2_anode=0.572955;
	cp0.s1_cathode=1.59351;  cp0.s2_cathode=0.513831;
	
	cp1.s1_anode=1.35426;     cp1.s2_anode=0.594413;
	cp1.s1_cathode=0.188271;  cp1.s2_cathode=0.21525;
	
	cp2.s1_anode=0.108834;    cp2.s2_anode=0.373974;
	cp2.s1_cathode=0.257157;  cp2.s2_cathode=0.357933;
	
	cp3.s1_anode=-0.152481;    cp3.s2_anode=0.556112;
	cp3.s1_cathode=-0.856589;  cp3.s2_cathode=0.0705065;

	DApplication* dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
        bfield = dapp->GetBfield(); // this should be run number based!
	
}

//---------------------------------
// ~DTrackHitSelectorALT2    (Destructor)
//---------------------------------
DTrackHitSelectorALT2::~DTrackHitSelectorALT2()
{

}

//---------------------------------
// GetCDCHits
//---------------------------------
void DTrackHitSelectorALT2::GetCDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out, int N) const
{
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DCDCTrackHit*> >cdchits_tmp;

  /// Determine the probability that for each CDC hit that it came from the 
  /// track with the given trajectory.
  ///
  /// This will calculate a probability for each CDC hit that
  /// it came from the track represented by the given
  /// DReference trajectory. The probability is based on
  /// the residual between the distance of closest approach
  /// of the trajectory to the wire and the drift time for
  /// time-based tracks and the distance to the wire for
  /// wire-based tracks.
  
  // Calculate beta of particle.
  //double my_mass=rt->GetMass();
  //  double one_over_beta =sqrt(1.0+my_mass*my_mass/rt->swim_steps[0].mom.Mag2());
  
  // The variance on the residual due to measurement error.
  double var=0.64*ONE_OVER_12;
   
  // To estimate the impact of errors in the track momentum on the variance of the residual,
  // use a helical approximation.
  //DVector3 origin=cdchits_in[0]->wire->origin;
  DVector3 origin=rt->swim_steps[0].origin;
  double Bz=bfield->GetBz(origin.X(),origin.Y(),origin.z());
  double a=-0.003*Bz*rt->q;
  double p=rt->swim_steps[0].mom.Mag();
  double p_over_a=p/a;
  double a_over_p=1./p_over_a;
  double lambda=M_PI_2-rt->swim_steps[0].mom.Theta();
  double cosl=cos(lambda);
  double sinl=sin(lambda);
  //double sinl2=sinl*sinl;
  double cosl2=cosl*cosl;
  double tanl=tan(lambda);
  double tanl2=tanl*tanl;
  double pt_over_a=cosl*p_over_a;
  double phi=rt->swim_steps[0].mom.Phi();
  double cosphi=cos(phi);
  double sinphi=sin(phi);
  
  double var_lambda_res=0.;
  double var_x0=0.01,var_y0=0.01;
  double mass=rt->GetMass();
  double one_over_beta=sqrt(1.+mass*mass/(p*p));
  double var_pt_factor=0.016*one_over_beta/(cosl*a);
  double var_pt_over_pt_sq=0.;

  // Keep track of straws and rings
  int old_straw=1000,old_ring=1000;

  // Loop over hits
  bool outermost_hit=true;
  vector<const DCDCTrackHit*>::const_reverse_iterator iter;
  for(iter=cdchits_in.rbegin(); iter!=cdchits_in.rend(); iter++){
    const DCDCTrackHit *hit = *iter;
    
    // Skip hit if it is on the same wire as the previous hit
    if (hit->wire->ring == old_ring && hit->wire->straw==old_straw){
      //_DBG_ << "ring " << hit->wire->ring << " straw " << hit->wire->straw << endl;
      continue;
    }
    old_ring=hit->wire->ring;
    old_straw=hit->wire->straw;

    // Find the DOCA to this wire
    double s;
    double doca = rt->DistToRT(hit->wire, &s);
    
    if(!finite(doca)) continue;
    if(!finite(s))continue;
    if (doca>MAX_DOCA) continue;
    
    const DReferenceTrajectory::swim_step_t *last_step = rt->GetLastSwimStep();
    
    // Compensate for the fact that the field at the "origin" of the 
    // track does not correspond to the average Bz used to compute pt
    double Bz_hit=bfield->GetBz(last_step->origin.x(),last_step->origin.y(),
				last_step->origin.z());
    double Bratio=Bz_hit/Bz;
    double invBratio=1./Bratio;
    pt_over_a*=invBratio;
    p_over_a*=invBratio;
    a_over_p*=Bratio;
  
    // Variances in angles due to multiple scattering
    double var_lambda = last_step->itheta02;
    double var_phi=var_lambda*(1.+tanl2);

    // Include uncertainty in phi due to uncertainty in the center of 
    // the circle
    var_phi+=0.09/(pt_over_a*pt_over_a);

    if (outermost_hit){   
      // Fractional variance in the curvature k due to resolution and multiple scattering
      double s_sq=s*s;
      double var_k_over_k_sq_res=var*p_over_a*p_over_a*0.0720/double(N+4)/(s_sq*s_sq)/cosl2;
      double var_k_over_k_sq_ms=var_pt_factor*var_pt_factor*last_step->invX0/s;
      // Fractional variance in pt
      var_pt_over_pt_sq=var_k_over_k_sq_ms+var_k_over_k_sq_res;
      
      // Variance in dip angle due to measurement error
      var_lambda_res=12.0*var*double(N-1)/double(N*(N+1))*cosl2*cosl2/s_sq;

      outermost_hit=false;
    }
    // Include error in lambda due to measurements
    var_lambda+=var_lambda_res;

    // Get "measured" distance to wire. 
    // For matching purposes this is assumed to be half a cell size
    double dist=0.4;
    
    // Residual
    double resi = dist - doca;

    // Variance in position due to multiple scattering
    double var_pos_ms=last_step->itheta02s2/48.;
	    
    // Variances in x and y due to uncertainty in track parameters
    double as_over_p=s*a_over_p;
    double sin_as_over_p=sin(as_over_p);
    double cos_as_over_p=cos(as_over_p);
    double one_minus_cos_as_over_p=1-cos_as_over_p;
    double diff1=sin_as_over_p-as_over_p*cos_as_over_p;
    double diff2=one_minus_cos_as_over_p-as_over_p*sin_as_over_p;
    double pdx_dp=pt_over_a*(cosphi*diff1-sinphi*diff2);
    double dx_dcosl=p_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
    double dx_dphi=-pt_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
    double var_x=var_x0+pdx_dp*pdx_dp*var_pt_over_pt_sq+var_pos_ms
      +dx_dcosl*dx_dcosl*sinl*sinl*var_lambda+dx_dphi*dx_dphi*var_phi;
    
    double pdy_dp=pt_over_a*(sinphi*diff1+cosphi*diff2);
    double dy_dcosl=p_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
    double dy_dphi=pt_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
    double var_y=var_y0+pdy_dp*pdy_dp*var_pt_over_pt_sq+var_pos_ms
      +dy_dcosl*dy_dcosl*sinl*sinl*var_lambda+dy_dphi*dy_dphi*var_phi;
    
    
    DVector3 origin=hit->wire->origin;
    DVector3 dir=hit->wire->udir;
    double uz=dir.z();
    double z0=origin.z();
    DVector3 wirepos=origin+(last_step->origin.z()-z0)/uz*dir;
    double dd_dx=(last_step->origin.x()-wirepos.x())/doca;
    double dd_dy=(last_step->origin.y()-wirepos.y())/doca;
    double var_d=dd_dx*dd_dx*var_x+dd_dy*dd_dy*var_y;

    double chisq=resi*resi/(var+var_d);
    
    // Use chi-sq probability function with Ndof=1 to calculate probability
    double probability = TMath::Prob(chisq, 1);
    if(probability>=MIN_HIT_PROB_CDC){
      pair<double,const DCDCTrackHit*>myhit;
      myhit.first=probability;
      myhit.second=hit;
      cdchits_tmp.push_back(myhit);
    }

    // Optionally fill debug tree
    if(cdchitsel){
      DVector3 pos = rt->GetLastDOCAPoint();
      
      cdchitdbg.fit_type = fit_type;
      cdchitdbg.p = p;
      cdchitdbg.theta = rt->swim_steps[0].mom.Theta();
      //	cdchitdbg.mass = my_mass;
      cdchitdbg.sigma = sqrt(var);
      // cdchitdbg.mom_factor = mom_factor;
      cdchitdbg.x = pos.X();
      cdchitdbg.y = pos.Y();
      cdchitdbg.z = pos.Z();
      cdchitdbg.s = s;
      // cdchitdbg.s_factor = s_factor;
      //cdchitdbg.itheta02 = last_step->itheta02;
      //cdchitdbg.itheta02s = last_step->itheta02s;
      //cdchitdbg.itheta02s2 = last_step->itheta02s2;
      cdchitdbg.dist = dist;
      cdchitdbg.doca = doca;
      cdchitdbg.resi = resi;
      // cdchitdbg.sigma_total = sigma_total;
      cdchitdbg.chisq = chisq;
      cdchitdbg.prob = probability;
      cdchitdbg.sig_phi=sqrt(var_phi);
      cdchitdbg.sig_lambda=sqrt(var_lambda);
      cdchitdbg.sig_pt=sqrt(var_pt_over_pt_sq);	

      cdchitsel->Fill();
      
      static bool printed_first = false;
      if(!printed_first){
	_DBG_<<"=== Printing first entry for CDC hit selector debug tree ==="<<endl;
	_DBG_<<"   fit_type = "<<cdchitdbg.fit_type<<endl;
	_DBG_<<"          p = "<<cdchitdbg.p<<endl;
	_DBG_<<"      theta = "<<cdchitdbg.theta<<endl;
	//_DBG_<<"       mass = "<<cdchitdbg.mass<<endl;
	_DBG_<<"      sigma = "<<cdchitdbg.sigma<<endl;
	//_DBG_<<" mom_factor = "<<cdchitdbg.mom_factor<<endl;
	_DBG_<<"          x = "<<cdchitdbg.x<<endl;
	_DBG_<<"          y = "<<cdchitdbg.y<<endl;
	_DBG_<<"          z = "<<cdchitdbg.z<<endl;
	_DBG_<<"          s = "<<cdchitdbg.s<<endl;
	//  _DBG_<<"   s_factor = "<<cdchitdbg.s_factor<<endl;
	//_DBG_<<"   itheta02 = "<<cdchitdbg.itheta02<<endl;
	//_DBG_<<"  itheta02s = "<<cdchitdbg.itheta02s<<endl;
	//_DBG_<<" itheta02s2 = "<<cdchitdbg.itheta02s2<<endl;
	_DBG_<<"       dist = "<<cdchitdbg.dist<<endl;
	_DBG_<<"       doca = "<<cdchitdbg.doca<<endl;
	_DBG_<<"       resi = "<<cdchitdbg.resi<<endl;
	//_DBG_<<"sigma_total = "<<cdchitdbg.sigma_total<<endl;
	_DBG_<<"      chisq = "<<cdchitdbg.chisq<<endl;
	_DBG_<<"       prob = "<<cdchitdbg.prob<<endl;
	
	printed_first = true;
      }
    }
    
    if(HS_DEBUG_LEVEL>10){
      _DBG_;
      if (probability>=MIN_HIT_PROB_CDC) jerr<<"---> ";
      jerr<<"s="<<s<<" t=" <<hit->tdrift   << " doca="<<doca<<" dist="<<dist<<" resi="<<resi<<" sigma="/*<<sigma_total*/<<" prob="<<probability<<endl;
    }
  }

  // Order according to ring number and probability, then put the hits in the 
  // output list with the following algorithm:  hits with the highest 
  // probability in a given ring are automatically put in the output list, 
  // but if there is more than one hit in a given ring, only those hits 
  // that are within +/-1 of the straw # of the most probable hit are added 
  // to the list.
  sort(cdchits_tmp.begin(),cdchits_tmp.end(),DTrackHitSelector_cdchit_cmp);
  old_straw=1000,old_ring=1000;
  for (unsigned int i=0;i<cdchits_tmp.size();i++){
    if (cdchits_tmp[i].second->wire->ring!=old_ring || 
	abs(cdchits_tmp[i].second->wire->straw-old_straw)==1){
      cdchits_out.push_back(cdchits_tmp[i].second);   
    }
    old_straw=cdchits_tmp[i].second->wire->straw;
    old_ring=cdchits_tmp[i].second->wire->ring;
  }
}

//---------------------------------
// GetFDCHits
//---------------------------------
void DTrackHitSelectorALT2::GetFDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out,int N) const
{
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DFDCPseudo*> >fdchits_tmp;
  
  /// Determine the probability that for each FDC hit that it came from the 
  /// track with the given trajectory.
  ///
  /// This will calculate a probability for each FDC hit that
  /// it came from the track represented by the given
  /// DReference trajectory. The probability is based on
  /// the residual between the distance of closest approach
  /// of the trajectory to the wire and the drift distance
  /// and the distance along the wire.
  
  // The variance on the residual due to measurement error.
  // For the kHelical case the distribution of docas to the wire is roughly
  // triangular (i.e., docas close to the wire are favored.)
  double var_anode = (fit_type==kHelical)?0.25/18.:0.25*ONE_OVER_12; // scale factor reflects field-sense wire separation
  double var_cathode =(fit_type==kHelical)?0.1024*ONE_OVER_12:0.0004; // scale factor reflects maximum lorentz deflection if fit_type=kHelical
      
  // To estimate the impact of errors in the track momentum on the variance of the residual,
  // use a helical approximation. 
  DVector3 origin=rt->swim_steps[0].origin;
  double z0=origin.z();
  double Bz=bfield->GetBz(origin.X(),origin.Y(),z0);
  double a=-0.003*Bz*rt->q;
  double p=rt->swim_steps[0].mom.Mag();
  double p_sq=p*p;
  double p_over_a=p/a;
  double a_over_p=1./p_over_a;
  double lambda=M_PI_2-rt->swim_steps[0].mom.Theta();
  double cosl=cos(lambda);
  double cosl2=cosl*cosl;
  double sinl=sin(lambda);
  double sinl2=sinl*sinl;
  double tanl=tan(lambda);
  double tanl2=tanl*tanl;
  double pt_over_a=cosl*p_over_a;
  double phi=rt->swim_steps[0].mom.Phi();
  double cosphi=cos(phi);
  double sinphi=sin(phi);
  double var_lambda=0.,var_phi=0.,var_lambda_res=0.;
      
  double var_z0=2.*tanl2*var_cathode*double(2*N-1)/double(N*(N+1));
  double var_x0=0.0,var_y0=0.0; 
  double var_pt_over_pt_sq=0.;
  if (fit_type==kHelical) var_x0=var_y0=0.01; // guess
  
  // Loop over hits
  bool most_downstream_hit=true;
  vector<const DFDCPseudo*>::const_reverse_iterator iter;
  for(iter=fdchits_in.rbegin(); iter!=fdchits_in.rend(); iter++){
    const DFDCPseudo *hit = *iter;
    
    // Find the DOCA to this wire
    double s;
    double doca = rt->DistToRT(hit->wire, &s); 

    if(!finite(doca)) continue; 
    if(!finite(s))continue;
    if (doca>MAX_DOCA)continue;

    const DReferenceTrajectory::swim_step_t *last_step = rt->GetLastSwimStep();
    // Position along trajectory
    double x=last_step->origin.x();
    double y=last_step->origin.y();
    double z=last_step->origin.z();

    // Direction variables for wire
    double cosa=hit->wire->udir.y();
    double sina=hit->wire->udir.x();

    // Cathode Residual
    double u=rt->GetLastDistAlongWire();
    double u_cathodes = hit->s;
    double resic = u - u_cathodes;
    
    // Get "measured" distance to wire.
    // For matching purposes this is assumed to be half a cell size
    double dist=0.25;
    
    // Take into account non-normal incidence to FDC plane
    double pz=last_step->mom.z();
    double tx=last_step->mom.x()/pz;
    double ty=last_step->mom.y()/pz;
    double tu=tx*cosa-ty*sina;
    double alpha=atan(tu);
    double cosalpha=cos(alpha);

    // Anode Residual
    double resi = dist - doca/cosalpha;

    // Initialize some probability-related variables needed later
    double probability=0.,chisq=0.;
 
    if (fit_type==kWireBased){
      // Correct for deflection of avalanche position due to Lorentz force
      double Bx,By,Bz;
      bfield->GetField(x,y,z,Bx,By,Bz);
      double sign=(x*cosa-y*sina-hit->w)<0?1:-1.;	
      double ucor=0.153*Bz*(1.-0.048*sqrt(Bx*Bx+By*By))*sign*doca;
      resic-=ucor;

      // Calculate chisq
      chisq = resi*resi/var_anode+resic*resic/var_cathode;
      
      // Probability of this hit being on the track
      probability = TMath::Prob(chisq,2);

    }

    // Deal with the kHelical case
    if (fit_type==kHelical){     
      double dz=z-z0;
      
      // The following is a correction to take into account that for the 
      // kHelical case, empirically there is a bias of the doca toward the 
      // wire...
      resi+=0.25/3.;

      // Compensate for the fact that the field at the "origin" of the 
      // track does not correspond to the average Bz used to compute pt
      double Bz_hit=bfield->GetBz(x,y,z);
      double Bratio=Bz_hit/Bz;
      double invBratio=1./Bratio;
      pt_over_a*=invBratio;
      p_over_a*=invBratio;
      a_over_p*=Bratio;
      
      // Variance in angles due to multiple scattering
      var_lambda = last_step->itheta02;
      var_phi=var_lambda*(1.+tanl2);
      
      // Include uncertainty in phi due to uncertainty in the center of the 
      // circle. 
      double var_xc=0.15*(1.+0.0005/(cosl2*cosl2+EPS));
      var_phi+=var_xc/(pt_over_a*pt_over_a);

      if (most_downstream_hit){
	// Fractional variance in the curvature k due to resolution and multiple scattering
	double s_sq=s*s;
	double var_k_over_k_sq_res=var_cathode*p_over_a*p_over_a
	  *0.0720/double(N+4)/(s_sq*s_sq)/cosl2;
	
	double mass=rt->GetMass();
	double one_over_beta=sqrt(1.+mass*mass/p_sq);
	double var_pt_factor=0.016*one_over_beta/(cosl*0.003*Bz_hit);
	double var_k_over_k_sq_ms=var_pt_factor*var_pt_factor*last_step->invX0/s;

	// Fractional variance in pt
	var_pt_over_pt_sq=var_k_over_k_sq_ms+var_k_over_k_sq_res;
	
	// Variance in dip angle due to measurement error	      
	var_lambda_res=12.0*var_cathode*double(N-1)/double(N*(N+1))
	  *sinl2*sinl2/s_sq;
      
	most_downstream_hit=false;
      }
      
      // Include error in lambda due to measurements
      var_lambda+=var_lambda_res;
      
      // Variance in position due to multiple scattering
      double var_pos_ms=last_step->itheta02s2/48.;
	     
      // Variances in x and y due to uncertainty in track parameters
      double as_over_p=s*a_over_p;
      double sin_as_over_p=sin(as_over_p);
      double cos_as_over_p=cos(as_over_p);
      double one_minus_cos_as_over_p=1-cos_as_over_p;
      double diff1=sin_as_over_p-as_over_p*cos_as_over_p;
      double diff2=one_minus_cos_as_over_p-as_over_p*sin_as_over_p;
      double pdx_dp=pt_over_a*(cosphi*diff1-sinphi*diff2);
      double dx_ds=cosl*(cosphi*cos_as_over_p-sinphi*sin_as_over_p);
      double ds_dcosl=dz*cosl/(sinl*sinl2);
      double dx_dcosl
	=p_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p)
	+dx_ds*ds_dcosl;
      double dx_dphi=-pt_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
      double var_x=var_x0+pdx_dp*pdx_dp*var_pt_over_pt_sq+var_pos_ms
	+dx_dcosl*dx_dcosl*sinl2*var_lambda+dx_dphi*dx_dphi*var_phi
	+dx_ds*dx_ds*var_z0/sinl2;
      
      double pdy_dp=pt_over_a*(sinphi*diff1+cosphi*diff2);
      double dy_ds=cosl*(sinphi*cos_as_over_p+cosphi*sin_as_over_p);
      double dy_dcosl
	=p_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p)
	+dy_ds*ds_dcosl;
      double dy_dphi=pt_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
      double var_y=var_y0+pdy_dp*pdy_dp*var_pt_over_pt_sq+var_pos_ms
	+dy_dcosl*dy_dcosl*sinl2*var_lambda+dy_dphi*dy_dphi*var_phi
	+dy_ds*dy_ds*var_z0/sinl2;
      
      // The following code attempts to address issues with dealing with very 
      // shallow tracks
      /*
	if (fit_type==kHelical){
	double var_empirical=1e-7*s*s/cosl2;
	var_y+=var_empirical;
	var_x+=var_empirical;
	}
      */
      
      // Rotate from global coordinate system into FDC local system
      double cos2a=cosa*cosa;
      double sin2a=sina*sina;
      double var_d=(cos2a*var_x+sin2a*var_y)/(cosalpha*cosalpha);
      double var_u=cos2a*var_y+sin2a*var_x;
    
      // Calculate chisq
      chisq = resi*resi/(var_d+var_anode)+resic*resic/(var_u+var_cathode);

      //chisq=resic*resic/(var_u+var_cathode);
      //chisq = resi*resi/(var_d+var_anode);
      
      // Probability of this hit being on the track
      probability = TMath::Prob(chisq,2);
    }


    if(probability>=MIN_HIT_PROB_FDC){
      pair<double,const DFDCPseudo*>myhit;
      myhit.first=probability;
      myhit.second=hit;
      fdchits_tmp.push_back(myhit);
    }
    
    // Optionally fill debug tree
    if(fdchitsel){
      DVector3 pos = rt->GetLastDOCAPoint();
		
      fdchitdbg.fit_type = fit_type;
      //fdchitdbg.hit_cdc_endplate = hit_cdc_endplate;
      fdchitdbg.p = p;
      fdchitdbg.theta = rt->swim_steps[0].mom.Theta();
      //fdchitdbg.mass = my_mass;
      fdchitdbg.sigma_anode = sqrt(var_anode);
      fdchitdbg.sigma_cathode = sqrt(var_cathode);
      //fdchitdbg.mom_factor_anode = mom_factor_anode;
      //fdchitdbg.mom_factor_cathode = mom_factor_cathode;
      fdchitdbg.x = pos.X();
      fdchitdbg.y = pos.Y();
      fdchitdbg.z = pos.Z();
      fdchitdbg.s = s;
      //fdchitdbg.s_factor_anode = s_factor_anode;
      //fdchitdbg.s_factor_cathode = s_factor_cathode;
      fdchitdbg.itheta02 = last_step->itheta02;
      fdchitdbg.itheta02s = last_step->itheta02s;
      fdchitdbg.itheta02s2 = last_step->itheta02s2;
      fdchitdbg.dist = dist;
      fdchitdbg.doca = doca;
      fdchitdbg.resi = resi;
      fdchitdbg.u = u;
      fdchitdbg.u_cathodes = u_cathodes;
      fdchitdbg.resic = resic;
      //fdchitdbg.sigma_anode_total = sigma_anode_total;
      //fdchitdbg.sigma_cathode_total = sigma_cathode_total;
      fdchitdbg.chisq = chisq;
      fdchitdbg.prob = probability;
      //double pull_anode = resi/sigma_anode_total;
      //double pull_cathode = resic/sigma_cathode_total;
      //fdchitdbg.prob_anode = TMath::Prob(pull_anode*pull_anode, 1);
      //  fdchitdbg.prob_cathode = TMath::Prob(pull_cathode*pull_cathode, 1);
      //fdchitdbg.pull_anode = pull_anode;
      //fdchitdbg.pull_cathode = pull_cathode;
      fdchitdbg.sig_phi=sqrt(var_phi);
      fdchitdbg.sig_lambda=sqrt(var_lambda);
      fdchitdbg.sig_pt=sqrt(var_pt_over_pt_sq);
      
      fdchitsel->Fill();
    }
    
    if(HS_DEBUG_LEVEL>10){
      _DBG_;
      if(probability>=MIN_HIT_PROB_FDC)jerr<<"----> ";
      jerr<<"s="<<s<<" doca="<<doca<<" dist="<<dist<<" resi="<<resi<<" resic="<<resic<<" chisq="<<chisq<<" prob="<<probability<<endl;
    }
  }
  // Order according to layer number and probability,then put the hits in the 
  // output list with the following algorithm:  hits with the highest 
  // probability in a given layer are automatically put in the output list, 
  // but if there is more than one hit in a given layer, only those hits 
  // that are within +/-1 of the wire # of the most probable hit are added 
  // to the list.
  sort(fdchits_tmp.begin(),fdchits_tmp.end(),DTrackHitSelector_fdchit_cmp);
  int old_layer=1000,old_wire=1000;
  for (unsigned int i=0;i<fdchits_tmp.size();i++){
    if (fdchits_tmp[i].second->wire->layer!=old_layer || 
	abs(fdchits_tmp[i].second->wire->wire-old_wire)==1){
      fdchits_out.push_back(fdchits_tmp[i].second);   
    }
    old_wire=fdchits_tmp[i].second->wire->wire;
    old_layer=fdchits_tmp[i].second->wire->layer;
  }
}

#if !defined(BREITWIGNER)
#define BREITWIGNER

#include "IUAmpTools/Amplitude.h"
#include "IUAmpTools/AmpParameter.h"
#include "GPUManager/GPUCustomTypes.h"

#include <utility>
#include <string>
#include <complex>
#include <vector>

#ifdef GPU_ACCELERATION
void GPUBreitWigner_exec( dim3 dimGrid, dim3 dimBlock, GPU_AMP_PROTO,
                          GDouble mass0, GDouble width0, int orbitL,
                          int daught1, int daught2 );

#endif // GPU_ACCELERATION

using std::complex;
using namespace std;

class Kinematics;

class BreitWigner : public Amplitude
{
  
public:
	
	BreitWigner() : Amplitude() { setDefaultStatus( true ); }
	BreitWigner( const AmpParameter& mass0, 
               const AmpParameter& width0, int orbitL, 
               pair<string,string> daughters ); 
	
  ~BreitWigner(){}
  
	string name() const { return "BreitWigner"; }
  
  complex< GDouble > calcAmplitude( GDouble** pKin ) const;
	  
  void updatePar( const AmpParameter& par );
  
	BreitWigner* newAmplitude( const vector< string >& args ) const;
	BreitWigner* clone() const;
  
#ifdef GPU_ACCELERATION

  void launchGPUKernel( dim3 dimGrid, dim3 dimBlock, GPU_AMP_PROTO ) const;

	bool isGPUEnabled() const { return true; }

#endif // GPU_ACCELERATION
  
private:
	
  AmpParameter m_mass0;
  AmpParameter m_width0;
  int m_orbitL;
  
  pair< string, string > m_daughters;  
};

#endif

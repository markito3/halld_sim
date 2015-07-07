
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#include "TLorentzVector.h"
#include "TLorentzRotation.h"

#include "IUAmpTools/Kinematics.h"
#include "AMPTOOLS_AMPS/Pi0SAID.h"

Pi0SAID::Pi0SAID( const vector< string >& args ) :
UserAmplitude< Pi0SAID >( args )
{
	assert( args.size() == 1 );
	Pgamma = atof( args[0].c_str() );

	FillDataTables();

	hCosTheta_Ebeam = new TH2F("hCosTheta_Ebeam","; E_{#gamma}; cos#theta; d#sigma/dcos#theta", 31, 1.475, 3.025, 41, -1.025, 1.025);
	hSigma_Ebeam = new TH2F("hSigma_Ebeam","; E_{#gamma}; cos#theta; #Sigma", 31, 1.475, 3.025, 41, -1.025, 1.025);

	for(int i=0; i<hCosTheta_Ebeam->GetXaxis()->GetNbins(); i++){
		for(int j=0; j<hCosTheta_Ebeam->GetYaxis()->GetNbins(); j++){
			hCosTheta_Ebeam->SetBinContent(i+1,j+1,DSG[i][j]);
			hSigma_Ebeam->SetBinContent(i+1,j+1,Sigma[i][j]);
		}
	}		
}


complex< GDouble >
Pi0SAID::calcAmplitude( GDouble** pKin ) const {
  
	TLorentzVector target  ( 0., 0., 0., 0.938);
	TLorentzVector beam   ( pKin[0][1], pKin[0][2], pKin[0][3], pKin[0][0] ); 
	TLorentzVector recoil ( pKin[1][1], pKin[1][2], pKin[1][3], pKin[1][0] ); 
	TLorentzVector p1     ( pKin[2][1], pKin[2][2], pKin[2][3], pKin[2][0] ); 
	
	TLorentzVector cm = recoil + p1;
	TLorentzRotation cmBoost( -cm.BoostVector() );
	
	TLorentzVector target_cm = cmBoost * target;
	TLorentzVector recoil_cm = cmBoost * recoil;
	
	TLorentzVector p1_cm = cmBoost * p1;
	GDouble phi = p1_cm.Phi();
	GDouble cosTheta = p1_cm.CosTheta();
	GDouble cos2Phi = cos(2.*phi);
	GDouble Eg = beam.E();

	int bin = hCosTheta_Ebeam->FindBin(Eg, cosTheta);
	GDouble DSG = hCosTheta_Ebeam->GetBinContent(bin);
	GDouble Sigma = hSigma_Ebeam->GetBinContent(bin);
	
	// weighted cross section from Igor Strakovsky (GWU/SAID collaboration)
	GDouble W = DSG * (1 - Pgamma * Sigma * cos2Phi);

	return complex< GDouble > ( sqrt(W) );
}

// select proper index for given Eg and CosTheta
void Pi0SAID::FillDataTables() {
	
	// Fill DSG data tables
	double DSG1500[41] = {1.1446, 1.232, 1.2297, 1.1623, 1.0535, 0.9252, 0.7962, 0.6814, 0.5913, 0.5325, 0.5073, 0.5145, 0.5499, 0.607, 0.6777, 0.7533, 0.8248, 0.8838, 0.9233, 0.9378, 0.9238, 0.8806, 0.8098, 0.7158, 0.6057, 0.489, 0.3773, 0.284, 0.2233, 0.2094, 0.2551, 0.3705, 0.5609, 0.824, 1.1475, 1.5043, 1.8486, 2.11, 2.1861, 1.9348, 1.1637};
        for(int i=0; i<41; i++) {
               DSG[0][i] = DSG1500[i];
        }
        double DSG1550[41] = {0.8826, 0.9824, 1.0043, 0.9649, 0.8834, 0.7797, 0.6718, 0.575, 0.5008, 0.4563, 0.4443, 0.4639, 0.5108, 0.578, 0.6569, 0.7382, 0.8125, 0.8712, 0.9072, 0.9155, 0.8934, 0.841, 0.7611, 0.6592, 0.5434, 0.424, 0.3129, 0.223, 0.1677, 0.1595, 0.2093, 0.3246, 0.5084, 0.7566, 1.0563, 1.3821, 1.693, 1.9278, 1.9995, 1.7886, 1.135};
        for(int i=0; i<41; i++) {
               DSG[1][i] = DSG1550[i];
        }
        double DSG1600[41] = {0.6698, 0.7561, 0.7867, 0.7681, 0.7124, 0.6346, 0.5503, 0.4737, 0.4157, 0.384, 0.3818, 0.4086, 0.4605, 0.5309, 0.6114, 0.6927, 0.7656, 0.8218, 0.8546, 0.8593, 0.8339, 0.779, 0.6982, 0.5974, 0.4849, 0.3709, 0.267, 0.185, 0.137, 0.1336, 0.1836, 0.2925, 0.4616, 0.6858, 0.9527, 1.2396, 1.5111, 1.7155, 1.7803, 1.6063, 1.0607};
        for(int i=0; i<41; i++) {
               DSG[2][i] = DSG1600[i];
        }
        double DSG1650[41] = {0.5092, 0.5674, 0.5954, 0.5905, 0.5569, 0.5038, 0.443, 0.3865, 0.3444, 0.3239, 0.3289, 0.3593, 0.4119, 0.4808, 0.5583, 0.6357, 0.7044, 0.7565, 0.7859, 0.7884, 0.7623, 0.7088, 0.6315, 0.5365, 0.4318, 0.3272, 0.2334, 0.1611, 0.1209, 0.1218, 0.1708, 0.272, 0.4254, 0.626, 0.862, 1.1138, 1.3508, 1.5289, 1.5865, 1.4385, 0.9697};
        for(int i=0; i<41; i++) {
               DSG[3][i] = DSG1650[i];
        }
        double DSG1700[41] = {0.3961, 0.4234, 0.4431, 0.4459, 0.4291, 0.3964, 0.3555, 0.3158, 0.2863, 0.2738, 0.2823, 0.3125, 0.3621, 0.4261, 0.4977, 0.569, 0.6322, 0.6801, 0.7068, 0.7088, 0.6847, 0.6355, 0.5649, 0.4787, 0.3844, 0.291, 0.2083, 0.1457, 0.1125, 0.1164, 0.1631, 0.2557, 0.3938, 0.5726, 0.782, 1.0047, 1.2144, 1.3723, 1.4238, 1.2916, 0.8687};
        for(int i=0; i<41; i++) {
               DSG[4][i] = DSG1700[i];
        }
        double DSG1750[41] = {0.3116, 0.3125, 0.322, 0.3281, 0.3237, 0.3076, 0.2836, 0.2586, 0.24, 0.2339, 0.2447, 0.2735, 0.3188, 0.3768, 0.4415, 0.5061, 0.5633, 0.6068, 0.6313, 0.6333, 0.6119, 0.5679, 0.5049, 0.4281, 0.3447, 0.2626, 0.1905, 0.1371, 0.1102, 0.1164, 0.1607, 0.2455, 0.3705, 0.5315, 0.7198, 0.9204, 1.1102, 1.2541, 1.301, 1.1766, 0.7744};
        for(int i=0; i<41; i++) {
               DSG[5][i] = DSG1750[i];
        }
        double DSG1800[41] = {0.248, 0.2322, 0.2323, 0.2388, 0.2422, 0.2379, 0.2265, 0.2124, 0.2015, 0.1994, 0.2103, 0.236, 0.276, 0.327, 0.3842, 0.4417, 0.4931, 0.5325, 0.5552, 0.5582, 0.5403, 0.5025, 0.4478, 0.3812, 0.3089, 0.2381, 0.1766, 0.1318, 0.1105, 0.1186, 0.1602, 0.2377, 0.3507, 0.4962, 0.667, 0.8501, 1.025, 1.1591, 1.2026, 1.0809, 0.6828};
        for(int i=0; i<41; i++) {
               DSG[6][i] = DSG1800[i];
        }
        double DSG1850[41] = {0.1988, 0.175, 0.1665, 0.1706, 0.1778, 0.1814, 0.1795, 0.174, 0.1693, 0.1705, 0.1815, 0.2044, 0.2392, 0.2837, 0.3339, 0.3846, 0.4302, 0.4656, 0.4864, 0.4899, 0.475, 0.4427, 0.3957, 0.3384, 0.2763, 0.216, 0.164, 0.127, 0.1109, 0.1207, 0.1599, 0.2309, 0.3337, 0.4662, 0.6226, 0.7921, 0.9561, 1.0835, 1.125, 1.0028, 0.5985};
        for(int i=0; i<41; i++) {
               DSG[7][i] = DSG1850[i];
        }
        double DSG1900[41] = {0.1587, 0.1368, 0.122, 0.1218, 0.1293, 0.1369, 0.1408, 0.1411, 0.1409, 0.1442, 0.1547, 0.1749, 0.2052, 0.2439, 0.2878, 0.3325, 0.3731, 0.405, 0.4243, 0.4285, 0.4168, 0.3898, 0.3502, 0.3018, 0.2496, 0.1991, 0.1562, 0.1265, 0.1149, 0.1258, 0.1623, 0.2266, 0.3193, 0.4393, 0.5824, 0.7397, 0.8948, 1.018, 1.0597, 0.9392, 0.5293};
        for(int i=0; i<41; i++) {
               DSG[8][i] = DSG1900[i];
        }
        double DSG1950[41] = {0.129, 0.115, 0.0948, 0.0883, 0.093, 0.1013, 0.1082, 0.1122, 0.115, 0.1198, 0.1301, 0.148, 0.1744, 0.2081, 0.2465, 0.2858, 0.3218, 0.3502, 0.3678, 0.3723, 0.363, 0.3407, 0.3077, 0.2674, 0.2243, 0.1832, 0.149, 0.1265, 0.1196, 0.132, 0.1663, 0.2246, 0.308, 0.4162, 0.5467, 0.6926, 0.8393, 0.9589, 1.0013, 0.8828, 0.4674};
        for(int i=0; i<41; i++) {
               DSG[9][i] = DSG1950[i];
        }
        double DSG2000[41] = {0.1053, 0.1038, 0.0793, 0.0661, 0.0665, 0.0738, 0.0821, 0.0886, 0.0939, 0.1003, 0.1105, 0.1269, 0.1503, 0.1798, 0.2134, 0.2477, 0.2791, 0.304, 0.3193, 0.3234, 0.3156, 0.2969, 0.2694, 0.2363, 0.2014, 0.1689, 0.143, 0.1274, 0.1253, 0.1395, 0.1721, 0.225, 0.2996, 0.3968, 0.5155, 0.6509, 0.7903, 0.9075, 0.9528, 0.8392, 0.4233};
        for(int i=0; i<41; i++) {
               DSG[10][i] = DSG2000[i];
        }
        double DSG2050[41] = {0.089, 0.102, 0.0744, 0.0543, 0.0491, 0.0536, 0.0613, 0.0685, 0.0747, 0.0815, 0.0913, 0.1061, 0.1267, 0.1528, 0.1823, 0.2126, 0.2402, 0.2621, 0.2757, 0.2796, 0.2733, 0.2579, 0.2354, 0.2087, 0.1812, 0.1565, 0.1379, 0.1283, 0.1299, 0.1448, 0.1747, 0.2212, 0.2861, 0.3714, 0.4775, 0.6017, 0.7341, 0.8504, 0.9018, 0.7997, 0.394};
        for(int i=0; i<41; i++) {
               DSG[11][i] = DSG2050[i];
        }
        double DSG2100[41] = {0.0806, 0.1071, 0.0765, 0.0493, 0.0381, 0.039, 0.0452, 0.0523, 0.0589, 0.0659, 0.0752, 0.0887, 0.1072, 0.1303, 0.1564, 0.1829, 0.207, 0.226, 0.2377, 0.2408, 0.2354, 0.2225, 0.2042, 0.1832, 0.1625, 0.145, 0.1333, 0.1294, 0.1348, 0.1506, 0.1779, 0.2183, 0.2737, 0.347, 0.4403, 0.5531, 0.6779, 0.7928, 0.8508, 0.7622, 0.3721};
        for(int i=0; i<41; i++) {
               DSG[12][i] = DSG2100[i];
        }
        double DSG2150[41] = {0.0746, 0.1131, 0.0803, 0.0476, 0.0315, 0.0293, 0.0341, 0.0409, 0.0476, 0.0547, 0.0637, 0.0761, 0.0928, 0.1134, 0.1364, 0.1597, 0.1807, 0.1969, 0.2066, 0.2089, 0.204, 0.1931, 0.1785, 0.1625, 0.1479, 0.1371, 0.1321, 0.1338, 0.1431, 0.1601, 0.1853, 0.2199, 0.2663, 0.3279, 0.4085, 0.5098, 0.6272, 0.7416, 0.8083, 0.7383, 0.3738};
        for(int i=0; i<41; i++) {
               DSG[13][i] = DSG2150[i];
        }
        double DSG2200[41] = {0.0746, 0.1218, 0.0868, 0.0493, 0.029, 0.024, 0.0271, 0.033, 0.0392, 0.0457, 0.0538, 0.065, 0.0798, 0.0981, 0.1186, 0.1391, 0.1573, 0.1711, 0.1791, 0.1805, 0.176, 0.1668, 0.1552, 0.1436, 0.1344, 0.1296, 0.1303, 0.1371, 0.1497, 0.1675, 0.1904, 0.2193, 0.2565, 0.3064, 0.3743, 0.464, 0.5739, 0.6883, 0.7653, 0.7179, 0.3873};
        for(int i=0; i<41; i++) {
               DSG[14][i] = DSG2200[i];
        }
        double DSG2250[41] = {0.0766, 0.1275, 0.0909, 0.0509, 0.0287, 0.0223, 0.0245, 0.0295, 0.0346, 0.0398, 0.0462, 0.0553, 0.0678, 0.0836, 0.1014, 0.1192, 0.135, 0.1467, 0.1531, 0.1539, 0.1496, 0.1419, 0.1329, 0.125, 0.1204, 0.1206, 0.1264, 0.1373, 0.1524, 0.1705, 0.1906, 0.2135, 0.2416, 0.2801, 0.3355, 0.4143, 0.5176, 0.6334, 0.7234, 0.7036, 0.4158};
        for(int i=0; i<41; i++) {
               DSG[15][i] = DSG2250[i];
        }
        double DSG2300[41] = {0.0817, 0.1317, 0.0932, 0.052, 0.0293, 0.0227, 0.0247, 0.0292, 0.0335, 0.0374, 0.0422, 0.0494, 0.0598, 0.0732, 0.0885, 0.1039, 0.1172, 0.1268, 0.1316, 0.1314, 0.1271, 0.1204, 0.1136, 0.109, 0.1087, 0.1138, 0.1244, 0.1397, 0.1579, 0.1769, 0.1953, 0.2132, 0.2333, 0.2611, 0.3048, 0.3727, 0.4693, 0.5863, 0.6897, 0.6993, 0.4599};
        for(int i=0; i<41; i++) {
               DSG[16][i] = DSG2300[i];
        }
        double DSG2350[41] = {0.0891, 0.1359, 0.0954, 0.0536, 0.0307, 0.0239, 0.0255, 0.0291, 0.0322, 0.0347, 0.038, 0.0437, 0.0525, 0.0644, 0.0782, 0.0921, 0.1041, 0.1125, 0.1164, 0.1158, 0.1118, 0.1062, 0.1014, 0.0997, 0.103, 0.1122, 0.1271, 0.146, 0.1665, 0.1858, 0.2019, 0.2145, 0.2262, 0.2432, 0.2749, 0.332, 0.4219, 0.5406, 0.6588, 0.7014, 0.5179};
        for(int i=0; i<41; i++) {
               DSG[17][i] = DSG2350[i];
        }
        double DSG2400[41] = {0.0988, 0.1348, 0.0926, 0.0529, 0.033, 0.0285, 0.0309, 0.034, 0.0354, 0.0354, 0.0359, 0.0386, 0.0445, 0.0538, 0.0652, 0.077, 0.0873, 0.0944, 0.0974, 0.0964, 0.0926, 0.0879, 0.0848, 0.0856, 0.092, 0.1046, 0.1228, 0.1445, 0.1667, 0.1861, 0.1999, 0.2078, 0.2122, 0.2201, 0.2421, 0.2909, 0.3772, 0.5006, 0.6368, 0.7156, 0.5913};
        for(int i=0; i<41; i++) {
               DSG[18][i] = DSG2400[i];
        }
        double DSG2450[41] = {0.1121, 0.1344, 0.0891, 0.051, 0.034, 0.0317, 0.0351, 0.038, 0.0382, 0.0366, 0.035, 0.0357, 0.0397, 0.0471, 0.0569, 0.0672, 0.0761, 0.082, 0.0842, 0.0828, 0.0792, 0.0754, 0.0739, 0.0771, 0.0865, 0.1027, 0.1244, 0.1493, 0.1736, 0.1934, 0.2056, 0.2092, 0.2071, 0.2065, 0.2194, 0.2606, 0.3435, 0.4721, 0.6266, 0.7425, 0.6795};
        for(int i=0; i<41; i++) {
               DSG[19][i] = DSG2450[i];
        }
        double DSG2500[41] = {0.1299, 0.1344, 0.0855, 0.0495, 0.0357, 0.036, 0.0405, 0.0432, 0.0423, 0.0389, 0.0354, 0.0341, 0.0364, 0.0423, 0.0507, 0.0598, 0.0676, 0.0726, 0.0741, 0.0724, 0.069, 0.0659, 0.0659, 0.0712, 0.0833, 0.1026, 0.1276, 0.1552, 0.1814, 0.2016, 0.2124, 0.2125, 0.2047, 0.197, 0.2025, 0.2381, 0.3201, 0.4562, 0.6314, 0.7864, 0.7865};
        for(int i=0; i<41; i++) {
               DSG[20][i] = DSG2500[i];
        }
	double DSG2550[41] = {0.1558, 0.1379, 0.0843, 0.0506, 0.0405, 0.0436, 0.0493, 0.0516, 0.0493, 0.0439, 0.0382, 0.0348, 0.035, 0.0391, 0.0459, 0.0535, 0.0601, 0.064, 0.0646, 0.0624, 0.0588, 0.0563, 0.0574, 0.0646, 0.0791, 0.1011, 0.1289, 0.1589, 0.1867, 0.2072, 0.2166, 0.2137, 0.2014, 0.1882, 0.1886, 0.2214, 0.3055, 0.4521, 0.6501, 0.8446, 0.9052};
        for(int i=0; i<41; i++) {
               DSG[21][i] = DSG2550[i];
        }
        double DSG2600[41] = {0.1961, 0.147, 0.0844, 0.051, 0.044, 0.05, 0.0574, 0.0601, 0.0573, 0.0507, 0.0435, 0.0384, 0.0369, 0.0392, 0.0442, 0.0501, 0.0548, 0.0571, 0.0564, 0.0533, 0.0495, 0.0476, 0.0502, 0.0596, 0.0772, 0.1027, 0.1341, 0.1675, 0.1977, 0.2195, 0.2285, 0.2234, 0.2074, 0.1898, 0.1862, 0.2171, 0.3042, 0.4613, 0.6809, 0.9116, 1.0261};
        for(int i=0; i<41; i++) {
               DSG[22][i] = DSG2600[i];
        }
        double DSG2650[41] = {0.2485, 0.1641, 0.0908, 0.0567, 0.052, 0.0602, 0.0688, 0.0716, 0.068, 0.0603, 0.0518, 0.0453, 0.0425, 0.0434, 0.0469, 0.0512, 0.0542, 0.0548, 0.0526, 0.0484, 0.044, 0.0421, 0.0455, 0.0565, 0.0763, 0.1045, 0.1386, 0.1743, 0.2063, 0.2287, 0.2372, 0.2304, 0.212, 0.1919, 0.1871, 0.2199, 0.314, 0.4859, 0.7302, 0.9978, 1.1623};
        for(int i=0; i<41; i++) {
               DSG[23][i] = DSG2650[i];
        }
        double DSG2700[41] = {0.3105, 0.1906, 0.1067, 0.072, 0.0696, 0.0798, 0.0893, 0.092, 0.0874, 0.0783, 0.0681, 0.0598, 0.055, 0.0537, 0.0549, 0.0566, 0.0571, 0.0553, 0.0509, 0.0452, 0.04, 0.0384, 0.043, 0.0563, 0.0791, 0.1107, 0.1484, 0.1873, 0.2216, 0.2452, 0.2535, 0.2454, 0.2251, 0.2033, 0.1986, 0.235, 0.3383, 0.5273, 0.7985, 1.1038, 1.3163};
        for(int i=0; i<41; i++) {
               DSG[24][i] = DSG2700[i];
        }
        double DSG2750[41] = {0.3987, 0.2256, 0.1306, 0.1012, 0.1083, 0.1267, 0.1412, 0.1458, 0.1405, 0.1288, 0.1148, 0.1022, 0.0928, 0.087, 0.0837, 0.0814, 0.0783, 0.0735, 0.0669, 0.0596, 0.0538, 0.0523, 0.0578, 0.0725, 0.0971, 0.1304, 0.1694, 0.209, 0.2431, 0.2655, 0.2719, 0.2617, 0.2397, 0.218, 0.2162, 0.2601, 0.3764, 0.5845, 0.8804, 1.2131, 1.4501};
        for(int i=0; i<41; i++) {
               DSG[25][i] = DSG2750[i];
        }
        double DSG2800[41] = {0.4971, 0.2786, 0.2201, 0.2542, 0.3245, 0.3944, 0.445, 0.4707, 0.4742, 0.4618, 0.4409, 0.4177, 0.3963, 0.3786, 0.3647, 0.3533, 0.3432, 0.333, 0.3224, 0.312, 0.3033, 0.2983, 0.2991, 0.3072, 0.3228, 0.3445, 0.3692, 0.392, 0.4076, 0.4107, 0.3985, 0.3717, 0.3373, 0.309, 0.3078, 0.3597, 0.4906, 0.7162, 1.0258, 1.3567, 1.5585};
        for(int i=0; i<41; i++) {
               DSG[26][i] = DSG2800[i];
        }
        double DSG2850[41] = {0.6103, 0.4674, 0.6002, 0.8518, 1.1235, 1.3611, 1.5411, 1.6602, 1.7263, 1.7524, 1.7519, 1.7366, 1.7149, 1.6919, 1.67, 1.6491, 1.6282, 1.6058, 1.5808, 1.5525, 1.5214, 1.4881, 1.4537, 1.419, 1.3836, 1.3465, 1.3051, 1.2559, 1.1956, 1.1214, 1.0334, 0.9358, 0.8383, 0.7572, 0.7148, 0.7363, 0.8437, 1.0443, 1.3131, 1.5653, 1.6177};
        for(int i=0; i<41; i++) {
               DSG[27][i] = DSG2850[i];
        }
        double DSG2900[41] = {0.7611, 1.0472, 1.7367, 2.5471, 3.3222, 3.9858, 4.511, 4.9005, 5.1722, 5.3504, 5.4591, 5.5194, 5.5471, 5.5531, 5.5435, 5.5205, 5.4839, 5.4323, 5.3634, 5.2757, 5.1678, 5.0394, 4.8903, 4.7209, 4.5315, 4.3218, 4.0917, 3.8408, 3.5696, 3.2804, 2.9783, 2.6728, 2.3787, 2.1159, 1.908, 1.7785, 1.7425, 1.7942, 1.8861, 1.9, 1.6055};
        for(int i=0; i<41; i++) {
               DSG[28][i] = DSG2900[i];
        }
        double DSG2950[41] = {0.9953, 2.4498, 4.4073, 6.4227, 8.2697, 9.8487, 11.1342, 12.1415, 12.9063, 13.4711, 13.8772, 14.1602, 14.3482, 14.461, 14.5109, 14.5046, 14.4439, 14.3279, 14.1544, 13.9207, 13.6248, 13.2653, 12.8422, 12.3565, 11.8105, 11.2072, 10.5509, 9.8472, 9.1037, 8.3307, 7.5419, 6.7553, 5.9934, 5.2816, 4.6462, 4.1084, 3.6744, 3.3205, 2.97, 2.4608, 1.5004};
        for(int i=0; i<41; i++) {
               DSG[29][i] = DSG2950[i];
        }
        double DSG3000[41] = {1.3858, 5.3273, 9.7794, 14.0987, 17.9804, 21.3061, 24.0612, 26.2853, 28.0432, 29.4068, 30.4438, 31.2127, 31.7599, 32.1196, 32.3147, 32.3588, 32.2584, 32.0153, 31.6284, 31.0955, 30.4153, 29.5871, 28.6129, 27.4967, 26.2456, 24.8689, 23.3794, 21.7926, 20.1278, 18.408, 16.6603, 14.9158, 13.2087, 11.5739, 10.0427, 8.6351, 7.3482, 6.1377, 4.8924, 3.3979, 1.288};
        for(int i=0; i<41; i++) {
               DSG[30][i] = DSG3000[i];
        }

	// Fill Sigma data tables
	double Sigma1500[41] = {0, 0.2962, 0.4677, 0.568, 0.6196, 0.6322, 0.6091, 0.5519, 0.4665, 0.369, 0.2858, 0.2404, 0.2388, 0.2696, 0.3162, 0.3661, 0.4121, 0.4513, 0.4827, 0.506, 0.5213, 0.528, 0.5252, 0.5111, 0.4822, 0.4339, 0.3615, 0.2701, 0.2069, 0.2765, 0.4849, 0.6812, 0.7982, 0.8556, 0.8787, 0.8807, 0.8655, 0.8297, 0.7575, 0.5934, 0};
        for(int i=0; i<41; i++) {
               Sigma[0][i] = Sigma1500[i];
        }
        double Sigma1550[41] = {0, 0.3222, 0.5052, 0.6103, 0.6636, 0.6758, 0.6504, 0.5887, 0.4975, 0.3965, 0.3162, 0.2805, 0.2897, 0.3271, 0.3752, 0.4224, 0.4635, 0.4967, 0.5216, 0.5383, 0.5464, 0.5454, 0.5334, 0.5072, 0.4609, 0.3851, 0.2664, 0.101, -0.0409, 0.0435, 0.3537, 0.6198, 0.7674, 0.8391, 0.8699, 0.8762, 0.863, 0.8268, 0.7512, 0.58, 0};
        for(int i=0; i<41; i++) {
               Sigma[1][i] = Sigma1550[i];
        }
        double Sigma1600[41] = {0, 0.3308, 0.524, 0.6356, 0.6922, 0.7054, 0.6788, 0.6141, 0.5189, 0.4157, 0.3384, 0.3098, 0.3257, 0.3665, 0.4143, 0.459, 0.4965, 0.5257, 0.5464, 0.5587, 0.5623, 0.5562, 0.5379, 0.5032, 0.4439, 0.3466, 0.1899, -0.0392, -0.2497, -0.1488, 0.2401, 0.5566, 0.7296, 0.8158, 0.8558, 0.8681, 0.8585, 0.8236, 0.7467, 0.5707, 0};
        for(int i=0; i<41; i++) {
               Sigma[2][i] = Sigma1600[i];
        }
        double Sigma1650[41] = {0, 0.3191, 0.5244, 0.6478, 0.7122, 0.73, 0.706, 0.6425, 0.5481, 0.4468, 0.3734, 0.3485, 0.3655, 0.4046, 0.4488, 0.4894, 0.5228, 0.5481, 0.5652, 0.5742, 0.5744, 0.5646, 0.5418, 0.5008, 0.4321, 0.3189, 0.1352, -0.1354, -0.3795, -0.2609, 0.1608, 0.4997, 0.6895, 0.7885, 0.8381, 0.8576, 0.8532, 0.8216, 0.7457, 0.5682, 0};
        for(int i=0; i<41; i++) {
               Sigma[3][i] = Sigma1650[i];
        }
        double Sigma1700[41] = {0, 0.2853, 0.5049, 0.6466, 0.7239, 0.7494, 0.7297, 0.6686, 0.5754, 0.4747, 0.401, 0.3747, 0.3892, 0.4251, 0.4663, 0.5042, 0.5352, 0.5586, 0.5741, 0.5816, 0.5805, 0.5693, 0.5447, 0.501, 0.4281, 0.308, 0.1137, -0.1672, -0.4071, -0.2827, 0.1249, 0.4595, 0.6557, 0.7636, 0.8215, 0.848, 0.8494, 0.8226, 0.7504, 0.5747, 0};
        for(int i=0; i<41; i++) {
               Sigma[4][i] = Sigma1700[i];
        }
        double Sigma1750[41] = {0, 0.2288, 0.464, 0.634, 0.7326, 0.7714, 0.76, 0.7045, 0.6154, 0.5164, 0.4406, 0.4086, 0.4159, 0.4451, 0.4809, 0.5149, 0.5433, 0.5647, 0.579, 0.5856, 0.5839, 0.5721, 0.5468, 0.5022, 0.4276, 0.3054, 0.1108, -0.1599, -0.3744, -0.2578, 0.1072, 0.4223, 0.6196, 0.7351, 0.8016, 0.8361, 0.8447, 0.8244, 0.7583, 0.5879, 0};
        for(int i=0; i<41; i++) {
               Sigma[5][i] = Sigma1750[i];
        }
        double Sigma1800[41] = {0, 0.1564, 0.402, 0.6087, 0.7364, 0.7926, 0.7909, 0.7412, 0.6548, 0.5546, 0.4726, 0.4312, 0.4293, 0.4516, 0.4831, 0.5146, 0.5417, 0.5627, 0.5771, 0.5842, 0.5833, 0.5724, 0.5483, 0.505, 0.4325, 0.3142, 0.13, -0.1142, -0.2935, -0.1945, 0.1128, 0.3967, 0.5884, 0.7085, 0.7823, 0.8245, 0.8407, 0.8279, 0.7698, 0.6076, 0};
        for(int i=0; i<41; i++) {
               Sigma[6][i] = Sigma1800[i];
        }
        double Sigma1850[41] = {0, 0.0722, 0.3107, 0.5614, 0.7298, 0.8096, 0.82, 0.7765, 0.693, 0.5915, 0.5029, 0.4512, 0.439, 0.4534, 0.4798, 0.5085, 0.5344, 0.5553, 0.5701, 0.5782, 0.5786, 0.5693, 0.5473, 0.5067, 0.438, 0.3268, 0.1578, -0.0551, -0.1996, -0.1163, 0.1377, 0.3874, 0.5687, 0.6897, 0.7687, 0.8174, 0.8404, 0.8349, 0.7852, 0.6328, 0};
        for(int i=0; i<41; i++) {
               Sigma[7][i] = Sigma1850[i];
        }
        double Sigma1900[41] = {0, 0.0013, 0.2, 0.4875, 0.7074, 0.8196, 0.8463, 0.811, 0.7311, 0.6283, 0.5327, 0.4701, 0.4468, 0.4528, 0.4738, 0.4996, 0.5242, 0.5448, 0.5602, 0.5694, 0.5712, 0.5639, 0.5444, 0.5072, 0.4439, 0.3426, 0.194, 0.0175, -0.0952, -0.0338, 0.1643, 0.3771, 0.5458, 0.6673, 0.7518, 0.8074, 0.8376, 0.8395, 0.7981, 0.6559, 0};
        for(int i=0; i<41; i++) {
               Sigma[8][i] = Sigma1900[i];
        }
        double Sigma1950[41] = {0, -0.032, 0.0858, 0.3762, 0.6575, 0.8182, 0.8708, 0.8487, 0.775, 0.6719, 0.5693, 0.4949, 0.4592, 0.4549, 0.4687, 0.4898, 0.5117, 0.531, 0.5459, 0.5552, 0.5577, 0.5516, 0.5339, 0.4997, 0.4417, 0.351, 0.2242, 0.0843, 0.0032, 0.0514, 0.2039, 0.3805, 0.5333, 0.6521, 0.74, 0.8015, 0.838, 0.8465, 0.8128, 0.6805, 0};
        for(int i=0; i<41; i++) {
               Sigma[9][i] = Sigma1950[i];
        }
        double Sigma2000[41] = {0, -0.014, 0.0006, 0.2278, 0.5633, 0.7936, 0.8867, 0.8853, 0.8224, 0.723, 0.6164, 0.5315, 0.4821, 0.4649, 0.4684, 0.482, 0.4987, 0.5145, 0.5271, 0.5351, 0.5371, 0.5311, 0.5144, 0.4826, 0.4299, 0.351, 0.2477, 0.1444, 0.0929, 0.1341, 0.2508, 0.3939, 0.5287, 0.6419, 0.7313, 0.7969, 0.8385, 0.8521, 0.824, 0.6995, 0};
        for(int i=0; i<41; i++) {
               Sigma[10][i] = Sigma2000[i];
        }
        double Sigma2050[41] = {0, 0.058, -0.0079, 0.0822, 0.4068, 0.7166, 0.8703, 0.902, 0.8562, 0.7632, 0.6538, 0.5595, 0.4981, 0.4697, 0.4641, 0.4709, 0.4828, 0.4953, 0.5059, 0.5129, 0.5149, 0.5103, 0.4966, 0.4706, 0.4286, 0.3682, 0.2945, 0.2269, 0.1979, 0.2291, 0.3126, 0.4227, 0.5365, 0.6408, 0.7287, 0.7963, 0.8408, 0.857, 0.8316, 0.7109, 0};
        for(int i=0; i<41; i++) {
               Sigma[11][i] = Sigma2050[i];
        }
        double Sigma2100[41] = {0, 0.1492, 0.0424, -0.021, 0.1944, 0.575, 0.8221, 0.9095, 0.8937, 0.8144, 0.706, 0.6029, 0.5282, 0.4857, 0.4679, 0.4648, 0.4691, 0.4758, 0.4822, 0.4864, 0.4872, 0.4831, 0.4727, 0.4542, 0.4259, 0.3885, 0.3473, 0.3151, 0.3074, 0.3334, 0.391, 0.4707, 0.5614, 0.6527, 0.7355, 0.8024, 0.8473, 0.8641, 0.839, 0.7193, 0};
        for(int i=0; i<41; i++) {
               Sigma[12][i] = Sigma2100[i];
        }
        double Sigma2150[41] = {0, 0.2302, 0.1051, -0.08, -0.0511, 0.3494, 0.7222, 0.8951, 0.9271, 0.8724, 0.7705, 0.6604, 0.5706, 0.5109, 0.4771, 0.4605, 0.4537, 0.4515, 0.4507, 0.4493, 0.4462, 0.4406, 0.432, 0.4205, 0.4068, 0.3932, 0.3831, 0.381, 0.3908, 0.4149, 0.4548, 0.5109, 0.581, 0.6594, 0.7366, 0.8021, 0.8468, 0.8625, 0.835, 0.7115, 0};
        for(int i=0; i<41; i++) {
               Sigma[13][i] = Sigma2150[i];
        }
        double Sigma2200[41] = {0, 0.2923, 0.165, -0.0943, -0.2762, 0.0297, 0.5296, 0.8259, 0.9348, 0.9234, 0.8384, 0.725, 0.6206, 0.5426, 0.4912, 0.4593, 0.4396, 0.4265, 0.4166, 0.4078, 0.3995, 0.3919, 0.3864, 0.3854, 0.3916, 0.4063, 0.428, 0.4525, 0.4765, 0.5003, 0.5278, 0.5647, 0.6152, 0.6783, 0.7463, 0.8068, 0.8479, 0.8591, 0.8255, 0.6944, 0};
        for(int i=0; i<41; i++) {
               Sigma[14][i] = Sigma2200[i];
        }
        double Sigma2250[41] = {0, 0.3451, 0.2267, -0.0627, -0.3781, -0.2161, 0.3172, 0.7061, 0.8957, 0.9436, 0.8914, 0.7844, 0.6692, 0.5743, 0.5061, 0.4596, 0.4276, 0.4042, 0.3857, 0.3704, 0.3584, 0.3518, 0.3547, 0.3717, 0.4058, 0.4537, 0.5055, 0.5503, 0.5825, 0.6036, 0.6197, 0.6389, 0.6689, 0.713, 0.7664, 0.8163, 0.8482, 0.8502, 0.8065, 0.6642, 0};
        for(int i=0; i<41; i++) {
               Sigma[15][i] = Sigma2250[i];
        }
        double Sigma2300[41] = {0, 0.3788, 0.2664, -0.0475, -0.4479, -0.3886, 0.1311, 0.5703, 0.8273, 0.941, 0.9381, 0.8525, 0.7339, 0.6227, 0.535, 0.47, 0.4215, 0.3834, 0.3518, 0.3253, 0.3053, 0.2965, 0.3068, 0.3439, 0.4089, 0.4901, 0.5685, 0.6293, 0.6685, 0.6896, 0.7001, 0.708, 0.722, 0.7478, 0.7841, 0.8199, 0.8396, 0.83, 0.7747, 0.6212, 0};
        for(int i=0; i<41; i++) {
               Sigma[16][i] = Sigma2300[i];
        }
        double Sigma2350[41] = {0, 0.4005, 0.2956, -0.0316, -0.4798, -0.5037, -0.0326, 0.4273, 0.7388, 0.9161, 0.9657, 0.9055, 0.7859, 0.6601, 0.5547, 0.4729, 0.409, 0.3566, 0.3115, 0.2729, 0.2439, 0.2326, 0.2516, 0.3106, 0.4059, 0.5153, 0.6128, 0.6839, 0.7276, 0.7499, 0.7581, 0.7595, 0.7616, 0.7711, 0.7897, 0.8094, 0.8155, 0.7939, 0.7276, 0.5661, 0};
        for(int i=0; i<41; i++) {
               Sigma[17][i] = Sigma2350[i];
        }
        double Sigma2400[41] = {0, 0.4131, 0.3148, -0.0273, -0.4587, -0.4585, -0.0757, 0.3108, 0.6128, 0.8347, 0.9603, 0.9662, 0.8736, 0.7428, 0.6199, 0.5193, 0.4383, 0.3707, 0.3119, 0.2613, 0.2244, 0.2138, 0.2473, 0.3353, 0.4632, 0.5954, 0.7017, 0.7725, 0.8125, 0.8302, 0.8328, 0.826, 0.8144, 0.8031, 0.7957, 0.7899, 0.7757, 0.74, 0.6649, 0.5016, 0};
        for(int i=0; i<41; i++) {
               Sigma[18][i] = Sigma2400[i];
        }
        double Sigma2450[41] = {0, 0.4161, 0.3259, -0.0376, -0.4716, -0.4536, -0.1151, 0.2251, 0.5096, 0.7478, 0.9229, 0.9894, 0.9338, 0.8084, 0.6738, 0.5568, 0.459, 0.3748, 0.2994, 0.233, 0.1836, 0.1702, 0.2174, 0.3335, 0.4895, 0.6374, 0.7477, 0.8173, 0.8551, 0.8708, 0.8711, 0.8596, 0.8376, 0.8067, 0.7713, 0.7379, 0.7059, 0.6629, 0.5875, 0.4321, 0};
        for(int i=0; i<41; i++) {
               Sigma[19][i] = Sigma2450[i];
        }
        double Sigma2500[41] = {0, 0.405, 0.3223, -0.0598, -0.4685, -0.4195, -0.1241, 0.1662, 0.4208, 0.6554, 0.8606, 0.9834, 0.9761, 0.868, 0.7285, 0.598, 0.4844, 0.3833, 0.2898, 0.2045, 0.1387, 0.1189, 0.1774, 0.3184, 0.4962, 0.6533, 0.764, 0.8314, 0.867, 0.8808, 0.8782, 0.8607, 0.8264, 0.7724, 0.704, 0.6417, 0.5992, 0.5613, 0.4976, 0.3612, 0};
        for(int i=0; i<41; i++) {
               Sigma[20][i] = Sigma2500[i];
        }
	double Sigma2550[41] = {0, 0.3773, 0.2958, -0.1039, -0.4679, -0.4043, -0.164, 0.0704, 0.2842, 0.4978, 0.7172, 0.903, 0.9825, 0.9367, 0.8217, 0.6925, 0.5699, 0.455, 0.3443, 0.239, 0.1538, 0.1232, 0.1881, 0.3455, 0.5312, 0.6835, 0.7845, 0.8428, 0.8714, 0.879, 0.8694, 0.841, 0.7874, 0.7007, 0.5889, 0.4968, 0.4574, 0.4418, 0.4034, 0.2953, 0};
        for(int i=0; i<41; i++) {
               Sigma[21][i] = Sigma2550[i];
        }
        double Sigma2600[41] = {0, 0.3287, 0.2515, -0.1551, -0.4704, -0.3792, -0.1636, 0.0367, 0.2189, 0.4054, 0.6096, 0.8096, 0.9403, 0.9542, 0.8771, 0.7612, 0.6361, 0.5087, 0.3779, 0.2457, 0.1319, 0.0861, 0.1625, 0.3426, 0.5369, 0.6836, 0.7753, 0.8257, 0.8479, 0.8495, 0.8322, 0.792, 0.7182, 0.5986, 0.4461, 0.3334, 0.3115, 0.3293, 0.3217, 0.2428, 0};
        for(int i=0; i<41; i++) {
               Sigma[22][i] = Sigma2600[i];
        }
        double Sigma2650[41] = {0, 0.2733, 0.1961, -0.1856, -0.4389, -0.3507, -0.1723, -0.0068, 0.1463, 0.3075, 0.4925, 0.6922, 0.8566, 0.9303, 0.9094, 0.8291, 0.7199, 0.5937, 0.4511, 0.2934, 0.1421, 0.0612, 0.1251, 0.308, 0.5013, 0.6423, 0.7284, 0.7744, 0.7922, 0.7877, 0.7606, 0.7038, 0.6025, 0.442, 0.2486, 0.1327, 0.1516, 0.2171, 0.2463, 0.1979, 0};
        for(int i=0; i<41; i++) {
               Sigma[23][i] = Sigma2650[i];
        }
        double Sigma2700[41] = {0, 0.217, 0.1229, -0.2213, -0.4185, -0.3549, -0.2209, -0.0918, 0.0305, 0.1619, 0.3182, 0.4999, 0.6785, 0.8074, 0.8606, 0.8456, 0.7817, 0.6811, 0.5451, 0.3731, 0.1858, 0.0628, 0.1032, 0.2738, 0.4525, 0.5806, 0.6579, 0.6976, 0.7093, 0.6966, 0.6567, 0.5794, 0.4468, 0.2454, 0.0215, -0.079, -0.0065, 0.1113, 0.1784, 0.1596, 0};
        for(int i=0; i<41; i++) {
               Sigma[24][i] = Sigma2700[i];
        }
        double Sigma2750[41] = {0, 0.1167, -0.0416, -0.3348, -0.4215, -0.3444, -0.2416, -0.1502, -0.0675, 0.019, 0.1218, 0.2475, 0.3886, 0.5228, 0.6247, 0.6799, 0.6864, 0.6466, 0.5608, 0.4289, 0.2661, 0.1291, 0.0972, 0.1773, 0.2983, 0.4031, 0.4744, 0.513, 0.5219, 0.5011, 0.4452, 0.3414, 0.1719, -0.0644, -0.2831, -0.3132, -0.1529, 0.0271, 0.1317, 0.137, 0};
        for(int i=0; i<41; i++) {
               Sigma[25][i] = Sigma2750[i];
        }
        double Sigma2800[41] = {0, -0.1056, -0.3422, -0.3973, -0.3228, -0.2394, -0.1744, -0.1251, -0.0845, -0.0457, -0.0031, 0.0462, 0.102, 0.1601, 0.2138, 0.2557, 0.2801, 0.2838, 0.2656, 0.2272, 0.1734, 0.1126, 0.056, 0.0141, -0.0082, -0.0138, -0.0116, -0.0133, -0.03, -0.0731, -0.1543, -0.2858, -0.4716, -0.6766, -0.7798, -0.6473, -0.35, -0.0768, 0.0852, 0.1233, 0};
        for(int i=0; i<41; i++) {
               Sigma[26][i] = Sigma2800[i];
        }
        double Sigma2850[41] = {0, -0.2665, -0.3163, -0.2395, -0.1663, -0.114, -0.0773, -0.0502, -0.028, -0.0072, 0.0143, 0.0374, 0.062, 0.0867, 0.1094, 0.1278, 0.1396, 0.1432, 0.1375, 0.1224, 0.0984, 0.0664, 0.028, -0.0151, -0.0614, -0.1101, -0.1617, -0.2183, -0.2834, -0.3618, -0.4587, -0.5765, -0.708, -0.8227, -0.854, -0.7311, -0.4685, -0.1803, 0.029, 0.1106, 0};
        for(int i=0; i<41; i++) {
               Sigma[27][i] = Sigma2850[i];
        }
        double Sigma2900[41] = {0, -0.2283, -0.1779, -0.1122, -0.0665, -0.036, -0.0151, 0.0004, 0.0132, 0.0249, 0.0368, 0.0491, 0.0616, 0.0738, 0.0846, 0.093, 0.0978, 0.0982, 0.0933, 0.0828, 0.0664, 0.0441, 0.016, -0.0178, -0.0569, -0.1015, -0.1516, -0.2078, -0.271, -0.3419, -0.4207, -0.5057, -0.5901, -0.658, -0.681, -0.6242, -0.4713, -0.2535, -0.0418, 0.085, 0};
        for(int i=0; i<41; i++) {
               Sigma[28][i] = Sigma2900[i];
        }
        double Sigma2950[41] = {0, -0.1343, -0.0797, -0.0366, -0.0089, 0.0091, 0.0212, 0.0299, 0.0371, 0.0435, 0.05, 0.0565, 0.063, 0.0691, 0.0741, 0.0775, 0.0786, 0.0767, 0.0714, 0.0621, 0.0486, 0.0307, 0.0082, -0.0189, -0.0509, -0.0877, -0.1294, -0.1762, -0.2282, -0.285, -0.3459, -0.4087, -0.4687, -0.5172, -0.5401, -0.5188, -0.4374, -0.295, -0.1179, 0.0355, 0};
        for(int i=0; i<41; i++) {
               Sigma[29][i] = Sigma2950[i];
        }
        double Sigma3000[41] = {0, -0.0604, -0.0189, 0.0088, 0.0257, 0.0362, 0.0428, 0.0473, 0.0507, 0.0537, 0.0567, 0.0597, 0.0626, 0.0651, 0.0668, 0.0673, 0.0661, 0.0628, 0.0569, 0.0481, 0.0361, 0.0206, 0.0015, -0.0214, -0.0483, -0.0792, -0.1142, -0.1533, -0.1962, -0.2428, -0.2919, -0.3418, -0.3894, -0.4293, -0.4532, -0.4503, -0.4086, -0.3202, -0.1884, -0.0374, 0};
        for(int i=0; i<41; i++) {
               Sigma[30][i] = Sigma3000[i];
        }


	return;
}

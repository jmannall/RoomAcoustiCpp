
#include "EmissionManager.h"

void Emission::UpdateParameters()
{
	UPDATE_PROFILE_SECTION(
		{
			UPDATE_PROFILE_TIME(attenuate.UpdateParameters(), "Attenuate");
			UPDATE_PROFILE_TIME(lpf.UpdateParameters(), "LPF");
			UPDATE_PROFILE_TIME(udfa.UpdateParameters(), "UDFA");
			UPDATE_PROFILE_TIME(udfai.UpdateParameters(), "UDFAI");
			UPDATE_PROFILE_TIME(nnBest.UpdateParameters(), "NNBest");
			UPDATE_PROFILE_TIME(nnSmall.UpdateParameters(), "NNSmall");
			UPDATE_PROFILE_TIME(utd.UpdateParameters(), "UTD");
			UPDATE_PROFILE_TIME(btm.UpdateParameters(), "BTM");
		},
		"Time for Updating Model Parameters");
}
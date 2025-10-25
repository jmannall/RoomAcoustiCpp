
/*
* @class ContextOptionalArguments
*
* @brief Declaration of the ContextOptionalArguments class
*
* @details Defines optional arguments to the Context class
*
*/

#ifndef RoomAcoustiCpp_ContextOptionalArguments_h
#define RoomAcoustiCpp_ContextOptionalArguments_h

// C++ headers
#include <optional>
#include <string>

namespace RAC
{
	namespace Spatialiser
	{
		/**
		 * @brief Provides optional initialization arguments for the context
		 */
		struct ContextOptionalArguments
		{
			/**
			 * @brief The prefix to add to any log file
			 */
			std::string logPrefix = "";

			/**
			 * @brief If set, overrides the number of audio threads to use
			 */
			std::optional<size_t> desiredAudioThreads;
		};
	}
}

#endif
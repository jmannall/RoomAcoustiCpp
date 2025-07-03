/*
* @class Access
*
* @brief Declaration of Access class
*/

#ifndef RoomAcoustiCpp_Access_h
#define RoomAcoustiCpp_Access_h

// C++ headers
#include <atomic>

namespace RAC
{
	namespace Common
	{
		/**
		* @brief Class used to manage access to a class
		* 
		* @details CanEdit, AllowAccess and PreventAccess can only be called from a single thread
		*/
		class Access
		{
		public:
			/**
			* @brief Default constructor
			*/
			Access() {}

			/**
			* @brief Default deconstructor
			*/
			~Access() {}

			/**
			* @brief True if accessFlag and inUse are false, false otherwise
			*/
			inline bool CanEdit() const { return !accessFlag.load() && inUse.load() == 0; }

		protected:
			/**
			* @return True if can access and not in use
			*/
			inline bool GetAccess()
			{
				if (!accessFlag.load())
					return false;

				inUse.fetch_add(1);

				if (!accessFlag.load())
				{
					FreeAccess();
					return false;
				}
				return true;
			}

			/**
			* @brief Set inUse to false
			*/
			inline void FreeAccess() { inUse.fetch_sub(1); }

			/**
			* @brief Set accessFlag to false
			*/
			inline void PreventAccess() { accessFlag.store(false); }

			/**
			* @brief Set accessFlag to true
			*/
			inline void AllowAccess() { accessFlag.store(true); }

		private:

			std::atomic<bool> accessFlag{ false };	// True if access is allowed, false otherwise
			std::atomic<int> inUse{ 0 };			// Number of threads currently using the access
		};
	}
}

#endif // RoomAcoustiCpp_Access_h
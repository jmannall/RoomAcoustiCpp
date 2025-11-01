/*
* @class Access
*
* @brief Declaration of Access and AtomicFlagGuard class
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
			virtual ~Access() {}

			/**
			* @brief True if accessFlag and inUse are false, false otherwise
			*/
			inline bool CanEdit() const { return !accessFlag.load(std::memory_order_acquire) && inUse.load(std::memory_order_acquire) == 0; }

		protected:
			/**
			* @return True if can access and not in use
			*/
			inline bool GetAccess()
			{
				if (!accessFlag.load(std::memory_order_acquire))
					return false;

				inUse.fetch_add(1);

				if (!accessFlag.load(std::memory_order_acquire))
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
			inline void PreventAccess() { accessFlag.store(false, std::memory_order_release); }

			/**
			* @brief Set accessFlag to true
			*/
			inline void AllowAccess() { accessFlag.store(true, std::memory_order_release); }

		private:

			std::atomic<bool> accessFlag{ false };	// True if access is allowed, false otherwise
			std::atomic<int> inUse{ 0 };			// Number of threads currently using the access
		};

		/**
		* @brief Class that provides a scoped guard for an atomic flag
		*/
		class AtomicFlagGuard
		{
		public:
			/**
			* @brief Constructor that sets the atomic flag to true and waits if it was already true
			* 
			* @params f The atomic flag to guard
			*/
			explicit AtomicFlagGuard(std::atomic<bool>& f, bool tryOnce = false) : flag(f)
			{
				if (tryOnce)
					ownsFlag = !flag.exchange(true, std::memory_order_acquire);
				else
				{
					while (flag.exchange(true, std::memory_order_acquire))
						std::this_thread::yield();
					ownsFlag = true;
				}
			}

			/**
			* @brief Deconstructor that sets the atomic flag to false
			*/
			~AtomicFlagGuard()
			{
				if (ownsFlag)
					flag.store(false, std::memory_order_release);
			}

			/**
			* @return True if the flag was successfully acquired, false otherwise
			*/
			bool Acquired() const noexcept { return ownsFlag; }

		private:
			std::atomic<bool>& flag;	// The atomic flag to guard
			bool ownsFlag = false;		// True if the flag was successfully acquired
		};
	}
}

#endif // RoomAcoustiCpp_Access_h
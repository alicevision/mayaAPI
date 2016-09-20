// Hand-crafted version of config/IlmBaseConfig.h for Windows
//
// Define and set to 1 if the target system has POSIX thread support
// and you want IlmBase to use it for multithreaded file I/O.
//

#undef HAVE_PTHREAD

//
// Define and set to 1 if the target system supports POSIX semaphores
// and you want OpenEXR to use them; otherwise, OpenEXR will use its
// own semaphore implementation.
//

#undef HAVE_POSIX_SEMAPHORES

#define ILMBASE_INTERNAL_NAMESPACE_CUSTOM 1
#define IMATH_INTERNAL_NAMESPACE Imath
#define IEX_INTERNAL_NAMESPACE Iex

#define IMATH_NAMESPACE alembicImath
#define IEX_NAMESPACE alembicIex


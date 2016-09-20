# The adskData header files reside in include/maya along with Maya's API
# header files. Normally files in that directory are included as follows:
#
#	#include <maya/header.h>
#
# The adskData files are independent of Maya so they cannot do that when
# one adskData header includes another. So we have to add include/maya
# itself to the list of header directories used by the compiler.
#
MetadataSample_EXTRA_INCLUDES = -I$(DEVKIT_INCLUDE)/maya

# It's presumed that you have libxml2 installed in the standard location
# with include files in /usr/include/libxml2. If it lives elsewhere you will
# have to adjust the path below accordingly.
#
MetadataXML_EXTRA_INCLUDES = -I$(DEVKIT_INCLUDE)/maya -I/usr/include/libxml2


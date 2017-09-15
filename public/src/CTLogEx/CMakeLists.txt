SET(WINPR_PATH    ${CMAKE_SOURCE_DIR}/3rdparty/winpr)
SET(WINPR_INCLUDE ${WINPR_PATH}/include)

INCLUDE_DIRECTORIES(
	./
	${CMAKE_SOURCE_DIR}/public/inc/CTLogEx
	${CMAKE_SOURCE_DIR}/public/inc/Linux
	${WINPR_INCLUDE}
	)

LIST(APPEND REF_FILES
	CTLogEx.cpp  CTLogEx_OsUnix.cpp iniparse.cpp
	)

ADD_DEFINITIONS(-g -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -fPIC -DPIC)


ADD_LIBRARY(CTLogEx STATIC ${REF_FILES})

check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources    
        #test/sionna-examples-test-suite.cc
        )
endif()    

build_lib(
    LIBNAME sionna
    SOURCE_FILES model/sionna-connection-handler.cc
                 helper/sionna-helper.cc
    HEADER_FILES model/sionna-connection-handler.h
                 helper/sionna-helper.h
    LIBRARIES_TO_LINK
	             ${libcore}
                 ${libnetwork}
                 ${libmobility}
    TEST_SOURCES test/sionna-test-suite.cc
                 ${examples_as_tests_sources}
)
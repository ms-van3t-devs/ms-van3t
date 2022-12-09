set(source_files
    model/traffic-control-app.cc
    helper/traffic-control-app-helper.cc)

set(header_files
    model/traffic-control-app.h
    helper/traffic-control-app-helper.h)

set(test_sources
)

build_lib(
  LIBNAME traci-applications
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libwifi}
    ${libapplications}
    ${libmobility}
    ${libinternet}
    ${libconfig-store}
    ${libstats}
    ${libtraci}
    ${libapplications}
  TEST_SOURCES ${test_sources}
)
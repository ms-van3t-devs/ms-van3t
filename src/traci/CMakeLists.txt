set(source_files
    model/traci-client.cc
    model/sumo-socket.cc
    model/sumo-storage.cc
    model/sumo-TraCIAPI.cc)

set(header_files
    model/traci-client.h
    model/sumo-TraCIAPI.h
    model/sumo-config.h
    model/sumo-socket.h
    model/sumo-storage.h
    model/sumo-TraCIConstants.h
    model/sumo-TraCIDefs.h)

set(test_sources
)

build_lib(
  LIBNAME traci
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
  ${libcore}
  ${libmobility}
  ${libinternet}
  ${libvehicle-visualizer}
  TEST_SOURCES ${test_sources}
)
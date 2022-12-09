set(source_files
    model/vehicle-visualizer.cc)

set(header_files
    model/vehicle-visualizer.h)

set(test_sources
    test/vehicle-visualizer-test-suite.cc)

build_lib(
  LIBNAME vehicle-visualizer
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
  TEST_SOURCES ${test_sources}
)
build_lib_example(
    NAME gps-tc-example
    SOURCE_FILES gps-tc-example.cc
    LIBRARIES_TO_LINK
    ${libtraci}
    ${libsionna}
    ${libgps-tc}
    ${libvehicle-visualizer}
)
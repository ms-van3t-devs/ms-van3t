build_lib_example(
    NAME v2i-areaSpeedAdvisor-lte 
    SOURCE_FILES v2i-areaSpeedAdvisor-lte.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${liblte}
    ${libtraci}
)
    
    
build_lib_example(
    NAME v2i-areaSpeedAdvisor-80211p
    SOURCE_FILES v2i-areaSpeedAdvisor-80211p.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libwave}
    ${libtraci}
)

build_lib_example(
    NAME v2i-emergencyVehicleWarning-80211p
    SOURCE_FILES v2i-emergencyVehicleWarning-80211p.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libwave}
    ${libtraci}
)

build_lib_example(
    NAME v2i-trafficManager-80211p
    SOURCE_FILES v2i-trafficManager-80211p.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libwave}
    ${libtraci}
)

build_lib_example(
    NAME v2i-trafficManager-LTE
    SOURCE_FILES v2i-trafficManager-LTE.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${liblte}
    ${libtraci}
)

build_lib_example(
    NAME v2v-emergencyVehicleAlert-cv2x
    SOURCE_FILES v2v-emergencyVehicleAlert-cv2x.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libcv2x}
    ${libtraci}
    )

build_lib_example(
    NAME v2v-emergencyVehicleAlert-80211p
    SOURCE_FILES v2v-emergencyVehicleAlert-80211p.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libwave}
    ${libtraci}
)

build_lib_example(
    NAME v2v-emergencyVehicleAlert-nrv2x
    SOURCE_FILES v2v-emergencyVehicleAlert-nrv2x.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libnr}
    ${libtraci}
)

build_lib_example(
    NAME v2x-emulator
    SOURCE_FILES v2x-emulator.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libfd-net-device}
    ${libtraci}
)

build_lib_example(
    NAME v2v-80211p-gps-tc-example
    SOURCE_FILES v2v-80211p-gps-tc-example.cc
    LIBRARIES_TO_LINK
    ${libautomotive}
    ${libwave}
    ${libgps-tc}
    ${libtraci}
)
build_lib_example(
        NAME v2p-vam-80211p
        SOURCE_FILES v2p-vam-80211p.cc
        LIBRARIES_TO_LINK
        ${libautomotive}
        ${libwave}
        ${libtraci}
)

build_lib_example(
        NAME v2v-simple-cam-exchange-80211p
        SOURCE_FILES v2v-simple-cam-exchange-80211p.cc
        LIBRARIES_TO_LINK
        ${libautomotive}
        ${libwave}
        ${libtraci}
)

build_lib_example(
        NAME v2v-congestion-80211p
        SOURCE_FILES v2v-congestion-80211p.cc
        LIBRARIES_TO_LINK
        ${libautomotive}
        ${libwave}
        ${libtraci}
)

build_lib_example(
        NAME v2v-cam-congestion-nrv2x
        SOURCE_FILES v2v-cam-congestion-nrv2x.cc
        LIBRARIES_TO_LINK
        ${libautomotive}
        ${libnr}
        ${libtraci}
)


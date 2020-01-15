-- premake5.lua

local tools = require ('tools')
local install_prefix = "/usr/local"
local ns3version = "3.28"
local PROTO_PATH    = "../../../libs/vsimrti-collections/src/main/resources/com/dcaiti/vsimrti/coupling"
local PROTO_CC_PATH = "../../../libs/vsimrti-collections/src/main/cpp/com/dcaiti/vsimrti/coupling"

--local pkgconfig = require 'pkgconfig'
--print ( pkgconfig.load ( 'zlib' ) )
--print ( pkgconfig.load ( 'glib-2.0' ) )

local autoconf = require 'autoconf'

autoconfigure {
        ['config.h'] = function (cfg)
                          check_include(cfg, 'HAVE_PTHREAD_H', 'pthread.h')
                          check_include(cfg, 'HAVE_NS3_APPLICATION_H', 'ns3/application.h')
                       end
}

local PROTOC = tools.check_bin ( 'protoc' )

newoption {
   trigger     = "generate-protobuf",
   description = "Generate/Regenerate protocol buffers with protobuf compiler"
}

newoption {
   trigger     = "install",
   description = "install target into '" .. install_prefix .. "'"
}


workspace "ns3-federate"
   configurations { "Debug", "Release" }

project "ns3-federate"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   files { "scratch/VSimRTI_Starter.cc"
         , "src/VSimRTI/model/**.h"
         , "src/VSimRTI/model/**.cc" 
         , PROTO_CC_PATH .. "/ClientServerChannel.h"
         , PROTO_CC_PATH .. "/ClientServerChannel.cc"
         , PROTO_CC_PATH .. "/ClientServerChannelMessages.pb.h"
         , PROTO_CC_PATH .. "/ClientServerChannelMessages.pb.cc"
         }

   includedirs { "/usr/include"
               , "/usr/include/libxml2"
               , "src"
               , PROTO_CC_PATH
               }

   libdirs { "/usr/lib" }

   links { "pthread"
         , "protobuf"
         , "xml2"
         }

  configuration "generate-protobuf"
    prebuildcommands { PROTOC .. " --cpp_out=" .. PROTO_CC_PATH
                       .. " --proto_path=" .. PROTO_PATH
                       .. " ClientServerChannelMessages.proto"
                     }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      links { "ns" .. ns3version .. "-antenna-debug"
--            , "ns" .. ns3version .. "-aodv-debug"
            , "ns" .. ns3version .. "-applications-debug"
            , "ns" .. ns3version .. "-bridge-debug"
--            , "ns" .. ns3version .. "-buildings-debug"
            , "ns" .. ns3version .. "-config-store-debug"
            , "ns" .. ns3version .. "-core-debug"
--            , "ns" .. ns3version .. "-csma-debug"
--            , "ns" .. ns3version .. "-csma-layout-debug"
--            , "ns" .. ns3version .. "-dsdv-debug"
--            , "ns" .. ns3version .. "-dsr-debug"
            , "ns" .. ns3version .. "-energy-debug"
--            , "ns" .. ns3version .. "-fd-net-device-debug"
--            , "ns" .. ns3version .. "-flow-monitor-debug"
            , "ns" .. ns3version .. "-internet-apps-debug"
            , "ns" .. ns3version .. "-internet-debug"
--            , "ns" .. ns3version .. "-lr-wpan-debug"
--            , "ns" .. ns3version .. "-lte-debug"
--            , "ns" .. ns3version .. "-mesh-debug"
            , "ns" .. ns3version .. "-mobility-debug"
            , "ns" .. ns3version .. "-mpi-debug"
--            , "ns" .. ns3version .. "-netanim-debug"
            , "ns" .. ns3version .. "-network-debug"
--            , "ns" .. ns3version .. "-nix-vector-routing-debug"
--            , "ns" .. ns3version .. "-olsr-debug"
--            , "ns" .. ns3version .. "-point-to-point-debug"
--            , "ns" .. ns3version .. "-point-to-point-layout-debug"
            , "ns" .. ns3version .. "-propagation-debug"
--            , "ns" .. ns3version .. "-sixlowpan-debug"
            , "ns" .. ns3version .. "-spectrum-debug"
            , "ns" .. ns3version .. "-stats-debug"
--            , "ns" .. ns3version .. "-tap-bridge-debug"
--            , "ns" .. ns3version .. "-test-debug"
--            , "ns" .. ns3version .. "-topology-read-debug"
            , "ns" .. ns3version .. "-traffic-control-debug"
--            , "ns" .. ns3version .. "-uan-debug"
--            , "ns" .. ns3version .. "-virtual-net-device-debug"
            , "ns" .. ns3version .. "-wave-debug"
            , "ns" .. ns3version .. "-wifi-debug"
--            , "ns" .. ns3version .. "-wimax-debug"
           }

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      links { "ns3-dev-antenna-optimized"
--            , "ns3-dev-aodv-optimized"
            , "ns3-dev-applications-optimized"
            , "ns3-dev-bridge-optimized"
--            , "ns3-dev-buildings-optimized"
            , "ns3-dev-config-store-optimized"
            , "ns3-dev-core-optimized"
--            , "ns3-dev-csma-optimized"
--            , "ns3-dev-csma-layout-optimized"
--            , "ns3-dev-dsdv-optimized"
--            , "ns3-dev-dsr-optimized"
            , "ns3-dev-energy-optimized"
--            , "ns3-dev-fd-net-device-optimized"
--            , "ns3-dev-flow-monitor-optimized"
            , "ns3-dev-internet-apps-optimized"
            , "ns3-dev-internet-optimized"
--            , "ns3-dev-lr-wpan-optimized"
--            , "ns3-dev-lte-optimized"
--            , "ns3-dev-mesh-optimized"
            , "ns3-dev-mobility-optimized"
            , "ns3-dev-mpi-optimized"
--            , "ns3-dev-netanim-optimized"
            , "ns3-dev-network-optimized"
--            , "ns3-dev-nix-vector-routing-optimized"
--            , "ns3-dev-olsr-optimized"
--            , "ns3-dev-point-to-point-optimized"
--            , "ns3-dev-point-to-point-layout-optimized"
            , "ns3-dev-propagation-optimized"
--            , "ns3-dev-sixlowpan-optimized"
            , "ns3-dev-spectrum-optimized"
            , "ns3-dev-stats-optimized"
--            , "ns3-dev-tap-bridge-optimized"
--            , "ns3-dev-test-optimized"
--            , "ns3-dev-topology-read-optimized"
            , "ns3-dev-traffic-control-optimized"
--            , "ns3-dev-uan-optimized"
--            , "ns3-dev-virtual-net-device-optimized"
            , "ns3-dev-wave-optimized"
            , "ns3-dev-wifi-optimized"
--            , "ns3-dev-wimax-optimized"
           }

    configuration "Debug"
        libdirs { "bin/Debug" }

    configuration "Release"
        libdirs { "bin/Release" }

    configuration "install"
        postbuildcommands { "mkdir -p " .. install_prefix .. "/bin"
                          , "cp bin/%{cfg.buildcfg}/ns3-federate " .. install_prefix .. "/bin"
                          }

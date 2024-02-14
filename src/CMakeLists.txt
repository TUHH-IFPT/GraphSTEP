find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

add_subdirectory(tools)

add_library(GraphSTEPLib SHARED
            Graph.cpp
            PullStep.cpp
            PushStep.cpp
            VersionControl.cpp
            DerivedStepTypes.cpp
            ManipulateGraph.cpp
            FilterGraph.cpp
            AnalyseGraph.cpp
            STEPAnalyser.cpp
)

target_include_directories(GraphSTEPLib PUBLIC 
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR}/tools
                            yaml-cpp/include
                        )

target_link_libraries(GraphSTEPLib  
                      ${STEPCODE_BUILD_DIR}/lib/libbase.so 
                      ${STEPCODE_BUILD_DIR}/lib/libstepcore.so 
                      ${STEPCODE_BUILD_DIR}/lib/libstepeditor.so 
                      ${STEPCODE_BUILD_DIR}/lib/libstepdai.so 
                      ${STEPCODE_BUILD_DIR}/lib/libsteputils.so 
                      ${STEPCODE_BUILD_DIR}/lib/libsdai_ap242.so 
                      nlohmann_json::nlohmann_json
                      spdlog::spdlog
                      Eigen3::Eigen
                      yaml-cpp
                      cpr::cpr
                      Tools
)
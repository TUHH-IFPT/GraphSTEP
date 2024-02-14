add_library(Tools SHARED
            RestTools.cpp
            CypherParser.cpp 
            TypesNeo4j.cpp
            AdjacencyMatrix.cpp
            Logger.cpp
)

target_include_directories(Tools PUBLIC ${EXTERNAL_DIR}/spdlog/include)

target_link_libraries(Tools PRIVATE cpr)
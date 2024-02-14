add_library(Tools SHARED
            RestTools.cpp
            CypherParser.cpp 
            TypesNeo4j.cpp
            AdjacencyMatrix.cpp
            Logger.cpp
)

target_link_libraries(Tools PRIVATE cpr::cpr spdlog::spdlog)
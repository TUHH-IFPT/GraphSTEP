#include <iostream>
#include "AnalyseGraph.h"
#include "Graph.h"
#include "FilterGraph.h"
#include "ManipulateGraph.h"
#include "PullStep.h"
#include "PushStep.h"
#include "VersionControl.h"

#include <bits/stdc++.h>

class GraphSTEPCLI {
    public:
    void printHelp() {
        std::cout << "Usage: graphstep [COMMAND] [OPTIONS]" << std::endl;
        std::cout << "COMMANDs:" << std::endl;
        std::cout << "  create [FILENAME]               Create a graph from a STEP file" << std::endl;
        std::cout << "  delete                          Delete the database" << std::endl;
        std::cout << "  read                            Read the database" << std::endl;
        std::cout << "  filter                          Filter the database" << std::endl;
        std::cout << "  restore-filter                  Restores the unfiltered state of the productgraph" << std::endl;
        std::cout << "  filter-brep [PARTNAME]          Filter the boundary representation of a part" << std::endl;
        std::cout << "  restore-brep                    Restores boundary representation in the productgraph" << std::endl;
        std::cout << "  add-part                        Add a part to the database" << std::endl;
        std::cout << "  duplicate-part                  Duplicate a part" << std::endl;
        std::cout << "  movePart [PARTNAME] [X] [Y] [Z] Move a part" << std::endl;
        std::cout << "  version-control-test            Test version control pipeline. Commits changes and checks a commit out" << std::endl;
    }

    int createGraph(std::string filePath) {
        auto databaseInfo = getDatabaseConfig();
        PushSTEP database(filePath, databaseInfo);
        database.deleteDatabase();

        if (!database.build()) {
            return -1;
        }
        return 0;
    }

    int readGraph(std::string outputDirectory) {
        auto databaseInfo = getDatabaseConfig();
        std::string out = "";
        if (outputDirectory.empty()) {
            out = databaseInfo.databaseName + "_out.stp";
        } else {
            out = outputDirectory + databaseInfo.databaseName + "_out.stp";
        }
        PullSTEP database(out, databaseInfo);
        database.writeStep();
        return 0;
    }

    int deleteDatabase() {
        auto databaseInfo = getDatabaseConfig();
        Graph graph(databaseInfo);
        graph.deleteDatabase();

        return 0;
    }

    int addPart(std::string filepath, std::string partName, std::string assemblyName) {
        auto databaseInfo = getDatabaseConfig();
        ManipulateGraph test_part1(databaseInfo);
        test_part1.addNewPart(filepath, "Cube", "test_assembly");
        return 0;
    }

    int duplicatePart(std::string newPartName, std::string assemblyPartName) {
        auto databaseInfo = getDatabaseConfig();
        ManipulateGraph manipulator(databaseInfo);
        manipulator.addNewProductOccurrence("newPartName", "assemblyPartName");

        return 0;
    }

    int movePart(std::string partName, Position position) {
        auto databaseInfo = getDatabaseConfig();
        ManipulateGraph manipulator(databaseInfo);
        manipulator.movePart(partName, position);

        return 0;
    }

    int filterGraph() {
        auto databaseInfo = getDatabaseConfig();
        DatabaseInfo makroDb = databaseInfo;
        makroDb.databaseName = "macro";

        FilterGraph test(makroDb);
        test.initDatabase(databaseInfo);
        test.collect(DatabaseFilter::Circle);
        test.collect(DatabaseFilter::Plane);
        test.collect(DatabaseFilter::Line);
        test.storeSubgraphs();
        test.replace(databaseInfo);
        test.restore();

        return 0;
    }

    int restoreFilter() {
        auto databaseInfo = getDatabaseConfig();
        DatabaseInfo makroDb = databaseInfo;
        makroDb.databaseName = "macro";
        
        FilterGraph test(makroDb);
        test.initDatabase(databaseInfo);
        test.restore();

        return 0;
    }

    int restoreBRep(std::string partName) {
        auto databaseInfo = getDatabaseConfig();
        ManipulateGraph productGraph(databaseInfo);
        DatabaseInfo makroDb = databaseInfo;
        makroDb.databaseName = "macro";
        ManipulateGraph macros(makroDb);

        macros.loadAdjacencyMatrix();
    
        auto brep = macros.getAdjacencyMatrix();
        productGraph.addBrep(brep, partName);
    
        return 0;
    }

    int filterBRep(std::string partName) {
        auto databaseInfo = getDatabaseConfig();
        ManipulateGraph productGraph(databaseInfo);
        DatabaseInfo makroDb = databaseInfo;
        makroDb.databaseName = "macro";
        ManipulateGraph macros(makroDb);

        // collect
        AdjacencyMatrix bRepPart = productGraph.collectBrep(partName);

        // Remove
        productGraph.deleteSubgraph(bRepPart);

        // Save
        macros.createGraph(bRepPart);

        return 0;
    }

    int versionControlTestPipeline(std::string dataDirectory) {
        auto databaseInfo = getDatabaseConfig();
        std::string filename = "test_assembly.stp";
        VersionControl control(databaseInfo);
        control.deleteDatabase();

        PushSTEP database(dataDirectory + "/" + filename,
                        databaseInfo);
        database.deleteDatabase();
        database.build();
        database.commitChanges("assembly_added");

        ManipulateGraph test_part1(databaseInfo);
        test_part1.addNewPart(dataDirectory + "/test_cube.stp", "Cube",
                            "test_assembly");

        test_part1.movePart("Cube", {10, 10, 10});

        test_part1.commitChanges("cube_added");

        control.checkout("assembly_added");

        return 0;
    }
};


std::vector<std::string> parseArguments(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    return args;
}

int main(int argc, char* argv[]) {
    
    // start stopwatch
    auto start = std::chrono::high_resolution_clock::now();
    GraphSTEPCLI graphCLI;

    if (argc < 2) {
        std::cout << "Error: No command provided.\n";
        graphCLI.printHelp();
        return 1;
    }

    std::string command = argv[1];
    
    if (command == "create") {
        if (argc != 3) {
            std::cout << "Error: Invalid number of arguments for create command.\n";
            graphCLI.printHelp();
            return 1;
        }
        std::string filePath = argv[2];
        return graphCLI.createGraph(filePath);
    }

    else if (command == "delete") {
        return graphCLI.deleteDatabase();
    }

    else if (command == "read") {
        if (argc != 3) {
            std::cout << "Error: Invalid number of arguments for read command.\n";
            graphCLI.printHelp();
            return 1;
        }
        std::string outputDirectory = argv[2];
        return graphCLI.readGraph(outputDirectory);
    }
    else if (command == "move-part") {
        if (argc != 6) {
            std::cout << "Error: Invalid number of arguments for move-part command.\n";
            graphCLI.printHelp();
            return 1;
        }
        return graphCLI.movePart(std::string(argv[2]), Position{std::stod(argv[3]), std::stod(argv[4]), std::stod(argv[5])});
    }
    else if (command == "add-part") {
        if (argc != 5) {
            std::cout << "Error: Invalid number of arguments for add-part command.\n";
            graphCLI.printHelp();
            return 1;
        }
        return graphCLI.addPart(std::string(argv[2]), std::string(argv[3]), std::string(argv[4]));
    }
    else if (command == "duplicate-part") {
        if (argc != 4) {
            std::cout << "Error: Invalid number of arguments for duplicate-part command.\n";
            graphCLI.printHelp();
            return 1;
        }
        return graphCLI.duplicatePart(std::string(argv[2]), std::string(argv[3]));
    }
    else if (command == "filter") {
        return graphCLI.filterGraph();
    }
    else if (command == "restore-filter") {
        return graphCLI.restoreFilter();
    }
    else if (command == "filter-brep") {
        if (argc != 3) {
            std::cout << "Error: Invalid number of arguments for filter-brep command.\n";
            graphCLI.printHelp();
            return 1;
        }
        return graphCLI.filterBRep(std::string(argv[2]));
    }
    else if (command == "restore-brep") {
        if (argc != 3) {
            std::cout << "Error: Invalid number of arguments for restore-brep command.\n";
            graphCLI.printHelp();
            return 1;
        }
        return graphCLI.restoreBRep(std::string(argv[2]));
    }
    else if (command == "version-control-test") {
        if (argc != 3) {
            std::cout << "Error: Invalid number of arguments for restore-brep command.\n";
            graphCLI.printHelp();
            return 1;
        }
        return graphCLI.versionControlTestPipeline(std::string(argv[2]));
    }
    else if (command == "help") {
        graphCLI.printHelp();
        return 0;
    } else {
        std::cout << "Error: Unknown command '" << command << "'.\n";
        graphCLI.printHelp();
        return 1;
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    // convert microseconds to seconds
    std::cout << "Time taken: " << duration.count() / 1000000.0 << " seconds"
              << std::endl;

    return 0;
}
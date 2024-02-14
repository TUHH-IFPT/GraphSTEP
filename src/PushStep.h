#pragma once

#include <filesystem>

#include "Graph.h"
#include "Tools.hpp"
#include "VersionControl.h"

/**
 * @brief PushStep
 * translates a STEP file into a Neo4j graph
**/

class STEPAnalyser {
   public:
    STEPAnalyser(std::string path);
    ~STEPAnalyser();

    int getNumEntities() { return m_numEntities; }

   private:
    void analyseFile();

    // object to store all instances/attributes etc. of the .stp file
    InstMgr m_lstInst;  
    std::string m_filePath;
    int m_numEntities;
};

class PushSTEP : public Graph {
   public:
    PushSTEP();
    PushSTEP(std::string path, DatabaseInfo databaseInfo);
    ~PushSTEP();

    std::pair<std::string, std::vector<std::string>> convertTypedSet(
        std::string select);

    void commitChanges(std::string message);

    void createNode(Node node) override;
    void createRelation(Node from, Node to, std::string relation) override;

    Blob getTrackedChanges() { return m_trackChanges; }

    // Create new graph
    bool build();

    // Create the cypher queries for all nodes
    bool createInstanceNodes();

    // Create the cypher queries to link the nodes to each other
    bool createRelations();

    // Return StepEntity struct from a given id (e.g. #24)
    EntityInfo getEntityInfoFromId(std::string idStr);

    // Search primitive attributes and create properties for the nodes
    void getAttributesForNodes(SDAI_Application_instance_ptr pInst, Node &node);

    // Read the attributes of the current entity which are needed to create
    // relationships between the nodes
    void getAttributesForRelations(SDAI_Application_instance_ptr pInst);

   private:
    std::string m_filePath;
    std::string m_fileName;

    // stores the name and the id of the current instance
    EntityInfo m_entity; 
    
    std::map<std::string, Node> m_nodeIdMap;
    Blob m_trackChanges;
};
#pragma once

// STEPCode includes
extern void SchemaInit(class Registry &);

#include <ExpDict.h>
#include <Registry.h>
#include <STEPaggregate.h>
#include <STEPattribute.h>
#include <STEPcomplex.h>
#include <STEPfile.h>
#include <errordesc.h>
#include <sdai.h>

#include <algorithm>
#include <string>

#include "sc_benchmark.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sc_getopt.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "AdjacencyMatrix.h"
#include "CypherParser.h"
#include "DatabaseError.hpp"
#include "DerivedStepTypes.h"
#include "RestTools.h"
#include "Logger.h"

/**
 * @brief Graph
 * base class for database manipulations
**/

using json = nlohmann::json;

namespace Neo4j {
// constants for json keys
const std::string HOST = "host";
const std::string DATABASE = "database";
const std::string CREDS = "user";
const std::string CREDS_NAME = "name";
const std::string CREDS_PASSWORD = "password";
}  // namespace Neo4j

struct Credentials {
    std::string name = "";
    std::string password = "";
};

struct DatabaseInfo {
    std::string hostName;
    std::string databaseName;
    Credentials credentials;
};

inline DatabaseInfo JsonStringToDatabaseInfo(const std::string &jsonString) {
    DatabaseInfo databaseInfo;
    json jsonCredentials = json::parse(jsonString);

    databaseInfo.hostName = jsonCredentials[Neo4j::HOST];
    databaseInfo.databaseName = jsonCredentials[Neo4j::DATABASE];
    databaseInfo.credentials.name =
        jsonCredentials[Neo4j::CREDS][Neo4j::CREDS_NAME];
    databaseInfo.credentials.password =
        jsonCredentials[Neo4j::CREDS][Neo4j::CREDS_PASSWORD];
    return databaseInfo;
}

inline DatabaseInfo getDatabaseConfig() {
    std::string yaml_path = getDataDir() + "database_config.yaml";
    YAML::Node config = YAML::LoadFile(yaml_path);
        
    std::string host = config["host"].as<std::string>();
    std::string databaseName = config["database_name"].as<std::string>();
    std::string user = config["credentials"]["user"].as<std::string>();
    std::string password = config["credentials"]["password"].as<std::string>();

    DatabaseInfo databaseInfo = {.hostName = config["host"].as<std::string>(),
                             .databaseName = config["database_name"].as<std::string>(),
                             .credentials = {.name = config["credentials"]["user"].as<std::string>(), 
                             .password = config["credentials"]["password"].as<std::string>()}};

    return databaseInfo;
}

class Graph {
   public:
    Graph();
    Graph(std::string path, DatabaseInfo databaseInfo);
    Graph(DatabaseInfo databaseInfo);
    ~Graph();

    void initRestInterface(DatabaseInfo databaseInfo);

    void createGraph(AdjacencyMatrix matrix);

    void deleteDatabase();
    void deleteSubgraph(AdjacencyMatrix subgraph);
    void deleteNode(Node node);

    virtual void createNode(Node node);
    virtual void createRelation(Node from, Node to, std::string relation);
    virtual void modifyNode(Node node, Node modification);
    virtual void modifyNode(Node node, Property newProperty);

    void setPath(std::string path) { m_path = path; }

    std::string getJsonFromCypher(const std::string &cypher);

    // adds a new statement object to the m_queries vector
    void pushQueryToJson(const std::string &query);

    // converts multiple cypher queries to a json string
    std::string cypherListToJson();

    // converts a single cypher query to a json string
    std::string cypherToJson(const std::string &cypher);

    // sends the queries and clears the m_queries vector
    std::string sendQueries();

    // sends a single cypher query
    std::string sendQuery(const std::string &query);

    // returns the entries of a aggregate attribute
    std::vector<string> getEntriesAggregate(string str);

    // return all labels contained in the graph
    std::vector<std::string> getAllLabels();

    // return all ids of a specific node
    std::vector<std::string> getNodeIds(std::string label);

    // returns all nodes of a specific label
    std::vector<Node> getNodes(std::string label);

    // returns the property of a node
    std::vector<Property> getProperties(Node node);

    // return all underlying nodes (aka. subtree)
    std::vector<Node> getTreeNodes(Node parentNode);

    // return subgraph from given node to its leaf nodes
    AdjacencyMatrix getSubgraph(Node node);

    // returns the children of a specific parent node and its relation
    std::vector<std::pair<Node, std::string>> getChildNodes(Node parentNode);

    // returns only the child nodes of a specific parent node
    std::vector<Node> getChildNodeList(Node parentNode);

    std::vector<Node> getAllParents(Node childNode, int depth = 1);
    std::vector<Node> getAllChildren(Node childNode, int depth = 1);

    Node getNextNode(Node from, std::string relation);

    // returns the node from the database that matches the given node
    Node getNode(Node node);

    AdjacencyMatrix nodesToAdjacencyMatrix(std::vector<Node> nodes);

    // append subgraph to existing graph
    void appendGraph(AdjacencyMatrix matrix);

    // json parser functions
    std::vector<Node> jsonToNodeList(std::string jsonString);

    // builds the adjacency matrix of the graph
    bool loadAdjacencyMatrix();

    void setAdjacencyMatrix(AdjacencyMatrix matrix) {
        m_matrix.clear();
        m_matrix.setAdjacencyMatrix(matrix);
    }

    AdjacencyMatrix getAdjacencyMatrix() { return m_matrix; }

   protected:
    // path to stepfile
    std::string m_path;
    
    // stores all cypher queries
    std::vector<std::string> m_cypherQueries;

    // file to store all cypher queries
    ofstream m_cypherLog;

    // used to exchange data with the server via restapi
    std::unique_ptr<RestInterface> m_pRest;
    
    // stores a cypher query
    CypherParser m_cypher;  

    // stores relations between all nodes and their properties
    AdjacencyMatrix m_matrix;

    // stores the database information (including credentials)
    DatabaseInfo m_databaseInfo;

    // object to store all instances/attributes etc. of the .stp file
    InstMgr m_lstInst;  

    // json array, stores multiple statement objects e.g. queries
    json m_queries;
};
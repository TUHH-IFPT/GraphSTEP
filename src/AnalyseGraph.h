#pragma once
#include <SdaiHeaderSchema.h>

#include "Graph.h"
#include "Tools.hpp"
#include "TypesNeo4j.h"

/**
 * @brief AnalyseGraph
 * analysis in terms of size and structure
**/

struct ProductHierarchy {
    Node base;
    Node subPart;
    Node occurance;
};

inline json propertiesToJson(std::vector<Property> properties) {
    json propertiesJson;

    for (auto &property : properties)
        propertiesJson[property.variable] = property.value;

    return propertiesJson;
}

inline json nodeToJson(Node node) {
    json nodeJson;
    nodeJson["label"] = node.getLabel();
    nodeJson["id"] = node.getId();
    nodeJson["properties"] = propertiesToJson(node.getProperties());

    return nodeJson;
}

class GraphAnalyser : public Graph {
   public:
    GraphAnalyser(DatabaseInfo databaseInfo);
    ~GraphAnalyser();

    int getNumNodes() { return m_numNodes; }
    int getNumEdges() { return m_numEdges; }
    std::string getProductHierarchyJson();
    std::string exportJsonString();

   private:
    void analyseGraph();
    void determineNumNodes();
    void determineNumEdges();
    void determineProductHierarchy();

    int m_numNodes;
    int m_numEdges;
    std::vector<ProductHierarchy> m_hierarchies;
};
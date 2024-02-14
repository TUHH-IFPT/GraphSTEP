#include "AnalyseGraph.h"

GraphAnalyser::GraphAnalyser(DatabaseInfo databaseInfo)
    : Graph(databaseInfo) {
    analyseGraph();
}

GraphAnalyser::~GraphAnalyser() {}

void GraphAnalyser::analyseGraph() {
    determineNumNodes();
    determineNumEdges();
    determineProductHierarchy();
}

void GraphAnalyser::determineNumNodes() {
    
    //MATCH (n) RETURN count(n) as count

    Node node;
    node.setVariable("a");

    std::string query = m_cypher.matchQuery(node, "count(n) as count");
    std::string response = sendQuery(query);

    if (!response.empty()) {
        json data = json::parse(response);
        m_numNodes = data["results"][0]["data"][0]["row"][0];
    }
}

void GraphAnalyser::determineNumEdges() {
    std::string query = "MATCH ()-[r]->() \n RETURN count(r) as count";
    std::string response = sendQuery(query);

    if (!response.empty()) {
        json data = json::parse(response);
        m_numEdges = data["results"][0]["data"][0]["row"][0];
    }
}

void GraphAnalyser::determineProductHierarchy() {
    std::vector<Node> nextAssemblyUsageOccurrences =
        getNodes("Next_Assembly_Usage_Occurrence");
    for (auto &node : nextAssemblyUsageOccurrences) {
        auto childNodes = getChildNodes(node);
        ProductHierarchy hierarchy;

        // cast node to NextAssemblyUsageOccurrence
        hierarchy.occurance = node;

        for (auto &childNode : childNodes) {
            if (childNode.second == "relating_product_definition")
                hierarchy.base = childNode.first;

            if (childNode.second == "related_product_definition")
                hierarchy.subPart = childNode.first;
        }
        m_hierarchies.push_back(hierarchy);
    }
}

std::string GraphAnalyser::exportJsonString() {
    json data;
    data["nodes"] = m_numNodes;
    data["edges"] = m_numEdges;
    return data.dump();
}

std::string GraphAnalyser::getProductHierarchyJson() {
    json data;
    for (auto &hierarchy : m_hierarchies) {
        json hierarchyJson;
        hierarchyJson["base"] = nodeToJson(hierarchy.base);
        hierarchyJson["subPart"] = nodeToJson(hierarchy.subPart);
        hierarchyJson["occurance"] = nodeToJson(hierarchy.occurance);
        data.push_back(hierarchyJson);
    }

    return data.dump(4);
}
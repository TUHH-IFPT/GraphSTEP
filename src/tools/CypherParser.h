#pragma once

/**
 * @brief CypherParser
 * used to build cypher query strings 
 * queries are send to the neo4j database via the RestTools module
**/

#include <iostream>
#include <vector>

#include "Tools.hpp"
#include "TypesNeo4j.h"

struct EntityInfo {
    std::string name;
    std::string fileId;
};

class CypherParser {
   public:
    CypherParser();
    ~CypherParser();

    // MATCH (a:from),(b:to)
    // CREATE (a)-[r:relation]->(b)
    std::string createRelation(Node from, Node to, std::string relation);

    // CREATE(node)
    std::string createNodeQuery(Node node);

    // MATCH(from) RETURN ret
    std::string matchQuery(Node from, std::string ret = "");

    // MATCH(from)-[relation]->(to) RETURN ret
    std::string matchQuery(Node from, std::string relation, Node to,
                           std::string ret = "");

    // MATCH(node) WHERE condition RETURN ret
    std::string conditionQuery(Node node, Property condition, std::string ret);

    // MATCH(node) DETACH DELETE node
    std::string deleteQuery(Node node);

    // MATCH(node) SET modified.property.variable = modified.property.value
    std::string modifyNodeQuery(Node node, Node modified);

    // MATCH(node) SET newProperty.variable = newProperty.value
    std::string modifyNodeQuery(Node node, Property newProperty);

    // returns a string that can be used in a cypher query to specify the depth
    // of a relation
    std::string depthString(std::string variable, int depth);
};
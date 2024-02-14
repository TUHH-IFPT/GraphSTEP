#pragma once

#include "Tools.hpp"
#include "TypesNeo4j.h"

/**
 * @brief AjacencyMatrix
 * implementation of an adjacency matrix
 * stores all relations between the nodes of a
     *graph.
     *
     *	Example:
     *  __________________________________________________
     * |         |      Node1       |        Node2        |
     * |_________|__________________|_____________________|
     * |  Node1  |    no Relation   |      Relation2      |
     * |_________|__________________|_____________________|
     * |  Node2  |     Relation1    |    no Relation      |
     * |_________|__________________|_____________________|
     *
**/

class AdjacencyMatrix {
   public:
    AdjacencyMatrix();
    ~AdjacencyMatrix();

    void setNodes(std::vector<Node> nodes) { m_nodes = nodes; }
    void setAdjacencyMatrix(AdjacencyMatrix matrix) {
        m_nodes = matrix.getNodes();
        m_relations = matrix.getRelationMatrix();
    }

    std::vector<Node> getNodes() { return m_nodes; }
    Matrix getRelationMatrix() { return m_relations; }

    void addNode(Node node) { m_nodes.push_back(node); }
    void insertNode(Node node) { m_nodes.insert(m_nodes.begin(), node); }
    void addRelationRow(std::vector<std::string> row) {
        m_relations.push_back(row);
    }

    void replaceNode(Node node, Node newNode);
    std::string toString();
    void clear();

    // returns the nodes that follow a specified relation from a given node
    Node getNextNode(Node from, std::string relation);

    // returns the parent of an entity which is part of a complex instance
    Node findComplexParent(Node childNode);

    std::vector<Node> findParents(Node childNode);

    // returns the properties of a node in the adjacency matrix
    std::vector<Property> findProperties(Node searchNode);

    // returns all relations of the given node
    std::vector<std::string> getNodeRelations(Node node);

    // searches a node in the AdjacencyMatrix and returns its child nodes
    std::vector<Node> findChildren(Node parentNode);

    // return nodes with specific label
    std::vector<Node> findNodes(std::string label);

    // searches complex nodes and sets their type to NodeType::Complex
    void markComplexNodes();

   private:
    std::vector<Node> m_nodes;
    Matrix m_relations;
};
#include "AdjacencyMatrix.h"

AdjacencyMatrix::AdjacencyMatrix() {}

AdjacencyMatrix::~AdjacencyMatrix() {}

void AdjacencyMatrix::replaceNode(Node node, Node newNode) {
    for (auto &entry : m_nodes) {
        if (entry.compare(node)) entry = newNode;
    }
}

std::string AdjacencyMatrix::toString() {
    std::string matrixStr = "";
    Matrix matrix;
    auto nodes_copy = m_nodes;
    auto relations_copy = m_relations;
    std::vector<std::string> row;
    row.push_back("");  // first element is empty
    for (auto &node : nodes_copy) {
        row.push_back(node.toString());
    }
    matrix.push_back(row);
    row.clear();

    int row_counter = 0;
    for (auto &node : nodes_copy) {
        row.push_back(node.toString());
        for (auto &relationStr : relations_copy[row_counter])
            row.push_back(relationStr);
        matrix.push_back(row);
        row.clear();
        ++row_counter;
    }

    return printMatrix(matrix);
}

void AdjacencyMatrix::clear() {
    m_nodes.clear();
    m_relations.clear();
}

Node AdjacencyMatrix::getNextNode(Node from, std::string relation) {
    Node to;
    int currentNode = 0;
    for (auto &node : m_nodes) {
        if (node.compare(from)) {
            int currentRelation = 0;
            for (auto &relationStr : m_relations[currentNode]) {
                if (relationStr.find(";") != std::string::npos) {
                    // multiple relations found!
                    std::vector<std::string> relations =
                        getListFromStrings(relationStr);
                    auto it =
                        std::find(relations.begin(), relations.end(), relation);

                    if (it != relations.end()) {
                        to = m_nodes[currentRelation];
                        break;
                    }
                } else if (relationStr == relation) {
                    to = m_nodes[currentRelation];
                    break;
                }
                ++currentRelation;
            }
        }
        ++currentNode;
    }

    if (to.getNodeType() == NodeType::COMPLEX) return findComplexParent(to);

    return to;
}

Node AdjacencyMatrix::findComplexParent(Node complexChildNode) {
    Node complexNode;

    std::vector<Node> parents = findParents(complexChildNode);

    for (auto &parent : parents) {
        if (parent.getLabel() == TYPE_COMPLEX) return parent;
    }

    return complexNode;
}

std::vector<Node> AdjacencyMatrix::findParents(Node childNode) {
    int currentNode = 0;
    std::vector<Node> parents;
    for (auto &node : m_nodes) {
        if (node.compare(childNode)) {
            int currentRow = 0;
            for (auto &row : m_relations) {
                if (!row[currentNode].empty()) {
                    // parent found!
                    parents.push_back(m_nodes[currentRow]);
                }

                ++currentRow;
            }
            return parents;
        }
        ++currentNode;
    }
    return parents;
}

std::vector<Property> AdjacencyMatrix::findProperties(Node searchNode) {
    std::vector<Property> properties;

    for (auto &node : m_nodes) {
        if (node.compare(searchNode)) return node.getProperties();
    }
    return properties;
}

std::vector<std::string> AdjacencyMatrix::getNodeRelations(Node nodeRelation) {
    std::vector<std::string> relations;
    int currentNode = 0;

    for (auto &node : m_nodes) {
        if (node.compare(nodeRelation)) {
            for (auto &relation : m_relations[currentNode]) {
                if (relation.find(";") != std::string::npos) {
                    // multiple relations found!
                    std::vector<std::string> multipleRelations =
                        getListFromStrings(relation);
                    relations.insert(relations.end(), multipleRelations.begin(),
                                     multipleRelations.end());
                } else if (!relation.empty())
                    relations.push_back(relation);
            }

            return relations;
        }
        ++currentNode;
    }
    return relations;
}

std::vector<Node> AdjacencyMatrix::findChildren(Node parentNode) {
    std::vector<Node> children;
    int currentNode = 0;

    for (auto &node : m_nodes) {
        if (node.compare(parentNode)) {
            std::vector<std::string> relations = m_relations[currentNode];
            int currentRelation = 0;
            for (auto &relation : relations) {
                // if relation is not empty --> node must be linked with another
                // one
                if (!relation.empty()) {
                    // if(!m_matrix.nodes[currentRelation].compare(parentNode))
                    children.push_back(m_nodes[currentRelation]);
                }

                ++currentRelation;
            }
        }
        ++currentNode;
    }
    return children;
}

std::vector<Node> AdjacencyMatrix::findNodes(std::string label) {
    std::vector<Node> nodes;

    for (auto &node : m_nodes) {
        if (node.getLabel() == label) nodes.push_back(node);
    }
    return nodes;
}

void AdjacencyMatrix::markComplexNodes() {
    std::vector<Node> complexNodes;

    // collect complex types
    for (auto &node : m_nodes) {
        if (node.getNodeType() == NodeType::COMPLEX) {
            std::vector<Node> children = findChildren(node);
            complexNodes.insert(complexNodes.end(), children.begin(),
                                children.end());
        }
    }

    // mark complex nodes
    for (auto &node : m_nodes) {
        for (auto &complexNode : complexNodes) {
            if (node.compare(complexNode)) node.setNodeType(NodeType::COMPLEX);
        }
    }
}

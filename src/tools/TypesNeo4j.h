#pragma once

#include "Tools.hpp"

/**
 * @brief TypesNeo4j
 * definition of data structures for Neo4j components
 * derivation of GraphSTEP speficic data types 
**/

const std::string TYPE_COMPLEX = "COMPLEX_TYPE";
const std::string TYPE_SET = "SET_TYPE";
const std::string TYPE_LIST = "LIST_TYPE";

struct Property {
    std::string variable = "";
    std::string value = "";
};

inline std::string propertyToString(Property property) {
    return property.variable + "=" + removeParentheses(property.value);
}

inline std::string propertiesToString(std::vector<Property> properties) {
    std::string propertyString = "";
    for (auto &property : properties) {
        if (property.variable != "FileId")
            propertyString += propertyToString(property) + "\n";
    }
    return propertyString;
}

// converts a Property struct to a "cypher string" 
// e.g. {property.variable:property.value}
std::string getPropertyStr(const std::vector<Property> &properties);

void sortProperties(std::vector<Property> &properties);

enum class NodeType { COMPLEX, SET, LIST, INSTANCE };

inline std::string nodeTypeToStr(NodeType type) {
    if (type == NodeType::SET) return TYPE_SET;
    if (type == NodeType::LIST) return TYPE_LIST;
    if (type == NodeType::COMPLEX) return TYPE_COMPLEX;

    return "";
}

class Node {
   public:
    Node();
    Node(std::string id);
    ~Node();

   protected:
    friend bool operator==(const Node &, const Node &);

   public:

    void addProperty(Property property) { m_properties.push_back(property); }

    void removeProperty(Property property);
    void deleteProperties() { m_properties.clear(); }

    // Warning: This overwrites the property vector
    void setProperties(std::vector<Property> properties) {
        m_properties = properties;
    }

    Property getProperty(std::string variable);

    void modifyProperty(std::string variable, std::string newValue);

    std::vector<Property> getProperties() { return m_properties; }

    std::string getVariable() { return m_variable; }
    void setVariable(std::string variable) { m_variable = variable; }

    // converts property values to neo4j strings
    void makeStringProperties();

    std::string getLabel() { return m_label; }

    inline void setLabel(std::string label) {
        m_label = label;
        setNodeType(m_label);
    }

    inline void setNodeType(NodeType type) { this->type = type; }

    inline void setNodeType(std::string type) {
        if (type == TYPE_COMPLEX)
            this->type = NodeType::COMPLEX;
        else if (type == TYPE_SET)
            this->type = NodeType::SET;
        else if (type == TYPE_LIST)
            this->type = NodeType::LIST;
        else
            this->type = NodeType::INSTANCE;
    }

    NodeType getNodeType() { return this->type; }

    // Returns the contents of the node (line by line)
    std::string toString();

    // Returns the cypher string
    std::string toCypher();

    bool isEmpty() {
        return (getLabel().empty() && getId().empty() && m_properties.empty() &&
                m_variable.empty());
    }

    // Clears member variables
    void clear();

    bool compare(Node toCompare);

    bool compareIds(std::string id_2);

    // Compares the properties with another node
    bool compareProperties(Node node_2);

    void setId(std::string id) { m_Id = id; }
    std::string getId() const { return m_Id; }

    std::string createId();

   private:
    std::string m_variable;  // optional
    std::string m_label;     // more lables possible in order to group nodes
    std::vector<Property> m_properties;  // list of all properties of one node
    std::string m_Id;

    NodeType type;
};

// Produces a new unique id from the ids of all other nodes
inline std::string createUnique(std::vector<Node> &nodes) {
    int maxNumber = 0;
    Node lastNode;

    for (auto node : nodes) {
        std::string id = node.getId();

        std::string numStr = id.substr(id.find("_idnumber_"), id.length());

        if (isInteger(numStr)) {
            int currentNumber = std::stoi(numStr);
            if (currentNumber > maxNumber) {
                maxNumber = currentNumber;
                lastNode = node;
            }
        }
    }

    std::string maxNumberStr = std::to_string(maxNumber);
    ++maxNumber;
    std::string latestId = lastNode.getId();
    replace(latestId, maxNumberStr, std::to_string(maxNumber));

    return latestId;
}

inline std::vector<Node> getNodesFromList(std::vector<Node> list,
                                          std::string label) {
    std::vector<Node> nodes;
    std::copy_if(list.begin(), list.end(), std::back_inserter(nodes),
                 [label](Node &obj) { return obj.getLabel() == label; });

    return nodes;
}

inline std::vector<Node> getNodesFromList(
    std::vector<std::pair<Node, std::string>> list, std::string label) {
    std::vector<Node> nodes;
    for (auto &entries : list) {
        if (entries.first.getLabel() == label) nodes.push_back(entries.first);
    }

    return nodes;
}

inline std::vector<Node> getNodesFromList(
    std::vector<std::pair<Node, std::string>> list) {
    std::vector<Node> nodes;
    for (auto &entries : list) nodes.push_back(entries.first);

    return nodes;
}

inline Node getNodeFromList(std::vector<std::pair<Node, std::string>> list,
                            std::string label) {
    auto nodes = getNodesFromList(list, label);
    return nodes[0];
}

inline Node getNodeFromList(std::vector<Node> list, std::string label) {
    auto nodes = getNodesFromList(list, label);
    return nodes[0];
}

// Delete multiple occurrences of a node in a NodeList
inline void makeNodeListUnique(std::vector<Node> &nodes) {
    for (auto nodeIterator = nodes.begin(); nodeIterator < nodes.end();
         nodeIterator++) {
        Node currentNode = *nodeIterator;
        for (auto compareIterator = nodeIterator + 1;
             compareIterator < nodes.end(); compareIterator++) {
            if (currentNode.compare(*compareIterator)) {
                nodes.erase(compareIterator);
            }
        }
    }
}

// ---------------------------------------------------- //
// --------------- Derived node classes --------------- //

class NextAssemblyUsageOccurrence : public Node {
   public:
    NextAssemblyUsageOccurrence();
    NextAssemblyUsageOccurrence(std::string part, int times_occured);
    ~NextAssemblyUsageOccurrence();

   private:
    void initNode();
    std::string m_label;
    std::string m_part;
    int m_timesOccured;
};

class ContextDependentShapeRepresentation : public Node {
   public:
    ContextDependentShapeRepresentation();
    ~ContextDependentShapeRepresentation();

   private:
    void initNode();
    std::string m_label;
};

class ShapeRepresentationship : public Node {
   public:
    ShapeRepresentationship();
    ~ShapeRepresentationship();

   private:
    void initNode();
    std::string m_label;
};

class RepresentationRelationshipWithTransformation : public Node {
   public:
    RepresentationRelationshipWithTransformation();
    ~RepresentationRelationshipWithTransformation();

   private:
    void initNode();
    std::string m_label;
};

class RepresentationRelationship : public Node {
   public:
    RepresentationRelationship();
    RepresentationRelationship(std::string name, std::string description);
    ~RepresentationRelationship();

   private:
    void initNode();
    std::string m_label;
    std::string m_name;
    std::string m_description;
};

class Direction : public Node {
   public:
    Direction();
    Direction(Position position);
    Direction(std::string name, Position position);
    ~Direction();

   private:
    void initNode();
    std::string m_label;
    std::string m_name;
    Position m_position;
};

class CartesianPoint : public Node {
   public:
    CartesianPoint();
    CartesianPoint(Position position);
    CartesianPoint(std::string name, Position position);
    ~CartesianPoint();

   private:
    void initNode();
    std::string m_label;
    std::string m_name;
    Position m_position;
};

class Axis2Placement3d : public Node {
   public:
    Axis2Placement3d();
    Axis2Placement3d(std::string name, std::string description);
    ~Axis2Placement3d();

   private:
    void initNode();
    std::string m_label;
    std::string m_name;
    std::string m_description;
};

class ItemDefinedTransformation : public Node {
   public:
    ItemDefinedTransformation();
    ItemDefinedTransformation(std::string name, std::string description);
    ~ItemDefinedTransformation();

   private:
    void initNode();
    std::string m_label;
    std::string m_name;
    std::string m_description;
};

class ProductDefinitionShape : public Node {
   public:
    ProductDefinitionShape();
    ProductDefinitionShape(std::string name, std::string description);
    ~ProductDefinitionShape();

   private:
    void initNode();
    std::string m_label;
    std::string m_name;
    std::string m_description;
};
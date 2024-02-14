#include "TypesNeo4j.h"

std::string getPropertyStr(const std::vector<Property> &properties) {
    if (properties.empty()) {
        return "";
    }

    std::string ret = "";

    std::vector<Property>::const_iterator itProperty = properties.begin();

    ret = "{";
    ret.append(itProperty->variable + ":" + itProperty->value);

    itProperty++;

    for (; itProperty != properties.end(); itProperty++) {
        // multiple properties available
        ret.append(",");
        ret.append(itProperty->variable + ":" + itProperty->value);
    }

    // just one property available
    ret.append("}");

    return ret;
}

void sortProperties(std::vector<Property> &properties) {
    sort(properties.begin(), properties.end(),
         [](const Property &a, const Property &b) {
             return a.variable < b.variable;
         });
}

// Class Node
Node::Node() : m_variable(""), m_label("") {}

Node::Node(std::string id) : m_variable(""), m_label("") {
    setId(id);
}

Node::~Node() {}

bool operator==(const Node &lhs, const Node &rhs) {
    if (lhs.getId() != rhs.getId()) return false;
    return true;
}

void Node::removeProperty(Property property) {
    for (size_t index = 0; index < m_properties.size(); ++index) {
        if (m_properties[index].variable == property.variable) {
            m_properties.erase(m_properties.begin() + index);
            return;
        }
    }
}

Property Node::getProperty(std::string variable) {
    for (size_t index = 0; index < m_properties.size(); ++index) {
        if (m_properties[index].variable == variable)
            return m_properties[index];
    }

    return Property();
}

void Node::modifyProperty(std::string variable, std::string newValue) {
    for (size_t index = 0; index < m_properties.size(); ++index) {
        if (m_properties[index].variable == variable) {
            m_properties[index].value = newValue;
            return;
        }
    }
}

void Node::makeStringProperties() {
    for (auto &property : m_properties)
        property.value = makeString(property.value);
}

std::string Node::toString() {
    return (getLabel() + "\n" + propertiesToString(getProperties()));
}

std::string Node::toCypher() {
    std::string cypher = "";
    std::string label = "";

    if (!m_label.empty()) label = ":" + m_label;

    if (m_Id.empty()){
        cypher += m_variable + label;
        // add additional node properties
        if (!m_properties.empty()) {
            cypher += "{";
            for (auto &property : m_properties) {
                cypher += property.variable + ":" + property.value + ",";
            }
            cypher.pop_back();
            cypher += "}";
        } 
    } else {
        cypher += m_variable + label + "{Id:'" + m_Id + "'";
        // add additional node properties
        if (!m_properties.empty()) {
            cypher += ",";
            for (auto &property : m_properties) {
                cypher += property.variable + ":" + property.value + ",";
            }
            cypher.pop_back();
            cypher += "}";
        } else
            cypher += "}";
    }
    return cypher;
}

void Node::clear() {
    m_Id.clear();
    m_variable.clear();
    m_label.clear();
}

// converts std::vector<Property> to std::vector<std::string>
// sorts the string lists and compare both lists
bool Node::compareProperties(Node node_2) {
    std::vector<std::string> propertyStrings_1;
    std::vector<std::string> propertyStrings_2;
    std::vector<Property> properties_2 = node_2.getProperties();

    for (auto &property_1 : this->m_properties)
        propertyStrings_1.push_back(propertyToString(property_1));

    for (auto &property_2 : properties_2)
        propertyStrings_2.push_back(propertyToString(property_2));

    std::sort(propertyStrings_1.begin(), propertyStrings_1.end());
    std::sort(propertyStrings_2.begin(), propertyStrings_2.end());

    return (propertyStrings_1 == propertyStrings_2);
}

std::string Node::createId() {
    m_Id = uuid::generateUuidV4();
    return m_Id;
}

bool Node::compare(Node toCompare) {
    if (compareIds(toCompare.getId())) return true;

    return false;
}

bool Node::compareIds(std::string toCompare) {
    if (this->getId() == toCompare) return true;

    return false;
}

// Derived node classes

NextAssemblyUsageOccurrence::NextAssemblyUsageOccurrence()
    : m_label("Next_Assembly_Usage_Occurrence"), m_part(""), m_timesOccured(1) {
    initNode();
    // CreateNode();
}

NextAssemblyUsageOccurrence::NextAssemblyUsageOccurrence(std::string part,
                                                         int timesOccured)
    : m_label("Next_Assembly_Usage_Occurrence"),
      m_part(part),
      m_timesOccured(timesOccured) {
    initNode();
    // CreateNode();
}

NextAssemblyUsageOccurrence::~NextAssemblyUsageOccurrence() {}

void NextAssemblyUsageOccurrence::initNode() {
    setLabel(m_label);

    addProperty({.variable = "id",
                 .value = m_part + ":" + std::to_string(m_timesOccured)});
    addProperty({.variable = "name",
                 .value = m_part + ":" + std::to_string(m_timesOccured)});
    addProperty({.variable = "description",
                 .value = m_part + ":" + std::to_string(m_timesOccured)});
    addProperty({.variable = "reference_designator",
                 .value = m_part + ":" + std::to_string(m_timesOccured)});

    createId();
}

ContextDependentShapeRepresentation::ContextDependentShapeRepresentation()
    : m_label("Context_Dependent_Shape_Representation") {
    initNode();
    // CreateNode();
}

ContextDependentShapeRepresentation::~ContextDependentShapeRepresentation() {}

void ContextDependentShapeRepresentation::initNode() {
    setLabel(m_label);
    createId();
}

ShapeRepresentationship::ShapeRepresentationship()
    : m_label("Shape_Representation_Relationship") {
    initNode();
    // CreateNode();
}

ShapeRepresentationship::~ShapeRepresentationship() {}

void ShapeRepresentationship::initNode() {
    setLabel(m_label);
    createId();
}

RepresentationRelationshipWithTransformation::
    RepresentationRelationshipWithTransformation()
    : m_label("Representation_Relationship_With_Transformation") {
    initNode();
    // CreateNode();
}

RepresentationRelationshipWithTransformation::
    ~RepresentationRelationshipWithTransformation() {}

void RepresentationRelationshipWithTransformation::initNode() {
    setLabel(m_label);
    createId();
}

RepresentationRelationship::RepresentationRelationship()
    : m_label("Representation_Relationship"), m_name(""), m_description("") {
    initNode();
    // CreateNode();
}

RepresentationRelationship::RepresentationRelationship(std::string name,
                                                       std::string description)
    : m_label("Representation_Relationship"),
      m_name(name),
      m_description(description) {
    initNode();
    // CreateNode();
}

RepresentationRelationship::~RepresentationRelationship() {}

void RepresentationRelationship::initNode() {
    setLabel(m_label);
    addProperty({.variable = "name", .value = m_name});
    addProperty({.variable = "description", .value = m_description});
    createId();
}

Direction::Direction()
    : m_label("Direction"), m_name(""), m_position(Position()) {
    initNode();
    // CreateNode();
}

Direction::Direction(Position position)
    : m_label("Direction"), m_name(""), m_position(position) {
    initNode();
    // CreateNode();
}

Direction::Direction(std::string name, Position position)
    : m_label("Direction"), m_name(name), m_position(position) {
    initNode();
    // CreateNode();
}

Direction::~Direction() {}

void Direction::initNode() {
    setLabel(m_label);
    addProperty({.variable = "name", .value = m_name});
    addProperty({.variable = "direction_ratios",
                 .value = positionToString(m_position)});

    createId();
}

CartesianPoint::CartesianPoint()
    : m_label("Cartesian_Point"), m_name(""), m_position(Position()) {
    initNode();
    // CreateNode();
}

CartesianPoint::CartesianPoint(Position position)
    : m_label("Cartesian_Point"), m_name(""), m_position(position) {
    initNode();
    // CreateNode();
}

CartesianPoint::CartesianPoint(std::string name, Position position)
    : m_label("Cartesian_Point"), m_name(name), m_position(position) {
    initNode();
    // CreateNode();
}

CartesianPoint::~CartesianPoint() {}

void CartesianPoint::initNode() {
    setLabel(m_label);
    addProperty({.variable = "name", .value = m_name});
    addProperty(
        {.variable = "coordinates", .value = positionToString(m_position)});

    createId();
}

Axis2Placement3d::Axis2Placement3d()
    : m_label("Axis2_Placement_3d"), m_name(""), m_description("") {
    initNode();
    // CreateNode();
}

Axis2Placement3d::Axis2Placement3d(std::string name, std::string description)
    : m_label("Axis2_Placement_3d"), m_name(name), m_description(description) {
    initNode();
    // CreateNode();
}

Axis2Placement3d::~Axis2Placement3d() {}

void Axis2Placement3d::initNode() {
    setLabel(m_label);
    addProperty({.variable = "name", .value = m_name});
    addProperty({.variable = "description", .value = m_description});
    createId();
}

ItemDefinedTransformation::ItemDefinedTransformation()
    : m_label("Item_Defined_Transformation"), m_name(""), m_description("") {
    initNode();
    // CreateNode();
}

ItemDefinedTransformation::ItemDefinedTransformation(std::string name,
                                                     std::string description)
    : m_label("Product_Definition_Shape"),
      m_name(name),
      m_description(description) {
    initNode();
    // CreateNode();
}

ItemDefinedTransformation::~ItemDefinedTransformation() {}

void ItemDefinedTransformation::initNode() {
    setLabel(m_label);
    addProperty({.variable = "name", .value = m_name});
    addProperty({.variable = "description", .value = m_description});
    createId();
}

ProductDefinitionShape::ProductDefinitionShape()
    : m_label("Product_Definition_Shape"), m_name(""), m_description("") {
    initNode();
    // CreateNode();
}

ProductDefinitionShape::ProductDefinitionShape(std::string name,
                                               std::string description)
    : m_label("Product_Definition_Shape"),
      m_name(name),
      m_description(description) {
    initNode();
    // CreateNode();
}

ProductDefinitionShape::~ProductDefinitionShape() {}

void ProductDefinitionShape::initNode() {
    setLabel(m_label);
    addProperty({.variable = "name", .value = m_name});
    addProperty({.variable = "description", .value = m_description});
    createId();
}
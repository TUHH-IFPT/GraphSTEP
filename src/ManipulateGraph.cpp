#include "ManipulateGraph.h"

Eigen::MatrixXd getXRotationMatrix(double radiant) {
    Eigen::MatrixXd xRotation(3, 3);
    xRotation << 1, 0, 0, 0, cos(radiant), -sin(radiant), 0, sin(radiant),
        cos(radiant);

    return xRotation;
}

Eigen::MatrixXd getYRotationMatrix(double radiant) {
    Eigen::MatrixXd yRotation(3, 3);
    yRotation << cos(radiant), 0, sin(radiant), 0, 1, 0, -sin(radiant), 0,
        cos(radiant);

    return yRotation;
}

Eigen::MatrixXd getZRotationMatrix(double radiant) {
    Eigen::MatrixXd zRotation(3, 3);
    zRotation << cos(radiant), -sin(radiant), 0, sin(radiant), cos(radiant), 0,
        0, 0, 1;

    return zRotation;
}

STEP_COS ManipulateGraph::rotateCoordinateSystem(STEP_COS coordinateSystem,
                                                    AXIS axis, double degrees) {
    Eigen::MatrixXd rotationMatrix;

    double x = degToRad(degrees);

    switch (axis) {
        case AXIS::X:
            rotationMatrix = getXRotationMatrix(x);
            break;
        case AXIS::Y:
            rotationMatrix = getYRotationMatrix(x);
            break;
        case AXIS::Z:
            rotationMatrix = getZRotationMatrix(x);
            break;
        deault:
            break;
    }

    STEP_COS coordinateSystemTransformed;
    coordinateSystemTransformed.xVector =
        rotationMatrix * coordinateSystem.xVector;
    coordinateSystemTransformed.zVector =
        rotationMatrix * coordinateSystem.zVector;


    return coordinateSystemTransformed;
}

STEP_COS ManipulateGraph::rotateCoordinateSystem(STEP_COS coordinateSystem,
                                                    Quaternion quaternion) {
    Eigen::MatrixXd rotationMatrix = getTransformationMatrix(quaternion);

    STEP_COS coordinateSystemTransformed;
    coordinateSystemTransformed.xVector =
        rotationMatrix * coordinateSystem.xVector;
    coordinateSystemTransformed.zVector =
        rotationMatrix * coordinateSystem.zVector;

    return coordinateSystemTransformed;
}

ManipulateGraph::ManipulateGraph() {}

ManipulateGraph::ManipulateGraph(DatabaseInfo databaseInfo)
    : Graph(databaseInfo) {}

ManipulateGraph::~ManipulateGraph() {}

void ManipulateGraph::commitChanges(std::string message) {
    m_trackChanges.setMessage(message);
    VersionControl control(this->m_databaseInfo);
    control.commitBlob(m_trackChanges);
}

void ManipulateGraph::movePart(std::string part, Position position) {
    Modified modifiedNode;
    Node itemDefinedTransformation = collectTransformation(part);

    Node axis2Placement =
        getNextNode(itemDefinedTransformation, "transform_item_2");
    Node location = getNextNode(axis2Placement, "location");
    Node locationModified = location;
    locationModified.modifyProperty("coordinates", positionToString(position));
    modifyNode(location, locationModified);

    modifiedNode.nodeId = location.getId();
    modifiedNode.propertyOld = location.getProperty("coordinates");
    modifiedNode.propertyNew = locationModified.getProperty("coordinates");

    Blob movedPart;
    movedPart.addModified(modifiedNode);
}

// calculate transformation matrix from quaternion
Eigen::MatrixXd ManipulateGraph::getTransformationMatrix(
    Quaternion quaternion) {
    Eigen::MatrixXd transformationMatrix(3, 3);

    double w = quaternion.w;
    double x = quaternion.x;
    double y = quaternion.y;
    double z = quaternion.z;

    transformationMatrix << 1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w,
        2 * x * z + 2 * y * w, 2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z,
        2 * y * z - 2 * x * w, 2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w,
        1 - 2 * x * x - 2 * y * y;

    return transformationMatrix;
}

void ManipulateGraph::rotatePart(std::string part, Quaternion quaternion) {
    Node itemDefinedTransformation = collectTransformation(part);

    Node axis2Placement =
        getNextNode(itemDefinedTransformation, "transform_item_2");

    Node axis =
        getNextNode(axis2Placement, "axis");  // contains direction of z axis
    Node refDirection = getNextNode(
        axis2Placement, "refDirection");  // contains direction of x axis

    auto directionPropertyZ = axis.getProperty("direction_ratios");
    auto directionPropertyX = refDirection.getProperty("direction_ratios");

    std::vector<double> directionZ = strToVector(directionPropertyZ.value);
    std::vector<double> directionX = strToVector(directionPropertyX.value);

    // modify vectors
    Eigen::Vector3d zAxis(directionZ[0], directionZ[1], directionZ[2]);
    Eigen::Vector3d xAxis(directionX[0], directionX[1], directionX[2]);

    STEP_COS coordinateSystem;
    coordinateSystem.xVector = xAxis;
    coordinateSystem.zVector = zAxis;

    coordinateSystem = rotateCoordinateSystem(coordinateSystem, quaternion);

    double x = coordinateSystem.zVector.coeff(0);
    double y = coordinateSystem.zVector.coeff(1);
    double z = coordinateSystem.zVector.coeff(2);

    // modify x and z axis
    Node axisModified = axis;
    axisModified.modifyProperty("direction_ratios",
                                positionToString({.x = x, .y = y, .z = z}));
    modifyNode(axis, axisModified);

    x = coordinateSystem.xVector.coeff(0);
    y = coordinateSystem.xVector.coeff(1);
    z = coordinateSystem.xVector.coeff(2);

    Node refDirectionModified = refDirection;
    refDirectionModified.modifyProperty(
        "direction_ratios", positionToString({.x = x, .y = y, .z = z}));
    modifyNode(refDirection, refDirectionModified);
}

void ManipulateGraph::rotatePart(
    std::string part, std::vector<std::pair<AXIS, double>> rotations) {
    Node itemDefinedTransformation = collectTransformation(part);

    Node axis2Placement =
        getNextNode(itemDefinedTransformation, "transform_item_2");

    Node axis =
        getNextNode(axis2Placement, "axis");  // contains direction of z axis
    Node refDirection = getNextNode(
        axis2Placement, "refDirection");  // contains direction of x axis

    auto directionPropertyZ = axis.getProperty("direction_ratios");
    auto directionPropertyX = refDirection.getProperty("direction_ratios");

    std::vector<double> directionZ = strToVector(directionPropertyZ.value);
    std::vector<double> directionX = strToVector(directionPropertyX.value);

    // modify vectors
    Eigen::Vector3d zAxis(directionZ[0], directionZ[1], directionZ[2]);
    Eigen::Vector3d xAxis(directionX[0], directionX[1], directionX[2]);

    STEP_COS coordinateSystem;
    coordinateSystem.xVector = xAxis;
    coordinateSystem.zVector = zAxis;

    for (auto &rotation : rotations)
        coordinateSystem = rotateCoordinateSystem(
            coordinateSystem, rotation.first, rotation.second);

    double x = coordinateSystem.zVector.coeff(0);
    double y = coordinateSystem.zVector.coeff(1);
    double z = coordinateSystem.zVector.coeff(2);

    // modify x and z axis
    Node axisModified = axis;
    axisModified.modifyProperty("direction_ratios",
                                positionToString({.x = x, .y = y, .z = z}));
    modifyNode(axis, axisModified);

    x = coordinateSystem.xVector.coeff(0);
    y = coordinateSystem.xVector.coeff(1);
    z = coordinateSystem.xVector.coeff(2);

    Node refDirectionModified = refDirection;
    refDirectionModified.modifyProperty(
        "direction_ratios", positionToString({.x = x, .y = y, .z = z}));
    modifyNode(refDirection, refDirectionModified);
}

Node ManipulateGraph::getProductDefinition(std::string partName) {
    Node product_definition;
    product_definition.setLabel("Product_Definition");
    product_definition.addProperty({.variable = "id", .value = partName});
    return getNode(product_definition);
}

Node ManipulateGraph::collectShapeRepresentation(std::string part) {
    auto productNodes = collectProductNodes(part);

    auto shapeDefRep =
        getNodesFromList(productNodes, "Shape_Definition_Representation");
    if (!shapeDefRep.empty())
        return getNodeFromList(getAllChildren(shapeDefRep[0]),
                               "Shape_Representation");

    return Node();
}

Node ManipulateGraph::collectTransformation(std::string part) {
    auto shapeRep = collectShapeRepresentation(part);
    if (shapeRep.isEmpty()) return Node();

    auto shapeRepParents = getAllParents(shapeRep);

    auto complexTypeList = getNodesFromList(shapeRepParents, TYPE_COMPLEX);

    if (complexTypeList.empty()) {
        Logger::error("Failed to load transformation from graph");
        throw std::runtime_error("No complex type found");
    }

    auto complexChildren = getChildNodes(complexTypeList[0]);

    Node repRelTransform = getNodeFromList(
        complexChildren, "Representation_Relationship_With_Transformation");

    auto itemDefinedTransformation = getChildNodes(repRelTransform);
    return itemDefinedTransformation[0].first;
}

Node ManipulateGraph::createComplex(std::vector<Node> nodes) {
    Node complex_type;
    complex_type.setLabel("COMPLEX_TYPE");
    complex_type.createId();
    createNode(complex_type);

    for (auto &node : nodes) createRelation(complex_type, node, "entry");

    return complex_type;
}

void ManipulateGraph::addNewProductOccurrence(std::string part,
                                                 std::string assembly) {
    Node productDefAssembly = getProductDefinition(assembly);
    Node productDefPart = getProductDefinition(part);

    NextAssemblyUsageOccurrence nextAssemblyUsageOccurance(part, 2);

    // link nextAssemblyUsageOccurance to assembly and part product_definition
    createNode(nextAssemblyUsageOccurance);

    createRelation(nextAssemblyUsageOccurance, productDefAssembly,
                   "relating_product_definition");
    createRelation(nextAssemblyUsageOccurance, productDefPart,
                   "related_product_definition");

    ProductDefinitionShape productDefinitionShape;
    createNode(productDefinitionShape);
    // Node product_def_shape = CreateProductDefinitionShape();
    createRelation(productDefinitionShape, nextAssemblyUsageOccurance,
                   "definition");

    //------------------------------------------------------------------------------------------------------------//
    // CreateItemDefinedTransformationTree()
    //------------------------------------------------------------------------------------------------------------//

    ItemDefinedTransformation itemDefinedTransformation;
    createNode(*dynamic_cast<Node *>(&itemDefinedTransformation));

    Node shape_representation = collectShapeRepresentation(
        part);  // this should be the shape_representation of the assembly?
    Node placement = getNextNode(shape_representation, "items_list_type_0");

    createRelation(*dynamic_cast<Node *>(&itemDefinedTransformation),
                   *dynamic_cast<Node *>(&placement), "transform_item_1");

    Axis2Placement3d axis2Placement3d;
    createNode(*dynamic_cast<Node *>(&axis2Placement3d));

    CartesianPoint cartesianPoint({.x = 0, .y = 0, .z = 0});
    createNode(*dynamic_cast<Node *>(&cartesianPoint));

    Direction directionAxis({.x = 0, .y = 0, .z = 1.});
    createNode(*dynamic_cast<Node *>(&directionAxis));

    Direction directionRef({.x = 1, .y = 0, .z = 0});
    createNode(*dynamic_cast<Node *>(&directionRef));

    createRelation(*dynamic_cast<Node *>(&axis2Placement3d), cartesianPoint,
                   "location");
    createRelation(*dynamic_cast<Node *>(&axis2Placement3d), directionAxis,
                   "axis");
    createRelation(*dynamic_cast<Node *>(&axis2Placement3d), directionRef,
                   "refDirection");
    createRelation(*dynamic_cast<Node *>(&itemDefinedTransformation),
                   *dynamic_cast<Node *>(&axis2Placement3d),
                   "transform_item_2");

    Node shapeRepresentationAssembly = collectShapeRepresentation(assembly);
    auto children = getChildNodes(shapeRepresentationAssembly);

    int maxNumber = 0;
    for (auto &child : children) {
        if (child.second.find("items_list_type") != std::string::npos) {
            int currentNumber = getNumberFromString(child.second);
            if (currentNumber > maxNumber) maxNumber = currentNumber;
        }
    }

    createRelation(shapeRepresentationAssembly, axis2Placement3d,
                   "items_list_type_" + std::to_string(++maxNumber));
    //------------------------------------------------------------------------------------------------------------//

    RepresentationRelationship representationRelationship;
    createNode(*dynamic_cast<Node *>(&representationRelationship));

    Node rep1 = collectShapeRepresentation(part);
    Node rep2 = collectShapeRepresentation(assembly);

    createRelation(*dynamic_cast<Node *>(&representationRelationship), rep1,
                   "rep_1");
    createRelation(*dynamic_cast<Node *>(&representationRelationship), rep2,
                   "rep_2");

    RepresentationRelationshipWithTransformation
        representationRelationshipWithTransformation;
    createNode(representationRelationshipWithTransformation);

    ShapeRepresentationship shapeRepresentationRelationship;
    createNode(*dynamic_cast<Node *>(&shapeRepresentationRelationship));

    std::vector<Node> complexNodes;
    complexNodes.push_back(*dynamic_cast<Node *>(&representationRelationship));
    complexNodes.push_back(
        *dynamic_cast<Node *>(&representationRelationshipWithTransformation));
    complexNodes.push_back(
        *dynamic_cast<Node *>(&shapeRepresentationRelationship));

    createComplex(complexNodes);

    createRelation(representationRelationshipWithTransformation,
                   itemDefinedTransformation, "transformation_operator");

    ContextDependentShapeRepresentation contextDependentShapeRepresentation;
    createNode(contextDependentShapeRepresentation);
    createRelation(contextDependentShapeRepresentation,
                   representationRelationship, "representation_relation");

    createRelation(contextDependentShapeRepresentation,
                   *dynamic_cast<Node *>(&productDefinitionShape),
                   "represented_product_relation");
}

void ManipulateGraph::createNode(Node node) {
    std::string query = m_cypher.createNodeQuery(node);
    m_trackChanges.addNewNode(node);
    sendQuery(query);
}

void ManipulateGraph::createRelation(Node from, Node to,
                                        std::string relation) {
    m_trackChanges.addNewRelation(from, to, relation);
    sendQuery(m_cypher.createRelation(from, to, relation));
}

void ManipulateGraph::modifyNode(Node node, Node modified) {
    std::string query = m_cypher.modifyNodeQuery(node, modified);

    Modified mod;
    mod.nodeId = node.getId();
    mod.propertyOld = node.getProperties()[0];
    mod.propertyNew = modified.getProperties()[0];

    m_trackChanges.addModified(mod);
    sendQuery(query);
}

void ManipulateGraph::deletePart(std::string part) {
    Property property = {.variable = "id", .value = makeString(part)};

    Node product;
    product.setLabel("Product");
    product.addProperty(property);
    product = getNode(product);

    if (product.isEmpty()) return;

    auto productParents = getAllParents(product);
    std::vector<Node> list;
    list.insert(list.end(), productParents.begin(), productParents.end());

    auto appliedDataTimeAssignment =
        getNodesFromList(productParents, "Applied_Date_And_Time_Assignment");

    auto appliedDataTimeAssignmentParents =
        getAllChildren(appliedDataTimeAssignment[0]);
    list.insert(list.end(), appliedDataTimeAssignmentParents.begin(),
                appliedDataTimeAssignmentParents.end());

    auto shapeDefRep =
        getNodesFromList(productParents, "Shape_Definition_Representation");
    if (!shapeDefRep.empty()) {
        auto shapeRep = getNodesFromList(getAllChildren(shapeDefRep[0]),
                                         "Shape_Representation");

        auto shapeRepParents = getAllParents(shapeRep[0]);
        list.insert(list.end(), shapeRepParents.begin(), shapeRepParents.end());

        auto shapeRepRelList = getNodesFromList(
            shapeRepParents, "Shape_Representation_Relationship");
        auto complexTypeList = getNodesFromList(shapeRepParents, TYPE_COMPLEX);
        auto complexChildren = getChildNodes(complexTypeList[0]);

        Node repRelTransform = getNodeFromList(
            complexChildren, "Representation_Relationship_With_Transformation");

        auto transformation = getAllChildren(repRelTransform);

        for (auto &shapeRepRel : shapeRepRelList) {
            auto nodes = getAllChildren(shapeRepRel);
            list.insert(list.end(), nodes.begin(), nodes.end());
            list.push_back(shapeRepRel);
        }
    }

    for (auto &delNode : list) deleteNode(delNode);
}

void ManipulateGraph::addNewPart(std::string filePath, std::string part,
                                    std::string assembly) {
    PushSTEP upload(filePath, this->m_databaseInfo);
    upload.build();
    m_trackChanges = upload.getTrackedChanges();
    addNewProductOccurrence(part, assembly);
}

std::vector<Node> ManipulateGraph::collectProductNodes(std::string product) {
    Property property = {.variable = "id", .value = makeString(product)};

    Node productNode;
    productNode.setLabel("Product");
    productNode.addProperty(property);
    productNode = getNode(productNode);

    if (productNode.isEmpty()) return std::vector<Node>();

    return getAllParents(productNode);
}
Node ManipulateGraph::collectManifoldSolidBrep(std::string part) {
    std::vector<Node> list;
    std::vector<Node> shape_nodes;
    auto products = collectProductNodes(part);

    auto shapeDefRep =
        getNodesFromList(products, "Shape_Definition_Representation");

    if (!shapeDefRep.empty()) {
        auto shapeRep = getNodesFromList(getAllChildren(shapeDefRep[0]),
                                         "Shape_Representation");
                auto shapeRepParents = getAllParents(shapeRep[0]);
        list.insert(list.end(), shapeRepParents.begin(), shapeRepParents.end());

        auto complexNodes = getNodesFromList(
            shapeRepParents, "COMPLEX_TYPE");

        // a:complexNodes[0]-[*]->(b:manifoldSolidBrep) return b
        std::string advancedBrep = "Manifold_Solid_Brep";
        Node manifoldSolidBrep;
        manifoldSolidBrep.setVariable("b");
        manifoldSolidBrep.setLabel(advancedBrep);

        std::string query = m_cypher.matchQuery(complexNodes[0], "*", manifoldSolidBrep, manifoldSolidBrep.getVariable());
        std::string jsonString = sendQuery(query);

        auto manifoldSolidBrepList = jsonToNodeList(jsonString);
        manifoldSolidBrep = manifoldSolidBrepList[0];
        manifoldSolidBrep.setLabel(advancedBrep);
        
        return manifoldSolidBrep;
    }
    
    Logger::error("Could not find Shape_Definition_Representation node");
    return Node();
}

AdjacencyMatrix ManipulateGraph::collectBrep(std::string part) {

    auto manifoldSolidBrep = collectManifoldSolidBrep(part);
    if (manifoldSolidBrep.isEmpty())
        throw std::runtime_error("No manifold solid brep found");

    auto matrix = nodesToAdjacencyMatrix(getAllChildren(manifoldSolidBrep));
    return matrix;
}

void ManipulateGraph::addBrep(AdjacencyMatrix matrix, std::string part) {
    createGraph(matrix);
    auto closedShell = matrix.findNodes("Closed_Shell");
    auto manifoldSolidBrep = collectManifoldSolidBrep(part);
    std::string query = this->m_cypher.createRelation(manifoldSolidBrep,
                                                      closedShell[0], "outer");

    sendQuery(query);
}
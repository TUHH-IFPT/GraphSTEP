#pragma once

#include <Eigen/Dense>
#include <cmath>

#include "Graph.h"
#include "PushStep.h"
#include "VersionControl.h"

/**
 * @brief ManipulateGraph
 * manipualtes the graph
 * e.g. rotate or move parts ...
**/

struct STEP_COS {
    Eigen::Vector3d xVector;
    Eigen::Vector3d zVector;
};

enum class AXIS { X = 0, Y = 1, Z = 2 };

class ManipulateGraph : public Graph {
   public:
    ManipulateGraph();
    ManipulateGraph(DatabaseInfo databaseInfo);
    ~ManipulateGraph();

    void commitChanges(std::string message);

    void createNode(Node node) override;
    void createRelation(Node from, Node to, std::string relation) override;
    void modifyNode(Node node, Node modified) override;

    void deletePart(std::string part);

    // Add new product_definition and link it with the assembly
    void addNewPart(std::string filePath, std::string part,
                    std::string assembly);

    // moves a part in an assembly
    // The name of the part is not neccecarily the same as the name of the file
    void movePart(std::string part, Position position);

    void addNewProductOccurrence(std::string part, std::string newName);

    // rotates a part in an assembly
    void rotatePart(std::string part,
                    std::vector<std::pair<AXIS, double>> rotations);
    void rotatePart(std::string part, Quaternion quaternion);

    // Get the entity ManifoldSolidBrep of a specific part
    Node collectManifoldSolidBrep(std::string part);

    // collect the adjacency matrix of a part
    AdjacencyMatrix collectBrep(std::string part);

    STEP_COS rotateCoordinateSystem(
        STEP_COS coordinateSystem, AXIS axis,
        double degrees);  // rotate with euler angles
    STEP_COS rotateCoordinateSystem(
        STEP_COS coordinateSystem,
        Quaternion quaternion);  // rotate with quaternion

    // create and link boundary representation to a part
    void addBrep(AdjacencyMatrix matrix, std::string part);

   private:
    std::vector<Node> collectProductNodes(std::string product);
    

    Node getProductDefinition(std::string partName);
    Node collectShapeRepresentation(std::string part);
    Node collectTransformation(std::string part);

    Node createComplex(std::vector<Node> nodes);

    Eigen::MatrixXd getTransformationMatrix(Quaternion quaternion);

    Blob m_trackChanges;
};
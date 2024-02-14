#include "Graph.h"
#include <filesystem>
#include <iostream>

const std::string logFile = "graphstep.log";

Graph::Graph() : m_path("") {
    Logger::initializeLogger(logFile);
}

Graph::Graph(std::string path, DatabaseInfo databaseInfo)
    : m_path(path){
    
    Logger::initializeLogger(logFile);
    initRestInterface(databaseInfo);
}

Graph::Graph(DatabaseInfo databaseInfo): m_path(""){
    Logger::initializeLogger(logFile);
    initRestInterface(databaseInfo);
}

Graph::~Graph() {}

void Graph::initRestInterface(DatabaseInfo databaseInfo) {
    m_databaseInfo = databaseInfo;
    m_pRest = std::make_unique<RestInterface>();

    m_pRest->setHost(databaseInfo.hostName);
    m_pRest->setCredentials(databaseInfo.credentials.name,
                            databaseInfo.credentials.password);
    m_pRest->setPath("db/" + databaseInfo.databaseName + "/tx/commit/");
}

void Graph::deleteDatabase() {
    std::string response = "";
    m_pRest->postRequest(getJsonFromCypher("MATCH (n) DETACH DELETE n"),
                         response);
}

void Graph::createGraph(AdjacencyMatrix matrix) {
    auto adjacencyMatrixNodes = matrix.getNodes();
    auto adjacencyMatrixRelations = matrix.getRelationMatrix();

    // Create nodes
    for (auto &node : adjacencyMatrixNodes)
        sendQuery(m_cypher.createNodeQuery(node));

    // Create edges
    size_t currentNode = 0;
    for (auto &node : adjacencyMatrixNodes) {
        size_t currentRelation = 0;
        for (auto &relation : adjacencyMatrixRelations[currentNode]) {
            if (!relation.empty()) {
                // Self loops are not allowed
                if (!node.compare(adjacencyMatrixNodes[currentRelation])) {
                    node.makeStringProperties();
                    adjacencyMatrixNodes[currentRelation]
                        .makeStringProperties();

                    if (relation.find(";") != std::string::npos) {
                        // Multiple relations found!
                        std::vector<std::string> relations =
                            getListFromStrings(relation);
                        for (auto &entry : relations)
                            sendQuery(m_cypher.createRelation(
                                node, adjacencyMatrixNodes[currentRelation],
                                entry));
                    } else
                        sendQuery(m_cypher.createRelation(
                            node, adjacencyMatrixNodes[currentRelation],
                            relation));
                }
            }
            ++currentRelation;
        }
        ++currentNode;
    }
}

void Graph::deleteNode(Node node) {
    node.setVariable("a");
    node.makeStringProperties();
    sendQuery(m_cypher.deleteQuery(node));
}

void Graph::deleteSubgraph(AdjacencyMatrix subgraph) {
    auto nodes = subgraph.getNodes();
    for (auto &node : nodes) deleteNode(node);
}

void Graph::createNode(Node node) {
    sendQuery(m_cypher.createNodeQuery(node));
}

void Graph::createRelation(Node from, Node to, std::string relation) {
    sendQuery(m_cypher.createRelation(from, to, relation));
}

void Graph::modifyNode(Node node, Node modified) {
    std::string query = m_cypher.modifyNodeQuery(node, modified);
    sendQuery(query);
}

void Graph::modifyNode(Node node, Property newProperty) {
    std::string query = m_cypher.modifyNodeQuery(node, newProperty);
    sendQuery(query);
}

std::string Graph::getJsonFromCypher(const std::string &cypher) {
    json jsonStatement;
    json jsonStatements;

    jsonStatement["statement"] = cypher;
    jsonStatements["statements"].push_back(jsonStatement);

    Logger::log("Cypher query: " + cypher);

    return jsonStatements.dump();
}

std::string Graph::cypherListToJson() {
    json queries;
    json statement;
    for (auto &cypher : m_cypherQueries) {
        statement["statement"] = cypher;
        queries["statements"].push_back(statement);
    }

    m_cypherQueries.clear();
    return queries.dump();
}

std::string Graph::cypherToJson(const std::string &cypher) {
    json queries;
    json statement;

    statement["statement"] = cypher;
    queries["statements"].push_back(statement);

    return queries.dump();
}

void Graph::pushQueryToJson(const std::string &cypher) {
    json statement;

    Logger::log("Cypher query: " + cypher);
    m_cypherQueries.push_back(cypher);
}

std::string Graph::sendQueries() {
    std::string response = "";
    std::string jsonData = cypherListToJson();

    HttpState state = m_pRest->postRequest(jsonData, response);

    checkResponseForErrors(response);

    // Logger::log("Neo4j response: " + response);

    switch (state) {
        case HttpState::HTTP_OK:
        case HttpState::HTTP_NO_CONTENT:
            break;
        default:
            Logger::error("Something went wrong... \n post request returned: ");
            Logger::error(httpStateToStr(state));
            break;
    }

    m_queries.clear();
    return response;
}

std::string Graph::sendQuery(const std::string &cypher) {
    std::string response = "";
    Logger::log("Query: " + cypher);

        HttpState state = m_pRest->postRequest(cypherToJson(cypher), response);

    checkResponseForErrors(response);


    switch (state) {
        case HttpState::HTTP_OK:
        case HttpState::HTTP_NO_CONTENT:
            break;
        default:
            Logger::error("Something went wrong... \n post request returned: ");
            Logger::error(httpStateToStr(state));
            break;
    }

    return response;
}

// Split the std::string of a aggregate attribute to determine its content and
// size
vector<std::string> Graph::getEntriesAggregate(std::string str) {
    // Input example: (#14,#30)
    vector<std::string> ret;
    std::string temp = str;

    // Remove parentheses
    str = removeParentheses(str);
    // str = str.substr(1, str.size() - 2);

    ret = getListFromStrings(str, ',');

    return ret;
}

std::vector<std::string> Graph::getAllLabels() {
    // MATCH (n) RETURN distinct labels(n)
    std::vector<std::string> labels;
    Node node;
    node.setVariable("n");

    std::string jsonString =
        sendQuery(m_cypher.matchQuery(node, "distinct labels(n)"));

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];
                for (auto &row : rows) {
                    if (!row[0].empty()) labels.push_back(row[0]);
                }
            }
        }
    }

    std::sort(labels.begin(), labels.end());
    return labels;
}

std::vector<std::string> Graph::getNodeIds(std::string label) {
    // MATCH( a:Axis2_Placement_3d) RETURN a

    Node node;
    node.setLabel(label);
    node.setVariable("a");

    std::string jsonString =
        sendQuery(m_cypher.matchQuery(node, node.getVariable()));
    std::vector<std::string> entities;

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];
                for (auto &row : rows) {
                    std::string fileId = row["Id"];
                    entities.push_back(fileId);
                }
            }
        }
    }
    return entities;
}

std::vector<Node> Graph::getNodes(std::string label) {
    std::vector<Node> nodes;
    auto nodeIds = getNodeIds(label);

    for (auto &nodeid : nodeIds) {
        Node node;
        node.setLabel(label);
        node.setId(nodeid);
        auto properties = getProperties(node);
        node.setProperties(properties);
        nodes.push_back(node);
    }
    return nodes;
}

std::vector<Node> Graph::getTreeNodes(Node parentNode) {
    // MATCH (a:Circle{Id:'Circle_76'})-[*]->(b) RETURN *, labels(b);

    std::vector<Node> treeNodes;

    Node from(parentNode.getId());
    from.setVariable("a");
    Node to;
    to.setVariable("b");
    std::string query = m_cypher.matchQuery(from, "*", to, "*, labels(b)");

    treeNodes = jsonToNodeList(sendQuery(query));
    makeNodeListUnique(treeNodes);

    return treeNodes;
}

std::vector<std::pair<Node, std::string>> Graph::getChildNodes(
    Node parentNode) {
    std::vector<std::pair<Node, std::string>> children;

    Node from(parentNode.getId());
    from.setVariable("a");

    Node to;
    to.setVariable("b");

    std::string query =
        m_cypher.matchQuery(from, "r", to, "b, labels(b), TYPE(r)");
    std::string jsonString = sendQuery(query);

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];

                //  "row": [
                //         {
                //             "name": "",
                //             "Id": 25
                //         },
                //         [
                //             "Draughting_Model"
                //         ],
                //         "rep_2"
                //     ],

                // row[0] contains json object with properties
                json properties = rows[0];

                Node node;
                for (auto &property : properties.items()) {
                    std::string value =
                        removeQuotation(property.value().dump());

                    if (property.key() == "Id")
                        node.setId(value);
                    else
                        node.addProperty(
                            {.variable = property.key(), .value = value});
                }

                // rows[1] contains the node label
                // should normally contain only one value
                for (auto &label : rows[1])  
                    node.setLabel(label);

                // rows[2] contains the relationship
                std::string relation = rows[2];

                children.push_back(std::make_pair(node, relation));
            }
        }
    }
    return children;
}

std::vector<Node> Graph::getChildNodeList(Node parentNode)
{
    std::vector<Node> children;

    Node from(parentNode.getId());
    from.setVariable("a");

    Node to;
    to.setVariable("b");

    std::string query =
        m_cypher.matchQuery(from, "r", to, "b, labels(b)");
    std::string jsonString = sendQuery(query);

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];

                // row[0] contains json object with properties
                json properties = rows[0];

                Node node;
                for (auto &property : properties.items()) {
                    std::string value =
                        removeQuotation(property.value().dump());

                    if (property.key() == "Id")
                        node.setId(value);
                    else
                        node.addProperty(
                            {.variable = property.key(), .value = value});
                }

                // rows[1] contains the node label
                // should normally contain only one value
                for (auto &label : rows[1])  
                    node.setLabel(label);

                children.push_back(node);
            }
        }
    }
    return children;
}

std::vector<Node> Graph::getAllParents(Node childNode, int depth) {
    // "from" node
    Node node;
    node.setVariable("a");

    // "to" node
    childNode.makeStringProperties();
    childNode.setVariable("b");

    std::string query =
        m_cypher.matchQuery(node, "*", childNode,
                            node.getVariable() + ", labels(a)");
    std::string jsonString = sendQuery(query);
    std::vector<Node> children;

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];

                // row[0] contains json object with properties
                json properties = rows[0];

                Node node;
                for (auto &property : properties.items()) {
                    std::string value =
                        removeQuotation(property.value().dump());

                    if (property.key() == "Id")
                        node.setId(value);
                    else
                        node.addProperty(
                            {.variable = property.key(), .value = value});
                }

                // rows[1] contains the node label
                // rows[1] is an array but should normally contain only one
                // value
                for (auto &label : rows[1]) {
                    node.setLabel(label);
                }

                children.push_back(node);
            }
        }
    }
    return children;
}

std::vector<Node> Graph::getAllChildren(Node childNode, int depth) {
    // MATCH(a:Product_Definition_Shape)
    // -[r:definition]->(b:Product_Definition{id:'assembly_part1'}) RETURN *
    std::vector<Node> children;

    childNode.setVariable("a");
    Node node;
    node.setVariable("b");
    std::string query;
    if(depth == -1){
        query = m_cypher.matchQuery(
            childNode, "*", node, "b, labels(b)");
    } else {
        query = m_cypher.matchQuery(
            childNode, m_cypher.depthString("r", 0), node, "b, labels(b), r");
    }

    std::string jsonString = sendQuery(query);

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];

                // row[0] contains json object with properties
                json properties = rows[0];

                Node node;
                for (auto &property : properties.items()) {
                    std::string value =
                        removeQuotation(property.value().dump());

                    if (property.key() == "Id")
                        node.setId(value);
                    else
                        node.addProperty(
                            {.variable = property.key(), .value = value});
                }

                // rows[1] contains the node label
                // rows[1] is an array but should normally contain only one
                // value
                for (auto &label : rows[1]) {
                    node.setLabel(label);
                }

                children.push_back(node);
            }
        }
    }
    return children;
}

AdjacencyMatrix Graph::getSubgraph(Node node) {
    AdjacencyMatrix subgraph;
    std::vector<std::string> row;

    subgraph.setNodes(getTreeNodes(node));
    subgraph.insertNode(node);

    auto subGraphNodes = subgraph.getNodes();

    for (auto &node : subGraphNodes) node.makeStringProperties();

    for (auto &treeNode : subGraphNodes) {
        row.clear();

        for (auto &temp : subGraphNodes) row.push_back("");

        auto children = getChildNodes(treeNode);

        // Populate matrix
        for (auto &child : children) {
            Node childNode = child.first;
            auto it = std::find_if(
                subGraphNodes.begin(), subGraphNodes.end(),
                [&childNode](Node &obj) { return obj.compare(childNode); });

            if (it != subGraphNodes.end()) {
                auto index = it - subGraphNodes.begin();
                if (row[index].empty())
                    row[index] = child.second;
                else
                    row[index] += ";" + child.second;
            }
        }
        subgraph.addRelationRow(row);
    }
    return subgraph;
}

void Graph::appendGraph(AdjacencyMatrix matrix) {
    createGraph(matrix);

    auto matrixNodes = matrix.getNodes();
    Property property = {.variable = "isMacro", .value = makeString("true")};

    Node modified = matrixNodes[0];
    modified.removeProperty(property);

    modifyNode(matrixNodes[0], modified);
}

std::vector<Node> Graph::jsonToNodeList(std::string jsonString) {
    std::vector<Node> nodes;

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                Node node;
                json rows = data["row"];

                // row[1] contains json object with properties
                json properties = rows[0];

                for (auto &property : properties.items()) {
                    std::string value =
                        removeQuotation(property.value().dump());

                    if (property.key() == "Id")
                        node.setId(value);
                    else
                        node.addProperty(
                            {.variable = property.key(), .value = value});
                }

                // rows[2] contains the node label
                // rows[2] is an array but should normally contain only one
                // value
                for (auto &label : rows[1]) {
                    node.setLabel(label);
                }
                nodes.push_back(node);
            }
        }
    }
    return nodes;
}

Node Graph::getNextNode(Node node, std::string relation) {
    node.setVariable("a");
    node.makeStringProperties();

    Node secondNode;
    secondNode.setVariable("b");

    std::string query =
        m_cypher.matchQuery(node, ":" + relation, secondNode, "b, labels(b)");
    std::string jsonString = sendQuery(query);

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];

                // row[0] contains json object with properties
                json properties = rows[0];

                Node node;
                for (auto &property : properties.items()) {
                    std::string value =
                        removeQuotation(property.value().dump());

                    if (property.key() == "Id")
                        node.setId(value);
                    else
                        node.addProperty(
                            {.variable = property.key(), .value = value});
                }

                // rows[1] contains the node label
                node.setLabel(rows[1][0]);

                return node;
            }
        }
    }
    return Node();
}

std::vector<Property> Graph::getProperties(Node node) {
    // MATCH (p:Vertex_Point) WHERE p.FileId=83 RETURN p;
    std::vector<Property> properties;
    node.setVariable("a");
    std::string query = m_cypher.matchQuery(node, "a");
    std::string jsonString = sendQuery(query);

    if (!jsonString.empty()) {
        // Parse json
        json jsonData = json::parse(jsonString);
        json results = jsonData["results"];

        for (auto &result : results) {
            json dataList = result["data"];
            for (auto &data : dataList) {
                json rows = data["row"];
                for (auto &row : rows) {
                    for (auto it = row.begin(); it != row.end(); ++it) {
                        if (it.key() != "Id") {
                            std::string value =
                                removeQuotation(it.value().dump());

                            filterString(value);

                            properties.push_back(
                                {.variable = it.key(), .value = value});
                        }
                    }
                }
            }
        }
    }

    sortProperties(properties);
    return properties;
}

Node Graph::getNode(Node node) {
    node.makeStringProperties();
    node.setVariable("a");
    std::string jsonString =
        sendQuery(m_cypher.matchQuery(node, node.getVariable()));

    std::vector<Node> nodes = jsonToNodeList(jsonString);

    if (nodes.empty()) {
        Logger::error("Node not found");
        return Node();
    }

    nodes[0].setLabel(node.getLabel());
    return nodes[0];
}

AdjacencyMatrix Graph::nodesToAdjacencyMatrix(std::vector<Node> nodes) {
    AdjacencyMatrix matrix;
    makeNodeListUnique(nodes);
    matrix.setNodes(nodes);
    std::vector<std::string> row;

    for (auto &currentNode : nodes) {
        row.clear();

        std::vector<std::pair<Node, std::string>> children =
            getChildNodes(currentNode);

        // the other ones are the relations
        for (auto &node_to : nodes) row.push_back("");

        // Populate matrix
        for (auto &child : children) {
            Node childNode = child.first;
            auto it = std::find_if(
                nodes.begin(), nodes.end(),
                [&childNode](Node &obj) { return obj.compare(childNode); });

            if (it != nodes.end()) {
                // "it" is an iterator to the first matching element
                auto index = it - nodes.begin();
                if (row[index].empty())
                    row[index] = child.second;
                else
                    row[index] += ";" + child.second;
            }
        }
        matrix.addRelationRow(row);
    }
    return matrix;
}

bool Graph::loadAdjacencyMatrix() {

    std::vector<std::string> labels = getAllLabels();

    if (labels.empty()) {
        Logger::error("Graph is empty");
        return false;
    }

    // Stores all different nodes including their properties
    std::vector<Node> nodes; 
    std::vector<std::string> row;

    for (auto &label : labels) {
        std::vector<std::string> entries = getNodeIds(label);

        for (auto entry : entries) {
            Node node(entry);
            node.setLabel(label);

            std::vector<Property> properties = getProperties(node);

            node.setProperties(properties);
            m_matrix.addNode(node);
        }
    }

    int currentColumn = 0;
    auto adjacencyMatrixNodes = m_matrix.getNodes();
    for (auto &currentNode : adjacencyMatrixNodes) {
        row.clear();

        std::vector<std::pair<Node, std::string>> children =
            getChildNodes(currentNode);

        // Initialize relations with empty string
        for (auto &node_to : adjacencyMatrixNodes) row.push_back("");

        // Populate matrix
        for (auto &child : children) {
            Node childNode = child.first;
            auto it = std::find_if(
                adjacencyMatrixNodes.begin(), adjacencyMatrixNodes.end(),
                [&childNode](Node &obj) { return obj.compare(childNode); });

            if (it != adjacencyMatrixNodes.end()) {
                // "it" is an iterator to the first matching element.
                auto index = it - adjacencyMatrixNodes.begin();
                if (row[index].empty())
                    row[index] = child.second;
                else{
                    // If more than one relation leads
                    // to the same node, separate the
                    // relation strings with semicolons
                    row[index] += ";" + child.second;  
                }
            }
        }
        m_matrix.addRelationRow(row);
    }

    // Marks all nodes that belong to a complex type
    // distinguishes between complex and normal nodes
    m_matrix.markComplexNodes();  

    return true;
}
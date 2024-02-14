#include "FilterGraph.h"
#include "ManipulateGraph.h"

FilterGraph::FilterGraph() {}

FilterGraph::FilterGraph(DatabaseInfo makroGraphInfo)
    : ManipulateGraph(makroGraphInfo), m_filter(DatabaseFilter::Undefined) {
}

FilterGraph::~FilterGraph() {}

void FilterGraph::initDatabase(DatabaseInfo mainGraphInfo) {
    m_main = std::make_unique<Graph>(mainGraphInfo);
}

void FilterGraph::collect(DatabaseFilter filter) {
    std::string labelFilter = databaseFilterToStr(filter);
    std::vector<std::string> nodeIds;
    if (m_main != nullptr)
        nodeIds = m_main->getNodeIds(labelFilter);
    else
        throw_database_error("pointer was null");

    for (auto nodeId : nodeIds) {
        Node node(nodeId);
        node.setLabel(labelFilter);
        node.setProperties(m_main->getProperties(node));
        m_SubGraphs.push_back(m_main->getSubgraph(node));
    }
}

void FilterGraph::storeSubgraphs() {
    deleteDatabase();

    for (auto &subgraph : m_SubGraphs) {
        auto subgraphNodes = subgraph.getNodes();
        subgraphNodes[0].addProperty(
            {.variable = "isMacro", .value = makeString("true")});
        subgraph.setNodes(subgraphNodes);
        createGraph(subgraph);
    }
}

void FilterGraph::replace(DatabaseInfo databaseInfo) {
    std::unique_ptr<Graph> mainDatabase =
        std::make_unique<Graph>(databaseInfo);

    for (auto &subgraph : m_SubGraphs) {
        auto subgraphNodes = subgraph.getNodes();
        for (auto it = subgraphNodes.begin(); it != subgraphNodes.end(); ++it) {
            // Modify first node --> point that will be linked with the main
            // graph later
            if (it == subgraphNodes.begin()) {
                Property property = {.variable = "isMacro",
                                     .value = makeString("true")};

                Node modified = *it;
                it->removeProperty(property);

                // Overwrite properties of the current node
                mainDatabase->modifyNode(*it, modified);
            } else {
                // delete the other ones (are already stored in makro database)
                mainDatabase->deleteNode(*it);
            }
        }
    }
}

void FilterGraph::replace(DatabaseInfo databaseInfo, Node link) {
    std::unique_ptr<Graph> mainDatabase =
        std::make_unique<Graph>(databaseInfo);

    for (auto &subgraph : m_SubGraphs) {
        auto subgraphNodes = subgraph.getNodes();
        for (auto it = subgraphNodes.begin(); it != subgraphNodes.end(); ++it) {
            // Modify first node --> point that will be linked with the main
            // graph later
            if (it->compare(link)) {
                Property property = {.variable = "isMacro",
                                     .value = makeString("true")};

                Node modified = *it;
                it->removeProperty(property);

                // Overwrite properties of the current node
                mainDatabase->modifyNode(*it, modified);
            } else {
                // delete the other ones (are already stored in makro database)
                mainDatabase->deleteNode(*it);
            }
        }
    }
}

void FilterGraph::loadSubgraphs() {
    m_SubGraphs.clear();

    Node node;
    node.setVariable("a");
    Property property = {.variable = "isMacro", .value = makeString("true")};

    std::string query = m_cypher.conditionQuery(node, property, "a, labels(a)");

    std::string jsonString = sendQuery(query);

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

                for (auto &label : rows[1]) {
                    node.setLabel(label);
                }
                nodes.push_back(node);
            }
        }
     }

    for (auto &node : nodes) {
        AdjacencyMatrix subgraph = getSubgraph(node);
        node.removeProperty(property);
        m_SubGraphs.push_back(subgraph);
    }
}

void FilterGraph::restore() {
    loadSubgraphs();
    for (auto &subgraph : m_SubGraphs) {
        m_main->appendGraph(subgraph);
    }
}
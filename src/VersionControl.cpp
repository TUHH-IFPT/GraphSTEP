#include "VersionControl.h"

#include "Tools.hpp"

const string separator = ";";
Blob::Blob() {}
Blob::~Blob() {}

std::string propertyToNeo4j(Property property) {
    return (property.variable + separator + removeQuotation(property.value) +
            separator);
}

Modified modifiedStrToData(std::string dataStr) {
    Modified data;
    std::vector<std::string> list = getListFromStrings(dataStr, separator[0]);
    data.nodeId = list[0];
    data.propertyOld.variable = list[1];
    data.propertyOld.value = list[2];
    data.propertyNew.variable = list[3];
    data.propertyNew.value = list[4];

    return data;
}

Node addedNodeStrToData(std::string dataStr) {
    Node data;
    std::vector<std::string> list = getListFromStrings(dataStr, separator[0]);
    data.setLabel(list[0]);
    data.setId(list[2]);
    list.erase(list.begin(), list.begin() + 3);

    for (auto iterator = list.begin(); iterator != list.end(); ++iterator) {
        data.addProperty(
            {.variable = *iterator, .value = makeString(*(++iterator))});
    }

    return data;
}

Relation addedRelationStrToData(std::string dataStr) {
    Relation data;
    std::vector<std::string> list = getListFromStrings(dataStr, separator[0]);

    data.nodeIdFrom = list[0];
    data.nodeIdTo = list[1];
    data.relation = list[2];

    return data;
}

VersionControl::VersionControl()
    : Graph(), m_latestId(""), m_branch(""){}

VersionControl::VersionControl(DatabaseInfo databaseInfo)
    : m_workDb(databaseInfo),
      m_latestId(""),
      m_branch("main"){

    DatabaseInfo historyDb = databaseInfo;
    historyDb.databaseName = "history";
    this->initRestInterface(historyDb);

    m_work = std::make_unique<PullSTEP>("", databaseInfo);
}

VersionControl::~VersionControl() {}

std::string VersionControl::getLatestId() {
    Blob blob;
    std::vector<string> labels = getAllLabels();

    Node next;
    Node current;
    current.setLabel("first_commit");
    current = getNode(current);
    current.deleteProperties();
    int counter = 0;

    do {
        next = getNextNode(current, "main");
        if (next.isEmpty()) {
            m_latestId = current.getId();
            break;
        } else {
            m_latestId = next.getId();
        }
        current.setLabel(next.getLabel());
        current.setId(next.getId());

        ++counter;
    } while (!next.isEmpty());

    return m_latestId;
}

void VersionControl::commitBlob(Blob blob) {
    Node commitNode;
    commitNode.setId(uuid::generateUuidV4());

    std::vector<Modified> modifiedNodes = blob.getModified();
    std::vector<Node> newNodes = blob.getNewNodes();
    std::vector<Relation> newRelations = blob.getNewRelations();

    if (getAllLabels().empty()) {
        Node firstNode("first_commit");
        firstNode.setLabel("first_commit");
        std::string query = m_cypher.createNodeQuery(firstNode);
        sendQuery(query);
        commitNode.setLabel(blob.getMessage());
        m_latestId = "first_commit";
    } else {
        commitNode.setLabel(blob.getMessage());
        m_latestId = getLatestId();
    }

    // Push date
    commitNode.addProperty(
        {.variable = "date", .value = makeString(getCurrentTime())});

    int counterModified = 0;
    int counterNodesAdded = 0;
    int counterRelationsAdded = 0;
    for (auto &modified : modifiedNodes) {
        std::string modifiedStr = modified.nodeId;
        modifiedStr += separator;
        modifiedStr += propertyToNeo4j(modified.propertyOld);
        modifiedStr += propertyToNeo4j(modified.propertyNew);
        commitNode.addProperty(
            {.variable = "modified_" + std::to_string(counterModified),
             .value = makeString(modifiedStr)});
        ++counterModified;
    }

    for (auto &nodeadded : newNodes) {
        std::string addedNodeStr = nodeadded.getLabel();
        addedNodeStr += separator + "Id" + separator + nodeadded.getId();
        std::vector<Property> properties = nodeadded.getProperties();
        addedNodeStr += separator;
        for (auto &property : properties) {
            property.value = removeQuotation(property.value);
            addedNodeStr += propertyToNeo4j(property);
        }

        commitNode.addProperty(
            {.variable = "node_added_" + std::to_string(counterNodesAdded),
             .value = makeString(addedNodeStr)});
        ++counterNodesAdded;
    }

    for (auto &relationAdded : newRelations) {
        std::string addedRelationStr;
        addedRelationStr += relationAdded.nodeIdFrom + separator +
                            relationAdded.nodeIdTo + separator +
                            relationAdded.relation;

        commitNode.addProperty(
            {.variable =
                 "relation_added_" + std::to_string(counterRelationsAdded),
             .value = makeString(addedRelationStr)});
        ++counterRelationsAdded;
    }

    // https://community.neo4j.com/t5/neo4j-graph-platform/can-i-add-jsonobject-as-a-value-to-a-property-in-a-node/m-p/34590
    // You can add a JSON string as a property to a node, but a JSON structure
    // is not supported

    std::string query = m_cypher.createNodeQuery(commitNode);
    this->sendQuery(query);

    if (commitNode.getLabel() != "first_commit") {
        Node from(m_latestId);

        Node to(commitNode.getId());
        sendQuery(m_cypher.createRelation(from, to, m_branch));
    }
}

void VersionControl::checkout(std::string commitId) {
    bool terminate = false;
    std::vector<string> labels = getAllLabels();

    if (labels.empty()) {
        Logger::log("database is empty");
        return;
    }

    Node currentNode;
    currentNode.setLabel("first_commit");
    currentNode = getNode(currentNode);
    currentNode = getNextNode(currentNode, "main");

    if(currentNode.isEmpty()){
        Logger::error("History structure is not consistent.");
        return;
    }

    m_work = std::make_unique<PullSTEP>("", this->m_workDb);
    m_work->deleteDatabase();

    if (std::find(labels.begin(), labels.end(), commitId) != labels.end()) {
        while (!terminate) {
            if ((currentNode.getLabel() == commitId) || currentNode.isEmpty())
                terminate = true;

            loadCommit(currentNode);
            currentNode = getNextNode(currentNode, "main");
        }
    } else {
        Logger::error("Commit \"" + commitId + "\" does not exist.");
        return;
    }

    Logger::log("checked out commit " + commitId);
}

void VersionControl::loadCommit(Node commit) {
    std::vector<Property> properties = commit.getProperties();

    m_work = std::make_unique<PullSTEP>("", m_workDb);

    for (auto &property : properties) {
        if (property.variable.find("modified_") != std::string::npos) {
            Modified modified = modifiedStrToData(property.value);
            Node node;
            node.setId(modified.nodeId);
            node.addProperty(modified.propertyOld);

            m_work->modifyNode(node, modified.propertyNew);
        } else if (property.variable.find("node_added_") != std::string::npos) {
            Node node = addedNodeStrToData(property.value);
            m_work->createNode(node);
        } else if (property.variable.find("relation_added_") !=
                   std::string::npos) {
            Relation relation = addedRelationStrToData(property.value);
            Node from;
            from.setId(relation.nodeIdFrom);
            from = m_work->getNode(from);

            Node to;
            to.setId(relation.nodeIdTo);
            to = m_work->getNode(to);
            m_work->createRelation(from, to, relation.relation);
        }
    }

    m_work->writeStep(false);
}

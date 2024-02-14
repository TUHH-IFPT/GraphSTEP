#pragma once

#include <tuple>

#include "PullStep.h"
#include "Tools.hpp"

/**
 * @brief VersionControl
 * keeps track of changes
 * restores previous versions
**/

struct Modified {
    std::string nodeId;
    Property propertyOld;
    Property propertyNew;
};

struct Relation {
    std::string nodeIdFrom;
    std::string nodeIdTo;
    std::string relation;
};

class Blob {
   public:
    Blob();
    ~Blob();

    void setModified(std::vector<Modified> modified) { m_modified = modified; }
    void setNewNodes(std::vector<Node> nodes) { m_newNodes = nodes; }
    void setRelations(std::vector<Relation> relations) {
        m_newRelations = relations;
    }

    void addModified(Modified modified) { m_modified.push_back(modified); }
    void addNewNode(Node node) { m_newNodes.push_back(node); }
    void addNewRelation(Node from, Node to, std::string relation) {
        m_newRelations.push_back({.nodeIdFrom = from.getId(),
                                  .nodeIdTo = to.getId(),
                                  .relation = relation});
    }

    void setMessage(std::string message) { m_message = message; }
    std::string getMessage() { return m_message; }

    std::vector<Modified> getModified() { return m_modified; }
    std::vector<Node> getNewNodes() { return m_newNodes; }
    std::vector<Relation> getNewRelations() { return m_newRelations; }

    void setId(std::string id) { m_id = id; }
    std::string getId() { return m_id; }

   private:
    std::vector<Modified> m_modified;
    std::vector<Node> m_newNodes;
    std::vector<Relation> m_newRelations;

    // Unique id
    std::string m_id;

    std::string m_message;
};

class VersionControl : public Graph {
   public:
    VersionControl();

    VersionControl(DatabaseInfo databaseInfo);
    ~VersionControl();

    // Writes blob object to history database and creates a unique identifier
    void commitBlob(Blob blob);

    void loadCommit(Node commit);

    // Returns stepfile of a specific version
    void checkout(std::string commitId);

    std::string getLatestId();

   protected:
    std::unique_ptr<PullSTEP> m_work;  // Graph where we currently work on

    DatabaseInfo
        m_workDb;  // Info about the database where we currently work on
    std::string m_latestId;
    std::string m_branch;    // Current branch
};
#pragma once

#include <SdaiHeaderSchema.h>

#include "Graph.h"
#include "Tools.hpp"
#include "TypesNeo4j.h"

/**
 * @brief PullStep
 * loads the graph into the AdjacencyMatrix data structure
 * creates a step file
**/

extern void SchemaInit(class Registry &);

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

class PullSTEP : public Graph {
   public:
    PullSTEP();
    PullSTEP(std::string outputPath, DatabaseInfo databaseInfo);
    ~PullSTEP();

    void populateEntity(STEPentity *ent, Node node);
    void populateComplexEntity(STEPcomplex *ent, Node complexNode);
    int writeStep(bool createAdjacencyMatrix = true);

    std::vector<std::string> getListEntries(Node listNode);

   private:
    // path to the generated step file
    std::string m_outputPath;

    // Link entity id (int) of the new stepfile with the node uuid (string)
    std::map<std::string, int> fileIdMap;

    // Counter used for the instance numbers (FileId)
    int m_entityCounter;

    // Store the entities that have to be written
    std::vector<STEPentity *> m_stepEntities; 

    // Final order of step entities
    InstMgr m_instances;

    // Registry pointer used to get some information about an entity
    Registry *m_registry;

    void appendStringAggregate(STEPattribute *attr, std::string aggr);
    bool appendSelectAggregate(STEPattribute *attr,
                               std::vector<std::string> entries);
    bool appendSelectTyped(STEPattribute *attr,
                           std::vector<std::string> entries, int fileId,
                           std::string typedName);
    bool appendEntityAggregate(STEPattribute *attr,
                               std::vector<std::string> entries);
};
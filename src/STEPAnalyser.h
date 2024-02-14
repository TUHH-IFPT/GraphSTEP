#pragma once

#include <filesystem>

#include "Graph.h"
#include "Tools.hpp"

/**
 * @brief STEPAnalyser
 * Analysis of a STEP file (e.g. number of entities)
**/

class STEPAnalyser {
   public:
    STEPAnalyser(std::string path);
    ~STEPAnalyser();

    int getNumEntities() { return m_numEntities; }

   private:
    void analyseFile();

    InstMgr m_lstInst;  // object to store all instances/attributes etc. of the
                        // Stepfile
    std::string m_filePath;
    int m_numEntities;
};

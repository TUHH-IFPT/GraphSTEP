#pragma once

#include "ManipulateGraph.h"

/**
 * @brief FilterGraph
 * used to filter certain subgraphs from the main graph
 * finds nodes that can be replaced with subnodes 
 * usally: nodes that lead to tree nodes:
 * * e.g. Circle, Plane, Line, Presentation_Style_Assignment
**/

enum class DatabaseFilter {
    Circle,
    Plane,
    Line,
    PresentationStyleAssignment,
    Undefined
};

inline std::string databaseFilterToStr(const DatabaseFilter filter) {
    if (filter == DatabaseFilter::Circle)
        return "Circle";
    else if (filter == DatabaseFilter::Plane)
        return "Plane";
    else if (filter == DatabaseFilter::Line)
        return "Line";
    else if (filter == DatabaseFilter::PresentationStyleAssignment)
        return "Presentation_Style_Assignment";

    return "";
}

class FilterGraph : public ManipulateGraph {
   public:
    FilterGraph();
    FilterGraph(DatabaseInfo makroGraphInfo);
    ~FilterGraph();

    void initDatabase(DatabaseInfo mainGraphInfo);

    void collect(DatabaseFilter filter);

    void deletePart(std::string part);

    void storeSubgraphs();

    void loadSubgraphs();

    // replaces the subgraph with a makro
    void replace(DatabaseInfo databaseInfo);
    void replace(DatabaseInfo databaseInfo, Node link);

    // restores the old state of the main database
    void restore();

    void addSubgraph(AdjacencyMatrix subgraph) {
        m_SubGraphs.push_back(subgraph);
    }

       private:
std::unique_ptr<Graph> m_main;
    std::vector<AdjacencyMatrix> m_SubGraphs;
    DatabaseFilter m_filter;
};
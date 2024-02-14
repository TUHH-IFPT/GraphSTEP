#include "CypherParser.h"

CypherParser::CypherParser() {}

CypherParser::~CypherParser() {}

std::string CypherParser::createRelation(Node from, Node to,
                                         std::string relation) {
    std::string cypherStr = "";

    from.makeStringProperties();
    to.makeStringProperties();

    std::vector<Property> properties_from = from.getProperties();
    std::vector<Property> properties_to = to.getProperties();

    from.setVariable("a");
    to.setVariable("b");

    if (!from.getLabel().empty())
        cypherStr +=
            "MATCH (" + from.getVariable() + ":" + from.getLabel() + "),";
    else
        cypherStr += "MATCH (" + from.getVariable() + "),";

    if (!to.getLabel().empty())
        cypherStr += "(" + to.getVariable() + ":" + to.getLabel() + ")\n";
    else
        cypherStr += "(" + to.getVariable() + ")\n";

    cypherStr +=
        "WHERE " + from.getVariable() + ".Id = " + makeString(from.getId());
    cypherStr += " AND " + to.getVariable() + ".Id = " + makeString(to.getId());

    // add additional properties as constraints for the query
    for (auto property_from = properties_from.begin();
         property_from != properties_from.end(); ++property_from)
        cypherStr += " AND " + from.getVariable() + "." +
                     property_from->variable + "=" + property_from->value;
    for (auto property_to = properties_to.begin();
         property_to != properties_to.end(); ++property_to)
        cypherStr += " AND " + to.getVariable() + "." + property_to->variable +
                     "=" + property_to->value;

    cypherStr += "\nCREATE (" + from.getVariable() + ")-[:" + relation +
                 "]->(" + to.getVariable() + ")\n";
    cypherStr += "RETURN * \n";

    return cypherStr;
}

std::string CypherParser::depthString(std::string variable, int depth) {
    std::string cypherRelation = "";

    if (depth < 0)
        cypherRelation += variable + "*";
    else if (depth == 0)
        cypherRelation += variable;
    else
        cypherRelation += variable + "*" + std::to_string(depth);

    return cypherRelation;
}

std::string CypherParser::createNodeQuery(Node node) {
    std::string query = "";
    query += "CREATE (";
    node.makeStringProperties();
    query += node.toCypher();
    query += ")";
    return query;
}

std::string CypherParser::matchQuery(Node from, std::string relation, Node to,
                                     std::string ret) {
    from.makeStringProperties();
    to.makeStringProperties();

    if (ret.empty())
        return "MATCH (" + from.toCypher() + ")-[" + relation + "]->(" +
               to.toCypher() + ")";

    return "MATCH (" + from.toCypher() + ")-[" + relation + "]->(" +
           to.toCypher() + ") RETURN " + ret;
}

std::string CypherParser::matchQuery(Node node, std::string ret) {
    if (ret.empty()) return "MATCH (" + node.toCypher() + ")";

    return "MATCH (" + node.toCypher() + ") RETURN " + ret;
}

std::string CypherParser::deleteQuery(Node node) {
    return "MATCH (" + node.toCypher() + ") DETACH DELETE " +
           node.getVariable();
}

std::string CypherParser::conditionQuery(Node node, Property condition,
                                         std::string ret) {
    return "MATCH (" + node.toCypher() + ") WHERE " + node.getVariable() + "." +
           condition.variable + "=" + condition.value + " RETURN " + ret;
}

std::string CypherParser::modifyNodeQuery(Node node, Node modified) {
    // MATCH (a:Circle { Id: "Circle_74" })
    // SET a:Circle, a.isMacro = "true"
    // DELETE a.name
    // DELETE a.radius

    node.makeStringProperties();
    node.setVariable("a");

    modified.makeStringProperties();
    std::string query = matchQuery(node) + "\n";

    std::vector<Property> toReplace = modified.getProperties();
    for (auto &property : toReplace)
        query += "SET " + node.getVariable() + "." + property.variable + "=" +
                 property.value + "\n";

    return query;
}

std::string CypherParser::modifyNodeQuery(Node node, Property newProperty) {
    node.makeStringProperties();
    node.setVariable("a");

    std::string query = matchQuery(node) + "\n";

    std::vector<Property> toDelete = node.getProperties();
    for (auto &property : toDelete)
        query +=
            "DELETE " + node.getVariable() + "." + property.variable + "\n";

    query += "DELETE " + node.getVariable() + "." + newProperty.variable + "\n";
    query += "SET " + node.getVariable() + "." + newProperty.variable + "=" +
             newProperty.value + "\n";

    return query;
}
#include "PushStep.h"

PushSTEP::PushSTEP() : Graph(), m_filePath(""), m_fileName("") {}

PushSTEP::PushSTEP(std::string path, DatabaseInfo databaseInfo)
    : Graph(path, databaseInfo) {
    m_filePath = path;
    m_fileName = std::filesystem::path(path).stem();
}

PushSTEP::~PushSTEP() {}

void PushSTEP::commitChanges(std::string message) {
    m_trackChanges.setMessage(message);
    VersionControl control(this->m_databaseInfo);
    control.commitBlob(m_trackChanges);
}

void PushSTEP::createNode(Node node) {
    std::string query = m_cypher.createNodeQuery(node);
    this->m_trackChanges.addNewNode(node);
    pushQueryToJson(query);
}

void PushSTEP::createRelation(Node from, Node to, std::string relation) {
    this->m_trackChanges.addNewRelation(from, to, relation);
    pushQueryToJson(m_cypher.createRelation(from, to, relation));
}

bool PushSTEP::build() {
    if (createInstanceNodes()) {
        sendQueries();
        Logger::log("queries for the nodes created");
        if (createRelations()) {
            sendQueries();
            Logger::log("queries for the relations created");
        } else {
            Logger::error("failed to create the queries for the relations");
            return false;
        }
    } else {
        Logger::error("failed to create the queries for the nodes");
        return false;
    }

    return true;
}

bool PushSTEP::createInstanceNodes() {
    Registry registry(SchemaInit);
    STEPfile stepFile(registry, m_lstInst, "", false);
    stepFile.ReadExchangeFile(m_path);

    // Number of instances
    int numInst = m_lstInst.InstanceCount();

    // Id of the according entity (e.g #11= ...)
    int entityId = 0;

    for (int i = 0; i < numInst; ++i) {
        // Current instance
        SDAI_Application_instance_ptr pInstance = m_lstInst.GetSTEPentity(i);

        // Name of current entity
        m_entity.name = pInstance->EntityName();

        // Id of the according entity (e.g #11= ...)
        m_entity.fileId = std::to_string(pInstance->StepFileId());

        // Add entity id to node to link the correct nodes to each other
        Node nodeInstance(m_entity.fileId);
        nodeInstance.setLabel(m_entity.name);

        // Check complex entity (e.g.
        // "#3=(NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.));")
        if (pInstance->IsComplex()) {
            STEPcomplex* pInstComplex = (STEPcomplex*)pInstance;
            std::string entityName = "";

            /* ----- Create COMPLEX_TYPE node ---------------------------*/
            Node complex_node(m_entity.fileId);
            complex_node.setLabel(TYPE_COMPLEX);
            complex_node.createId();
            m_nodeIdMap[m_entity.fileId] = complex_node;
            createNode(complex_node);

            /* ----- Create COMPLEX_TYPE subnodes ---------------------------*/
            while (pInstComplex != nullptr) {
                SDAI_Application_instance_ptr compInst =
                    (SDAI_Application_instance_ptr)pInstComplex;
                m_entity.name = compInst->EntityName();
                m_entity.fileId = std::to_string(pInstance->StepFileId());

                // Entity id of the subnode is the same as the parents one
                Node node(m_entity.fileId);
                node.setLabel(m_entity.name);

                getAttributesForNodes(compInst, node);
                node.createId();
                m_nodeIdMap[m_entity.fileId + m_entity.name] = node;
                createNode(node);

                pInstComplex = pInstComplex->sc;
            }
        } else {
            getAttributesForNodes(pInstance, nodeInstance);
            nodeInstance.createId();
            m_nodeIdMap[m_entity.fileId + m_entity.name] = nodeInstance;
            createNode(nodeInstance);
        }
    }

    return true;
}

void PushSTEP::getAttributesForNodes(SDAI_Application_instance_ptr pInst,
                                     Node& node) {
    if (pInst == nullptr) {
        Logger::error("failed to read the attributes");
        return;
    }

    // Stores all attributes of the current node
    STEPattributeList attrList = pInst->attributes;

    int attrCnt = attrList.EntryCount();

    // No attributes ...
    if (attrCnt == 0) return;

    // Iterate through all attributes of one entity
    for (int i = 0; i < attrCnt; ++i) {
        STEPattribute* attr = &attrList.operator[](i);

        const AttrDescriptor* attrDes = attr->getADesc();

        // Type of the Attribute
        BASE_TYPE attrType = attr->NonRefType();

        // Name of the attribute
        std::string attrName = attrDes->Name();

        STEPattribute* redefAttr =
            attr->RedefiningAttr();  // redefined attribute

        // if a redefined attribute exits, overwrite the current attribute with
        // the redefined one only the redefined attribute contains values e.g.
        // attr = "*" but redefAttr = "#23"
        if (redefAttr) {
            // Get the subtype, not the supertype
            if (attr->IsDerived()) continue;
        }

        // Remove the supertype from the string (if entity inherits from another
        // one)
        if (attrName.find(".") != std::string::npos) {
            attrName =
                attrName.substr(attrName.find(".") + 1, attrName.length());

            if (attrName.find(".") != std::string::npos)
                throw_database_error("something went wrong");
        }

        switch (attrType) {
            // TYPE == PRIMITVE
            case sdaiSTRING: {
                // Stepcode strings contain parentheses (if they aren't empty)
                // --> no need to call makeString()
                Property property;
                property.variable = attrName;
                if (attr->asStr().empty())
                    property.value = makeString(attr->asStr());
                else if (attr->asStr() == "*")
                    property.value = makeString(attr->asStr());
                else
                    property.value = attr->asStr();

                node.addProperty(property);
            } break;
            case sdaiINTEGER:
            case sdaiREAL:
            case sdaiENUMERATION:
            case sdaiBOOLEAN:
            case sdaiLOGICAL: {
                // Push properties to the node if the attribute type is a
                // std::string, integer or any other "basic" datatype
                if (!attr->asStr().empty()) {
                    Property property = {.variable = attrName,
                                         .value = makeString(attr->asStr())};
                    node.addProperty(property);
                }
            } break;
            case LIST_TYPE: {
                STEPaggregate_ptr aggr = attr->Aggregate();
                int aggrCnt = aggr->EntryCount();
                if (aggrCnt == 0) {
                    // Add empty list
                    Property property = {.variable = attrName,
                                         .value = makeString("()")};
                    node.addProperty(property);
                    continue;
                }
                std::string attribute = attr->asStr();
                if (attribute.find("#") == std::string::npos) {
                    // Must be a primitive type
                    // e.g. attr->asStr() == (0.,0.,1.)

                    Property property = {.variable = attrName,
                                         .value = makeString(attr->asStr())};
                    node.addProperty(property);
                }
            } break;
            case SET_TYPE:
            case BAG_TYPE:
            case ARRAY_TYPE: {
                STEPaggregate_ptr aggr = attr->Aggregate();
                int aggrCnt = aggr->EntryCount();

                if (aggrCnt == 0) continue;

                std::string attribute = attr->asStr();
                if (attribute.find("#") == std::string::npos) {
                    // Must be a primitive type
                    // e.g. attr->asStr() == (0.,0.,1.)

                    Property property = {.variable = attrName,
                                         .value = makeString(attr->asStr())};
                    node.addProperty(property);
                }
            } break;
            case sdaiINSTANCE:
            case sdaiSELECT: {
                std::string attribute = attr->asStr();
                if (attribute.find("*") != std::string::npos)  // Found
                {
                    node.addProperty({.variable = attrName,
                                      .value = makeString(attr->asStr())});
                    continue;
                } else if (attribute.find("#") ==
                           std::string::npos)  // Not found
                {
                    // Must be a primitive type
                    // e.g. attr->asStr() ==

                    node.addProperty({.variable = attrName,
                                      .value = makeString(attr->asStr())});
                }
            } break;
            default:
                break;
        }
    }
}

bool PushSTEP::createRelations() {
    Registry registry(SchemaInit);
    STEPfile stepFile(registry, m_lstInst, "", false);
    stepFile.ReadExchangeFile(m_path);

    int numInst = m_lstInst.InstanceCount();

    std::string entityName = "";

    // Id of the according entity (e.g #11= ...)
    int entityId = 0;

    for (int i = 0; i < numInst; ++i) {
        // Current instance
        SDAI_Application_instance_ptr pInstance = m_lstInst.GetSTEPentity(i);

        m_entity.name = pInstance->EntityName();
        m_entity.fileId = std::to_string(pInstance->StepFileId());

        if (pInstance->IsComplex()) {
            STEPcomplex* pInstComplex = (STEPcomplex*)pInstance;
            std::string entityName = "";

            m_entity.name = TYPE_COMPLEX;

            int complex_counter = 0;
            while (pInstComplex != nullptr) {
                SDAI_Application_instance_ptr compInst =
                    (SDAI_Application_instance_ptr)pInstComplex;
                m_entity.name = compInst->EntityName();
                m_entity.fileId = std::to_string(pInstance->StepFileId());

                Node from = m_nodeIdMap[m_entity.fileId];
                Node to = m_nodeIdMap[m_entity.fileId + m_entity.name];

                createRelation(
                    from, to,
                    "entry{num: " + std::to_string(complex_counter) + "}");
                ++complex_counter;

                getAttributesForRelations(compInst);

                pInstComplex = pInstComplex->sc;
            }
        } else {
            getAttributesForRelations(pInstance);
        }
    }
    return true;
}

EntityInfo PushSTEP::getEntityInfoFromId(std::string idStr) {
    // Deletes null-terminator
    idStr.erase(std::find(idStr.begin(), idStr.end(), '\0'), idStr.end());

    // Removes "#"
    idStr = idStr.substr(1, idStr.size());

    if (!isInteger(idStr)) {
        return {.name = "", .fileId = ""};
    }

    // Convert the id to an int
    int idNum = std::stoi(idStr);

    // Searches a certain entity
    MgrNode* mgrnode = m_lstInst.FindFileId(idNum);
    auto instance = mgrnode->GetApplication_instance();

    return {.name = instance->EntityName(), .fileId = idStr};
}

void PushSTEP::getAttributesForRelations(SDAI_Application_instance_ptr pInst) {
    if (pInst == nullptr) {
        Logger::error("failed to read the attributes");
        return;
    }

    // Attribute list
    STEPattributeList attrList = pInst->attributes;

    // Attribute list count
    int attrCnt = attrList.EntryCount();

    // No attributes ...
    if (attrCnt == 0) return;

    // Iterate through all attributes of an entity
    for (int i = 0; i < attrCnt; ++i) {
        STEPattribute* attr = &attrList.operator[](i);

        const AttrDescriptor* attrDes = attr->getADesc();

        // Type of the attribute
        BASE_TYPE attrType = attr->NonRefType();

        // Name of the attribute
        std::string attrName = attrDes->Name();

        STEPattribute* redefAttr =
            attr->RedefiningAttr();  // Redefined attribute
        // If a redefined attribute exits, overwrite the current attribute with
        // the redefined one Only the redefined attribute contains values e.g.
        // attr = "*" but redefAttr = "#23"
        if (redefAttr) {
            // We are not interested in the supertype of the attribute --> skip
            if (attr->IsDerived())
                continue;
            else
                attr = redefAttr;
        } else {
            if (attr->IsDerived()) auto test = true;
        }

        if (attrName.find(".") != std::string::npos) {
            attrName =
                attrName.substr(attrName.find(".") + 1, attrName.length());
            if (attrName.find(".") != std::string::npos)
                throw std::runtime_error("something went wrong ...");
        }

        // TYPE: SELECT and INSTANCE
        if (attrType == sdaiSELECT || attrType == sdaiINSTANCE) {
            auto test = attr->NonRefType();
            std::string value = attr->asStr();

            if (value.find("#") != std::string::npos) {
                if (value[0] == '#') {
                    // Normal set
                    Node from = m_nodeIdMap[m_entity.fileId + m_entity.name];

                    EntityInfo entity = getEntityInfoFromId(attr->asStr());
                    Node to = m_nodeIdMap[entity.fileId + entity.name];
                    createRelation(from, to, attrName);

                    continue;
                } else {
                    // Typed set
                    // e.g. SET_REPRESENTATION_ITEM((#854,#853))

                    auto data = convertTypedSet(value);
                    Node from = m_nodeIdMap[m_entity.fileId + m_entity.name];

                    Node intermediateNode;
                    intermediateNode.createId();
                    intermediateNode.setLabel("SelectInstance");
                    intermediateNode.addProperty(
                        {.variable = "type", .value = data.first});

                    createNode(intermediateNode);
                    createRelation(from, intermediateNode, attrName);

                    int counter = 0;

                    for (auto entry : data.second) {
                        EntityInfo entity = getEntityInfoFromId(entry);
                        Node to = m_nodeIdMap[entity.fileId + entity.name];

                        createRelation(intermediateNode, to,
                                       "entry_" + std::to_string(counter));
                        ++counter;
                    }
                }
            }
        }

        // TYPES With many entries
        if (attrType == SET_TYPE || attrType == BAG_TYPE ||
            attrType == LIST_TYPE || attrType == ARRAY_TYPE) {
            STEPaggregate_ptr aggr = attr->Aggregate();
            int aggrCnt = aggr->EntryCount();

            if (aggrCnt == 0) continue;

            // Make sure to link nodes and no "primitive data types"
            if (attr->asStr().find("#") != std::string::npos) {
                int counter = 0;
                std::vector<std::string> attrList =
                    getEntriesAggregate(attr->asStr());
                Node from = m_nodeIdMap[m_entity.fileId + m_entity.name];

                for (auto entry : attrList) {
                    Node to;

                    if (entry.find("#") != std::string::npos) {
                        EntityInfo entity = getEntityInfoFromId(entry);
                        to = m_nodeIdMap[entity.fileId + entity.name];
                    }
                    if (!(to.getLabel() == "")) {
                        createRelation(
                            from, to,
                            attrName + "_list_type_" + std::to_string(counter));
                        ++counter;
                    }
                }
            }
        }
    }
}

std::pair<std::string, std::vector<std::string>> PushSTEP::convertTypedSet(
    std::string select) {
    std::string select_name;
    auto pos_1 = select.find("(");

    if (pos_1 == std::string::npos)
        throw std::invalid_argument("select not typed");

    select_name = select.substr(0, pos_1);

    auto pos_2 = select.find(")");
    std::string aggregate = select.substr(pos_1, pos_2);
    aggregate.pop_back();
    aggregate = removeParentheses(aggregate);
    std::vector<std::string> entries = getEntriesAggregate(aggregate);

    return std::make_pair(select_name, entries);
}

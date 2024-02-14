#include "PullStep.h"

PullSTEP::PullSTEP() : Graph(), m_entityCounter(0), m_registry(nullptr) {}

PullSTEP::PullSTEP(std::string outputPath, DatabaseInfo databaseInfo)
    : Graph(databaseInfo), m_entityCounter(0), m_outputPath(outputPath) {}

PullSTEP::~PullSTEP() {}

std::vector<std::string> PullSTEP::getListEntries(Node listNode) {
    std::vector<std::string> entries;
    bool terminate = false;
    int nodeCounter = 0;

    while (!terminate) {
        Node node = m_matrix.getNextNode(listNode,
                                         "entry" + std::to_string(nodeCounter));
        if (node.isEmpty())
            terminate = true;
        else
            entries.push_back(node.getId());

        ++nodeCounter;
    }
    return entries;
}

struct find_property_variable : std::unary_function<Property, bool> {
    string variable;
    find_property_variable(string variable) : variable(variable) {}
    bool operator()(Property const &m) const { return m.variable == variable; }
};

bool PullSTEP::appendEntityAggregate(STEPattribute *attr,
                                     std::vector<std::string> entries) {
    EntityAggregate_ptr aggr = new EntityAggregate();

    for (auto entry : entries) {
        int fileIdNew = 0;
        if (fileIdMap.find(entry) != fileIdMap.end()) {
            fileIdNew = fileIdMap[entry];
        } else {
            Logger::error("entity does not exist at the moment ...");
            return false;
        }
        aggr->AddNode(
            new EntityNode(m_instances.GetApplication_instance(fileIdNew)));
    }

    attr->Aggregate(aggr);
    return true;
}

bool PullSTEP::appendSelectAggregate(STEPattribute *attr,
                                     std::vector<std::string> entries) {
    SelectAggregate_ptr aggr = new SelectAggregate();

    for (auto entry : entries) {
        int fileIdNew = 0;
        if (fileIdMap.find(entry) != fileIdMap.end()) {
            fileIdNew = fileIdMap[entry];
        } else {
            Logger::error("entity does not exist at the moment ...");
            return false;
        }

        EntitySelect *sel =
            new EntitySelect((SelectTypeDescriptor *)m_instances
                                 .GetApplication_instance(fileIdNew)
                                 ->eDesc,
                             attr->ReferentType());
        sel->AssignEntity(m_instances.GetApplication_instance(fileIdNew));

        aggr->AddNode(new SelectNode(sel));
    }
    attr->Aggregate(aggr);
    return true;
}

bool PullSTEP::appendSelectTyped(STEPattribute *attr,
                                 std::vector<std::string> entries, int fileId,
                                 std::string typeName) {
    EntityAggregate_ptr aggr = new EntityAggregate();

    for (auto entry : entries) {
        int fileIdNew = 0;
        if (fileIdMap.find(entry) != fileIdMap.end()) {
            fileIdNew = fileIdMap[entry];
        } else {
            Logger::error("entity does not exist at the moment ...");
            return false;
        }

        aggr->AddNode(
            new EntityNode(m_instances.GetApplication_instance(fileIdNew)));
    }

    TypedSelect *sel =
        new TypedSelect(NULL, attr->aDesc->NonRefTypeDescriptor());
    sel->SetTypeName(typeName);
    sel->AssignEntity(m_instances.GetApplication_instance(fileId));
    sel->AddEntityAggregate(aggr);
    attr->ptr.sh = (SDAI_Select *)sel;
    return true;
}

void PullSTEP::appendStringAggregate(STEPattribute *attr, std::string aggr) {
    StringAggregate_ptr aggrStr = new StringAggregate();
    aggrStr->AddNode(new StringNode(removeParentheses(aggr).c_str()));

    attr->Aggregate(aggrStr);
}

void PullSTEP::populateEntity(STEPentity *ent, Node node) {
    std::string entityName = ent->EntityName();
    std::string log = "Populating " + entityName + " which has " +
                      std::to_string(ent->AttributeCount()) + " attributes.";
    Logger::log(log);

    std::string label = ent->EntityName();

    // Return properties of the node == primitive attributes of the entities
    // (INT, STRING, ENUM ...)
    std::vector<Property> properties = m_matrix.findProperties(node);
    std::vector<std::string> relations = m_matrix.getNodeRelations(node);

    // Relations to the aggregate nodes
    std::vector<std::string> aggregate_entries;

    // Remove _list_type_ from relations
    std::string toReplace = "_list_type_";
    for (auto &relation : relations) {
        if (relation.find(toReplace) != std::string::npos) {
            aggregate_entries.push_back(relation);

            relation = relation.substr(0, relation.find(toReplace));
        }
    }
    // Stores the value of the current attribute
    std::string attrValue = "";

    ent->ResetAttributes();

    STEPattribute *attr = ent->NextAttribute();

    while (attr != 0) {
        // True if the value of an attribute comes from the properties of a node
        bool isProperty = false;

        // If the attribute is derived --> set _derive to zero
        // see page 9 of paper:
        //   Design of a C++ Software Library for Implementing EXPRESS: The NIST
        //   STEP Class Library
        //
        //      "The derive flag in STEPattribute is used to specify that an
        //      attribute is derived (by way of an EXPRESS DERIVE statement) in
        //      a subtype of the entity instance."
        //
        //      "A value of “true” in this field indicates that the value would
        //      be written as an asterisk in a Part 21 file."
        if (attr->IsDerived() && (node.getNodeType() != NodeType::COMPLEX)) {
            attr->Derive(false);
            attr = ent->NextAttribute();
            continue;
            // Append the attributes to the subtype! not the supertype!
            // e.g.:
            // 	the current entity is: shape_definition_representation
            // 	the supertype of shape_definition_representation is
            // property_definition_representation
            // 	 shape_definition_representation has two attributes (definition,
            // used_representation) which are derived from
            // property_definition_representation 	but: stepcode will show
            // four attributes: 	     two attributes of the subtype
            // (property_definition_representation.definition,
            // property_definition_representation.used_representation)
            // 	           --> the value on the left of the dot is the supertype
            // 	     two attributes of the supertype (definition,
            // used_representation) 	the supertype attributes are derived:
            // _derive == true --> should be set to false 	the subtype
            // attributes are not derived:  _derive == false 	the value must
            // always be written to the subtype attribute (e.g.
            // property_definition_representation.definition,
            // property_definition_representation.used_representation)
        }

        const AttrDescriptor *attrDesc = attr->aDesc;
        std::string stepAttribute(attrDesc->Name());

        if (stepAttribute.find(".") != std::string::npos) {
            stepAttribute = stepAttribute.substr(stepAttribute.find(".") + 1,
                                                 stepAttribute.length());

            if (stepAttribute.find(".") != std::string::npos)
                throw_database_error("something went wrong");
        }

        Logger::log("Found attribute \"" + stepAttribute + "\" of type \"" +
                    attrDesc->TypeName() + "\"");

        // Check if attribute specified by the step standard is contained in the
        // neo4j-graph
        auto it = std::find_if(properties.begin(), properties.end(),
                               find_property_variable(stepAttribute));

        // Check if iterator is valid
        if (it != properties.end()) {
            isProperty = true;
            attrValue = it->value;
        } else {
            bool found = false;
            for (auto &relation_entry : relations) {
                if (relation_entry.find(stepAttribute) != std::string::npos)
                    found = true;
            }
            if (!found) {
                // Attribute not found ... Skip this attribute
                attr = ent->NextAttribute();
                continue;
            }
        }

        switch (attrDesc->NonRefType()) {
            case INTEGER_TYPE:
            case REAL_TYPE:
            case NUMBER_TYPE:
                break;
            case STRING_TYPE: {
                attrValue = makeString(attrValue);
            } break;
            case ENUM_TYPE:
            case BOOLEAN_TYPE:
            case LOGICAL_TYPE: {
                // Convert string to normal string first
                attrValue = removeQuotation(attrValue);
            } break;
            case SET_TYPE:
            case LIST_TYPE:
            case ARRAY_TYPE: {
                if (!isProperty) {
                    std::vector<std::string> entries;
                    std::vector<std::string> aggregate_entries_filtered;

                    for (auto &entry : aggregate_entries) {
                        if (entry.find(stepAttribute) != std::string::npos)
                            aggregate_entries_filtered.push_back(entry);
                    }

                    std::sort(aggregate_entries_filtered.begin(),
                              aggregate_entries_filtered.end(),
                              compareIntStrings);

                    for (auto &aggregate : aggregate_entries_filtered) {
                        Node aggregateNode =
                            m_matrix.getNextNode(node, aggregate);
                        entries.push_back(aggregateNode.getId());
                    }

                    switch (attr->BaseType()) {
                        case sdaiINSTANCE:
                            if (!appendEntityAggregate(attr, entries))
                                ent->NextAttribute();  // Skip attribute if it
                                                       // doesn't exist
                            break;
                        case sdaiSELECT:
                            if (!appendSelectAggregate(attr, entries))
                                ent->NextAttribute();  // Skip attribute if it
                                                       // doesn't exist
                            break;
                        default:
                            // No instances, just primitive variables (string,
                            // int, real ...)
                            appendStringAggregate(attr, attrValue);
                            break;
                    }
                    entries.clear();
                } else {
                    // No instances, just primitive variables (string, int, real
                    // ...)
                    appendStringAggregate(attr, attrValue);
                }
                attr = ent->NextAttribute();
                continue;
            } break;
            case SELECT_TYPE: {
                if (isProperty) {
                    // Just print the string
                    break;
                }

                int fileIdNew = 0;

                // Store the original FileID of the next node
                Node temp = m_matrix.getNextNode(node, stepAttribute);

                if (temp.getLabel() == "SelectInstance") {
                    std::vector<std::string> entries;
                    auto list = getChildNodes(temp);
                    // std::sort(list.begin(), list.end());

                    for (auto &entry : list)
                        entries.push_back(entry.first.getId());
                    Property type = temp.getProperty("type");
                    appendSelectTyped(attr, entries, fileIdNew, type.value);

                    attr = ent->NextAttribute();
                    continue;
                } else {
                    std::string fileIdOriginal = "";
                    fileIdOriginal = temp.getId();

                    // Use original FileId to return the new one
                    if (fileIdMap.find(fileIdOriginal) != fileIdMap.end())
                        fileIdNew = fileIdMap[fileIdOriginal];
                    else
                        throw_database_error(
                            "entity does not exist (node id: " +
                            fileIdOriginal + ")");

                    EntitySelect *sel = new EntitySelect(
                        NULL, attr->aDesc->NonRefTypeDescriptor());
                    sel->AssignEntity(
                        m_instances.GetApplication_instance(fileIdNew));

                    // Link the select type with the current attribute
                    attr->ptr.sh = (SDAI_Select *)sel;

                    attr = ent->NextAttribute();
                    continue;
                }
            } break;
            case sdaiINSTANCE: {
                if (isProperty) break;

                int fileIdNew = 0;

                // Store the orginial FileID of the next node
                std::string fileIdOriginal = "";
                Node temp = m_matrix.getNextNode(node, stepAttribute);
                fileIdOriginal = temp.getId();

                // use original FileId to return the new one
                if (fileIdMap.find(fileIdOriginal) != fileIdMap.end())
                    fileIdNew = fileIdMap[fileIdOriginal];
                else
                    throw_database_error("entity does not exist (node id: " +
                                         fileIdOriginal + ")");

                //----------------Link Attribute With
                // Entity--------------------//
                attr->ptr.c = new (SDAI_Application_instance *);
                *(attr->ptr.c) = m_instances.GetApplication_instance(fileIdNew);

                // Skip the StrToVal function
                attr = ent->NextAttribute();
                continue;
            } break;
            default:
                break;
        }

        Logger::log("Read attribute with value: " + attrValue);

        attr->StrToVal(attrValue.c_str());
        attrValue.clear();
        attr = ent->NextAttribute();
    }
}

void PullSTEP::populateComplexEntity(STEPcomplex *ent, Node complexNode) {
    std::vector<Node> children = m_matrix.findChildren(complexNode);

    while (ent) {
        Node childNode;
        std::string entity = ent->EntityName();
        for (auto &child : children) {
            if (entity == child.getLabel()) {
                childNode = child;
                break;
            }
        }

        if (childNode.getId().empty()) {
            // Id not found ... Skip entity
            continue;
        }

        populateEntity(ent, childNode);
        ent = ent->sc;
    }
}

bool isIntermediateNode(std::string node) {
    if (node == TYPE_COMPLEX || node == TYPE_LIST || node == "SelectInstance" ||
        node == "null_node")
        return true;

    return false;
}

int PullSTEP::writeStep(bool createAdjacencyMatrix) {
    if (createAdjacencyMatrix) loadAdjacencyMatrix();

    m_registry = new Registry(SchemaInit);
    STEPfile *sfile = new STEPfile(*m_registry, m_instances, "", false);

    m_registry->ResetSchemas();
    m_registry->ResetEntities();

    // Build file header
    InstMgr *header_instances = sfile->HeaderInstances();

    SdaiFile_name *fn = (SdaiFile_name *)sfile->HeaderDefaultFileName();
    header_instances->Append((SDAI_Application_instance *)fn, completeSE);
    fn->name_("'" + m_databaseInfo.databaseName + "_out.stp'");
    fn->time_stamp_("''");
    fn->author_()->AddNode(new StringNode("''"));
    fn->organization_()->AddNode(new StringNode("''"));
    fn->preprocessor_version_("''");
    fn->originating_system_("''");
    fn->authorization_("''");

    SdaiFile_description *fd =
        (SdaiFile_description *)sfile->HeaderDefaultFileDescription();
    header_instances->Append((SDAI_Application_instance *)fd, completeSE);
    fd->description_()->AddNode(new StringNode("''"));
    fd->implementation_level_("'2;1'");

    SdaiFile_schema *fs = (SdaiFile_schema *)sfile->HeaderDefaultFileSchema();
    header_instances->Append((SDAI_Application_instance *)fs, completeSE);
    fs->schema_identifiers_()->AddNode(
        new StringNode("'AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF { 1 0 "
                       "10303 442 1 1 4 }'"));

    int num_ents = m_registry->GetEntityCnt();

    // Print out what schema we're running through.
    const SchemaDescriptor *schema = m_registry->NextSchema();
    std::string schemaName(schema->Name());
    Logger::log("Building entities in schema " + schemaName);

    auto adjacencyMatrixNodes = m_matrix.getNodes();
    if (adjacencyMatrixNodes.empty()) {
        Logger::error("matrix contains no nodes");
        return -1;
    }

    // Collect all complex labels
    std::vector<std::string> complexLabels;

    for (auto &complexType : adjacencyMatrixNodes) {
        std::vector<std::string> entityNames;
        if (complexType.getLabel() == TYPE_COMPLEX) {
            std::vector<Node> children = m_matrix.findChildren(complexType);
            for (auto &child : children)
                entityNames.push_back(child.getLabel());

            // Copy all complex labels in one vector to make it easier to
            // determine a complex label in all labels
            complexLabels.insert(complexLabels.end(), entityNames.begin(),
                                 entityNames.end());

            // See
            // (https://github.com/stepcode/stepcode/blob/4bab26918d32d4bf51fa7fcf920f09da94a0bd7b/src/cllazyfile/sectionReader.cc)
            const int s = entityNames.size();
            const char **names = new const char *[s + 1];
            names[s] = 0;
            for (int i = 0; i < s; i++) {
                names[i] = entityNames[i].c_str();
            }

            STEPcomplex *complex =
                new STEPcomplex(m_registry, names, m_entityCounter);
            delete[] names;

            STEPcomplex *stepcomplex = complex->head;

            while (stepcomplex) stepcomplex = stepcomplex->sc;

            m_instances.Append((SDAI_Application_instance *)complex,
                               completeSE);

            fileIdMap.insert({complexType.getId(), m_entityCounter});

            m_entityCounter++;
        }
    }

    // Remove duplicates
    std::sort(complexLabels.begin(), complexLabels.end());
    complexLabels.erase(unique(complexLabels.begin(), complexLabels.end()),
                        complexLabels.end());

    // "Loop" through the schema, building one of each entity type.
    const EntityDescriptor *ent;

    // Counts the "normal" not complex entities
    int countStepEntities = 0;

    // Build "normal" entities
    for (auto &node : adjacencyMatrixNodes) {
        std::string label = node.getLabel();
        ent = m_registry->FindEntity(label.c_str());

        if (ent == nullptr) {
            if (!isIntermediateNode(label))
                throw_database_error("entity: " + label + " was not found");

            continue;
        }
        if (node.getNodeType() == NodeType::COMPLEX) {
            // Entry is complex -> skip
            continue;
        }

        // Check if key exists
        if (fileIdMap.find(node.getId()) == fileIdMap.end()) {
            // Add new map entry
            fileIdMap.insert({node.getId(), m_entityCounter});

            // Build object, using its name, through the registry
            m_stepEntities.push_back(m_registry->ObjCreate(ent->Name()));

            // Add each realized entity to the instance list
            m_instances.Append(m_stepEntities[countStepEntities], completeSE);

            ++countStepEntities;
            ++m_entityCounter;
        }
    }

    for (auto &entry : fileIdMap) {
        STEPentity *entity = m_instances.GetApplication_instance(entry.second);

        Node node(entry.first);

        if (entity->IsComplex()) {
            node.setLabel(TYPE_COMPLEX);
            populateComplexEntity((STEPcomplex *)entity, node);
        } else {
            node.setLabel(entity->EntityName());
            populateEntity(entity, node);
        }
    }

    if (m_outputPath.empty()) {
        Logger::warning("No output path given, using default path");
        m_outputPath = m_databaseInfo.databaseName + "_out.stp";
    }

    Logger::log("Writing STEPfile to output file " + m_outputPath);

    ofstream step_out(m_outputPath);
    sfile->WriteExchangeFile(step_out);

    delete (sfile);
    delete (m_registry);

    return 0;
}
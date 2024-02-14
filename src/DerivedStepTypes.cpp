#include "DerivedStepTypes.h"

TypedSelect::TypedSelect(const SelectTypeDescriptor *s, const TypeDescriptor *t)
    : SDAI_Select(s, t) {
    m_entityaggregate = new EntityAggregate;
    m_pAppInst = NULL;
    m_name = "";
}
TypedSelect::TypedSelect() : SDAI_Select() {}

void TypedSelect::STEPwrite_content(ostream &out, const char *currSch) const {
    out << m_name << "(";
    m_entityaggregate->STEPwrite(out);
    out << ")";

    return;
}

const TypeDescriptor *TypedSelect::AssignEntity(SDAI_Application_instance *se) {
    m_pAppInst = se;
    // return value actually not needed
    return se->eDesc->NonRefTypeDescriptor();
}

void TypedSelect::AddEntityAggregate(EntityAggregate_ptr o) {
    m_entityaggregate->ShallowCopy(*o);
}

void TypedSelect::STEPwrite_verbose(ostream &out, const char *currSch) const {
    const TypeDescriptor *td = CurrentUnderlyingType();
    std::string tmp;

    if (td) {
        // If we have a legal underlying type, get its name acc
        // to the current schema.
        StrToUpper(td->Name(""), tmp);
    }

    out << tmp << "(";
    m_entityaggregate->STEPwrite(out, currSch);
    out << ")";
    return;
}

EntitySelect::EntitySelect(const SelectTypeDescriptor *s,
                           const TypeDescriptor *t)
    : SDAI_Select(s, t) {
    m_pAppInst = NULL;
}
EntitySelect::EntitySelect() : SDAI_Select() {}

void EntitySelect::STEPwrite_content(ostream &out, const char *currSch) const {
    m_pAppInst->STEPwrite_reference(out);
    return;
}

const TypeDescriptor *EntitySelect::AssignEntity(
    SDAI_Application_instance *se) {
    m_pAppInst = se;
    // return value actually not needed
    return se->eDesc->NonRefTypeDescriptor();
}

void EntitySelect::STEPwrite_verbose(ostream &out, const char *currSch) const {
    const TypeDescriptor *td = CurrentUnderlyingType();
    m_pAppInst->STEPwrite_reference(out);
    return;
}
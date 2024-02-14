#pragma once
#include "Graph.h"

/**
 * @brief DerivedStepTypes
 * Extensions of some STEPCode data types
**/

// class derived from SDAI_Select used to create to append a entity to a select
// attribute
class TypedSelect : public SDAI_Select {
   public:
    TypedSelect(const SelectTypeDescriptor *s, const TypeDescriptor *t);
    TypedSelect();

   protected:
    SDAI_Application_instance *m_pAppInst;
    EntityAggregate_ptr m_entityaggregate;
    std::string m_name;

   public:
    void SetTypeName(std::string name) { m_name = name; }
    const TypeDescriptor *AssignEntity(SDAI_Application_instance *se);
    void AddEntityAggregate(EntityAggregate_ptr o);

    void STEPwrite_content(ostream &out = std::cout,
                           const char *currSch = 0) const;

    void STEPwrite_verbose(ostream &out = std::cout,
                           const char *currSch = 0) const;

    // dummy implementations of pure virtual funcs
    SDAI_Select *NewSelect() { return (SDAI_Select *)0; }
    BASE_TYPE ValueType() const { return sdaiBOOLEAN; }
    Severity StrToVal_content(const char * /*a*/, InstMgrBase * /*m*/) {
        return SEVERITY_NULL;
    }
    Severity STEPread_content(std::istream &i, InstMgrBase *m, const char *c,
                              int n, const char *d) {
        (void)i;
        (void)m;
        (void)c;
        (void)n;
        (void)d;
        return SEVERITY_NULL;
    }
};

// class derived from SDAI_Select used to create to append a entity to a select
// attribute
class EntitySelect : public SDAI_Select {
   public:
    EntitySelect(const SelectTypeDescriptor *s, const TypeDescriptor *t);
    EntitySelect();

   protected:
    SDAI_Application_instance *m_pAppInst;

   public:
    const TypeDescriptor *AssignEntity(SDAI_Application_instance *se);

    void STEPwrite_content(ostream &out = std::cout,
                           const char *currSch = 0) const;

    void STEPwrite_verbose(ostream &out = std::cout,
                           const char *currSch = 0) const;

    // dummy implementations of pure virtual funcs
    SDAI_Select *NewSelect() { return (SDAI_Select *)0; }
    BASE_TYPE ValueType() const { return sdaiBOOLEAN; }
    Severity StrToVal_content(const char * /*a*/, InstMgrBase * /*m*/) {
        return SEVERITY_NULL;
    }
    Severity STEPread_content(std::istream &i, InstMgrBase *m, const char *c,
                              int n, const char *d) {
        (void)i;
        (void)m;
        (void)c;
        (void)n;
        (void)d;
        return SEVERITY_NULL;
    }
};
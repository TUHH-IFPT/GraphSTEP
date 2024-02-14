#include "STEPAnalyser.h"

STEPAnalyser::STEPAnalyser(std::string path)
    : m_filePath(path), m_numEntities(0) {
    analyseFile();
}

STEPAnalyser::~STEPAnalyser() {}

void STEPAnalyser::analyseFile() {
    Registry registry(SchemaInit);
    STEPfile stepFile(registry, m_lstInst, "", false);
    stepFile.ReadExchangeFile(m_filePath);
    m_numEntities = m_lstInst.InstanceCount();
}
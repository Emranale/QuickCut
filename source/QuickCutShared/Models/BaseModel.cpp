
#include "QuickCutShared/QuickCutPCH.h"
#include "BaseModel.h"

BaseModel::BaseModel() noexcept
    : m_Uuid(std::move(QuickCut::createUuid()))
    , m_Name()
    , m_LastModified(std::move(QuickCut::getDateTime()))
{
}

BaseModel::BaseModel(const QString & id,
                     const QString & name,
                     const QString & lastModified) noexcept
    : m_Uuid(id)
    , m_Name(name)
    , m_LastModified(lastModified)
{
}

BaseModel::BaseModel(QString && id, QString && name, QString && lastModified) noexcept
    : m_Uuid(std::move(id))
    , m_Name(std::move(name))
    , m_LastModified(std::move(lastModified))
{
}

const QString & BaseModel::getId() const
{
    return m_Uuid;
}

const QString & BaseModel::getName() const
{
    return m_Name;
}

void BaseModel::setName(const QString & name)
{
    m_Name         = name;
    m_LastModified = QuickCut::getDateTime();
}

const QString & BaseModel::getLastModified() const
{
    return m_LastModified;
}

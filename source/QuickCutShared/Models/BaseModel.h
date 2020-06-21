
#pragma once

#include "pch.h"

class BaseModel
{
protected:
    BaseModel() noexcept;

    BaseModel(std::string && id, std::string && name, std::string && lastModified) noexcept;

    BaseModel(const std::string & id,
              const std::string & name,
              const std::string & lastModified) noexcept;

    virtual ~BaseModel() = default;

public:
    const std::string & getId() const;

    const std::string & getName() const;
    void                setName(const std::string & name);

    const std::string & getLastModified() const;

protected:
    std::string m_Uuid;
    std::string m_Name;
    std::string m_LastModified;
};
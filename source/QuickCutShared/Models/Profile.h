
#pragma once

#include "BaseModel.h"

#include "QuickCutShared/Managers/ActionManager.h"

class ActionManager;

class Profile : public BaseModel
{
public:
    // Constructs new profile
    Profile() noexcept;

    // Constructs existing profile
    Profile(const QString & id,
            const QString & name,
            const QString & lastModified,
            bool            active = false) noexcept;

    Profile(QString && id,
            QString && name,
            QString && lastModified,
            bool       active = false) noexcept;

    // Constructs copy
    Profile(const Profile & profile) = default;
    Profile(Profile && profile)      = default;

    ~Profile();

    bool isActive() const;
    void setActive(bool active);

    ActionManager &       getActionManager();
    const ActionManager & getActionManager() const;

    Profile & operator=(const Profile & profile) = default;
    Profile & operator=(Profile && profile) = default;

private:
    bool          m_Active;
    ActionManager m_Actions;
};

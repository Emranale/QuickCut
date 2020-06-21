
#pragma once

#include "Types.h"

namespace boost
{
    namespace property_tree
    {
        /*
         * Originally the boost::write_json function treats even numbers as strings, therefore
         * this function will parse the input and make the json file look as it should be.
         */
        void write_jsonEx(const std::string & path, const JSON & ptree);

        QString get(JSON & ptree, const QString & path, const QString & defaultValue);
        JSON &  put(JSON & ptree, const QString & path, const QString & value);
    } // namespace property_tree
} // namespace boost

namespace Hook
{
    /*
     * The keyboard hook will check if this key pattern matched, if it matched,
     * it will reload the profiles file so changes will take affect on the
     * system without having to restart the application nor the service.
     */
    void sendReloadSignal();
} // namespace Hook

namespace QuickCut
{
    QString getDateTime();
    QString createUuid();
} // namespace QuickCut

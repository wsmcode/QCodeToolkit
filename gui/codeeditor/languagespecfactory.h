#ifndef LANGUAGESPECFACTORY_H
#define LANGUAGESPECFACTORY_H

#include "languagespec.h"
#include <editor_config.h>

class LanguageSpecFactory
{
public:
    static LanguageSpec *createSpec(Language language);
    static QString languageName(Language language);
    static QStringList fileExtensions(Language language);
};

#endif // LANGUAGESPECFACTORY_H

#include "cpplanguagespec.h"
#include "javalanguagespec.h"
#include "javascriptlanguagespec.h"
#include "languagespecfactory.h"
#include "pythonlanguagespec.h"
#include "rustlanguagespec.h"

LanguageSpec *LanguageSpecFactory::createSpec(Language language)
{
    switch(language)
    {
    case Language::C:
    case Language::CPP:
        return new CppLanguageSpec;
    case Language::Python:
        return new PythonLanguageSpec;
    case Language::Java:
        return new JavaLanguageSpec;
    case Language::JavaScript:
        return new JavaScriptLanguageSpec;
    case Language::Rust:
        return new RustLanguageSpec;
    default:
        return new CppLanguageSpec; // 默认返回C++
    }
}

QString LanguageSpecFactory::languageName(Language language)
{
    auto spec = createSpec(language);
    QString name = spec->languageName();
    delete spec;
    return name;
}

QStringList LanguageSpecFactory::fileExtensions(Language language)
{
    auto spec = createSpec(language);
    QStringList extensions = spec->fileExtensions();
    delete spec;
    return extensions;
}

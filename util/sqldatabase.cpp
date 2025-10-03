#include "sqldatabase.h"

#include <QVariant>
#include <QDebug>
#include <QRegularExpression>

SQLDatabase::SQLDatabase(QObject *parent) : QObject(parent)
{
}

bool SQLDatabase::connectToDatabase()
{
    if(m_sqlDatabase.isOpen())
        return true;

    m_sqlDatabase = QSqlDatabase::addDatabase("QSQLITE");
    // 自命名
    m_sqlDatabase.setDatabaseName("sqlite.db");
    if(!m_sqlDatabase.open())
    {
        m_lastError = m_sqlDatabase.lastError().text();
        qCritical() << "Database connection failed:" << m_sqlDatabase.lastError().text();
        emit connectionFail();
        return false;
    }
    else
    {
        emit connectionSuccess();
        return true;
    }

}

bool SQLDatabase::createTable(QString tableName, QStringList fieldNameList)
{
    // 检查数据库连接
    if(!connectToDatabase())
    {
        m_lastError = "Database connection failed in createTable";
        qCritical() << m_lastError;
        return false;
    }

    // 构造字段定义子句
    QString fields = fieldNameList.join(",");
    // 组装完整SQL语句
    QString sql = QString("CREATE TABLE IF NOT EXISTS %1(%2);").arg(tableName).arg(fields);

    // 执行SQL语句
    QSqlQuery query(m_sqlDatabase);

    return query.exec(sql);
}

bool SQLDatabase::tableExists(const QString &tableName)
{
    if(!connectToDatabase())
    {
        m_lastError = "Database connection failed in tableExists";
        qCritical() << m_lastError;
        return false;
    }

    // SQLite 检查表是否存在
    QSqlQuery query(m_sqlDatabase);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:tableName");
    query.bindValue(":tableName", tableName);

    if(!query.exec())
    {
        m_lastError = query.lastError().text();
        qWarning() << "Table check error:" << query.lastError().text();
        return false;
    }

    return query.next();
}

int SQLDatabase::insertValues(const QString &tableName, QStringList fieldNameList, QStringList valuesList)
{
    // 检查数据库连接
    if(!connectToDatabase()) return -1;

    // 检查表是否存在
    if(!tableExists(tableName)) {
        m_lastError = "Table does not exist: " + tableName;
        qCritical() << m_lastError;
        return -1;
    }

    if(fieldNameList.isEmpty() || valuesList.isEmpty())
    {
        m_lastError = "InsertValues: Empty field or value list";
        qWarning() << m_lastError;
        return -1;
    }

    if(fieldNameList.size() != valuesList.size())
    {
        m_lastError = QString("Field/value count mismatch: %1 vs %2")
                      .arg(fieldNameList.size()).arg(valuesList.size());
        qWarning() << m_lastError;
        return -1;
    }

    // 使用参数化查询防止SQL注入
    QString fields = fieldNameList.join(",");
    QStringList placeholdersList;
    for(int i = 0; i < valuesList.size(); i++)
        placeholdersList << "?";
    QString placeholders = placeholdersList.join(",");
    QString sql = QString("INSERT INTO %1 (%2) VALUES(%3)").arg(tableName).arg(fields).arg(placeholders);

    QSqlQuery query(m_sqlDatabase);
    query.prepare(sql);
    for(const QString& value : valuesList)
    {
        query.addBindValue(value);
    }

    if(query.exec())
    {
        QSqlQuery query(m_sqlDatabase);
        if(query.exec("SELECT last_insert_rowid()") && query.next())
        {
            return query.value(0).toInt();
        }
    }

    m_lastError = query.lastError().text();
    return -1;
}

int SQLDatabase::insertValues(const QString &tableName, const QMap<QString, QVariant> &data)
{
    // 检查数据库连接
    if(!connectToDatabase()) return -1;

    // 检查表是否存在
    if(!tableExists(tableName))
    {
        m_lastError = "Table does not exist: " + tableName;
        qCritical() << m_lastError;
        return -1;
    }

    if(data.isEmpty())
    {
        m_lastError = "InsertValues: Empty data";
        qWarning() << m_lastError;
        return -1;
    }

    // 使用参数化查询防止SQL注入
    QStringList fieldNames = data.keys();
    QString fields = fieldNames.join(",");
    QStringList placeholdersList;
    for(int i = 0; i < fieldNames.size(); i++)
        placeholdersList << "?";
    QString placeholders = placeholdersList.join(",");
    QString sql = QString("INSERT INTO %1 (%2) VALUES(%3)").arg(tableName).arg(fields).arg(placeholders);

    QSqlQuery query(m_sqlDatabase);
    query.prepare(sql);
    for(const QString& key : fieldNames)
    {
        query.addBindValue(data.value(key));
    }

    if(query.exec())
    {
        QSqlQuery query(m_sqlDatabase);
        if(query.exec("SELECT last_insert_rowid()") && query.next())
        {
            return query.value(0).toInt();
        }
    }

    m_lastError = query.lastError().text();
    return -1;
}

bool SQLDatabase::updateValues(const QString &tableName, const QString &setClause, const QString &whereClause)
{
    if(!connectToDatabase()) return false;
    if(!tableExists(tableName))
    {
        m_lastError = "Table dose not exist: " + tableName;
        qCritical() << m_lastError;
        return false;
    }

    QString sql = QString("UPDATE %1 SET %2").arg(tableName).arg(setClause);
    if(!whereClause.isEmpty())
    {
        sql += " WHERE " + whereClause;
    }

    QSqlQuery query(m_sqlDatabase);
    bool result = query.exec(sql);
    if(!result) m_lastError = query.lastError().text();

    return result;
}

bool SQLDatabase::deleteValues(const QString &tableName, const QString &whereClause)
{
    if(!connectToDatabase()) return false;
    if(!tableExists(tableName))
    {
        m_lastError = "Table dose not exist: " + tableName;
        qCritical() << m_lastError;
        return false;
    }

    QString sql = QString("DELETE FROM %1").arg(tableName);
    if(!whereClause.isEmpty())
    {
        sql += " WHERE " + whereClause;
    }

    QSqlQuery query(m_sqlDatabase);
    bool result = query.exec(sql);
    if(!result) m_lastError = query.lastError().text();

    return result;
}

QVector<QVector<QVariant> > SQLDatabase::executeQuery(const QString &queryStr)
{
    QVector<QVector<QVariant>> results;

    if(!connectToDatabase()) return results;

    // 执行参数化查询
    QSqlQuery query(m_sqlDatabase);

    // 执行查询并处理结果
    if(!query.exec(queryStr))
    {
        m_lastError = query.lastError().text();
        qCritical() << "Query failed:" << m_lastError << "\nSQL:" << queryStr;
        return results;
    }

    // 获取结果集
    int columnCount = query.record().count();
    while(query.next())
    {
        QVector<QVariant> row;
        row.reserve(columnCount);
        for(int col = 0; col < columnCount; col++)
        {
            row.append(query.value(col));
        }
        results.append(row);
    }
    return results;

}

QVector<QVector<QVariant>> SQLDatabase::selectTable(const QString &tableName,
                                                    const QStringList &fieldNames,
                                                    const QString &filter)
{
    QVector<QVector<QVariant>> results;

    // 检查数据库连接
    if(!connectToDatabase())
    {
        m_lastError = "Database not connected for select operation";
        qCritical() << m_lastError;
        return results;
    }

    // 验证表是否存在
    if(!tableExists(tableName))
    {
        m_lastError = "Select failed: Table doesn't exist - " + tableName;
        qCritical() << m_lastError;
        return results;
    }

    // 构建安全的SQL查询
    QString sql = buildSelectQuery(tableName, fieldNames, filter);

    return executeQuery(sql);
}

bool SQLDatabase::beginTransaction()
{
    if(m_transactionCount > 0)
    {
        m_lastError = "Nested transactions are not supported";
        return false;
    }

    QSqlQuery query(m_sqlDatabase);
    bool result = query.exec("BEGIN TRANSACTION");

    if (result) m_transactionCount = 1;
    else m_lastError = query.lastError().text();

    return result;
}

bool SQLDatabase::commitTransaction()
{
    if(m_transactionCount != 1)
    {
        m_lastError = "No active transaction to commit";
        return false;
    }

    QSqlQuery query(m_sqlDatabase);
    bool result = query.exec("COMMIT");

    if (result) m_transactionCount = 0;
    else m_lastError = query.lastError().text();


    return result;
}

bool SQLDatabase::rollbackTransaction()
{
    if(m_transactionCount != 1)
    {
        m_lastError = "No active transaction to rollback";
        return false;
    }

    QSqlQuery query(m_sqlDatabase);
    bool result = query.exec("ROLLBACK");

    if (result) m_transactionCount = 0;
    else m_lastError = query.lastError().text();

    return result;
}

QString SQLDatabase::sanitizeIdentifier(const QString &identifier)
{
    QString result = identifier;
    return result.replace(QRegularExpression("[^a-zA-Z0-9_]"), "");
}

void SQLDatabase::closeDatabase()
{
    if(m_sqlDatabase.isOpen())
    {
        QString connectionName = m_sqlDatabase.connectionName();
        m_sqlDatabase.close();
        m_sqlDatabase = QSqlDatabase(); // 重置
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QString SQLDatabase::lastError() const
{
    return m_lastError;
}

QString SQLDatabase::buildSelectQuery(const QString &tableName,
                                      const QStringList &fieldNames,
                                      const QString &filter)
{
    // 安全过滤表名和字段名
    QString safeTableName = sanitizeIdentifier(tableName);

    QStringList safeFieldNames;
    for(const QString &field : fieldNames)
    {
        safeFieldNames << sanitizeIdentifier(field);
    }

    // 处理所有字段的情况
    QString fields = safeFieldNames.isEmpty() ? "*" : safeFieldNames.join(",");

    // 构建基础SQL
    QString sql = QString("SELECT %1 FROM %2").arg(fields).arg(safeTableName);

    // 添加过滤条件（如果有）
    if(!filter.isEmpty())
    {
        sql += " WHERE " + filter;
    }

    return sql;
}

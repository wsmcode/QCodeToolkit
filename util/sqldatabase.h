#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QMessageBox>

class SQLDatabase : public QObject
{
    Q_OBJECT
public:
    explicit SQLDatabase(QObject *parent = nullptr);

    bool connectToDatabase();
    // 建表，给出表名和字段定义列表
    bool createTable(QString tableName, QStringList fieldNameList);
    bool tableExists(const QString &tableName);
    // 插入数据(一次一行)返回插入的ID
    int insertValues(const QString &tableName, QStringList fieldNameList, QStringList valuesList);
    int insertValues(const QString &tableName, const QMap<QString, QVariant> &data);
    // 更新数据
    bool updateValues(const QString &tableName, const QString &setClause, const QString &whereClause);
    // 删除数据
    bool deleteValues(const QString &tableName, const QString &whereClause);

    // 自定义sql查询
    QVector<QVector<QVariant>> executeQuery(const QString &queryStr);

    QVector<QVector<QVariant>> selectTable(const QString &tableName, const QStringList &fieldNames,
                                            const QString &filter);

    // 事务支持
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // 标识符安全过滤
    QString sanitizeIdentifier(const QString &identifier);

    void closeDatabase();
    // 获取最后执行的错误信息
    QString lastError() const;

signals:
    void connectionFail();
    void connectionSuccess();

private:
    QString buildSelectQuery(const QString &tableName,
                             const QStringList &fieldNames,
                             const QString &filter);

    QSqlDatabase m_sqlDatabase;
    QString m_lastError;
    int m_transactionCount = 0;

};

#endif // SQLDATABASE_H

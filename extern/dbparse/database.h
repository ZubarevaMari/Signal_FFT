#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
//#include <QString>
//#include <QVariant>
//#include <QString>

//#include <QSqlTableModel>

#include "datasource.h"


struct dbdata_t
{
    QStringList keys;
    QList<QVariant> data;
    QVariant value(QString key)
    {
        if( keys.contains(key) )
            return data.at( keys.indexOf(key) );
        return QVariant();
    }

    size_t count(){ return keys.count();}
};


typedef QList<dbdata_t> settings_t;


// Reading all data from table of database
class DataBase
{
public:
    DataBase(QString path);
    ~DataBase();
    bool open();
    void close();
    settings_t readDataFromTable(const QString tableName);
    QSqlDatabase & getDataBase(){return db;}
private:
     QSqlDatabase db;
};


// Saving data from database
struct dbParams
{
    QMap<int, source_stg_t > source_stg;
    QMultiMap<int, setting_stg_t > setting_stg;
};

struct NetParams
{
    QMap<int, sourceNET_stg_t > source_stg;
    QMultiMap<int, param_stg_t > param_stg;
};


// Save settings and sources from database
class DataBaseSetting
{
public:
    DataBaseSetting(const QString &dbPath);
    ~DataBaseSetting();
    void load(dbParams &dbsettings, NetParams &dbnetparams);
private:
    QMultiMap<int, setting_stg_t> getSettings(QMap<int, source_stg_t > &source_stg);
    QMap<int, source_stg_t >  getSourceSettings(NetParams &dbnetparams);
private:
    DataBase db_;
};


// Save settings and sources from Net database
class DataBaseNetSetting
{
public:
    DataBaseNetSetting(const QString &dbPath);
    ~DataBaseNetSetting();
    void load(NetParams &page, int canal = 0);
private:
    QMultiMap<int, param_stg_t> getNetParamSettings(QMap<int, sourceNET_stg_t > &source_stg, int canal = 0);
    QMap<int, sourceNET_stg_t >  getNetSourceSettings(int canal = 0);
private:
    DataBase db_;
};

#endif // DATABASE_H

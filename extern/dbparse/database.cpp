#include "database.h"
#include <QDebug>

// Name of tables and colums in database
#define KEY_SETTING_TABLE           "settings"
#define KEY_SOURCES_TABLE           "sources"

#define KEY_ID_SETTINGS             "ID"
#define KEY_PAUSE_SET               "pause_for_setup"
#define KEY_COMMUTATOR_COMMAND      "commutator_command"
#define KEY_PAUSE_COMMUTATOR        "pause_for_commutator"
#define KEY_GENERATOR_COMMAND       "generator_command"
#define KEY_PAUSE_GENERATOR         "pause_for_generator"
#define KEY_ECE_DATA                "ece_data"
#define KEY_ECE_FREQUENCY           "ece_frequency"
#define KEY_ECE_PACK_COUNT          "ece_count_of_package"
#define KEY_ECE_DELAY               "ece_delay"
#define KEY_SOURCE_SETTINGS         "source_of_signal"
#define KEY_PAUSE_SOURCE            "pause_for_source"
#define KEY_SOURCE_VALID_DIFF       "valid_difference"
#define KEY_EXPECTED_VAL_CHANNEL    "expected_value"
#define KEY_MULTIMETER              "need_multimeter"
#define KEY_TYPE_PROCESSING         "type_of_processing"
#define KEY_MAX                     "max_limit"
#define KEY_MIN                     "min_limit"
#define KEY_PAUSE_MULTIMETER        "pause_for_multimeter"
#define KEY_COMMENT                 "comment"
#define KEY_USER_MESSAGE            "user_message"
#define KEY_NEED_USER               "need_user"
#define KEY_IS_USED                 "is_used"

#define KEY_ID_SOURCE               "ID"
#define KEY_SOURCE                  "source"
#define KEY_NAME_SOURCE             "Name"
#define KEY_CANAL_NUM               "canal_number"
#define KEY_COMPARE_VAL             "compare_value"
#define KEY_NOT_GENERATE            "not_generate"
#define KEY_CLIPPED_SIGNAL          "clipped_signal"


#define KEY_PARAMS_TABLE        "params"

#define KEY_ID_PARAMS           "Индекс параметра"
#define KEY_GROUPE              "Группа"
#define KEY_ACCUMULATION_DATA   "Включить накопление данных"
#define KEY_STATISTIC           "Включить статистику"
#define KEY_SCROLL              "Включить скролл"
#define KEY_WSIZE               "Размер окна"
#define KEY_DISABLE             "Выключить отображение"
#define KEY_GRAPH_HIDE          "Скрыть график"
#define KEY_FORMAT              "Формат"
#define KEY_ALIGN               "Выравнивание"

#define KEY_ID                  "ID"
#define KEY_NET_SOURCE          "Источник"
#define KEY_NAME                "Наименование"
#define KEY_START_ADDRESS       "Стартовый адрес"
#define KEY_INDEX               "Индекс"
#define KEY_STEP                "Шаг"
#define KEY_SIZE                "Размер"
#define KEY_COUNT               "Количество"
#define KEY_FREQ               "Частота"
#define KEY_ORDER               "Порядок байт"
#define KEY_START_BIT           "Первый бит"
#define KEY_END_BIT             "Последний бит"
#define KEY_SIGN                "Знак"
#define KEY_CMP                 "ЦМР"
#define KEY_CALIBRATION         "Тарировки"

#define KEY_IP                  "Ip адрес"
#define KEY_SRC_PORT            "Src порт"
#define KEY_DST_PORT            "Dst порт"



DataBase::DataBase(QString path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
}

DataBase::~DataBase()
{
}

bool DataBase::open()
{
    return db.open();
}

void DataBase::close()
{
    db.close();
}


settings_t DataBase::readDataFromTable(const QString tableName)
{
    settings_t  tables;

    if( db.isOpen() )
    {
        QSqlQuery query;

        QString str = "SELECT * FROM " + tableName;

        if( !query.exec(str) )
            return tables;

        QSqlRecord rec = query.record();
        QStringList keys;

        for( int i = 0; i < rec.count(); ++i )
            keys << rec.fieldName(i);

        while ( query.next() )
        {
            dbdata_t tableData;
            tableData.keys = keys;

            for( QString &key : keys )
                tableData.data.append( query.value( rec.indexOf( key ) ) );
            tables << tableData;
        }
    }
    return tables;
}


DataBaseSetting::DataBaseSetting(const QString &dbPath):
    db_(dbPath)
{
    if(!db_.open())
    {
        qDebug()<<Q_FUNC_INFO;
        throw std::runtime_error("Не удалось открыть базу данных");
    }
}


DataBaseSetting::~DataBaseSetting()
{
    db_.close();
}


void DataBaseSetting::load(dbParams &dbsettings, NetParams &dbnetparams)
{
    dbsettings.source_stg = getSourceSettings(dbnetparams);
    dbsettings.setting_stg = getSettings(dbsettings.source_stg );
}


// load control settings from database (table settings)
QMultiMap<int, setting_stg_t> DataBaseSetting::getSettings(QMap<int, source_stg_t > &source_stg)
{
    QMultiMap <int, setting_stg_t > setting_stg;


    settings_t settings = db_.readDataFromTable(KEY_SETTING_TABLE);

    for(auto st:settings)
    {
        setting_stg_t stg;
        int sourid = st.value(KEY_SOURCE_SETTINGS).toInt();
        stg.source = source_stg.contains(sourid) ? source_stg[sourid] : source_stg_t();
        stg.setup_pause = st.value(KEY_PAUSE_SET).toInt();
        stg.commut_cmd = st.value(KEY_COMMUTATOR_COMMAND).toString();
        stg.commut_pause = st.value(KEY_PAUSE_COMMUTATOR).toInt();
        stg.gener_cmd = st.value(KEY_GENERATOR_COMMAND).toString();
        stg.gener_pause = st.value(KEY_PAUSE_GENERATOR).toInt();
        stg.source_pause = st.value(KEY_PAUSE_SOURCE).toInt();
        stg.source_valid_diff = st.value(KEY_SOURCE_VALID_DIFF).toString().replace(",", ".").toDouble();
//        qDebug() << st.value(KEY_EXPECTED_VAL_CHANNEL).toString() << st.value(KEY_EXPECTED_VAL_CHANNEL).toString().toUInt(nullptr, 16);
        QString exp_val = st.value(KEY_EXPECTED_VAL_CHANNEL).toString();
        if(exp_val.isEmpty()) stg.expected_val_channel = LONG_MIN;
        else stg.expected_val_channel = exp_val.toUInt(nullptr, 16);
//        stg.expected_val_channel = st.value(KEY_EXPECTED_VAL_CHANNEL).toString().toUInt(nullptr, 16);
        stg.multim_measure = st.value(KEY_MULTIMETER).toInt();
        stg.multim_pause = st.value(KEY_PAUSE_MULTIMETER).toInt();
        stg.type_processing = st.value(KEY_TYPE_PROCESSING).toString();
        stg.max_limit = st.value(KEY_MAX).toString().replace(",", ".").toDouble();
        stg.min_limit = st.value(KEY_MIN).toString().replace(",", ".").toDouble();
        stg.comment = st.value(KEY_COMMENT).toString();
        stg.user_mess = st.value(KEY_USER_MESSAGE).toString();
        stg.user_need = st.value(KEY_NEED_USER).toBool();
        stg.is_used = st.value(KEY_IS_USED).toBool();

        stg.ece.delay = st.value(KEY_ECE_DELAY).toInt();
        stg.ece.frequency = st.value(KEY_ECE_FREQUENCY).toString().replace(",", ".").toDouble();
        stg.ece.pack_count = st.value(KEY_ECE_PACK_COUNT).toInt();
        stg.ece.data_count = 0;
        QStringList data_ece_input = st.value(KEY_ECE_DATA).toString().split(" ", Qt::SkipEmptyParts);
        for(int i = 0; i < data_ece_input.size() && i < 256; i++)
        {
            stg.ece.data[i] = data_ece_input[i].toUInt(nullptr, 16);
            stg.ece.data_count += 1;
        }


        auto id = st.value(KEY_ID_SETTINGS).toInt();
        setting_stg.insert(id, stg);
    }

    return setting_stg;
}

// load source settings from database (table sources)
QMap<int, source_stg_t >  DataBaseSetting::getSourceSettings(NetParams &dbnetparams)
{
    QMap<int, source_stg_t > sources_stg;

//    static std::map<uint8_t,size_t> packetsSize =
//    {
//        {0, 0},
//        {1,1300},
//        {2,1300},
//        {3,1292},
//        {4,1292},
//        {5,420},
//        {12,1460}
//    };

    settings_t sources = db_.readDataFromTable(KEY_SOURCES_TABLE);
    for( auto src : sources )
    {
        source_stg_t stg;

        int id = src.value(KEY_ID_SOURCE).toInt();
        stg.source = src.value(KEY_SOURCE).toInt();
        stg.name = src.value(KEY_NAME_SOURCE).toString();
        stg.canal_number = src.value(KEY_CANAL_NUM).toInt();
        stg.compare_value = src.value(KEY_COMPARE_VAL).toBool();
        stg.not_generate = src.value(KEY_NOT_GENERATE).toBool();
        stg.is_clipped_signal = src.value(KEY_CLIPPED_SIGNAL).toBool();

        stg.size = dbnetparams.param_stg.value(id).count;

        //if(stg.size == 0)stg.size = packetsSize[stg.ip.toIPv4Address()&0xff];

        sources_stg[id] = stg;
    }

    return sources_stg;
}

DataBaseNetSetting::DataBaseNetSetting(const QString &dbPath):
    db_(dbPath)
{
    if(!db_.open())
    {
        qDebug()<<Q_FUNC_INFO;
        throw std::runtime_error("Не удалось открыть базу данных");
    }
}

DataBaseNetSetting::~DataBaseNetSetting()
{
    db_.close();
}

void DataBaseNetSetting::load(NetParams &page, int canal)
{
    page.source_stg = getNetSourceSettings(canal);
    page.param_stg = getNetParamSettings( page.source_stg, canal );
}

QMultiMap<int, param_stg_t> DataBaseNetSetting::getNetParamSettings( QMap<int, sourceNET_stg_t > &source_stg, int canal)
{
    QMultiMap <int, param_stg_t > param_stg;

        settings_t params = db_.readDataFromTable(KEY_PARAMS_TABLE);

        for(auto pm:params)
        {
            param_stg_t stg;
            int sid = pm.value(KEY_NET_SOURCE).toInt();
            if ((canal != 0) && (canal != sid)) continue;

            stg.name = pm.value(KEY_NAME).toString();
            stg.source = source_stg.contains(sid) ? source_stg[sid] : sourceNET_stg_t();
            stg.address = pm.value(KEY_START_ADDRESS).toInt();
            stg.index = pm.value(KEY_INDEX).toInt();
            stg.step = pm.value(KEY_STEP).toInt();
            stg.size = pm.value(KEY_SIZE).toInt();
            stg.count = pm.value(KEY_COUNT).toInt();
            stg.freq = pm.value(KEY_FREQ).toInt();
            stg.order = pm.value(KEY_ORDER).toBool();
            stg.start_bit = pm.value(KEY_START_BIT).toInt();
            stg.end_bit = pm.value(KEY_END_BIT).toInt();
            stg.sign = pm.value(KEY_SIGN).toBool();
            stg.cmp = pm.value(KEY_CMP).toDouble();
            stg.calib = pm.value(KEY_CALIBRATION).toDouble();

            auto id = pm.value(KEY_ID).toInt();

            if (canal == 0) param_stg.insert(id, stg);
            if (canal == sid)
            {
                param_stg.insert(stg.name.toInt(), stg);
            }

           // param_stg.insert(id, stg);
        }

    return param_stg;
}


QMap<int, sourceNET_stg_t >  DataBaseNetSetting::getNetSourceSettings(int canal)
{
    QMap<int, sourceNET_stg_t > sources_stg;


    static std::map<uint8_t,size_t> packetsSize =
    {
        {1,1300},
        {2,1300},
        {3,1292},
        {4,1292},
        {5, 1261 /*420*/},
        {12,1460}
    };

    settings_t sources = db_.readDataFromTable(KEY_SOURCES_TABLE);
    for( auto src : sources )
    {
        sourceNET_stg_t stg;

        int id = src.value(KEY_ID).toInt();
        if(canal != id && canal != 0) continue;

        stg.name = src.value(KEY_NAME).toString();
        stg.ip = QHostAddress(src.value(KEY_IP).toString());
        stg.src_port = src.value(KEY_SRC_PORT).toInt();
        stg.dst_port = src.value(KEY_DST_PORT).toInt();
        stg.size = src.value(KEY_SIZE).toInt();

        if(stg.size == 0)stg.size = packetsSize[stg.ip.toIPv4Address()&0xff];

        if (canal == 0) sources_stg[id] = stg;
        if(canal == id)
        {
            sources_stg[id] = stg;
            break;
        }


        //sources_stg[id] = stg;
    }

    return sources_stg;
}

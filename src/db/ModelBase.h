#ifndef __MODEL_BASE_H__
#define __MODEL_BASE_H__

#include <vector>
#include <algorithm>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/prepared_statement.h>

#include "Field.h"
#include "ModelMacro.h"
#include "../Logger.h"

using namespace std;
// using namespace sql;


class ModelObject
{
public:
    ModelObject(bool exist=false);

    template<class ValType>
    void SetFieldByIndex(int index, ValType val)
    {
        field_list_[index-1]->Set(&val);
    }

    Field * GetFieldByIndex(int index);
    void SetExist();
    bool Exist();
public:
    Field ** field_list_;

protected:
    bool exist_;
};



// binss:
// "The only portable way of using templates at the moment is to implement them in header files by using inline functions"
// If implement them in implement files, you have to explicit instantiation of all the types the template which will be used with
// However, the type to be explicit is defined by user and would be created by macro in the upper level file
// So I choose to implement them in here finally
//
template<typename ModelObjectName>
class Model
{
public:
    Model(const char *name, int field_num):name_(name), field_num_(field_num), logger_("Model", DEBUG, true)
    {
        driver_ = NULL;
        con_ = NULL;
        stmt_ = NULL;
        pstmt_ = NULL;
        res_ = NULL;
        update_index_ = NULL;

        // 初始化model数据
        primary_key_num_ = 0;
        ModelObjectName object;
        for(int i=1; i<=field_num_; i++)
        {
            if(object.GetFieldByIndex(i)->IsPrimaryKey())
            {
                primary_key_num_++;
            }
        }
    }
    ~Model()
    {
        delete update_index_;
        delete res_;
        delete stmt_;
        if(con_)
        {
            con_->close();
            delete con_;
        }
    }
    int Init()
    {
        // 连接db
        try
        {
            driver_ = get_driver_instance();
            con_ = driver_->connect(DB_HOST, DB_USER, DB_PASSWORD);
            con_->setSchema(DB_NAME);
        }
        catch(sql::SQLException &e)
        {
            logger_<<CRITICAL<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
            return -100;
        }

        // 初始化各种语句
        ModelObjectName object;

        // insert语句
        insert_stmt_ = "INSERT INTO " + name_ + "(";
        string arg_str = "";
        for(int i=1; i<field_num_; i++)
        {
            insert_stmt_ += object.GetFieldByIndex(i)->GetName() + ",";
            arg_str += "?,";
        }
        insert_stmt_ += object.GetFieldByIndex(field_num_)->GetName();
        arg_str += "?";
        insert_stmt_  += ") VALUES (" + arg_str + ")";

        // update语句
        update_index_ = new int[field_num_];
        update_stmt_ = "UPDATE " + name_ + " SET ";
        string where_str = " WHERE ";
        int where_count = 0;
        for(int i=1; i<=field_num_; i++)
        {
            Field * field = object.GetFieldByIndex(i);
            if(field->IsPrimaryKey())
            {
                where_str += field->GetName() + "=? and ";
                update_index_[field_num_ - primary_key_num_ + where_count] = i;
                where_count ++;
            }
            else
            {
                update_stmt_ += field->GetName() + "=?,";
                update_index_[i - 1 - where_count] = i;
            }
        }
        where_str = where_str.substr(0, where_str.size() - 4);
        update_stmt_ = update_stmt_.substr(0, update_stmt_.size() - 1) + where_str;

        // delete语句
        delete_stmt_ = "DELETE FROM " + name_ + " WHERE ";
        for(int i=1; i<=field_num_; i++)
        {
            Field * field = object.GetFieldByIndex(i);
            if(field->IsPrimaryKey())
            {
                delete_stmt_ += field->GetName() + "=? and ";
            }
        }
        delete_stmt_ = delete_stmt_.substr(0, delete_stmt_.size()-4);

        logger_<<DEBUG<<delete_stmt_<<endl;
        logger_<<DEBUG<<insert_stmt_<<endl;
        logger_<<DEBUG<<update_stmt_<<endl;
        // 校验
        int ret = Varify();
        return ret;
    }

    int Varify()
    {
    // 校验列数
        try
        {
            stmt_ = con_->createStatement();
            res_ = stmt_->executeQuery("select * from " + name_ + " limit 0");
            sql::ResultSetMetaData *res_meta = res_->getMetaData();
            int db_field_num = res_meta->getColumnCount();
            if(field_num_ != db_field_num)
            {
                logger_<<ERROR<<"Field num varify failed! model num["<<field_num_<<"] database num["<<db_field_num<<"]"<<endl;
                return -1;
            }

            // 校验主键数
            sql::DatabaseMetaData *db_meta = con_->getMetaData();
            sql::ResultSet * table_key_set = db_meta->getPrimaryKeys(con_->getCatalog(), con_->getSchema(), name_);
            int db_primary_key_num = table_key_set->rowsCount();
            if(primary_key_num_ != db_primary_key_num)
            {
                logger_<<ERROR<<"Primary key num varify failed! model["<<primary_key_num_<<"] != database["<<db_primary_key_num<<"]"<<endl;
                return -2;
            }
            vector<string> primary_keys;
            if(db_primary_key_num > 0)
            {
                while(table_key_set->next())
                {
                    primary_keys.push_back(table_key_set->getString(4));
                }
            }

            // 校验字段
            ModelObjectName object;
            for(int i=1; i<=db_field_num; i++)
            {
                Field * field = object.GetFieldByIndex(i);
                const string & field_name = field->GetName();
                if(field_name != res_meta->getColumnLabel(i))
                {
                    logger_<<ERROR<<"field name error, model["<<field_name<<"] database["<<res_meta->getColumnLabel(i)<<"]"<<endl;
                    return -3;
                }
                string type = res_meta->getColumnTypeName(i);
                bool match = false;
                switch(field->GetType())
                {
                    case INT:
                    {
                        match = (type == "INT"); break;
                    }
                    case STRING:
                    {
                        match = (type == "TEXT" || type == "VARCHAR" || type == "CHAR"); break;
                    }
                    default:
                    {
                        logger_<<ERROR<<"unknown type! skip"<<endl;
                    }
                }
                if(!match)
                {
                    logger_<<ERROR<<"field type error, model["<<field->GetType()<<"] database["<<res_meta->getColumnTypeName(i)<<"]"<<endl;
                    return -4;
                }
                // 校验自增
                if(field->IsAutoIncrement() && !res_meta->isAutoIncrement(i))
                {
                    logger_<<ERROR<<"auto increment match error, model define field["<<i<<"] is auto increment field"<<endl;
                    return -5;
                }
                if(res_meta->isAutoIncrement(i) && !field->IsAutoIncrement())
                {
                    logger_<<ERROR<<"auto increment match error, database define field["<<i<<"] is auto increment field"<<endl;
                    return -5;
                }

                // 校验主键
                if(field->IsPrimaryKey())
                {
                    if(find(primary_keys.begin(), primary_keys.end(), field_name) == primary_keys.end())
                    {
                        logger_<<ERROR<<"primary key match error, cannot find primary key["<<field_name<<"]"<<endl;
                        return -6;
                    }
                }
            }
            return 0;
        }
        catch(sql::SQLException &e)
        {
            logger_<<CRITICAL<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
            return -100;
        }
    }

    vector<ModelObjectName> All()
    {
        vector<ModelObjectName> objects;
        string cmd = "SELECT * FROM " + name_;
        try
        {
            stmt_ = con_->createStatement();
            res_ = stmt_->executeQuery(cmd);

            while (res_->next())
            {
                ModelObjectName object;
                object.SetExist();
                for(int i=1; i <= field_num_; i++)
                {
                    switch(object.GetFieldByIndex(i)->GetType())
                    {
                        case INT:
                        {
                            object.SetFieldByIndex(i, res_->getInt(i)); break;
                        }
                        case STRING:
                        {
                            object.SetFieldByIndex(i, res_->getString(i)); break;
                        }
                        default:
                        {
                            logger_<<ERROR<<"unknown type! skip"<<endl;
                            break;
                        }
                    }
                }
                objects.push_back(object);
            }
        }
        catch(sql::SQLException &e)
        {
            logger_<<CRITICAL<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
        }
        return objects;
    }

    int Insert(ModelObjectName & object)
    {
        try
        {
            pstmt_ = con_->prepareStatement(insert_stmt_);
            for (int i = 1; i <= field_num_; i++)
            {
                Field * field = object.GetFieldByIndex(i);
                if(field->IsAutoIncrement())
                {
                    if(!field->IsModified())
                    {
                        pstmt_->setNull(i, sql::DataType::INTEGER);
                        continue;
                    }
                }
                switch(field->GetType())
                {
                    case INT:
                    {
                        pstmt_->setInt(i, *dynamic_cast<IntField *>(field)); break;
                    }
                    case STRING:
                    {
                        pstmt_->setString(i, sql::SQLString(*dynamic_cast<StringField *>(field))); break;
                    }
                    default:
                    {
                        logger_<<ERROR<<"unknown type! break"<<endl;
                        return -1;
                    }
                }
            }
            pstmt_->executeUpdate();
            return 0;
        }
        catch(sql::SQLException &e)
        {
            logger_<<CRITICAL<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
            return -100;
        }

    }

    int Update(ModelObjectName & object)
    {
        try
        {
            pstmt_ = con_->prepareStatement(update_stmt_);
            for (int i = 1; i <= field_num_; i++)
            {
                Field * field = object.GetFieldByIndex(update_index_[i-1]);

                if(field->IsPrimaryKey() && field->IsModified())
                {
                    logger_<<ERROR<<"Don't support to update primary key!"<<endl;
                    return -1;
                }
                switch(field->GetType())
                {
                    case INT:
                    {
                        pstmt_->setInt(i, *dynamic_cast<IntField *>(field)); break;
                    }
                    case STRING:
                    {
                        pstmt_->setString(i, sql::SQLString(*dynamic_cast<StringField *>(field))); break;
                    }
                    default:
                    {
                        logger_<<ERROR<<"unknown type! skip"<<endl;
                        break;
                    }
                }
            }
            pstmt_->executeUpdate();
            return 0;
        }
        catch(sql::SQLException &e)
        {
            logger_<<CRITICAL<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
            return -100;
        }
    }

    int Delete(ModelObjectName & object)
    {
        try
        {
            pstmt_ = con_->prepareStatement(delete_stmt_);
            for (int i = 1, j = 1; i <= field_num_; i++)
            {
                Field * field = object.GetFieldByIndex(i);
                if(field->IsPrimaryKey())
                {
                    switch(field->GetType())
                    {
                        case INT:
                        {
                            pstmt_->setInt(j, *dynamic_cast<IntField *>(field)); break;
                        }
                        case STRING:
                        {
                            pstmt_->setString(j, sql::SQLString(*dynamic_cast<StringField *>(field))); break;
                        }
                        default:
                        {
                            logger_<<ERROR<<"unknown type! skip"<<endl;
                            break;
                        }
                    }
                    j++;
                }
            }
            pstmt_->executeUpdate();
            return 0;
        }
        catch(sql::SQLException &e)
        {
            logger_<<CRITICAL<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
            return -100;
        }
    }

    int Save(ModelObjectName & object)
    {
        if(object.Exist())
        {
            return Update(object);
        }
        else
        {
            return Insert(object);
        }
        return 0;
    }
    int Save(ModelObjectName * objects, int length)
    {
        for(int i=0; i<length; i++)
        {
            Save(objects[i]);
        }
        return 0;
    }
    int Save(vector<ModelObjectName> &objects)
    {
        for(unsigned int i=0; i<objects.size(); i++)
        {
            Save(objects[i]);
        }
        return 0;
    }

private:
    sql::Driver *driver_;
    sql::Connection *con_;
    sql::Statement *stmt_;
    sql::PreparedStatement *pstmt_;
    sql::ResultSet *res_;
    string insert_stmt_;
    string update_stmt_;
    string delete_stmt_;
    int * update_index_;

    string name_;
    int field_num_;
    int primary_key_num_;
    Logger logger_;
};

#endif
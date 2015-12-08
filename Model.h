/***********************************************************
* FileName:      Model.h
* Author:        binss
* Create:        2015-11-27 17:25:56
* Description:   No Description
***********************************************************/

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/prepared_statement.h>


using namespace std;
// using namespace sql;

#define DB_HOST "tcp://172.17.0.3:3306"
#define DB_USER "binss"
#define DB_PASSWORD "123456"
#define DB_NAME "dudu"

enum FieldType
{
    INT = 1,
    STRING = 2,
};

class Field
{
public:
    Field(int index, FieldType type, string name, bool is_primary_key, bool is_auto_increment): index_(index),
                    type_(type), name_(name), is_primary_key_(is_primary_key), is_auto_increment_(is_auto_increment)
    {

    }
    Field(const Field& obj): index_(obj.index_), type_(obj.type_), name_(obj.name_), is_primary_key_(obj.is_primary_key_),
                    is_auto_increment_(obj.is_auto_increment_)
    {
    }
    FieldType & GetType()
    {
        return type_;
    }
    string & GetName()
    {
        return name_;
    }
    bool IsPrimaryKey()
    {
        return is_primary_key_;
    }
    bool IsAutoIncrement()
    {
        return is_auto_increment_;
    }
    virtual void Set(void * data_ptr) = 0;
    virtual void Copy(void * obj_ptr) = 0;
protected:
    int index_;
    FieldType type_;
    string name_;
    bool is_primary_key_;
    bool is_auto_increment_;
};

class IntField: public Field
{
public:
    IntField(int index, string name, bool is_primary_key=false, bool is_auto_increment=false):
                    Field(index, INT, name, is_primary_key, is_auto_increment)
    {
        value_ = 0;
    }
    IntField(const IntField& obj): Field(obj)
    {
        value_ = obj.value();
    }
    const IntField & operator=(const int & value)
    {
        value_ = value;
        return *this;
    }
    operator int()
    {
        return value_;
    }
    int value() const
    {
        return value_;
    }
    void Set(void * data_ptr)
    {
        value_ = *(int *)data_ptr;
    }
    void Copy(void * obj_ptr)
    {
        value_ = *(IntField *)obj_ptr;
    }

protected:
    int value_;
};

class AutoField: public IntField
{
public:
    AutoField(int index, string name, bool is_primary_key=false) :
                    IntField(index, name, is_primary_key, true)
    {
        is_default_ = true;
    }
    // 运算符重载无法继承，需要重新定义
    const AutoField & operator=(const int & value)
    {
        cout<<"warning: you have set the value of auto increment field!"<<endl;
        is_default_ = false;
        value_ = value;
        return *this;
    }
    operator int()
    {
        return value_;
    }
    bool IsDefault()
    {
        return is_default_;
    }

private:
    bool is_default_;
};

class StringField: public Field
{
public:
    StringField(int index, string name, bool is_primary_key=false): Field(index, STRING, name, is_primary_key, false), value_("")
    {
    }
    StringField(const StringField & obj): Field(obj)
    {
        value_ = obj.value();
    }
    const StringField & operator=(const string & value)
    {
        value_ = value;
        return *this;
    }
    // operator string()
    // {
    //     return value_;
    // }
    operator const char *()
    {
        return value_.c_str();
    }
    string value() const
    {
        return value_;
    }

    void Set(void * data_ptr)
    {
        value_ = string(*(const char **)data_ptr);
    }
    void Copy(void * obj_ptr)
    {
        value_ = *(StringField *)obj_ptr;
    }

private:
    string value_;
};

class ModelObject
{
public:
    ModelObject(bool exist=false): exist_(exist)
    {

    }
    template<class ValType>
    void SetFieldByIndex(int index, ValType val)
    {
        field_list_[index-1]->Set(&val);
    }
    Field * GetFieldByIndex(int index)
    {
        return field_list_[index-1];
    }
    void SetExist()
    {
        exist_ = true;
    }
    bool Exist()
    {
        return exist_;
    }
public:
    Field ** field_list_;

protected:
    bool exist_;
};


template<typename ModelObjectName>
class Model
{
public:
    Model(const char *name, int field_num):name_(name), field_num_(field_num)
    {
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
    }
    int Init()
    {
        // 连接db
        driver = get_driver_instance();
        con = driver->connect(DB_HOST, DB_USER, DB_PASSWORD);
        con->setSchema(DB_NAME);

        // 初始化各种语句
        ModelObjectName object;
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
        cout<<insert_stmt_<<endl;
        // insert_stmt_ = "INSERT INTO User(Id,Name,Price) VALUES (?,?,?)";

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
        cout<<update_stmt_<<endl;

        // update_stmt_ = "UPDATE Cars SET Name=?,Price=? where Id=?";
        // 校验
        int ret = Varify();
        return ret;
    }

    int Varify()
    {
        // 校验列数
        stmt = con->createStatement();
        sql::ResultSet *res = stmt->executeQuery("select * from " + name_ + " limit 0");
        sql::ResultSetMetaData *res_meta = res->getMetaData();
        int db_field_num = res_meta->getColumnCount();
        if(field_num_ != db_field_num)
        {
            cout<<"Field num varify failed! model num["<<field_num_<<"] database num["<<db_field_num<<"]"<<endl;
            return -1;
        }

        // 校验主键数
        sql::DatabaseMetaData *db_meta = con->getMetaData();
        sql::ResultSet * table_key_set = db_meta->getPrimaryKeys(con->getCatalog(), con->getSchema(), name_);
        int db_primary_key_num = table_key_set->rowsCount();
        if(primary_key_num_ != db_primary_key_num)
        {
            cout<<"Primary key num varify failed! model["<<primary_key_num_<<"] != database["<<db_primary_key_num<<"]"<<endl;
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
            string & field_name = field->GetName();
            if(field_name != res_meta->getColumnLabel(i))
            {
                cout<<"field name error, model["<<field_name<<"] database["<<res_meta->getColumnLabel(i)<<"]"<<endl;
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
                    cout<<"unknown type! skip"<<endl;
                }
            }
            if(!match)
            {
                cout<<"field type error, model["<<field->GetType()<<"] database["<<res_meta->getColumnTypeName(i)<<"]"<<endl;
                return -4;
            }

            // 校验主键
            if(field->IsPrimaryKey())
            {
                if(find(primary_keys.begin(), primary_keys.end(), field_name) == primary_keys.end())
                {
                    cout<<"primary key match error, cannot find primary key["<<field_name<<"]"<<endl;
                    return -5;
                }
            }
        }
        return 0;
    }

    vector<ModelObjectName> All()
    {
        string cmd = "SELECT * FROM " + name_;
        try
        {
            stmt = con->createStatement();
            sql::ResultSet *res = stmt->executeQuery(cmd);
            sql::ResultSetMetaData *res_meta = res->getMetaData();

            vector<ModelObjectName> objects;
            while (res->next())
            {
                ModelObjectName object;
                object.SetExist();
                for(int i=1; i <= field_num_; i++)
                {
                    switch(object.GetFieldByIndex(i)->GetType())
                    {
                        case INT:
                        {
                            object.SetFieldByIndex(i, res->getInt(i)); break;
                        }
                        case STRING:
                        {
                            object.SetFieldByIndex(i, res->getString(i)); break;
                        }
                        default:
                        {
                            cout<<"unknown type! skip"<<endl;
                            break;
                        }
                    }
                }
                objects.push_back(object);
            }
            return objects;
        }
        catch (sql::SQLException &e)
        {
            cout<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
        }
    }

    int Insert(ModelObjectName & object)
    {
        sql::PreparedStatement *pstmt = con->prepareStatement(insert_stmt_);
        for (int i = 1; i <= field_num_; i++)
        {
            Field * field = object.GetFieldByIndex(i);
            if(field->IsAutoIncrement())
            {
                if(((AutoField *)field)->IsDefault())
                {
                    pstmt->setNull(i, sql::DataType::INTEGER);
                    continue;
                }
            }
            switch(field->GetType())
            {
                case INT:
                {
                    pstmt->setInt(i, *(IntField *)field); break;
                }
                case STRING:
                {
                    pstmt->setString(i, sql::SQLString(*(StringField *)field)); break;
                }
                default:
                {
                    cout<<"unknown type! skip"<<endl;
                    break;
                }
            }
        }
        pstmt->executeUpdate();
        return 0;
    }

    int Update(ModelObjectName & object)
    {
        cout<<update_stmt_<<endl;
        sql::PreparedStatement *pstmt = con->prepareStatement(update_stmt_);
        for (int i = 1; i <= field_num_; i++)
        {
            Field * field = object.GetFieldByIndex(update_index_[i-1]);
            // cout<<update_index_[i-1]<<endl;
            switch(field->GetType())
            {
                case INT:
                {
                    pstmt->setInt(i, *(IntField *)field); break;
                }
                case STRING:
                {
                    pstmt->setString(i, sql::SQLString(*(StringField *)field)); break;
                }
                default:
                {
                    cout<<"unknown type! skip"<<endl;
                    break;
                }
            }
        }
        pstmt->executeUpdate();
        return 0;
    }

    int Save(ModelObjectName & object)
    {
        if(object.Exist())
        {
            Update(object);
        }
        else
        {
            Insert(object);
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
        for(int i=0; i<objects.size(); i++)
        {
            save(objects[i]);
        }
        return 0;
    }

private:
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;

    string insert_stmt_;
    string update_stmt_;
    int * update_index_;
private:
    string name_;
    int field_num_;
    int primary_key_num_;
};

#define FOR_1()   set_1();
#define FOR_2()   FOR_1() set_2();
#define FOR_3()   FOR_2() set_3();
#define FOR_N(n)  FOR_##n()

#define FIELD(index, name, type, is_primary_key) \
    type name{index, #name, is_primary_key}; \
    void set_##index() \
    { \
        field_list_[index-1] = &name; \
    }

#define _DEFINE_MODEL_TAIL(FIELDS) \
    FIELDS \
};



#define DEFINE_MODEL(object_name, field_num) \
    class object_name; \
    class object_name##Model: public Model<object_name> \
    { \
    public: \
        object_name##Model():Model(#object_name, field_num){} \
    }; \
    class object_name: public ModelObject \
    { \
    public: \
        object_name() \
        { \
            field_list_ = new Field *[field_num]; \
            FOR_N(field_num) \
        } \
        object_name(const object_name& obj): ModelObject(obj.exist_) \
        { \
            field_list_ = new Field *[field_num]; \
            FOR_N(field_num) \
            for(int i=0; i<field_num; i++) \
            { \
                field_list_[i]->Copy(obj.field_list_[i]); \
            } \
        } \
        _DEFINE_MODEL_TAIL


DEFINE_MODEL(User, 3)(
    FIELD(1, Id, AutoField, true)
    FIELD(2, Name, StringField, false)
    FIELD(3, Price, IntField, false)
)

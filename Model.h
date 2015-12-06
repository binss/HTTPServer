/***********************************************************
* FileName:      Model.h
* Author:        binss
* Create:        2015-11-27 17:25:56
* Description:   No Description
***********************************************************/

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

#define DB_HOST "tcp://172.17.0.3:3306"
#define DB_USER "binss"
#define DB_PASSWORD "123456"
#define DB_NAME "dudu"

class Field
{
public:
    Field(int index, bool is_primary_key): index_(index), is_primary_key_(is_primary_key)
    {

    }
    Field(const Field& obj): index_(obj.index_), is_primary_key_(obj.is_primary_key_)
    {
    }
    virtual void Set(void * ptr) = 0;

protected:
    int index_;
    bool is_primary_key_;
    Field * next_;
};

class IntField: public Field
{
public:
    IntField(int index, bool is_primary_key = false) : Field(index, is_primary_key)
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
    void Set(void * ptr)
    {
        value_ = *(int *)ptr;
    }
private:
    int value_;
};

class StringField: public Field
{
public:
    StringField(int index, bool is_primary_key = false): Field(index, is_primary_key), value_("")
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
    operator string()
    {
        return value_;
    }
    operator const char *()
    {
        return value_.c_str();
    }
    string value() const
    {
        return value_;
    }
    void Set(void * ptr)
    {
        value_ = string(*(const char **)ptr);
    }
private:
    string value_;
};

class ModelObject
{
public:
    template<class ValType>
    void SetFieldByIndex(int index, ValType val)
    {
        field_list_[index-1]->Set(&val);
    }
public:
    Field ** field_list_;

protected:
    int counter_;

private:
    bool exist_;
    int fields_;
};


template<typename ModelObjectName>
class Model
{
public:
    Model(const char *name):name_(name)
    {
        driver = get_driver_instance();
        con = driver->connect(DB_HOST, DB_USER, DB_PASSWORD);
        con->setSchema(DB_NAME);

        stmt = con->createStatement();
        ResultSet *res = stmt->executeQuery("select * from " + name_);
        ResultSetMetaData *res_meta = res->getMetaData();
        cols = res_meta->getColumnCount();
        col_types = new string[cols];
        col_names = new string[cols];
        for(int i = 0; i < cols; ++i)
        {
            cout<<res_meta->getColumnLabel(i+1)<<" ";
            cout<<res_meta->getColumnTypeName(i+1)<<" ";
            cout<<res_meta->getColumnDisplaySize(i+1)<<endl;
            col_types[i] = res_meta->getColumnTypeName(i+1);
            col_names[i] = res_meta->getColumnLabel(i+1);
        }
        // DatabaseMetaData *db_meta = con->getMetaData();
        // ResultSet * table_key_set = db_meta->getPrimaryKeys(con->getCatalog(), con->getSchema(), "Cars");
        // int primary_key_num = table_key_set->rowsCount();
        // if(primary_key_num > 0)
        // {
        //     int i=0;
        //     while(table_key_set->next())
        //     {
        //         string key = table_key_set->getString(4);
        //         for(int j=0; j<cols; j++)
        //         {
        //             if(col_names[j] == key)
        //             {
        //                 primary_key_indexs.insert(j);
        //             }
        //         }
        //         i++;
        //     }
        // }

    }

    vector<ModelObjectName> All()
    {
        string cmd = "SELECT * FROM " + name_;
        try
        {
            stmt = con->createStatement();
            ResultSet *res = stmt->executeQuery(cmd);
            ResultSetMetaData *res_meta = res->getMetaData();

            vector<ModelObjectName> objects;
            while (res->next())
            {
                ModelObjectName object;
                for(int i=1; i <= cols; i++)
                {
                    string &type = col_types[i-1];
                    if(type == "INT")
                    {
                        object.SetFieldByIndex(i, res->getInt(i));
                    }
                    else if(type == "TEXT")
                    {
                        object.SetFieldByIndex(i, res->getString(i));
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

private:
    Driver *driver;
    Connection *con;
    Statement *stmt;

private:
    string name_;

    int cols;
    string *col_types;
    string *col_names;
};

#define FOR_1()   set_1();
#define FOR_2()   FOR_1() set_2();
#define FOR_3()   FOR_2() set_3();
#define FOR_N(n)  FOR_##n()

#define FIELD(index, name, type, is_primary_key) \
    type name{index, is_primary_key}; \
    void set_##index() { field_list_[index-1] = &name; }

#define _DEFINE_MODEL_TAIL(FIELDS) \
    FIELDS \
};

#define DEFINE_MODEL(object_name, cols) \
    class object_name; \
    class object_name##Model: public Model<object_name> \
    { \
    public: \
        object_name##Model():Model(#object_name){} \
    }; \
    class object_name: public ModelObject \
    { \
    public: \
        object_name() \
        { \
            field_list_ = new Field *[cols]; \
            FOR_N(cols) \
        } \
        _DEFINE_MODEL_TAIL


DEFINE_MODEL(User, 3)(
    FIELD(1, Id, IntField, true)
    FIELD(2, Name, StringField, true)
    FIELD(3, Price, IntField, true)
)


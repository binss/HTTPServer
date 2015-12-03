/***********************************************************
* FileName:      Model.h
* Author:        binss
* Create:        2015-11-27 17:25:56
* Description:   No Description
***********************************************************/

#include <string>
#include <iostream>
#include <vector>
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

#define DB_HOST "tcp://172.17.0.2:3306"
#define DB_USER "binss"
#define DB_PASSWORD "123456"
#define DB_NAME "dudu"

template<typename ModelType>
class Model
{
public:
    Model(const char *str)
    {
        table_name = string(str);
        driver = get_driver_instance();
        con = driver->connect(DB_HOST, DB_USER, DB_PASSWORD);
        con->setSchema(DB_NAME);

        stmt = con->createStatement();
        ResultSet *res = stmt->executeQuery("select * from " + table_name);
        ResultSetMetaData *res_meta = res->getMetaData();
        cols = res_meta->getColumnCount();
        col_types = new string[cols];
        col_names = new string[cols];
        for (int i = 0; i < cols; ++i)
        {
            cout.width(20);
            cout<<res_meta->getColumnLabel(i+1);
            cout.width(20);
            cout<<res_meta->getColumnTypeName(i+1);
            cout.width(20);
            cout<<res_meta->getColumnDisplaySize(i+1)<<endl<<endl;
            col_types[i] = res_meta->getColumnTypeName(i+1);
            col_names[i] = res_meta->getColumnLabel(i+1);
        }

    }
    ~Model()
    {
        // delete res;
        delete stmt;
        delete con;
    }

    virtual int Set(stringstream &ss)
    {
        cout<<"You should implement the Set(stringstream &ss) function in subclass!"<<endl;
    }
    virtual int Set(ModelType & model)
    {
        cout<<"You should implement the Set(ModelType & model) function in subclass!"<<endl;
    }

    vector<ModelType> & Get()
    {
        string cmd = "SELECT * FROM " + table_name;
        try
        {
            stmt = con->createStatement();
            ResultSet *res = stmt->executeQuery(cmd);
            ResultSetMetaData *res_meta = res->getMetaData();
            stringstream ss;
            while (res->next())
            {
                for(int i=1; i <= cols; i++)
                {
                    // string type = res_meta->getColumnTypeName(i);
                    string &type = col_types[i-1];
                    if(type == "INT")
                    {
                        ss<<res->getInt(i)<<endl;
                    }
                    else if(type == "TEXT")
                    {
                        // 以'\n'截断
                        ss<<res->getString(i)<<endl;;
                    }
                }
                this->Set(ss);
            }

        }
        catch (sql::SQLException &e)
        {
          cout<<"SQLException: "<<e.what()<<"["<<e.getErrorCode()<<"] state:"<< e.getSQLState()<<endl;
        }
        return this->objects;
    }

    int Insert(stringstream &ss)
    {
        string name_str = "";
        string arg_str = "";
        for(int i=0; i<cols-1; i++)
        {
            name_str += col_names[i] + ",";
            arg_str += "?,";
        }
        name_str += col_names[cols-1];
        arg_str += "?";
        // cout<<name_str<<endl;
        // cout<<arg_str<<endl;
        PreparedStatement *pstmt = con->prepareStatement("INSERT INTO Cars(" + name_str + ") VALUES (" + arg_str + ")");
        for (int i = 1; i <= cols; i++)
        {
            string & type = col_types[i-1];
            if(type == "INT")
            {
                int temp;
                ss>>temp;
                cout<<temp<<endl;
                pstmt->setInt(i, temp);
            }
            else if(type == "TEXT")
            {
                string temp;
                ss>>temp;
                cout<<temp<<endl;
                pstmt->setString(i, temp);
            }
        }
        pstmt->executeUpdate();
    }

    int Update(stringstream &ss)
    {

    }

    int Save(ModelType & model)
    {
        if(model._exist)
        {

        }
        this->Set(model);
    }

    int Save(ModelType * models, int length)
    {
        for(int i=0; i<length; i++)
        {
            Set(models[i]);
        }
    }
public:

protected:
    vector<ModelType> objects;

private:
    Driver *driver;
    Connection *con;
    Statement *stmt;

    string table_name;
    int cols;
    string *col_types;
    string *col_names;


};

struct DT_User
{
    int Id;
    string Name;
    int Price;

    bool _exist;
};


class User: public Model<DT_User>
{
public:
    User():Model("Cars")
    {

    }
    int Set(stringstream &ss)
    {
        DT_User user;
        ss>>user.Id;
        ss>>user.Name;
        ss>>user.Price;

        user._exist = true;
        objects.push_back(user);
    }
    int Set(DT_User & user)
    {
        stringstream ss;
        ss<<user.Id<<endl;
        ss<<user.Name<<endl;
        ss<<user.Price<<endl;
        Insert(ss);
    }
};

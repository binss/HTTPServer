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


template<typename ModelObjectName>
class Model
{
public:
    Model(const char *name, int field_num);
    ~Model();
    int Init();
    int Varify();

    vector<ModelObjectName> All();
    vector<ModelObjectName> Filter(FilterMap filters);

    int Insert(ModelObjectName & object);
    int Update(ModelObjectName & object);
    int Delete(ModelObjectName & object);
    int Save(ModelObjectName & object);
    int Save(ModelObjectName * objects, int length);
    int Save(vector<ModelObjectName> &objects);

private:
    int SetField(int index, Field * field);
    int GetField(int index, ModelObjectName & object);
    vector<ModelObjectName> Query(string filter_stat);

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

#include "ModelBaseDef.h"

#endif

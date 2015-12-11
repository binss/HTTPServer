/***********************************************************
* FileName:      Field.cpp
* Author:        binss
* Create:        2015-12-08 21:17:43
* Description:   No Description
***********************************************************/

#include "Field.h"

Logger FIELD_LOG("Field", DEBUG, true);

Field::Field(int index, FieldType type, string name, bool is_primary_key, bool is_auto_increment): index_(index),
                type_(type), name_(name), is_primary_key_(is_primary_key), is_auto_increment_(is_auto_increment)
{
    is_modified_ = false;
}

Field::Field(const Field& obj): index_(obj.index_), type_(obj.type_), name_(obj.name_), is_primary_key_(obj.is_primary_key_),
                is_auto_increment_(obj.is_auto_increment_), is_modified_(obj.is_modified_)
{
}

FieldType Field::GetType()
{
    return type_;
}

const string & Field::GetName()
{
    return name_;
}

bool Field::IsPrimaryKey()
{
    return is_primary_key_;
}

bool Field::IsAutoIncrement()
{
    return is_auto_increment_;
}

bool Field::IsModified()
{
    return is_modified_;
}



IntField::IntField(int index, string name, bool is_primary_key, bool is_auto_increment):
                Field(index, INT, name, is_primary_key, is_auto_increment)
{
    value_ = 0;
}

IntField::IntField(const IntField& obj): Field(obj)
{
    value_ = obj.value_;
}

const IntField & IntField::operator=(const int & value)
{
    is_modified_ = true;
    value_ = value;
    return *this;
}

IntField::operator int() const
{
    return value_;
}

void IntField::Set(void * data_ptr)
{
    value_ = *static_cast<int *>(data_ptr);
}

void IntField::Copy(void * obj_ptr)
{
    value_ = *static_cast<IntField *>(obj_ptr);
}



AutoField::AutoField(int index, string name, bool is_primary_key):
                IntField(index, name, is_primary_key, true)
{
}

// 运算符重载无法继承，需要重新定义
const AutoField & AutoField::operator=(const int & value)
{
    FIELD_LOG<<WARNING<<"You have set the value of auto increment field!"<<endl;
    is_modified_ = true;
    value_ = value;
    return *this;
}



StringField::StringField(int index, string name, bool is_primary_key): Field(index, STRING, name, is_primary_key, false), value_("")
{
}

StringField::StringField(const StringField & obj): Field(obj), value_(obj.value_)
{
}

const StringField & StringField::operator=(const string & value)
{
    is_modified_ = true;
    value_ = value;
    return *this;
}

StringField::operator const char *()
{
    return value_.c_str();
}

void StringField::Set(void * data_ptr)
{
    value_ = string(*static_cast<const char **>(data_ptr));
}

void StringField::Copy(void * obj_ptr)
{
    value_ = *static_cast<StringField *>(obj_ptr);
}

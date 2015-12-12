/***********************************************************
* FileName:      Field.h
* Author:        binss
* Create:        2015-12-08 21:17:41
* Description:   No Description
***********************************************************/
#ifndef __FIELD_H__
#define __FIELD_H__

#include <string>
#include <iostream>
#include "../Logger.h"
#include "../Constants.h"

using namespace std;

class Field
{
public:
    Field(int index, FieldType type, string name, bool is_primary_key, bool is_auto_increment);
    Field(const Field& obj);
    FieldType GetType();
    const string & GetName();
    bool IsPrimaryKey();
    bool IsAutoIncrement();
    bool IsModified();

    virtual void Set(void * data_ptr) = 0;
    virtual void Copy(void * obj_ptr) = 0;

protected:
    int index_;
    FieldType type_;
    string name_;

    bool is_primary_key_;
    bool is_auto_increment_;
    bool is_modified_;
};

class IntField: public Field
{
public:
    IntField(int index, string name, bool is_primary_key=false, bool is_auto_increment=false);
    IntField(const IntField& obj);
    const IntField & operator=(const int & value);
    operator int() const;
    void Set(void * data_ptr);
    void Copy(void * obj_ptr);

protected:
    int value_;
};

class AutoField: public IntField
{
public:
    AutoField(int index, string name, bool is_primary_key=false);
    // 赋值运算符重载无法继承，需要重新定义
    const AutoField & operator=(const int & value);
};

class StringField: public Field
{
public:
    StringField(int index, string name, bool is_primary_key=false);
    StringField(const StringField & obj);
    const StringField & operator=(const string & value);
    operator const char *();
    void Set(void * data_ptr);
    void Copy(void * obj_ptr);

protected:
    string value_;
};

// class FloatField: public Field
// {
// public:
//     FloatField(int index, string name, bool is_primary_key=false);
//     FloatField(const FloatField& obj);
//     const FloatField & operator=(const float & value);
//     operator float() const;
//     void Set(void * data_ptr);
//     void Copy(void * obj_ptr);

// protected:
//     float value_;
// };

// binss:
// don't support float because connector c++ api don't support getFloat
// so use double instead
class DoubleField: public Field
{
public:
    DoubleField(int index, string name, bool is_primary_key=false);
    DoubleField(const DoubleField& obj);
    const DoubleField & operator=(const long double & value);
    operator long double() const;
    void Set(void * data_ptr);
    void Copy(void * obj_ptr);

protected:
    long double value_;
};

class BooleanField: public Field
{
public:
    BooleanField(int index, string name, bool is_primary_key=false);
    BooleanField(const BooleanField& obj);
    const BooleanField & operator=(const bool & value);
    operator bool() const;
    void Set(void * data_ptr);
    void Copy(void * obj_ptr);

protected:
    bool value_;
};

class DateField: public StringField
{
public:
    DateField(int index, string name, bool is_primary_key=false);
    DateField(const DateField& obj);
    // 赋值运算符重载无法继承，需要重新定义
    // const DateField & operator=(const DateField & value);
    const DateField & operator=(const string & value);
    // operator const char *() const;
    void Set(void * data_ptr);
    void Copy(void * obj_ptr);

public:
    int year_;
    int month_;
    int day_;

protected:
    string value_;
};
#endif

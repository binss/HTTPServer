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
    FieldType GetType();
    const string & GetName();
    int GetIndex();

    bool IsPrimaryKey();
    bool IsAutoIncrement();
    bool IsModified();
    bool IsNull();

    void SetNull();

    virtual void Set(void * data_ptr) = 0;
    virtual void Copy(void * obj_ptr);

protected:
    int index_;
    FieldType type_;
    string name_;

    bool is_primary_key_;
    bool is_auto_increment_;
    bool is_modified_;
    bool is_null_;
};

class IntField: public Field
{
public:
    IntField(int index, string name, bool is_primary_key=false, bool is_auto_increment=false);
    const IntField & operator=(const int & value);
    operator int() const;
    virtual void Set(void * data_ptr);
    virtual void Copy(void * obj_ptr);

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
    const StringField & operator=(const string & value);
    operator const char *();
    virtual void Set(void * data_ptr);
    virtual void Copy(void * obj_ptr);

protected:
    string value_;
};

// binss:
// don't support float because connector c++ api don't support getFloat
// so use double instead
class DoubleField: public Field
{
public:
    DoubleField(int index, string name, bool is_primary_key=false);
    const DoubleField & operator=(const long double & value);
    operator long double() const;
    virtual void Set(void * data_ptr);
    virtual void Copy(void * obj_ptr);

protected:
    long double value_;
};

class BooleanField: public Field
{
public:
    BooleanField(int index, string name, bool is_primary_key=false);
    const BooleanField & operator=(const bool & value);
    operator bool() const;
    virtual void Set(void * data_ptr);
    virtual void Copy(void * obj_ptr);

protected:
    bool value_;
};

class DateField: public StringField
{
public:
    DateField(int index, string name, bool is_primary_key=false);
    DateField(int year, int month, int day);
    DateField(time_t time);

    const DateField & operator=(const DateField & value);
    virtual void Set(void * data_ptr);
    virtual void Copy(void * obj_ptr);

    int Year();
    int Month();
    int Day();

protected:
    int year_;
    int month_;
    int day_;
};


class TimeField: public StringField
{
public:
    TimeField(int index, string name, bool is_primary_key=false);
    TimeField(int hour, int minute, int second);
    TimeField(time_t time);

    const TimeField & operator=(const TimeField & value);
    virtual void Set(void * data_ptr);
    void Copy(void * obj_ptr);

    int Hour();
    int Minute();
    int Second();

protected:
    int hour_;
    int minute_;
    int second_;
};

// don't use multiple inheritance from DateField and TimeField because it is a diamond inheritance
// I have tried using virtual inheritance, it seem to work fine. However, when using base class pointer(field *) to call the copy function,
// It called the DateField version and cored dump, maybe there are some problems about virtual inheritance with virtual function?
class DateTimeField: public StringField
{
public:
    DateTimeField(int index, string name, bool is_primary_key=false);
    DateTimeField(int year, int month, int day, int hour, int minute, int second);
    DateTimeField(time_t time);

    const DateTimeField & operator=(const DateTimeField & value);
    virtual void Set(void * data_ptr);
    virtual void Copy(void * obj_ptr);

protected:
    int year_;
    int month_;
    int day_;
    int hour_;
    int minute_;
    int second_;
};


#endif

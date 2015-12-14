/***********************************************************
* FileName:      Field.cpp
* Author:        binss
* Create:        2015-12-08 21:17:43
* Description:   No Description
***********************************************************/

#include <regex>
#include "Field.h"

Logger FIELD_LOG("Field", DEBUG, true);

Field::Field(int index, FieldType type, string name, bool is_primary_key, bool is_auto_increment): index_(index),
                type_(type), name_(name), is_primary_key_(is_primary_key), is_auto_increment_(is_auto_increment)
{
    is_modified_ = false;
    is_null_ = false;
}

FieldType Field::GetType()
{
    return type_;
}

const string & Field::GetName()
{
    return name_;
}

int Field::GetIndex()
{
    return index_;
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

void Field::SetNull()
{
    is_null_ = true;
}

bool Field::IsNull()
{
    return is_null_;
}

void Field::Copy(void * obj_ptr)
{
    Field * ptr = static_cast<Field *>(obj_ptr);
    is_primary_key_ = ptr->is_primary_key_;
    is_auto_increment_ = ptr->is_auto_increment_;
    is_modified_ = ptr->is_modified_;
    is_null_ = ptr->is_null_;
}



// -----------------------IntField-----------------------
IntField::IntField(int index, string name, bool is_primary_key, bool is_auto_increment):
                Field(index, INT, name, is_primary_key, is_auto_increment)
{
    value_ = 0;
}

const IntField & IntField::operator=(const int & value)
{
    is_modified_ = true;
    is_null_ = false;
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
    Field::Copy(obj_ptr);
    value_ = static_cast<IntField *>(obj_ptr)->value_;
}


// -----------------------AutoField-----------------------
AutoField::AutoField(int index, string name, bool is_primary_key):
                IntField(index, name, is_primary_key, true)
{
}

// 运算符重载无法继承，需要重新定义
const AutoField & AutoField::operator=(const int & value)
{
    FIELD_LOG<<WARNING<<"You have set the value of auto increment field!"<<endl;
    is_modified_ = true;
    is_null_ = false;
    value_ = value;
    return *this;
}


// -----------------------StringField-----------------------
StringField::StringField(int index, string name, bool is_primary_key): Field(index, STRING, name, is_primary_key, false), value_("")
{
}

const StringField & StringField::operator=(const string & value)
{
    is_modified_ = true;
    is_null_ = false;
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
    Field::Copy(obj_ptr);
    value_ = static_cast<StringField *>(obj_ptr)->value_;
}



// -----------------------DoubleField-----------------------
DoubleField::DoubleField(int index, string name, bool is_primary_key):
                Field(index, DOUBLE, name, is_primary_key, false)
{
    value_ = 0;
}

const DoubleField & DoubleField::operator=(const long double & value)
{
    is_modified_ = true;
    is_null_ = false;
    value_ = value;
    return *this;
}

DoubleField::operator long double() const
{
    return value_;
}

void DoubleField::Set(void * data_ptr)
{
    value_ = *static_cast<long double *>(data_ptr);
}

void DoubleField::Copy(void * obj_ptr)
{
    Field::Copy(obj_ptr);
    value_ = static_cast<DoubleField *>(obj_ptr)->value_;
}



// -----------------------BooleanField-----------------------
BooleanField::BooleanField(int index, string name, bool is_primary_key):
                Field(index, BOOLEAN, name, is_primary_key, false)
{
    value_ = 0;
}

const BooleanField & BooleanField::operator=(const bool & value)
{
    is_modified_ = true;
    is_null_ = false;
    value_ = value;
    return *this;
}

BooleanField::operator bool() const
{
    return value_;
}

void BooleanField::Set(void * data_ptr)
{
    value_ = *static_cast<bool *>(data_ptr);
}

void BooleanField::Copy(void * obj_ptr)
{
    Field::Copy(obj_ptr);
    value_ = static_cast<BooleanField *>(obj_ptr)->value_;
}



// -----------------------DateField-----------------------
DateField::DateField(int index, string name, bool is_primary_key): StringField(index, name, is_primary_key)
{
    year_ = 0;
    month_ = 0;
    day_ = 0;
}

DateField::DateField(int year, int month, int day): StringField(0, "temp", false)
{
    year_ = year;
    month_ = month;
    day_ = day;
    value_ = to_string(year_) + "-" + to_string(month_) + "-" + to_string(day_);
}

DateField::DateField(time_t time): StringField(0, "temp", false)
{
    struct tm * time_ptr = gmtime(&time);
    year_ = 1900 + time_ptr->tm_year;
    month_ = 1 + time_ptr->tm_mon;
    day_ = time_ptr->tm_mday;
    value_ = to_string(year_) + "-" + to_string(month_) + "-" + to_string(day_);
}


const DateField & DateField::operator=(const DateField & obj)
{
    is_modified_ = true;
    is_null_ = false;
    // 只拷贝数据
    value_ = obj.value_;
    year_ = obj.year_;
    month_ = obj.month_;
    day_ = obj.day_;
    return *this;
}

void DateField::Set(void * data_ptr)
{
    value_ = string(*static_cast<const char **>(data_ptr));
    if(value_ != "")
    {
        regex reg("(\\d+)-(\\d+)-(\\d+)");
        smatch token;
        if(regex_match(value_, token, reg))
        {
            year_ = stoi(token[1]);
            month_ = stoi(token[2]);
            day_ = stoi(token[3]);
        }
        else
        {
            FIELD_LOG<<ERROR<<"Can't decode date["<<value_<<"]"<<endl;
        }
    }
}

void DateField::Copy(void * obj_ptr)
{
    Field::Copy(obj_ptr);
    DateField * ptr = static_cast<DateField *>(obj_ptr);
    value_ = ptr->value_;
    year_ = ptr->year_;
    month_ = ptr->month_;
    day_ = ptr->day_;
}

int DateField::Year()
{
    return year_;
}

int DateField::Month()
{
    return month_;
}

int DateField::Day()
{
    return day_;
}


// -----------------------TimeField-----------------------
TimeField::TimeField(int index, string name, bool is_primary_key): StringField(index, name, is_primary_key)
{
    hour_ = 0;
    minute_ = 0;
    second_ = 0;
}

TimeField::TimeField(int hour, int minute, int second): StringField(0, "temp", false)
{
    hour_ = hour;
    minute_ = minute;
    second_ = second;
    value_ = to_string(hour_) + ":" + to_string(minute_) + ":" + to_string(second_);
}

TimeField::TimeField(time_t time): StringField(0, "temp", false)
{
    struct tm * time_ptr = gmtime(&time);
    hour_ = time_ptr->tm_hour;
    minute_ = time_ptr->tm_min;
    second_ = time_ptr->tm_sec;
    value_ = to_string(hour_) + ":" + to_string(minute_) + ":" + to_string(second_);
}


const TimeField & TimeField::operator=(const TimeField & obj)
{
    is_modified_ = true;
    is_null_ = false;
    // 只拷贝数据
    value_ = obj.value_;
    hour_ = obj.hour_;
    minute_ = obj.minute_;
    second_ = obj.second_;
    return *this;
}

void TimeField::Set(void * data_ptr)
{
    value_ = string(*static_cast<const char **>(data_ptr));
    if(value_ != "")
    {
        regex reg("(\\d+):(\\d+):(\\d+)");
        smatch token;
        if(regex_match(value_, token, reg))
        {
            hour_ = stoi(token[1]);
            minute_ = stoi(token[2]);
            second_ = stoi(token[3]);
        }
        else
        {
            FIELD_LOG<<ERROR<<"Can't decode time["<<value_<<"]"<<endl;
        }
    }
}

void TimeField::Copy(void * obj_ptr)
{
    Field::Copy(obj_ptr);
    TimeField * ptr = static_cast<TimeField *>(obj_ptr);
    value_ = ptr->value_;
    hour_ = ptr->hour_;
    minute_ = ptr->minute_;
    second_ = ptr->second_;
}

int TimeField::Hour()
{
    return hour_;
}

int TimeField::Minute()
{
    return minute_;
}

int TimeField::Second()
{
    return second_;
}



// -----------------------DateTimeField-----------------------
DateTimeField::DateTimeField(int index, string name, bool is_primary_key): StringField(index, name, is_primary_key)
{
    year_ = 0;
    month_ = 0;
    day_ = 0;
    hour_ = 0;
    minute_ = 0;
    second_ = 0;
}

DateTimeField::DateTimeField(int year, int month, int day, int hour, int minute, int second): StringField(0, "temp", false)
{
    year_ = year;
    month_ = month;
    day_ = day;
    hour_ = hour;
    minute_ = minute;
    second_ = second;
    value_ = to_string(year_) + "-" + to_string(month_) + "-" + to_string(day_); + " " + to_string(hour_) + ":" + to_string(minute_) + ":" + to_string(second_);
}

DateTimeField::DateTimeField(time_t time): StringField(0, "temp", false)
{
    struct tm * time_ptr = gmtime(&time);
    year_ = 1900 + time_ptr->tm_year;
    month_ = 1 + time_ptr->tm_mon;
    day_ = time_ptr->tm_mday;
    hour_ = time_ptr->tm_hour;
    minute_ = time_ptr->tm_min;
    second_ = time_ptr->tm_sec;
    value_ = to_string(year_) + "-" + to_string(month_) + "-" + to_string(day_); + " " + to_string(hour_) + ":" + to_string(minute_) + ":" + to_string(second_);
}


const DateTimeField & DateTimeField::operator=(const DateTimeField & obj)
{
    is_modified_ = true;
    is_null_ = false;
    // 只拷贝数据
    value_ = obj.value_;
    year_ = obj.year_;
    month_ = obj.month_;
    day_ = obj.day_;
    hour_ = obj.hour_;
    minute_ = obj.minute_;
    second_ = obj.second_;
    return *this;
}

void DateTimeField::Set(void * data_ptr)
{
    value_ = string(*static_cast<const char **>(data_ptr));
    if(value_ != "")
    {
        regex reg("(\\d+)-(\\d+)-(\\d+) (\\d+):(\\d+):(\\d+)");
        smatch token;
        if(regex_match(value_, token, reg))
        {
            year_ = stoi(token[1]);
            month_ = stoi(token[2]);
            day_ = stoi(token[3]);
            hour_ = stoi(token[4]);
            minute_ = stoi(token[5]);
            second_ = stoi(token[6]);
        }
        else
        {
            FIELD_LOG<<ERROR<<"Can't decode datetime["<<value_<<"]"<<endl;
        }
    }
}

void DateTimeField::Copy(void * obj_ptr)
{
    Field::Copy(obj_ptr);
    DateTimeField * ptr = static_cast<DateTimeField *>(obj_ptr);
    value_ = ptr->value_;
    year_ = ptr->year_;
    month_ = ptr->month_;
    day_ = ptr->day_;
    hour_ = ptr->hour_;
    minute_ = ptr->minute_;
    second_ = ptr->second_;
}


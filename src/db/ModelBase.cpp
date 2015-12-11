/***********************************************************
* FileName:      ModelBase.cpp
* Author:        binss
* Create:        2015-12-08 23:30:39
* Description:   No Description
***********************************************************/


#include "ModelBase.h"

ModelObject::ModelObject(bool exist): exist_(exist)
{
    field_list_ = NULL;
}

Field * ModelObject::GetFieldByIndex(int index)
{
    return field_list_[index-1];
}

void ModelObject::SetExist()
{
    exist_ = true;
}

bool ModelObject::Exist()
{
    return exist_;
}


/***********************************************************
* FileName:      Model.h
* Author:        binss
* Create:        2015-11-27 17:25:56
* Description:   No Description
***********************************************************/

#ifndef __MODEL_H__
#define __MODEL_H__

#include "db/ModelBase.h"

DEFINE_MODEL(User, 7)(
    FIELD(1, Id, AutoField, true)
    FIELD(2, Name, StringField, false)
    FIELD(3, Balance, DoubleField, false)
    FIELD(4, Vip, BooleanField, false)
    FIELD(5, Birthday, DateField, false)
    FIELD(6, AppointmentTime, TimeField, false)
    FIELD(7, RegistrationTime, DateTimeField, false)

)

#endif


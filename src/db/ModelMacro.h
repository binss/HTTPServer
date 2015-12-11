/***********************************************************
* FileName:      ModelMacro.h
* Author:        binss
* Create:        2015-12-08 23:32:43
* Description:   No Description
***********************************************************/

#ifndef __MODEL_MACRO_H__
#define __MODEL_MACRO_H__

#define FOR_1()     set_1();
#define FOR_2()     FOR_1() set_2();
#define FOR_3()     FOR_2() set_3();
#define FOR_4()     FOR_3() set_4();
#define FOR_5()     FOR_4() set_5();
#define FOR_6()     FOR_5() set_6();
#define FOR_7()     FOR_6() set_7();
#define FOR_8()     FOR_7() set_8();
#define FOR_9()     FOR_8() set_9();
#define FOR_10()    FOR_9() set_10();
#define FOR_N(n)    FOR_##n()

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


#endif

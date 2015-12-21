/***********************************************************
* FileName:      View.cpp
* Author:        binss
* Create:        2015-11-10 21:47:55
* Description:   No Description
***********************************************************/

#include "View.h"

Logger LOG("View", DEBUG, true);

void main_page(Request &request, Response &response)
{
    if(request.META.METHOD == "GET")
    {
        response.SetCookie("username", "binss", GetTime(60000));
        response.SetCookie("email", "i@binss.me", GetTime(60000));
        response.SetCode(200);
        response.SetFile("index.html");
    }
    else if(request.META.METHOD == "POST")
    {
        LOG<<DEBUG<<request.POST["name"]<<endl;
        response.SetCode(200);

        response.SetRawString("OK");
        // response.SetFile("index.html");

    }
    // LOG<<DEBUG<<"\nGET\n"<<request.GET<<endl;
    // LOG<<DEBUG<<"\nCOOKIE\n"<<request.COOKIE<<endl;
}

void upload_page(Request &request, Response &response)
{
    if(request.META.METHOD == "GET")
    {
        response.SetCode(200);
        response.SetFile("upload.html");
    }
    else if(request.META.METHOD == "POST")
    {
        LOG<<DEBUG<<request.POST["name"]<<endl;
        response.SetCode(200);
        response.SetRawString("OK");
    }
}


void user_page(Request &request, Response &response)
{
    if(request.META.METHOD == "GET")
    {

        UserModel user_model;
        if(user_model.Init() == 0)
        {
            LOG<<DEBUG<<"init ok"<<endl;
            // vector<User> users = user_model.All({{"Id", "-"}});
            vector<User> users = user_model.Filter({{"Vip", "=true"}, {"Id", "<100"}}, {{"Id", "+"}});

            for(unsigned int i=0; i<users.size(); i++)
            {
                LOG<<DEBUG<<users[i].Id<<" "<<users[i].Name<<" "<<users[i].Balance<<" "<<users[i].Vip<<" "<<users[i].Birthday<<" ";
                LOG<<users[i].AppointmentTime<<" "<<users[i].RegistrationTime<<endl;
            }
            User & user = users[0];
            user.Balance = 2.33;
            user.Vip = true;
            user.Birthday = DateField(2000,2,1);
            user.AppointmentTime = TimeField(11,20,0);
            user.RegistrationTime = DateTimeField(time(0));
            user_model.Save(user);

        }

        response.SetCode(200);
        response.SetRawString("OK");
    }
}


void error_404(Request &request, Response &response)
{
    response.SetCode(404);
    response.SetFile("error/404.html");
}

void error_403(Request &request, Response &response)
{
    response.SetCode(403);
    response.SetFile("error/403.html");
}


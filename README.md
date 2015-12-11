#HTTPServer

HTTPServer is a Web framework written in C++. It is designed to rapid development in order to develop a website in few minutes. When I reading the *Unix Network Programming* and *HTTP: The Definitive Guide*, I sensed that if I don't make a practice of the knowledges which I have learning from the books, I will forget them quickly. This is why I decided to develop HTTPServer.

HTTPServer looks like Django, a Python Web framework, because I'm a fans of Django. With the help of Django, I developed [my blog](http://www.binss.me/) in a month and attracted by it MVC design. So like Django, the only thing you should do is defining views and models.

HTTPServer is still in the toy stage currently, so there are so many features to and so many bugs to fix. However, I will try my best to make it perfect.

Finally, please forgive my poor English.

##Usage

1. Define your view in view.h and implement it in view.cpp:
  ```
    void main_page(Request &request, Response &response);

    void main_page(Request &request, Response &response)
    {
        if(request.META.METHOD == "GET")
        {
            response.SetCookie("username", "binss", GetTime(60000));
            response.SetCookie("email", "i@binss.me", GetTime(60000));
            response.SetCode(200);
            response.SetFile("index.html");
        }
    }
  ```

2. Define the map between uri and view in Mapper.cpp
  ```
    void Mapper::InitViewMap()
    {
        view_map_["/"] = main_page;
    }
  ```

3. Add the index.html to templates folder

4. Build and run the server in bin folder

[Optional]

By defining models, you can access to mysql database.

1. use macro to define the model:
  ```
    DEFINE_MODEL(User, 3)(
        FIELD(1, Id, AutoField, true)
        FIELD(2, Name, StringField, false)
        FIELD(3, Price, IntField, false)
    )
  ```

2. define the database information in Constants.h
  ```
    #define DB_HOST "tcp://172.17.0.3:3306"
    #define DB_USER "binss"
    #define DB_PASSWORD "123456"
    #define DB_NAME "dudu"
  ```

3. use model in the view:
  ```
    UserModel user_model;
    if(user_model.Init() == 0)
    {
        LOG<<DEBUG<<"init ok"<<endl;
        vector<User> users = user_model.All();
        for(unsigned int i=0; i<users.size(); i++)
        {
            LOG<<DEBUG<<users[i].Id<<" "<<users[i].Name<<" "<<users[i].Price<<endl;
        }
        if(users.size() > 0)
        {
            User & user = users[0];
            user.Price = 1200;
            user_model.Save(user);
        }
    }
  ```

##Features to be implemented in next stage

1. [Model]define more fields of mysql

2. [Model]support conditional query

3. [View]support templates rendering


##License
Please see [LICENSE](LICENSE).

#include <iostream>
#include <iomanip>
#include <pqxx/pqxx>

/* Созданные  таблицы 

    Users                             - наименование таблицы
    (
	ID 			SERIAL PRIMARY KEY,   - номер созданной строки 
	first_name  VARCHAR(20) UNIQUE,   - имя пользователя
    last_name	VARCHAR(20) UNIQUE,   - фамилия пользователя
	passwording VARCHAR(15),          - пароль пользователя
	root_order  Boolean               - проверка пользователя на возможность вносить изменения
    )


    All_Items                           - наименование таблицы          
    (
	ID				SERIAL PRIMARY KEY, - номер созданной строки 
	nameing_product	text UNIQUE         - наименование товара
	value_product 	integer,            - кол-во продукта на складе
	cost_product	integer             - стоимость продукта на текущий момент храниться стоимость будет в копейках
    )                                     было жалко тратить байты на тип данных money(8 байт) и решил взять инт(4 байт), 
                                          тк кол-во стомоисти и наличие товара не превысит(21'474'836) 


    Admins                                   - наименование таблицы
    (
	id 			SERIAL PRIMARY KEY,          - номер созданной строки 
	first_name 	text UNIQUE,                 - имя
	last_name 	text UNIQUE,                 - фамилия   
    password    text,                        - пароль   
    glav_admin	boolean                      - определяет главного админа	
    )

    Админы не разрешается взаймодествовать с другими таблицами, кроме Users. 
    Они занимаются только добавлением и удалением сотрудников, могут дать право редактировать таблицу All_items сотруднику.

    glav_admin - поле содержит true только у одного пользователя и оно определяет, кто может вносить изменения 
    в саму таблицу Admins.

*/

class Users_and_Adimns{
    struct namings_user{
        std::string name, password;
        bool root_user;
    };
    struct namings_admins{
        std::string name, password;
        bool glav_admins;
    };
    namings_user* user { nullptr }; namings_admins* admin{nullptr};
public:
    Users_and_Adimns()
        {}
    virtual void reconnect(){};
    void set_user_or_admin(const bool& check, std::string name, std::string password, bool root){
        // Занос данных пользователя для использования их в будущем.
        delete user, admin;
        if(check){
            user->name = name; user->password = password; user->root_user = root; return;
        }
        admin->name = name; admin->password = password; admin->glav_admins = root;
    }
    bool get_person(){
        // Возможность узнать человеку, кто он( админ = 1 или пользователь = 0 ). 

        [[unlikely]]if(user == nullptr) return 1;
        [[likely]]return 0;
    }
};
class All_Items : public Users_and_Adimns{
    public:
    bool order_str(const std::string& str){
        for(int i = 0; i < str.size(); i++){
            [[likely]] if(str[i] == '*' or str[i] == '\\' or str[i] == '/') {
                std::cout <<  "Проверьте корректность введенных данных или обратитесь к администратору.";
                return 0;}
        }
        return 1;
    }
    pqxx::work work_with_All_Items; 
    pqxx::result result_All_Items;
    explicit All_Items(pqxx::connection& connect) : Users_and_Adimns(), work_with_All_Items(connect)
        {
            time_t now = time(0);
            char* date = ctime(&now);
            std::cout << "Текущее время: "<< date << "Желаю Вам продуктивной работы!\n\n";
        }

    const std::string set_man_admin_or_user() { 
        // Проверка на наличие пользователя в БД.

        std::string first_name, last_name, password;
        printf("Все введеные данные не допустимы, если будут содержать знаки \\, *, /\n");
        printf("Укажите Ваше имя = "); std::cin >> first_name;
        printf("Укажите Вашу фамилию = "); std::cin >> last_name;
        printf("Укажите Ваш пароль = "); std::cin >> password;

        [[unlikely]]if(order_str(first_name + last_name + password)){

        result_All_Items = work_with_All_Items.exec("select 'admins', first_name, last_name, password, glav_admin \
                                                    from Admins \
                                                    union \
                                                    select 'users', first_name, last_name, passwording, root_order \
                                                    from users");
        for(const auto& resulter : result_All_Items){
            if(first_name == resulter[1].as<std::string>() &&
               last_name  == resulter[2].as<std::string>() &&
               password   == resulter[3].as<std::string>())
               {
                if("users" == resulter[0].as<std::string>()){
                    set_user_or_admin(1, first_name, password, resulter[4].as<bool>());
                }else{
                    set_user_or_admin(0, first_name, password, resulter[4].as<bool>());
                }
                return "Добро пожаловать " + first_name + ' ' + last_name + '!';
               }
        }
        }
    };
    void check_tables(const char* based){
        // Отображение всей таблицы

        if(order_str(based)){
            result_All_Items = work_with_All_Items.exec("select * from " + std::string(based));    
            for(int i = 0; i < result_All_Items.columns(); i++)
            std::cout << result_All_Items.column_name(i) << std::setw(20);
            std::cout << '\n';
            for(const auto& r: result_All_Items){
                for(int i = 0; i < result_All_Items.columns(); i++){
                    std::cout << r[i].as<std::string>() << std::setw(20);
                }    
                std::cout << '\n';
        }
        }
    }
    
    void insert_item_to_db(){
        if(get_person()){
            // Админ, добавляет данные в таблицу пользователи
        }else{
            // Пользователь и может просто смотреть данные, заполнять таблицу склада
        }
    }

    void increment_and_decrement_cost_item(const char* item){
        // Увеличение или уменьшение стоимости товара

        if(get_person()){
            // Пользователь и может просто смотреть данные, заполнять таблицу склада
        }else{
            // выход из функций, тк зашёл админ.
        }
    }
    void increment_and_decrement_value_item(const char* item){
        // Увеличение или уменьшение кол-во товара на складе

        if(get_person()){
            // Пользователь и может просто смотреть данные, заполнять таблицу склада
        }else{
            // выход из функций, тк зашёл админ.
        }
    }
};


int main() {
        /*
    добавить или убрать товар
    добавить или уменшить кол-во товаров на складе
    посмотреть цену 
    добавить или уменьшить цену на товар
        */
    
    constexpr const char* db_host = "localhost";
    constexpr const char* db_user = "misha";
    constexpr const char* db_pass = "denis";
    constexpr const char* db_name = "newT";
    
    pqxx::connection conn("dbname=" + std::string(db_name)      +
                          " user=" + std::string(db_user)       +
                          " password=" + std::string(db_pass)   +
                          " host=" + std::string(db_host));
    //All_Items obj(conn);
    pqxx::work wo(conn);

    // if(conn.is_open()){
    //     All_Items a;
    // }else{
    //     std::cout << "Подключение к базе данных не прошло, введите корректные данные.";
    // }


    return 0;
}
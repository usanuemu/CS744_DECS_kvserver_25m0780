#include "httplib.h"
#include <iostream>
#include <string>

void print_response(const std::string& name, httplib::Result& res){
    std::cout << name << std::endl;
    if(res){
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body << std::endl;
    }
    else{
        std::cout << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
}

int main() {
    httplib::Client cli("localhost", 5002);
    cli.set_connection_timeout(5, 0); // 5 sec timeout

    std::cout << "Creating key 'hello' with value 'world'" << std::endl;
    httplib::Result res_post = cli.Post("/kv/hello", "world", "text/plain");
    print_response("POST /kv/hello", res_post);

    std::cout << "Creating key 'pass' with value 'fnqo8ucadn*&^C-nqcpaocP)@Daa'" << std::endl;
    res_post = cli.Post("/kv/pass", "fnqo8ucadn*&^C-nqcpaocP)@Daa", "text/plain");
    print_response("POST /kv/hello", res_post);

    std::cout << "Creating key 'phoneNo' with value '9230750231'" << std::endl;
    res_post = cli.Post("/kv/phoneNo", "9230750231", "text/plain");
    print_response("POST /kv/hello", res_post);

    std::cout << "Creating key 'name' with value 'Shivang'" << std::endl;
    res_post = cli.Post("/kv/name", "Shivang", "text/plain");
    print_response("POST /kv/hello", res_post);

    std::cout << "\nReading key 'pass'" << std::endl;
    httplib::Result res_get1 = cli.Get("/kv/pass");
    print_response("GET /kv/hello", res_get1);

    std::cout << "\nReading key 'hello'" << std::endl;
    httplib::Result res_get2 = cli.Get("/kv/hello");
    print_response("GET /kv/hello", res_get2);

    std::cout << "\nDeleting key 'hello'" << std::endl;
    httplib::Result res_del = cli.Delete("/kv/hello");
    print_response("DELETE /kv/hello", res_del);

    std::cout << "\nReading deleted key 'hello'" << std::endl;
    httplib::Result res_get3 = cli.Get("/kv/hello");
    print_response("GET /kv/hello (4)", res_get3);

    return 0;
}

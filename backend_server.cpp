#include "httplib.h"
#include <pqxx/pqxx>
#include <iostream>
#include <string>


const std::string POSTGRES_CONN_STRING = "dbname=keyvaluedb user=kvserver password=kvServer host=localhost";

int main() {
    using namespace httplib;
    Server svr;


    svr.Post("/kv/:key", [&](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        std::string value = req.body;

        try {
            pqxx::connection conn(POSTGRES_CONN_STRING);
            pqxx::work txn(conn);

            // updating and creating both in one sql query
            std::string sql =
                "INSERT INTO kv_store (key, value) VALUES ($1, $2) "
                "ON CONFLICT (key) DO UPDATE SET value = $2";

            txn.exec_params(sql, key, value);
            txn.commit();

            res.set_content("OK", "text/plain");
            std::cout << "[Backend] POST: Key '" << key << "' set" << std::endl;
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
            std::cerr << "[Backend] POST Error: " << e.what() << std::endl;
        }
    });

    svr.Get("/kv/:key", [&](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");

        try {
            pqxx::connection conn(POSTGRES_CONN_STRING);
            pqxx::work txn(conn);

            std::string sql = "SELECT value FROM kv_store WHERE key = $1";
            pqxx::result r = txn.exec_params(sql, key);

            if (r.empty()) {
                res.status = 404;
                res.set_content("Not Found", "text/plain");
                std::cout << "[Backend] GET: Key '" << key << "' not found" << std::endl;
            }
            else {
                res.set_content(r[0][0].as<std::string>(), "text/plain");
                std::cout << "[Backend] GET: Key '" << key << "' found" << std::endl;
            }
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
            std::cerr << "[Backend] GET Error: " << e.what() << std::endl;
        }
    });

    svr.Delete("/kv/:key", [&](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");

        try {
            pqxx::connection conn(POSTGRES_CONN_STRING);
            pqxx::work txn(conn);

            std::string sql = "DELETE FROM kv_store WHERE key = $1";
            pqxx::result r = txn.exec_params(sql, key);

            if (r.affected_rows() == 0) {
                res.status = 404;
                res.set_content("Not Found", "text/plain");
                std::cout << "[Backend] DELETE: Key '" << key << "' not found" << std::endl;
            }
            else {
                res.set_content("Deleted", "text/plain");
                std::cout << "[Backend] DELETE: Key '" << key << "' deleted" << std::endl;
            }
            txn.commit();
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
            std::cerr << "[Backend] DELETE Error: " << e.what() << std::endl;
        }
    });

    std::cout << "Backend server listening on http://0.0.0.0:5003" << std::endl;
    svr.listen("0.0.0.0", 5003);

    return 0;
}

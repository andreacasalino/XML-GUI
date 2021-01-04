#include <httplib.h>
#include <iostream>
#include <XML-model.h>

using namespace httplib;
using namespace std;

int main() {

  Server svr;
  XML_model model;

  svr.Post("/getJSON", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(model.GetJSON(), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/getNodeType", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content("n", "text/plain");
    }
    res.set_content(model.GetNodeType(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/import", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content(model.GetJSON(), "text/plain");
    }
    res.set_content(model.Import(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/export", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content("null", "text/plain");
    }
    model.Export(*opt);
    res.set_content("null", "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/delete", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content(model.GetJSON(), "text/plain");
    }
    res.set_content(model.Delete(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/rename", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content(model.GetJSON(), "text/plain");
    }
    res.set_content(model.Rename(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/nestTag", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content(model.GetJSON(), "text/plain");
    }
    res.set_content(model.NestTag(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/nestAttribute", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content(model.GetJSON(), "text/plain");
    }
    res.set_content(model.NestAttribute(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.Post("/setValue", [&model](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body);
    if(nullptr == opt) {
      res.set_content(model.GetJSON(), "text/plain");
    }
    res.set_content(model.SetValue(*opt), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  svr.listen("localhost", 3000);

  return EXIT_FAILURE;
}
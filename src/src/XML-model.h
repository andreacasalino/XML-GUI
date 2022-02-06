#include <JSONstream.h>
#include <OptionalString.h>
#include <RequestOptions.h>
#include <XML-Parser/Parser.h>
#include <functional>

class XML_model {
public:
  XML_model();

  inline const std::string &GetJSON() const { return this->dataJSON; };

  std::string GetNodeType(const gui::RequestOptions &opt);

  std::string Import(const gui::RequestOptions &opt);

  void Export(const gui::RequestOptions &opt);

  std::string Delete(const gui::RequestOptions &opt);

  std::string Rename(const gui::RequestOptions &opt);

  std::string NestTag(const gui::RequestOptions &opt);

  std::string NestAttribute(const gui::RequestOptions &opt);

  std::string SetValue(const gui::RequestOptions &opt);

private:
  struct xmlNodePosition {
    std::vector<xmlPrs::Name> pathFromRoot;
    OptionalString attributeName; // empty for tag
  };
  void updateJsonNodes();
  void updateJsonTag(gui::json::arrayJSON &nodes, gui::json::arrayJSON &edges,
                     std::size_t &counter, const xmlPrs::Tag& tag, const std::string& tag_name,
                     const xmlNodePosition &parentPosition,
                     const std::size_t &parentId);

  // data
  xmlPrs::Root root;
  std::map<std::size_t, xmlNodePosition> nodesInfo;
  std::string dataJSON;
};
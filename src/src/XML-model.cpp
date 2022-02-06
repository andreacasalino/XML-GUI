#include <XML-model.h>
#include <iostream>
#include <fstream>

#define COLOR_TAG "#0A5407"
#define COLOR_TAG_ROOT "#D26C00"
#define COLOR_ATTR "#A59407"

XML_model::XML_model() { this->updateJsonNodes(); }

void printTag(gui::json::arrayJSON &nodes, const std::size_t &counter,
              const std::string &name, const bool &isRoot = false) {
  gui::json::structJSON s;
  s.addElement("label", gui::json::String("<" + name + ">"));
  if (isRoot) {
    s.addElement("color", gui::json::String(COLOR_TAG_ROOT));
  } else {
    s.addElement("color", gui::json::String(COLOR_TAG));
  }
  s.addElement("id", gui::json::String(std::to_string(counter)));
  nodes.addElement(s);
};

void printAttribute(gui::json::arrayJSON &nodes, const std::size_t &counter,
                    const std::string &name, const std::string &value) {
  gui::json::structJSON s;
  s.addElement("label", gui::json::String(name + "=" + value));
  s.addElement("color", gui::json::String(COLOR_ATTR));
  s.addElement("id", gui::json::String(std::to_string(counter)));
  s.addElement("shape", gui::json::String("box"));
  nodes.addElement(s);
};

void printEdge(gui::json::arrayJSON &edges, const std::size_t &from,
               const std::size_t &to, const bool &leads2Tag) {
  gui::json::structJSON s;
  s.addElement("from", gui::json::String(std::to_string(from)));
  s.addElement("to", gui::json::String(std::to_string(to)));
  if (leads2Tag) {
    s.addElement("color", gui::json::String(COLOR_TAG));
    s.addElement("arrows", gui::json::String("from"));
  } else {
    s.addElement("color", gui::json::String(COLOR_ATTR));
  }
  edges.addElement(s);
};

void XML_model::updateJsonTag(gui::json::arrayJSON& nodes, gui::json::arrayJSON& edges,
    std::size_t& counter, const xmlPrs::Tag& tag, const std::string& tag_name,
    const xmlNodePosition& parentPosition,
    const std::size_t& parentId) {
    xmlNodePosition position(parentPosition);
    position.pathFromRoot.push_back(tag_name);
    ++counter;
    this->nodesInfo.emplace(counter, position);
    printTag(nodes, counter, tag_name);
    printEdge(edges, parentId, counter, true);
    std::size_t thisId = counter;
    const auto& attributes = tag.getAttributes();
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        xmlNodePosition attrPos(position);
        attrPos.attributeName.set(it->first);
        ++counter;
        this->nodesInfo.emplace(counter, attrPos);
        printAttribute(nodes, counter, it->first, it->second);
        printEdge(edges, thisId, counter, false);
    }
    for (const auto& [name, nested] : tag.getNested()) {
        this->updateJsonTag(nodes, edges, counter, *nested, name, position, thisId);
    }
}

void XML_model::updateJsonNodes() {
  this->nodesInfo.clear();
  std::size_t counter = 0;
  xmlNodePosition rootPosition;
  this->nodesInfo.emplace(counter, rootPosition);
  gui::json::arrayJSON nodes, edges;
  printTag(nodes, 0, this->root.getName(), true);
  const auto& attributes = root.getAttributes();
  for (auto it = attributes.begin(); it != attributes.end(); ++it) {
    xmlNodePosition attrPos(rootPosition);
    attrPos.attributeName.set(it->first);
    ++counter;
    this->nodesInfo.emplace(counter, attrPos);
    printAttribute(nodes, counter, it->first, it->second);
    printEdge(edges, 0, counter, false);
  }
  for (const auto& [name, nested] : root.getNested()) {
      this->updateJsonTag(nodes, edges, counter, *nested, name, rootPosition, 0);
  }
  gui::json::structJSON json;
  json.addElement("nodes", nodes);
  json.addElement("edges", edges);
  this->dataJSON = json.str();
}

#define FIND_OPT_O(RETURN_FAIL)                                                \
  auto itOpt = opt.getValues().find('o');                                      \
  if (itOpt == opt.getValues().end()) {                                        \
    std::cout << "options not found" << std::endl;                             \
    return RETURN_FAIL;                                                        \
  }

std::string XML_model::GetNodeType(const gui::RequestOptions &opt) {
  FIND_OPT_O("u")
  std::string result = "n";
  auto it = this->nodesInfo.find(std::atoi(itOpt->second[0].c_str()));
  if (it != this->nodesInfo.end()) {
    if (nullptr == it->second.attributeName) {
      result = "t";
      std::cout << "is a tag" << std::endl;

    } else {
      result = "a";
      std::cout << "is an attribute" << std::endl;
    }
  } else {
    std::cout << "undefined node type" << std::endl;
  }
  return result;
}

std::string XML_model::Import(const gui::RequestOptions &opt) {
  FIND_OPT_O(this->GetJSON())
  try {
    std::cout << "importing " << itOpt->second[0] << std::endl;
    auto new_data = xmlPrs::parse_xml(itOpt->second[0]);
    if (std::get_if<xmlPrs::Error>(&new_data) != nullptr) {
        throw std::get<xmlPrs::Error>(new_data);
    }
    std::cout << "success" << std::endl;
    auto& as_root = std::get<xmlPrs::Root>(new_data);
    this->root.setName(as_root.getName());
    this->root = std::move(as_root);
    this->updateJsonNodes();
  } catch (...) {
  }
  return this->GetJSON();
}

void XML_model::Export(const gui::RequestOptions &opt) {
  FIND_OPT_O()
  try {
    std::cout << "exporting " << itOpt->second[0] << std::endl;
    std::ofstream stream(itOpt->second[0]);
    stream << this->root;
    std::cout << "success" << std::endl;
  } catch (...) {
    return;
  }
}

std::string XML_model::Delete(const gui::RequestOptions &opt) {
  FIND_OPT_O(this->GetJSON())
  std::size_t id = std::atoi(itOpt->second[0].c_str());
  if (id == 0) {
    std::cout << "request to delete root refused" << std::endl;
    return this->GetJSON();
  }
  auto it = this->nodesInfo.find(id);
  if (it != this->nodesInfo.end()) {
    auto &tag = this->root.getDescendant(it->second.pathFromRoot);
    if (nullptr == it->second.attributeName) {
      std::cout << "deleting tag " << it->second.pathFromRoot.back() << std::endl;
      tag.remove();
    } else {
      std::cout << "deleting attribute " << *it->second.attributeName
                << std::endl;
      auto itA = tag.getAttributes().find(*it->second.attributeName);
      tag.getAttributes().erase(itA);
    }
    this->updateJsonNodes();
    return this->GetJSON();
  }
  std::cout << "delete undefined node" << std::endl;
  return this->GetJSON();
}

std::string XML_model::Rename(const gui::RequestOptions &opt) {
  FIND_OPT_O(this->GetJSON())
  if (itOpt->second.size() < 2)
    return this->GetJSON();
  std::size_t id = std::atoi(itOpt->second[0].c_str());
  auto it = this->nodesInfo.find(id);
  if (it != this->nodesInfo.end()) {
    auto& tag = this->root.getDescendant(it->second.pathFromRoot);
    if (nullptr == it->second.attributeName) {
      std::cout << "renaming tag " << it->second.pathFromRoot.back() << std::endl;
      tag.rename(itOpt->second[1]);
    } else {
      std::cout << "renaming attribute " << *it->second.attributeName
                << std::endl;
      auto rangeA = tag.getAttributes().equal_range(*it->second.attributeName);
      for (auto itA = rangeA.first; itA != rangeA.second; ++itA) {
          const std::string attr_val = itA->second;
          tag.getAttributes().erase(itA);
          tag.getAttributes().emplace(itOpt->second[1], attr_val);
      }
    }
    this->updateJsonNodes();
    return this->GetJSON();
  }
  std::cout << "rename undefined node" << std::endl;
  return this->GetJSON();
}

std::string XML_model::NestTag(const gui::RequestOptions &opt) {
  FIND_OPT_O(this->GetJSON())
  if (itOpt->second.size() < 2)
    return this->GetJSON();
  std::size_t parentId = std::atoi(itOpt->second[0].c_str());
  auto it = this->nodesInfo.find(parentId);
  if (it != this->nodesInfo.end() && (nullptr == it->second.attributeName)) {
    auto& tag = this->root.getDescendant(it->second.pathFromRoot);
    std::cout << "nest tag " << itOpt->second[1] << " to tag " << it->second.pathFromRoot.back()
              << std::endl;
    tag.addNested(itOpt->second[1]);
    this->updateJsonNodes();
    return this->GetJSON();
  }
  std::cout << "nest tag to undefined node" << std::endl;
  return this->GetJSON();
}

std::string XML_model::NestAttribute(const gui::RequestOptions &opt) {
  FIND_OPT_O(this->GetJSON())
  if (itOpt->second.size() < 2)
    return this->GetJSON();
  std::size_t parentId = std::atoi(itOpt->second[0].c_str());
  auto it = this->nodesInfo.find(parentId);
  if (it != this->nodesInfo.end() && (nullptr == it->second.attributeName)) {
    auto& tag = this->root.getDescendant(it->second.pathFromRoot);
    std::cout << "nest attribute " << itOpt->second[1] << " to tag "
              << it->second.pathFromRoot.back() << std::endl;
    tag.getAttributes().emplace(itOpt->second[1], "undefined");
    this->updateJsonNodes();
    return this->GetJSON();
  }
  std::cout << "nest attribute to undefined node" << std::endl;
  return this->GetJSON();
}

std::string XML_model::SetValue(const gui::RequestOptions &opt) {
  FIND_OPT_O(this->GetJSON())
  if (itOpt->second.size() < 2)
    return this->GetJSON();
  std::size_t id = std::atoi(itOpt->second[0].c_str());
  auto it = this->nodesInfo.find(id);
  if (it != this->nodesInfo.end() && (nullptr != it->second.attributeName)) {
    auto& tag = this->root.getDescendant(it->second.pathFromRoot);
    std::cout << "set value of attribute in tag " << it->second.pathFromRoot.back() << std::endl;
    auto itA = tag.getAttributes().find(*it->second.attributeName);
    itA->second = itOpt->second[1];
    this->updateJsonNodes();
    return this->GetJSON();
  }
  std::cout << "set value of undefined node" << std::endl;
  return this->GetJSON();
}

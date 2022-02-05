#include <XML-model.h>
#include <iostream>

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

void XML_model::updateJsonTag(gui::json::arrayJSON &nodes,
                              gui::json::arrayJSON &edges, std::size_t &counter,
                              const xmlPrs::Root &tag,
                              const xmlNodePosition &parentPosition,
                              const std::size_t &parentId) {
  xmlNodePosition position(parentPosition);
  position.pathFromRoot.push_back(tag.getName());
  ++counter;
  this->nodesInfo.emplace(counter, position);
  printTag(nodes, counter, tag.getName());
  printEdge(edges, parentId, counter, true);
  std::size_t thisId = counter;
  const std::multimap<std::string, std::string> &attributes =
      tag.getAttributesConst();
  for (auto it = attributes.begin(); it != attributes.end(); ++it) {
    xmlNodePosition attrPos(position);
    attrPos.attributeName.set(it->first);
    ++counter;
    this->nodesInfo.emplace(counter, attrPos);
    printAttribute(nodes, counter, it->first, it->second);
    printEdge(edges, thisId, counter, false);
  }
  xmlPrs::Tag::ConstIterator nestedTags = tag.getNestedAllConst();
  for (auto it = nestedTags.begin(); it != nestedTags.end(); ++it) {
    this->updateJsonTag(nodes, edges, counter, *it->second, position, thisId);
  }
}

void XML_model::updateJsonNodes() {
  this->nodesInfo.clear();
  std::size_t counter = 0;
  xmlNodePosition rootPosition;
  this->nodesInfo.emplace(counter, rootPosition);
  gui::json::arrayJSON nodes, edges;
  printTag(nodes, 0, this->data.getRoot().getName(), true);
  const std::multimap<std::string, std::string> &attributes =
      this->data.getRoot().getAttributesConst();
  for (auto it = attributes.begin(); it != attributes.end(); ++it) {
    xmlNodePosition attrPos(rootPosition);
    attrPos.attributeName.set(it->first);
    ++counter;
    this->nodesInfo.emplace(counter, attrPos);
    printAttribute(nodes, counter, it->first, it->second);
    printEdge(edges, 0, counter, false);
  }
  xmlPrs::Tag::ConstIterator rootNested =
      this->data.getRoot().getNestedAllConst();
  for (auto it = rootNested.begin(); it != rootNested.end(); ++it) {
    this->updateJsonTag(nodes, edges, counter, *it->second, rootPosition, 0);
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
    xmlPrs::Parser newData(itOpt->second[0]);
    std::cout << "success" << std::endl;
    this->data = std::move(newData);
    this->updateJsonNodes();
  } catch (...) {
  }
  return this->GetJSON();
}

void XML_model::Export(const gui::RequestOptions &opt) {
  FIND_OPT_O()
  try {
    std::cout << "exporting " << itOpt->second[0] << std::endl;
    this->data.reprint(itOpt->second[0]);
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
    xmlPrs::Tag &tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    if (nullptr == it->second.attributeName) {
      std::cout << "deleting tag " << tag.getName() << std::endl;
      tag.remove();
    } else {
      std::cout << "deleting attribute " << *it->second.attributeName
                << std::endl;
      auto itA =
          this->data.getRoot().getAttributes().find(*it->second.attributeName);
      this->data.getRoot().getAttributes().erase(itA);
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
    xmlPrs::Tag &tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    if (nullptr == it->second.attributeName) {
      std::cout << "renaming tag " << tag.getName() << std::endl;
      tag.setName(itOpt->second[1]);
    } else {
      std::cout << "renaming attribute " << *it->second.attributeName
                << std::endl;
      tag.setAttributeName(*it->second.attributeName, itOpt->second[1]);
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
    xmlPrs::Tag &tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    std::cout << "nest tag " << itOpt->second[1] << " to tag " << tag.getName()
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
    xmlPrs::Tag &tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    std::cout << "nest attribute " << itOpt->second[1] << " to tag "
              << tag.getName() << std::endl;
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
    xmlPrs::Tag &tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    std::cout << "set value of attribute in tag " << tag.getName() << std::endl;
    auto r = tag.getAttributes().equal_range(*it->second.attributeName);
    r.first->second = itOpt->second[1];
    this->updateJsonNodes();
    return this->GetJSON();
  }
  std::cout << "set value of undefined node" << std::endl;
  return this->GetJSON();
}

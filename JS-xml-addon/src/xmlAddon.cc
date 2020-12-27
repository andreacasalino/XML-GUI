#include "../xmlAddon.h"
#include <iostream>
using namespace Napi;

#define COLOR_TAG "#0A5407"
#define COLOR_ATTR "#A59407"

#define ARGS_CHECK(NUMBERS_EXPECTED) \
  Napi::Env env = info.Env(); \
    if (info.Length() != NUMBERS_EXPECTED) { \
      Napi::TypeError::New(env, "Wrong number of arguments")\
      .ThrowAsJavaScriptException();\
  }

#define STRING_CHECK(POSITION) \
    if (!info[POSITION].IsString()) { \
      Napi::TypeError::New(env, "input should be a string") \
      .ThrowAsJavaScriptException();\
  }

#define AS_STRING(POSITION) std::string(info[POSITION].As<Napi::String>().Utf8Value())

#define AS_SIZE_T(POSITION) static_cast<std::size_t>(std::atoi(AS_STRING(POSITION).c_str()))

xmlJS::xmlJS(const Napi::CallbackInfo& info) 
  : ObjectWrap(info) {
  ARGS_CHECK(0)
  this->updateJsonNodes();

  this->commands.emplace("/getJSON" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing getJSON" << std::endl;
    return this->dataJSON; 
  });
  this->commands.emplace("/getNodeType" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing getNodeType" << std::endl;
    ARGS_CHECK(2)
    nodeType nodeT = this->GetNodeType(AS_SIZE_T(1));
    switch (nodeT) {
      case nodeType::tag:
      return "t";
    case nodeType::attribute:
      return "a";
    }
    return "n"; 
  });
  this->commands.emplace("/import" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing import" << std::endl;
    ARGS_CHECK(2)
    this->Import(AS_STRING(1));
    return this->dataJSON; 
  });
  this->commands.emplace("/export" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing export" << std::endl;
    ARGS_CHECK(2)
    this->Export(AS_STRING(1)); 
    return ""; 
  });
  this->commands.emplace("/delete" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing delete" << std::endl;
    ARGS_CHECK(2)
    this->Delete(AS_SIZE_T(1));
    return this->dataJSON; 
  });
  this->commands.emplace("/rename" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing rename" << std::endl;
    ARGS_CHECK(3)
    this->Rename(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
  this->commands.emplace("/nestTag" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing nestTag" << std::endl;
    ARGS_CHECK(3)
    this->NestTag(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
  this->commands.emplace("/nestAttribute" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing nestAttribute" << std::endl;
    ARGS_CHECK(3)
    this->NestAttribute(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
  this->commands.emplace("/setValue" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing setValue" << std::endl;
    ARGS_CHECK(3)
    this->SetValue(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
}

Napi::Value xmlJS::ProcessRequest(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env(); 
    if (info.Length() == 0) { 
      Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }
  for(std::size_t k=0; k<info.Length(); ++k){
    STRING_CHECK(k)
  }

  auto it = this->commands.find(AS_STRING(0));
  if(it == this->commands.end()){
    return Napi::String::New(env, "");
  }
  return Napi::String::New(env, it->second(info).c_str());
}

void printTag(JSONArrayStream& nodes, const std::size_t& counter, const std::string& name){
  std::stringstream s;
  s << "{\"label\":\"<" << name << ">\"";
  s << ",\"color\":\"" << COLOR_TAG << "\"";
  s << ",\"id\":\"" << counter << "\"";
  s << "}";
  nodes << s.str();
};

void printAttribute(JSONArrayStream& nodes, const std::size_t& counter, const std::string& name, const std::string& value){
  std::stringstream s;
  s << "{\"label\":\"" << name << "=" << value << "\"";
  s << ",\"color\":\"" << COLOR_ATTR << "\"";
  s << ",\"id\":\"" << counter << "\"";
  s << ",\"shape\":\"box\"";
  s << "}";
  nodes << s.str();
};

void printEdge(JSONArrayStream& edges, const std::size_t& from, const std::size_t& to, const bool& leads2Tag){
  std::stringstream s;
  s << "{\"from\":\"" << from << "\"";
  s << ",\"to\":\"" << to << "\"";
  if(leads2Tag){
    s << ",\"color\":\"" << COLOR_TAG << "\"";
    s << ",\"arrows\":\"from\"";
  }
  else{
    s << ",\"color\":\"" << COLOR_ATTR << "\"";
  }
  s << "}";
  edges << s.str();
};

void xmlJS::updateJsonTag(JSONArrayStream& nodes, JSONArrayStream& edges, std::size_t& counter, const xmlPrs::Tag& tag, const xmlNodePosition& parentPosition, const std::size_t& parentId) {
  xmlNodePosition position(parentPosition);
  position.pathFromRoot.push_back(tag.getName());
  ++counter;
  this->nodesInfo.emplace(counter, position);
  printTag(nodes, counter, tag.getName());
  printEdge(edges, parentId, counter, true);
  std::size_t thisId = counter;
  const std::multimap<std::string, std::string>& attributes = tag.getAttributes();
  for(auto it= attributes.begin(); it!=attributes.end(); ++it){
    xmlNodePosition attrPos(position);
    attrPos.attributeName.set(it->first);
    ++counter;
    this->nodesInfo.emplace(counter, attrPos);
    printAttribute(nodes, counter, it->first, it->second);   
    printEdge(edges, thisId, counter, false);   
  }
  xmlPrs::Tag::ConstIterator nestedTags = tag.getNestedAll();
  for(auto it= nestedTags.begin(); it!=nestedTags.end(); ++it){
    this->updateJsonTag( nodes, edges, counter, *it->second, position, thisId);
  }
}

void xmlJS::updateJsonNodes() {
  this->nodesInfo.clear();
  std::size_t counter = 0;
  xmlNodePosition rootPosition;
  this->nodesInfo.emplace(counter, rootPosition);
  JSONArrayStream nodes, edges;
  printTag(nodes, 0, this->data.getRoot().getName());
  auto rootNested = this->data.getRoot().getNestedAll();
  for(auto it = rootNested.begin(); it!=rootNested.end(); ++it){
    this->updateJsonTag(nodes, edges, counter, *it->second, rootPosition, 0);
  }

  std::stringstream json;
  json << "{\"nodes\":";
  json << nodes.reset();
  json << ",\"edges\":";
  json << edges.reset();
  json << "}";
  this->dataJSON = json.str();
}

xmlJS::nodeType xmlJS::GetNodeType(const std::size_t& id) {
  nodeType result = nodeType::undefined;
  auto it = this->nodesInfo.find(id);
  if(it!=this->nodesInfo.end()) {
    if(nullptr == it->second.attributeName) {
      result = nodeType::tag;
      std::cout << "is a tag" << std::endl;
    }
    else{
      result = nodeType::attribute;
      std::cout << "is an attribute" << std::endl;
    }
  }
  else {
    std::cout << "undefined node type" << std::endl;
  }
  return result;
}

void xmlJS::Import(const std::string& fileName){
  try {
    std::cout << "importing " << fileName << std::endl;
    xmlPrs::Parser newData(fileName);
    std::cout << "success" << std::endl;
    this->data = std::move(newData);
    this->updateJsonNodes();
  }
  catch(...) {
    return;
  }
}

void xmlJS::Export(const std::string& fileName){
  try {
    std::cout << "exporting " << fileName << std::endl;
    this->data.reprint(fileName);
    std::cout << "success" << std::endl;
  }
  catch(...) {
    return;
  }
}

void xmlJS::Delete(const std::size_t& id){
  if(id == 0) {
    std::cout << "request to delete root refused" << std::endl;
    return;
  }
  auto it = this->nodesInfo.find(id);
  if(it != this->nodesInfo.end()){
    auto tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    if(nullptr == it->second.attributeName) {
      std::cout << "deleting tag " << tag.getName() << std::endl;
      tag.remove();
    }
    else{
      std::cout << "deleting attribute " << *it->second.attributeName << std::endl;
      auto itA = this->data.getRoot().getAttributes().find( *it->second.attributeName );
      this->data.getRoot().getAttributes().erase(itA);
    }
    this->updateJsonNodes();
    return;
  }
  std::cout << "delete undefined node" << std::endl;
}

void xmlJS::Rename(const std::size_t& id, const std::string& newName){
  auto it = this->nodesInfo.find(id);
  if(it != this->nodesInfo.end()){
    auto tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    if(nullptr == it->second.attributeName) {
      std::cout << "renaming tag " << tag.getName() << std::endl;
      tag.setName(newName);
    }
    else{
      std::cout << "renaming attribute " << *it->second.attributeName << std::endl;
      tag.setAttributeName(*it->second.attributeName, newName);
    }
    this->updateJsonNodes();
    return;
  }
  std::cout << "rename undefined node" << std::endl;
}

void xmlJS::NestTag(const std::size_t& parentId, const std::string& tagName){
  auto it = this->nodesInfo.find(parentId);
  if(it != this->nodesInfo.end() && (nullptr == it->second.attributeName)){
    auto tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    std::cout << "nest tag to tag " << tag.getName() << std::endl;
    tag.addNested(tagName);
    this->updateJsonNodes();
    return;
  }
  std::cout << "nest tag to undefined node" << std::endl;
}

void xmlJS::NestAttribute(const std::size_t& parentId, const std::string& attrName){
  auto it = this->nodesInfo.find(parentId);
  if(it != this->nodesInfo.end() && (nullptr == it->second.attributeName)){
    auto tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    std::cout << "nest attribute to tag " << tag.getName() << std::endl;
    tag.getAttributes().emplace(attrName, "undefined");
    this->updateJsonNodes();
    return;
  }
  std::cout << "nest attribute to undefined node" << std::endl;
}

void xmlJS::SetValue(const std::size_t& id, const std::string& value){
  auto it = this->nodesInfo.find(id);
  if(it != this->nodesInfo.end() && (nullptr != it->second.attributeName)){
    auto tag = this->data.getRoot().getNested(it->second.pathFromRoot);
    std::cout << "set value of attribute in tag " << tag.getName() << std::endl;
    auto r = tag.getAttributes().equal_range(*it->second.attributeName);
    r.first->second = value;
    this->updateJsonNodes();
    return;
  }
  std::cout << "set value of undefined node" << std::endl;
}

Napi::Object xmlJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "xmlJS", {
    InstanceMethod("ProcessRequest", &xmlJS::ProcessRequest)
  });

  Napi::FunctionReference constructor;
  constructor = Napi::Persistent(func);
  exports.Set("xmlJS", func);
  return exports;
}

Napi::Object Init (Napi::Env env, Napi::Object exports) {
    xmlJS::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)

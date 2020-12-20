#include "xmlAddon.h"
#include <ParserError.h>
#include <sstream>
using namespace Napi;

OptionalString::OptionalString(const OptionalString& o) {
  if(nullptr != o.content){
    this->content = std::make_unique<std::string>(o.content);
  }
}

OptionalString& OptionalString::operator=(const OptionalString& o){
  this->content.reset();
  if(nullptr != o.content){
    this->content = std::make_unique<std::string>(o.content);
  }
  return *this;
}

OptionalString& OptionalString::operator=(OptionalString&& o){
  this->content = std::move(o.content);
  return *this;
}

bool operator==(std::nullptr_t, const OptionalString& s){
  return (nullptr == s.content);
}

bool operator!=(std::nullptr_t, const OptionalString& s){
  return !(nullptr == s);
}

void OptionalString::set(const std::string& value){
  if(nullptr == this->content){
    this->content = std::make_unique<std::string>(value);
  }
  else {
    *this->content = value;
  }
}

const std::string& OptionalString::get() const{
  return *this->content.get();
}



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

xmlJS::xmlJS(const Napi::CallbackInfo& info) : ObjectWrap(info) {
  ARGS_CHECK(0)
  this->data = std::make_unique<xmlPrs::Parser>();
  this->updateJsonNodes();

  this->commands.emplace("/getJSON" , [this](const Napi::CallbackInfo& info) -> std::string { return this->GetJSON(); });
  this->commands.emplace("/getNodeType" , [this](const Napi::CallbackInfo& info) -> std::string { 
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
    ARGS_CHECK(2)
    this->Import(AS_STRING(1));
    return this->GetJSON(); 
  });
  this->commands.emplace("/export" , [this](const Napi::CallbackInfo& info) -> std::string { 
    ARGS_CHECK(2)
    this->Export(AS_STRING(1)); 
    return ""; 
  });
  this->commands.emplace("/delete" , [this](const Napi::CallbackInfo& info) -> std::string { 
    ARGS_CHECK(2)
    this->Delete(AS_SIZE_T(1));
    return this->GetJSON(); 
  });
  this->commands.emplace("/rename" , [this](const Napi::CallbackInfo& info) -> std::string { 
    ARGS_CHECK(3)
    this->Rename(AS_SIZE_T(1), AS_STRING(2));
    return this->GetJSON(); 
  });
  this->commands.emplace("/nestTag" , [this](const Napi::CallbackInfo& info) -> std::string { 
    ARGS_CHECK(3)
    this->NestTag(AS_SIZE_T(1), AS_STRING(2));
    return this->GetJSON(); 
  });
  this->commands.emplace("/nestAttribute" , [this](const Napi::CallbackInfo& info) -> std::string { 
    ARGS_CHECK(3)
    this->NestAttribute(AS_SIZE_T(1), AS_STRING(2));
    return this->GetJSON(); 
  });
  this->commands.emplace("/setValue" , [this](const Napi::CallbackInfo& info) -> std::string { 
    ARGS_CHECK(3)
    this->SetValue(AS_SIZE_T(1), AS_STRING(2));
    return this->GetJSON(); 
  });
}

Napi::Value xmlJS::ProcessRequest(const Napi::CallbackInfo& info){
  ARGS_CHECK(2)
  STRING_CHECK(0)
  STRING_CHECK(1)

  auto it = this->commands.find(AS_STRING(0));
  if(it == this->commands.end()){
    return Napi::String::New(env, "");
  }
  return Napi::String::New(env, it->second(info).c_str());
}

std::string xmlJS::GetJSON(){
  std::stringstream json;

  auto nodePrinter = [&json]( const std::map<std::size_t, xmlNodePosition>::iterator& it ){
    json << "{\"id\":" << std::to_string(it->first) << "}";
    //TODO add other things
  };

  auto edgePrinter = [&json]( const std::map<std::size_t, xmlNodePosition>::iterator& it ){
    json << "{\"from\":" << std::to_string(it->second.parentId);
    json << ",\"to\":" << std::to_string(it->first) << "}";
    //TODO add other things
  };

  json << "{\"nodes\":[";
  auto itN= this->jsonNodes.begin();
  nodePrinter(itN);
  ++itN;
  for(itN; itN!=this->jsonNodes.end(); ++itN){
    json << ",";
    nodePrinter(itN);
  }
  json << "],\"edges\":[";
  itN= this->jsonNodes.begin();
  ++itN;
  if(itN != this->jsonNodes.end()){
    edgePrinter(itN);
    ++itN;
    for(itN; itN!=this->jsonNodes.end(); ++itN){
      json << ",";
      edgePrinter(itN);
    }
  }
  json << "]}";
  return json.str();
}

void xmlJS::updateJsonTag(std::size_t& counter, const xmlPrs::TagHandler& tag, const xmlNodePosition& parentPosition, const std::size_t& parentId) {
  xmlNodePosition position(parentPosition);
  position.pathFromRoot.push_back(tag.GetTagName());
  position.parentId = parentId;
  ++counter;
  this->jsonNodes.emplace(counter, position);
  std::size_t thisId = counter;
  auto attributes = tag.GetAttributeAll();
  for(auto it= attributes.begin(); it!=attributes.end(); ++it){
    xmlNodePosition attrPos(position);
    attrPos.attributeName.set(it->first);
    attrPos.parentId = thisId;
    ++counter;
    this->jsonNodes.emplace(counter, attrPos);      
  }
  auto nestedTags = tag.GetNestedAll();
  for(auto it= nestedTags.begin(); it!=nestedTags.end(); ++it){
    this->updateJsonTag(counter, *it, position, thisId);
  }
}

void xmlJS::updateJsonNodes() {
  this->jsonNodes.clear();
  std::size_t counter = 0;
  xmlNodePosition rootPosition;
  rootPosition.parentId = 0;
  this->jsonNodes.emplace(counter, rootPosition);
  this->updateJsonTag(counter, this->data->GetRoot(), rootPosition, 0);
}

xmlJS::nodeType xmlJS::GetNodeType(const std::size_t& id) {
  auto it = this->jsonNodes.find(id);
  if(it == this->jsonNodes.end()){
    return nodeType::undefined;
  }
  if(nullptr == it->second.attributeName) {
    return nodeType::attribute;
  }
  return nodeType::tag;
}

void xmlJS::Import(const std::string& fileName){
  std::unique_ptr<xmlPrs::Parser> newStructure;
  try {
    newStructure = std::make_unique<xmlPrs::Parser>(fileName);
  }
  catch(...) {
    return;
  }
  this->data = std::move(newStructure);
  this->updateJsonNodes();
}

void xmlJS::Export(const std::string& fileName){
  try {
    this->data->Reprint(fileName);
  }
  catch(...) {
    return;
  }
}

void xmlJS::Delete(const std::size_t& id){
  if(id == 0) return;
  auto it = this->jsonNodes.find(id);
  if(it != this->jsonNodes.end()){
    auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
    if(nullptr == it->second.attributeName) {
      tag.Remove();
    }
    else{
      tag.RemoveAttribute( it->second.attributeName.get());
    }
    this->updateJsonNodes();
  }
}

void xmlJS::Rename(const std::size_t& id, const std::string& newName){
  auto it = this->jsonNodes.find(id);
  if(it != this->jsonNodes.end()){
    auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
    if(nullptr == it->second.attributeName) {
      tag.SetTagName(newName);
    }
    else{
      tag.SetAttributeName(it->second.attributeName.get(), newName);
    }
    this->updateJsonNodes();
  }
}

void xmlJS::NestTag(const std::size_t& parentId, const std::string& tagName){
  auto it = this->jsonNodes.find(parentId);
  if(it != this->jsonNodes.end() && (nullptr == it->second.attributeName)){
    auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
    tag.AddNested(tagName);
    this->updateJsonNodes();
  }
}

void xmlJS::NestAttribute(const std::size_t& parentId, const std::string& attrName){
  auto it = this->jsonNodes.find(parentId);
  if(it != this->jsonNodes.end() && (nullptr == it->second.attributeName)){
    auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
    tag.AddAttribute(attrName, "undefined");
    this->updateJsonNodes();
  }
}

void xmlJS::SetValue(const std::size_t& id, const std::string& value){
  auto it = this->jsonNodes.find(id);
  if(it != this->jsonNodes.end() && (nullptr != it->second.attributeName)){
    auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
    auto valOld = tag.GetAttributeValueFirst(it->second.attributeName.get());
    tag.SetAttributeValue(it->second.attributeName.get(), valOld, value);
    this->updateJsonNodes();
  }
}

Napi::Object xmlJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  xmlPrs::UseThrowError();
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

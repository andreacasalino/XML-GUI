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

#define TRY_CATCH(TRY_ACTIONS) \
  try { \
    TRY_ACTIONS\
  }\
  catch(...) {\
    return;\
  }

xmlJS::xmlJS(const Napi::CallbackInfo& info) : ObjectWrap(info) {
  ARGS_CHECK(0)
  this->selectedNode = -1;
  this->data = std::make_unique<xmlPrs::Parser>();

  this->commands.emplace("/getJSON" , [this](const Napi::CallbackInfo& info) -> std::string { return this->GetJSON(); });
  this->commands.emplace("/select" , [this](const Napi::CallbackInfo& info) -> std::string { 
    char nodeT = this->Select(info);
    switch (nodeT) {
      case nodeType::tag:
      return "t";
    case nodeType::attribute:
      return "a";
    }
    return "n"; 
  });
  this->commands.emplace("/deselect" , [this](const Napi::CallbackInfo& info) -> std::string { this->DeSelect(); return ""; });

  this->commands.emplace("/import" , [this](const Napi::CallbackInfo& info) -> std::string { this->Import(info); return ""; });
  this->commands.emplace("/export" , [this](const Napi::CallbackInfo& info) -> std::string { this->Export(info); return ""; });
  this->commands.emplace("/delete" , [this](const Napi::CallbackInfo& info) -> std::string { this->Delete(info); return ""; });
  this->commands.emplace("/rename" , [this](const Napi::CallbackInfo& info) -> std::string { this->Rename(info); return ""; });
  this->commands.emplace("/nest" , [this](const Napi::CallbackInfo& info) -> std::string { this->NestTag(info); return ""; });
  this->commands.emplace("/attribute" , [this](const Napi::CallbackInfo& info) -> std::string { this->AddAttribute(info); return ""; });
  this->commands.emplace("/value" , [this](const Napi::CallbackInfo& info) -> std::string { this->SetValue(info); return ""; });
}

Napi::Value xmlJS::ProcessRequest(const Napi::CallbackInfo& info){
  ARGS_CHECK(2)
  STRING_CHECK(0)

  auto it = this->commands.find(AS_STRING(0));
  if(it == this->commands.end()){
    return Napi::String::New(env, "");
  }
  return Napi::String::New(env, it->second(info).c_str());
}

std::string xmlJS::GetJSON(){
  std::stringstream json;

  auto nodePrinter = [&json]( const std::map<std::string, xmlNodePosition>::iterator& it ){
    json << "{\"id\":" << std::to_string(it->first) << "}";
    //TODO add other things
  };

  auto edgePrinter = [&json]( const std::map<std::string, xmlNodePosition>::iterator& it ){
    json << "{\"from\":" << std::to_string(itN->second.parentId);
    json << ",\"to\":" << std::to_string(itN->first) << "}";
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
  edgePrinter(itN);
  ++itN;
  for(itN; itN!=this->jsonNodes.end(); ++itN){
    json << ",";
    edgePrinter(itN);
  }
  json << "]}";
  return json.str();
}

void xmlJS::updateJsonTag(std::size_t& counter, const xmlPrs::TagHandler& tag, const xmlNodePosition& parentPosition, const std::size_t& parentId) {
  xmlNodePosition position(parentPosition);
  position.pathFromRoot.push_back(tag.GetTagName());
  position.parentId = parentId;
  if(counter > 0){
    this->jsonNodes.emplace(counter, position);
  }
  std::size_t thisId = counter;
  auto nestedTags = tag.GetNestedAll();
  for(auto it= nestedTags.begin(); it!=nestedTags.end(); ++it){
    ++counter;
    this->updateJsonTag(counter, *it, position);
  }
  auto attributes = tag.GetAttributeAll();
  for(auto it= attributes.begin(); it!=attributes.end(); ++it){
    xmlNodePosition attrPos(position);
    attrPos.attributeName.set(it->first);
    attrPos.parentId = thisId;
    ++counter;
    this->jsonNodes.emplace(counter, attrPos);      
  }
}

void xmlJS::updateJsonNodes() {
  this->jsonNodes.clear();
  std::size_t counter = 0;
  xmlNodePosition rootPosition;
  this->updateJsonTag(counter, this->data->GetRoot(), rootPosition);
}

xmlJS::nodeType xmlJS::Select(const Napi::CallbackInfo& info) {
  std::size_t id = info[1].As<std::size_t>();
  auto it = this->jsonNodes.find(id);
  if(it == this->jsonNodes.end()){
    nodeType::undefined;
  }
  this->selectedNode = it->first;
  if(nullptr == it->second.attributeName) {
    nodeType::attribute;
  }
  nodeType::tag;
}

void xmlJS::DeSelect(){
  this->selectedNode = -1;
}

void xmlJS::Import(const Napi::CallbackInfo& info){
  STRING_CHECK(1)
  std::unique_ptr<xmlPrs::Parser> newStructure;
  TRY_CATCH(newStructure = std::make_unique<xmlPrs::Parser>(AS_STRING(1));)
  this->data = std::move(newStructure);
  this->updateJsonNodes();
  this->DeSelect();
}

void xmlJS::Export(const Napi::CallbackInfo& info){
  STRING_CHECK(1)
  TRY_CATCH(this->data->Reprint(AS_STRING(1));)
}

void xmlJS::Delete(const Napi::CallbackInfo& info){
  if(-1 == this->selectedNode) return;
  auto it = this->jsonNodes.find(static_cast<std::size_t>(this->selectedNode));
  auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
  if(nullptr == it->second.attributeName){
    tag.Remove();
    return;
  }
  tag.RemoveAttribute( it->second.attributeName.get());
  this->updateJsonNodes();
  this->DeSelect();
}

void xmlJS::Rename(const Napi::CallbackInfo& info){
  STRING_CHECK(1)
  if(-1 == this->selectedNode) return;
  auto it = this->jsonNodes.find(static_cast<std::size_t>(this->selectedNode));
  auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
  if(nullptr == it->second.attributeName){
    tag.SetTagName(AS_STRING(1));
    return;
  }
  tag.SetAttributeName(it->second.attributeName.get(), AS_STRING(1));
  this->updateJsonNodes();
  this->DeSelect();
}

void xmlJS::NestTag(const Napi::CallbackInfo& info){
  STRING_CHECK(0)
  if(-1 == this->selectedNode) return;
  auto it = this->jsonNodes.find(static_cast<std::size_t>(this->selectedNode));
  auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
  if(nullptr == it->second.attributeName){
    tag.AddNested(AS_STRING(1));
    this->updateJsonNodes();
    this->DeSelect();
  }
}

void xmlJS::AddAttribute(const Napi::CallbackInfo& info){
  STRING_CHECK(1)
  if(-1 == this->selectedNode) return;
  auto it = this->jsonNodes.find(static_cast<std::size_t>(this->selectedNode));
  auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
  if(nullptr == it->second.attributeName){
    tag.AddAttribute(AS_STRING(1), "undefined");
    this->updateJsonNodes();
    this->DeSelect();
  }
}

void xmlJS::SetValue(const Napi::CallbackInfo& info){
  STRING_CHECK(1)
  if(-1 == this->selectedNode) return;
  auto it = this->jsonNodes.find(static_cast<std::size_t>(this->selectedNode));
  auto tag = this->data->GetRoot().GetNested(it->second.pathFromRoot);
  if(nullptr != it->second.attributeName){
    auto name = tag.GetAttributeValueFirst(it->second.attributeName.get());
    tag.SetAttributeValue(it->second.attributeName, name, AS_STRING(1));
  }
  this->updateJsonNodes();
  this->DeSelect();
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

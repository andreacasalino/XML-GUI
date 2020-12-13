#include "xmlAddon.h"
#include <ParserError.h>
using namespace Napi;

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

xmlJS::xmlJS(const Napi::CallbackInfo& info) : ObjectWrap(info) {
  ARGS_CHECK(0)
}

Napi::Value xmlJS::Import(const Napi::CallbackInfo& info) {
  ARGS_CHECK(1)
  STRING_CHECK(0)
  try {
    std::unique_ptr<xmlPrs::Parser> newStructure = std::make_unique<xmlPrs::Parser>(AS_STRING(0));
    this->structure = std::move(newStructure);
    return Napi::String::New(env, "");
    // TODO parse new strct and send back
  }
  catch(...) {
    return Napi::String::New(env, "");
  }
}

void xmlJS::Export(const Napi::CallbackInfo& info){
  ARGS_CHECK(1)
  STRING_CHECK(0)
  if(nullptr != this->structure) {
    this->structure->Reprint(AS_STRING(0));
  }
}

Napi::Object xmlJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  xmlPrs::UseThrowError();
  Napi::Function func = DefineClass(env, "xmlJS", {
    InstanceMethod("Import", &xmlJS::Import),
    InstanceMethod("Export", &xmlJS::Export)
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

// https://napi.inspiredware.com/getting-started/objectwrap.html#src-object-wrap-demo-cc-and-src-object-wrap-demo-h

#include <napi.h>
#include <TagHandler.h>

class xmlJS : public Napi::ObjectWrap<xmlJS> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    xmlJS(const Napi::CallbackInfo& info);

    Napi::Value Import(const Napi::CallbackInfo&);

    void Export(const Napi::CallbackInfo&);
private:
    std::unique_ptr<xmlPrs::Parser> structure;
};
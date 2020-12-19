// https://napi.inspiredware.com/getting-started/objectwrap.html#src-object-wrap-demo-cc-and-src-object-wrap-demo-h

#include <napi.h>
#include <TagHandler.h>
#include <map>
#include <functional>

class OptionalString {
public:
    OptionalString() = default;    
    OptionalString(const OptionalString& o);
    OptionalString& operator=(const OptionalString& o);
    OptionalString(OptionalString&& o) = default;
    OptionalString& operator=(OptionalString&& o);

    friend bool operator==(std::nullptr_t, const OptionalString&);
    friend bool operator!=(std::nullptr_t, const OptionalString&);

    void set(const std::string& value);
    const std::string& get() const;
private:
    std::unique_ptr<std::string> content;
};

class xmlJS : public Napi::ObjectWrap<xmlJS> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    xmlJS(const Napi::CallbackInfo& info);

    Napi::Value ProcessRequest(const Napi::CallbackInfo&);

private:
    std::string GetJSON();

    enum nodeType{ undefined, tag, attribute};
    nodeType Select(const Napi::CallbackInfo&);

    void DeSelect();

    void Import(const Napi::CallbackInfo&);

    void Export(const Napi::CallbackInfo&);

    void Delete(const Napi::CallbackInfo&);

    void Rename(const Napi::CallbackInfo&);

    void NestTag(const Napi::CallbackInfo&);

    void AddAttribute(const Napi::CallbackInfo&);

    void SetValue(const Napi::CallbackInfo&);

private:
    struct xmlNodePosition {
        std::vector<std::string> pathFromRoot;
        OptionalString           attributeName; // empty for tag
        std::size_t              parentId;
    };    
    void updateJsonNodes();
    void updateJsonTag(std::size_t& counter, const xmlPrs::TagHandler& tag, const xmlNodePosition& parentPosition, const std::size_t& parentId);

    std::map<std::string, std::function<std::string(const Napi::CallbackInfo& info)>> commands;
// data
    std::unique_ptr<xmlPrs::Parser>        data;
    std::map<std::size_t, xmlNodePosition> jsonNodes;
    int                                    selectedNode; // -1 when not selected
};
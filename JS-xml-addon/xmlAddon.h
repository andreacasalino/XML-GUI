// https://napi.inspiredware.com/getting-started/objectwrap.html#src-object-wrap-demo-cc-and-src-object-wrap-demo-h

#include <napi.h>
#include <TagHandler.h>
#include <map>
#include <functional>
#include <sstream>

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

class JSONArrayStream {
public:
    JSONArrayStream();

    void add(const std::string& element);

    std::string get();
private:
    bool isFirstElement;
    std::stringstream stream;
};

class xmlJS : public Napi::ObjectWrap<xmlJS> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    xmlJS(const Napi::CallbackInfo& info);

    Napi::Value ProcessRequest(const Napi::CallbackInfo&);

private:
    enum nodeType{ undefined, tag, attribute};
    nodeType GetNodeType(const std::size_t& id);

    void Import(const std::string& fileName);

    void Export(const std::string& fileName);

    void Delete(const std::size_t& id);

    void Rename(const std::size_t& id, const std::string& newName);

    void NestTag(const std::size_t& parentId, const std::string& tagName);

    void NestAttribute(const std::size_t& parentId, const std::string& attrName);

    void SetValue(const std::size_t& id, const std::string& value);

private:
    struct xmlNodePosition {
        std::vector<std::string> pathFromRoot;
        OptionalString           attributeName; // empty for tag
    };    

    void updateJsonNodes();
    void updateJsonTag(JSONArrayStream& nodes, JSONArrayStream& edges, std::size_t& counter, const xmlPrs::TagHandler& tag, const xmlNodePosition& parentPosition, const std::size_t& parentId);

    std::map<std::string, std::function<std::string(const Napi::CallbackInfo&)>> commands;
// data
    std::unique_ptr<xmlPrs::Parser>        data;
    std::map<std::size_t, xmlNodePosition> nodesInfo;
    std::string                            dataJSON;
};
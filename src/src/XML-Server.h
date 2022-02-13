#include <HttpGui/Server.h>
#include <XML-Parser/Tag.h>
#include <optional>

namespace xmlPrs {
    struct AttributePtr {
        Tag* parent;
        Attributes::iterator attribute;
    };
    using EntityPtr = std::variant<Tag*, AttributePtr>;

    struct EntityPtrAndParent {
        EntityPtr ptr;
        std::size_t parent;
    };

    using EntityMap = std::vector<EntityPtrAndParent>;

    class XMLServer : public gui::Server {
    public:
        XMLServer();

    protected:
        gui::Actions getPOSTActions() final;
        gui::Actions getGETActions() final { return {}; };

        const std::string& GetJSON();

        const EntityPtr& FindEntity(const gui::Request& req);
        std::string FindEntityType(const gui::Request& req);

        void Import(const gui::Request& req);

        void Export(const gui::Request& req);

        void Delete(const gui::Request& req);

        void Rename(const gui::Request& req);

        void NestTag(const gui::Request& req);

        void NestAttribute(const gui::Request& req);

        void SetAttributeValue(const gui::Request& req);

    private:
        Root xml;

        void updateEntityMap();
        EntityMap xml_map;

        void updateJSON();
        std::unique_ptr<nlohmann::json> xml_json;
    };
}


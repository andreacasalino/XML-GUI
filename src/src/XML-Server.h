#include <HttpGui/Server.h>
#include <XML-Parser/Tag.h>
#include <optional>

namespace xmlPrs {
    struct AttributePtr {
        Tag* parent;
        Attributes::iterator attribute;
    };

    struct EntityPtr : public std::variant<Tag*, AttributePtr> {
        EntityPtr(Tag* ptr, std::size_t parent_id);
        EntityPtr(const AttributePtr& ptr, std::size_t parent_id);

        std::size_t parent_id;
    };

    class XMLServer : public gui::Server {
    public:
        XMLServer();

    protected:
        gui::Actions getPOSTActions() final;
        gui::Actions getGETActions() final;

        const nlohmann::json& GetJSON();

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

        void updateEntities();
        std::vector<EntityPtr> xml_entities;

        void updateJSON();
        std::unique_ptr<nlohmann::json> xml_json;
    };
}


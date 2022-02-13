#include <XML-Server.h>
#include <XML-Parser/Parser.h>
#include <fstream>
#include <sstream>

namespace xmlPrs {
    XMLServer::XMLServer() {
        updateEntities();
    }

    namespace {
        template<typename TagCase, typename AttributeCase>
        void use_entity_pointer(const EntityPtr& entity_ptr, const TagCase& tag_case, const AttributeCase& attribute_case) {
            struct Visitor {
                const TagCase& tag_case;
                const AttributeCase& attribute_case;

                void operator()(Tag* ptr) const {
                    tag_case(ptr);
                };

                void operator()(const AttributePtr& ptr) const {
                    attribute_case(ptr);
                };
            };

            std::visit(Visitor{tag_case, attribute_case}, entity_ptr);
        }
    }

    const nlohmann::json& XMLServer::GetJSON() {
        if (nullptr == xml_json) {
            updateJSON();
        }
        return *xml_json;
    }

    const EntityPtr& XMLServer::FindEntity(const gui::Request& req) {
        const std::size_t entity_id = static_cast<std::size_t>(std::atoi((*req["entity"]).c_str()));
        if (entity_id >= xml_entities.size()) {
            throw std::runtime_error{ "Unknown entity" };
        }
        return xml_entities[entity_id];
    }

    std::string XMLServer::FindEntityType(const gui::Request& req) {
        auto ptr = FindEntity(req);
        std::string result;
        use_entity_pointer(ptr, 
            [&result](Tag* ptr) { result = "tag"; },
            [&result](const AttributePtr& ptr) { result = "attribute"; });
        return result;
    }

    void XMLServer::Import(const gui::Request& req) {
        auto imported = parse_xml(*req["file"]);
        auto* as_error = std::get_if<Error>(&imported);
        if (nullptr != as_error) {
            throw* as_error;
        }
        auto& as_xml = std::get<Root>(imported);
        xml.setName(as_xml.getName());
        xml = std::move(as_xml);
        updateEntities();
    }

    void XMLServer::Export(const gui::Request& req) {
        std::ofstream stream(*req["file"]);
        if (!stream.is_open()) {
            throw std::runtime_error{"Invalid file location"};
        }
        stream << xml;
    }

    void XMLServer::Delete(const gui::Request& req) {
        auto ptr = FindEntity(req);
        use_entity_pointer(ptr,
            [this](Tag* ptr) { 
                if (ptr == &this->xml) {
                    throw std::runtime_error{"can't delete root"};
                }
                ptr->remove(); 
            },
            [](const AttributePtr& ptr) { ptr.parent->getAttributes().erase(ptr.attribute); });
        updateEntities();
    }

    void XMLServer::Rename(const gui::Request& req) {
        auto ptr = FindEntity(req);
        use_entity_pointer(ptr,
            [&req](Tag* ptr) { ptr->rename(*req["name"]); },
            [&req](const AttributePtr& ptr) { 
                auto value = ptr.attribute->second;
                ptr.parent->getAttributes().erase(ptr.attribute);
                ptr.parent->getAttributes().emplace(*req["name"], value);
            });
        updateEntities();
    }

    void XMLServer::NestTag(const gui::Request& req) {
        auto ptr = FindEntity(req);
        use_entity_pointer(ptr,
            [&req](Tag* ptr) {  ptr->addNested(*req["name"]); },
            [&req](const AttributePtr& ptr) { throw std::runtime_error{ "Can't nest tag to attribute" }; });
        updateEntities();
    }

    void XMLServer::NestAttribute(const gui::Request& req) {
        auto ptr = FindEntity(req);
        use_entity_pointer(ptr,
            [&req](Tag* ptr) { ptr->getAttributes().emplace(*req["name"], "undefined"); },
            [&req](const AttributePtr& ptr) { throw std::runtime_error{ "Can't nest attribute to attribute" }; });
        updateEntities();
    }

    void XMLServer::SetAttributeValue(const gui::Request& req) {
        auto ptr = FindEntity(req);
        use_entity_pointer(ptr,
            [&req](Tag* ptr) { throw std::runtime_error{ "This is not an attribute whose value can be modified" }; },
            [&req](const AttributePtr& ptr) {
                auto name = ptr.attribute->first;
                ptr.parent->getAttributes().erase(ptr.attribute);
                ptr.parent->getAttributes().emplace(name, *req["value"]);
            });
        updateEntities();
    }

    EntityPtr::EntityPtr(Tag* ptr, const int parent_id)
        : std::variant<Tag*, AttributePtr>(ptr)
        , parent_id(parent_id) {
    }

    EntityPtr::EntityPtr(const AttributePtr& ptr, const int parent_id)
        : std::variant<Tag*, AttributePtr>(ptr)
        , parent_id(parent_id) {
    }

    void XMLServer::updateEntities() {
        this->xml_json.reset();

        struct TagAndParent {
            Tag* tag;
            int parent;
        };

        xml_entities.clear();
        std::list<TagAndParent> open;
        open.push_back(TagAndParent{&this->xml, -1});
        while (!open.empty()) {
            auto to_explore = open.front();
            open.pop_front();
            int added_id = static_cast<int>(xml_entities.size());
            xml_entities.emplace_back(to_explore.tag, to_explore.parent);
            // explore attributes
            for (auto it = to_explore.tag->getAttributes().begin(); it != to_explore.tag->getAttributes().end(); ++it) {
                xml_entities.emplace_back(AttributePtr{to_explore.tag, it}, added_id );
            }
            // add to open set children to explore it later
            for (const auto& [name, tag]: to_explore.tag->getNested()) {
                open.push_back(TagAndParent{ tag.get(), added_id});
            }
        }
    }

    namespace {
        static const std::string COLOR_TAG = "#0A5407";
        static const std::string COLOR_TAG_ROOT = "#D26C00";
        static const std::string COLOR_ATTR = "#A59407";

        void add_attribute(nlohmann::json& nodes, const Attributes::iterator& attribute, const std::size_t id) {
            auto& node = nodes.emplace_back();
            std::stringstream label_stream;
            label_stream << attribute->first << '=' << attribute->second;
            node["label"] = label_stream.str();
            node["color"] = COLOR_ATTR;
            node["id"] = id;
            node["shape"] = "box";
        }

        const std::string& get_tag_name(const Tag* tag) {
            if (tag->hasFather()) {
                const std::string* result = nullptr;
                for (auto it = tag->getFather().getNested().begin(); it != tag->getFather().getNested().end(); ++it) {
                    if (it->second.get() == tag) {
                        result = &it->first;
                        break;
                    }
                }
                return *result;
            }
            return dynamic_cast<const Root*>(tag)->getName();
        }

        void add_tag(nlohmann::json& nodes, const Tag* tag, const std::size_t id) {
            auto& node = nodes.emplace_back();
            std::stringstream label_stream;
            label_stream << '<' << get_tag_name(tag) << '>';
            node["label"] = label_stream.str();
            if (tag->hasFather()) {
                node["color"] = COLOR_TAG;
            }
            else {
                node["color"] = COLOR_TAG_ROOT;
            }
            node["id"] = id;
        };

        void add_edge(nlohmann::json& edges, const EntityPtr& ptr, const std::size_t id) {
            auto& edge = edges.emplace_back();
            edge["from"] = ptr.parent_id;
            edge["to"] = id;
            use_entity_pointer(ptr,
                [&edge](Tag* ptr) {
                    edge["color"] = COLOR_TAG;
                    edge["arrows"] = "from";
                },
                [&edge](const AttributePtr& ptr) {
                    edge["color"] = COLOR_ATTR;
                });
        };
    }

    void XMLServer::updateJSON() {
        xml_json = std::make_unique<nlohmann::json>();
        auto& nodes = (*xml_json)["nodes"];
        auto& edges = (*xml_json)["edges"];
        for (std::size_t id = 0; id < xml_entities.size(); ++id) {
            use_entity_pointer(xml_entities[id],
                [&](Tag* ptr) {
                    add_tag(nodes, ptr, id);
                },
                [&](const AttributePtr& ptr) {
                    add_attribute(nodes, ptr.attribute, id);
                });
            add_edge(edges, xml_entities[id], id);
        }
    }

    gui::Actions XMLServer::getGETActions() {
        gui::Actions result;
        return result;
    }

    gui::Actions XMLServer::getPOSTActions() {
        gui::Actions result;
        result.emplace("getJSON", [this](const gui::Request& req, gui::Response& resp) {
            resp = this->GetJSON();
            });
        result.emplace("getNodeType", [this](const gui::Request& req, gui::Response& resp) {
            resp = this->FindEntityType(req);
            });
        result.emplace("default_example", [this](const gui::Request& req, gui::Response& resp) {
            std::stringstream stream;
            stream << EXAMPLE_FOLDER << "XML_example_01.xml";
            resp = stream.str();
            });
        result.emplace("import", [this](const gui::Request& req, gui::Response& resp) {
            this->Import(req);
            resp = this->GetJSON();
        });
        result.emplace("export", [this](const gui::Request& req, gui::Response& resp) {
            this->Export(req);
        });
        result.emplace("delete", [this](const gui::Request& req, gui::Response& resp) {
            this->Delete(req);
            resp = this->GetJSON();
        });
        result.emplace("rename", [this](const gui::Request& req, gui::Response& resp) {
            this->Rename(req);
            resp = this->GetJSON();
        });
        result.emplace("nestTag", [this](const gui::Request& req, gui::Response& resp) {
            this->NestTag(req);
            resp = this->GetJSON();
        });
        result.emplace("nestAttribute", [this](const gui::Request& req, gui::Response& resp) {
            this->NestAttribute(req);
            resp = this->GetJSON();
        });
        result.emplace("setValue", [this](const gui::Request& req, gui::Response& resp) {
            this->SetAttributeValue(req);
            resp = this->GetJSON();
        });
        return result;
    }
}

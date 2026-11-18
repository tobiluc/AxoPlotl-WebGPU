#pragma once
#include "AxoPlotl/core/objects/BaseObject.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace AxoPlotl::IO
{

using WriterFunc = std::function<bool(
    const ObjectBase&,
    std::ostream&
    )>;

using ReaderFunc = std::function<
    std::unique_ptr<ObjectBase>(std::istream&)
    >;

struct Serializer {
    std::string type_name;
    WriterFunc writer;
    ReaderFunc reader;
};

class SerializerRegistry
{
public:
    void register_serializer(
        const std::string& _name,
        Serializer _serializer
        )
    {
        serializers[_name] = std::move(_serializer);
    }

    const Serializer& get(const std::string& _name) const
    {
        return serializers.at(_name);
    }

private:
    std::unordered_map<std::string, Serializer> serializers;
};

template<typename ObjectT>
bool write_object(
    const ObjectBase& object,
    std::ostream& stream
    ) {
    const auto& typed_object = dynamic_cast<const ObjectT&>(object);
    return typed_object.serialize(stream);
}

template<typename ObjectT>
std::unique_ptr<ObjectBase> read_object(
    std::istream& stream
    )
{
    auto object = std::make_unique<ObjectT>();
    if (!object->deserialize(stream)) {return nullptr;}
    return object;
}

SerializerRegistry registry;

template<typename ObjectT>
void register_type(
    SerializerRegistry& registry,
    const std::string& name
    ) {
    registry.register_serializer(name, {
       name,
       write_object<ObjectT>,
       read_object<ObjectT>
    });
}

// bool OpenVolumeMeshObject::save_to_file(const std::filesystem::path& _path) const
// {
//     return OVM::IO::ovmb_write(_path, mesh())
//     == OVM::IO::WriteResult::Ok;
// }
// struct ObjectIO
// {
//     std::filesystem::path path;
//     ObjectIOTraits traits;
//     NLOHMANN_DEFINE_TYPE_INTRUSIVE(ObjectIO,
//                                    path, traits);
// };

// TODO: How to do this?

}

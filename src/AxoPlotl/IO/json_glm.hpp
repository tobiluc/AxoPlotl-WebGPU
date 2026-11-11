#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace glm
{

template<length_t DIM, typename FT, qualifier Q>
void to_json(nlohmann::json& j, const vec<DIM, FT, Q>& v)
{
    j = nlohmann::json::array();

    for (length_t i = 0; i < DIM; ++i) {
        j.push_back(v[i]);
    }
}

template<length_t DIM, typename FT, qualifier Q>
void from_json(const nlohmann::json& j, vec<DIM, FT, Q>& v)
{
    if (!j.is_array() || j.size() != DIM) {
        throw nlohmann::json::type_error::create(
            302,
            "Expected an array with " + std::to_string(DIM) + " elements",
            &j
            );
    }

    for (length_t i = 0; i < DIM; ++i) {
        j.at(i).get_to(v[i]);
    }
}

template<typename FT, qualifier Q>
void to_json(nlohmann::json& j, const qua<FT, Q>& q) {
    j = nlohmann::json{
        {"x", q.x},{"y", q.y},{"z", q.z},{"w", q.w}
    };
}

template<typename FT, qualifier Q>
void from_json(const nlohmann::json& j, qua<FT, Q>& q) {
    j.at("x").get_to(q.x);
    j.at("y").get_to(q.y);
    j.at("z").get_to(q.z);
    j.at("w").get_to(q.w);
}

}

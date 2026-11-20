#include "Scene.hpp"
#include "AxoPlotl/IO/file_access.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#include <AxoPlotl/core/Application.hpp>
#include <fstream>

namespace AxoPlotl::internal
{

void Scene::init(Application *_app)
{
    app_ = _app;

    // Coordinate Axes Cross

    origin_position_buffer_ = create_bufferT<Vec4>(
        app_->gpu_context().device, {Vec4(0,0,0,1)});

    unit_frame_renderer_.init(UINT32_MAX,
                        app()->gpu_context(),
                        app()->render_context(),
                        origin_position_buffer_);
    glm::mat3x4 m;
    m[0] = {1,0,0,1};
    m[1] = {0,1,0,1};
    m[2] = {0,0,1,1};
    unit_frame_renderer_.update_matrix_data({m});
}

std::shared_ptr<OpenVolumeMeshObject> Scene::add_mesh(const VolMesh &&_mesh)
{
    return add_object<OpenVolumeMeshObject>(std::move(_mesh));
}

std::shared_ptr<OpenVolumeMeshObject> Scene::add_mesh(const std::filesystem::path& _path)
{
    auto rr = IO::read_mesh(_path);
    if (rr.status != IO::ReadMeshStatus::OK) {
        return nullptr;
    }
    return add_object<OpenVolumeMeshObject>(std::move(rr.mesh), _path);
}

void Scene::render(wgpu::RenderPassEncoder _render_pass, const ViewSize _view_size, bool _allow_input)
{
    // Get Width and Height
    camera_.update(app_->window(), _allow_input);
    const auto& view = camera_.get_view_matrix();
    const auto& proj = camera_.get_projection_matrix(_view_size);

    // The light can point in the camera forward direction
    if (settings.light_from_camera) {
        directional_light.direction = camera_.orientation() * Camera::FORWARD;
    }

    const auto view_proj = proj * view;

    // Render Coordinate Axis Cross at Origin
    unit_frame_renderer_.draw_symmetric() = false;
    unit_frame_renderer_.enabled() = settings.show_axis_cross;
    unit_frame_renderer_.vector_scale() = 1.0f;
    unit_frame_renderer_.axis_colors() = {{{1,0,0,1},{0,1,0,1},{0,0,1,1}}};
    unit_frame_renderer_.render(_view_size,
        _render_pass,
        view_proj);

    // Render Camera Target at Orbit Target
    unit_frame_renderer_.enabled() = settings.show_orbit_target_cross;
    unit_frame_renderer_.draw_symmetric() = true;
    unit_frame_renderer_.vector_scale() = 0.0002f;
    unit_frame_renderer_.axis_colors() = {{{0,0,0,1},{0,0,0,1},{0,0,0,1}}};
    unit_frame_renderer_.render(_view_size,
        _render_pass, proj
         * glm::translate(glm::mat4(1),
            Camera::FORWARD*camera().near_plane()));

    // Render Scene Objects
    for (const auto& obj : objects_) {
        obj->render(_render_pass, _view_size, view, proj);
    }

    // Remove deleted objects
    // This is deferred to avoid buffers being destroyed
    // before the command buffer is submitted
    app_->call_deferred([this]() {
        objects_.erase(
            std::remove_if(objects_.begin(), objects_.end(), [&](const std::shared_ptr<ObjectBase>& _obj) {
                return _obj->deleted();
            }), objects_.end());
        objects_.shrink_to_fit();
    });
}

void Scene::render_settings_ui()
{
    if (ImGui::BeginMenu("Camera")) {
        camera().render_settings_ui(app_->window());
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Light")) {
        ImGui::ColorEdit3("Color", &directional_light.color[0]);
        ImGui::InputFloat3("Direction", &directional_light.direction[0]);
        gui::InputSlider("Strength", &directional_light.strength);
        ImGui::Checkbox("From Camera", &settings.light_from_camera);
        ImGui::EndMenu();
    }

    ImGui::ColorEdit3("Background", settings.clear_color);
    ImGui::Checkbox("Axis Cross", &settings.show_axis_cross);
    ImGui::Checkbox("Orbit Cross", &settings.show_orbit_target_cross);
    ImGui::Checkbox("Zoom to New Object", &settings.zoom_to_new_object);
}

bool Scene::save_to_file(const std::filesystem::path& _path) const
{
    std::ofstream file(_path);
    if (!file) {return false;}

    try
    {
        nlohmann::json j(*this);

        // std::vector<ObjectIO> out;
        // out.reserve(num_objects());
        // for (const auto& obj : objects_) {
        //     if (obj->deleted()) {continue;}
        //     ObjectIO o;
        //     o.path =
        //         std::to_string(obj->id()) +
        //         "_" + obj->name() +
        //         obj->io_traits().file_ext;
        //     o.traits = obj->io_traits();
        //     if (obj->save_to_file(o.path)) {
        //         out.push_back(o);
        //     }
        // }
        // j["objects"] = out;

        file << j.dump(4);
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error saving Scene: " << e.what() << std::endl;
        return false;
    }
}

bool Scene::load_from_file(const std::filesystem::path& _path)
{
    std::ifstream file(_path);
    if (!file) {return false;}

    try
    {
        nlohmann::json j;
        file >> j;
        auto scene = j.get<Scene>();
        this->camera_ = scene.camera();
        this->settings = scene.settings;
        this->directional_light = scene.directional_light;

        // for (auto& obj : objects_) {
        //     obj->deleted() = true;
        // }
        // if (j.contains("objects")) {
        //     for (const ObjectIO& i : j.at("objects")) {
        //         if (i.traits.type_id == "ovm_obj") {
        //             this->add_mesh(i.path);
        //         }
        //     }
        // }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading Scene: " << e.what() << std::endl;
        return false;
    }
}

}

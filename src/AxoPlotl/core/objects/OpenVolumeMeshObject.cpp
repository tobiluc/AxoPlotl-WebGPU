#include "OpenVolumeMeshObject.hpp"
#include "AxoPlotl/gui/widgets/draw_basic_renderer_settings.hpp"
#include "AxoPlotl/gui/widgets/draw_toggle.hpp"
#include "AxoPlotl/properties/property_calculations.hpp"
#include "IconsFontAwesome7.h"
#include "imgui.h"
#include <AxoPlotl/core/Application.hpp>
#include <OpenVolumeMesh/IO/ovmb_write.hh>

namespace AxoPlotl
{
OpenVolumeMeshObject::OpenVolumeMeshObject(
    internal::Scene* _scene,
    VolMesh &_mesh,
    const std::optional<std::filesystem::path>& _filepath)
    :
    ObjectBase(_scene, ""),
    mesh_data_(std::move(_mesh)),
    filepath_(_filepath)
{
    this->name_ = _filepath.has_value()?
                      _filepath.value().stem().string() :
                      "New Volume Mesh Object " + std::to_string(id_);

    if (_filepath.has_value()) {
        // std::cerr << _filepath.value() << std::endl;
        auto filename_prop = _mesh.request_mesh_property<std::string>("AxoPlotl::filepath");
        _mesh.set_persistent(filename_prop);
        filename_prop[OVM::MeshHandle(0)] = *_filepath;
    }
}

bool OpenVolumeMeshObject::on_mouse_just_pressed(const Picker::Pixel& _p, const Mouse::Button _button)
{
    const auto& device = scene()->app()->gpu_context().device;
    switch (pick_mode_)
    {
        case PickMode::PROP_VIS:
        {
            if (_button == Mouse::Button::RIGHT) {
                if (_p.entity.object_id == id() && _p.entity == picked_pixel_.entity) {
                    // unslecet already selected
                    picked_pixel_ = Picker::Pixel{};
                } else {
                    request_pick_popup_ = true;
                    picked_pixel_ = _p;
                }
            }
            break;
        }
        case PickMode::MINECRAFT_ADD:
        {
            if (_p.entity.entity_type == Picker::EntityType::CELL) {
                OVM::CH ch(_p.entity.entity_index);
                if (ch.is_valid()) {
                    for (auto ch2 : mesh().cell_cells(ch)) {
                        renderers_.entity_renderer<OVM::Entity::Cell>().bits().write_bit(device, ch2.uidx(), true);
                    }
                }
            }
            break;
        }
        case PickMode::MINECRAFT_SUB:
        {
            if (_p.entity.entity_type == Picker::EntityType::CELL) {
                OVM::CH ch(_p.entity.entity_index);
                if (ch.is_valid()) {
                    renderers_.entity_renderer<OVM::Entity::Cell>().bits().write_bit(device, ch.uidx(), false);
                }
            }
            break;
        }
        default: break;
    }
    return true;
}

bool OpenVolumeMeshObject::on_mouse_pressed(const Picker::Pixel& _p, const Mouse::Button _button)
{
    return false;
}

void OpenVolumeMeshObject::draw_settings_ui()
{
    draw_name_input_ui();

    ImGui::PushID(0);
    gui::Toggle(renderers_.entity_renderer<OVM::Entity::Vertex>().enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Vertices")) {
        gui::draw_basic_renderer_settings(renderers_.entity_renderer<OVM::Entity::Vertex>());
        ImGui::EndMenu();
    }
    ImGui::PopID();

    ImGui::PushID(1);
    gui::Toggle(renderers_.entity_renderer<OVM::Entity::Edge>().enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Edges")) {
        gui::draw_basic_renderer_settings(renderers_.entity_renderer<OVM::Entity::Edge>());
        ImGui::EndMenu();
    }
    ImGui::PopID();

    ImGui::PushID(2);
    gui::Toggle(renderers_.entity_renderer<OVM::Entity::Face>().enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Faces")) {
        gui::draw_basic_renderer_settings(renderers_.entity_renderer<OVM::Entity::Face>());
        ImGui::EndMenu();
    }
    ImGui::PopID();

    ImGui::PushID(3);
    gui::Toggle(renderers_.entity_renderer<OVM::Entity::Cell>().enabled());
    ImGui::SameLine();
    if (ImGui::BeginMenu("Cells")) {
        gui::draw_basic_renderer_settings(renderers_.entity_renderer<OVM::Entity::Cell>());
        ImGui::EndMenu();
    }
    ImGui::PopID();

    // Clip Box
    // Each entity technically has their own, but we
    // just modify all at once.
    const auto& bbox = bounding_box();
    ClipBox& cb = renderers_.entity_renderer<OVM::Entity::Vertex>().clip_box();
    gui::draw_clipbox_settings(cb, bbox);
    renderers_.set_clip_box(cb);
}

void OpenVolumeMeshObject::draw_info_ui()
{
    ImGui::Text("V/E/F/C = %zu/%zu/%zu/%zu",
        mesh().n_vertices(), mesh().n_edges(),
        mesh().n_faces(), mesh().n_cells());
    ImGui::Text("BBox Min (%f, %f, %f)",
        bbox_.min()[0], bbox_.min()[1], bbox_.min()[2]);
    ImGui::Text("BBox Max (%f, %f, %f)",
        bbox_.max()[0], bbox_.max()[1], bbox_.max()[2]);
}

void OpenVolumeMeshObject::draw_selected_ui()
{
    // static const char* modes[]{"Property Selection","Dilation","Erosion"};
    // int mode = static_cast<int>(pick_mode_);
    // if (ImGui::Combo("Mode", &mode, modes, 3)) {
    //     pick_mode_ = static_cast<PickMode>(mode);
    // }
    const auto& gpu_context = scene()->app()->gpu_context();

    switch(pick_mode_)
    {
    case PickMode::PROP_VIS:
    {
        if (ImGui::BeginTabBar("##xx"))
        {
            if (ImGui::BeginTabItem(ICON_FA_EYE_DROPPER))
            {
                if (ImGui::BeginMenu("Select Property")) {
                    if (auto p = prop_vis_.draw_property_selection_menu_ui(mesh())) {
                        visualize_property(p->name, p->entity_type, p->type_name);
                    }
                    ImGui::EndMenu();
                }
                prop_vis_.draw_property_visualization_settings_menu_ui(renderers_);
                ImGui::EndTabItem();
            }
            if (mesh().n_persistent_props<OVM::Entity::Mesh>() && ImGui::BeginTabItem(ICON_FA_GLOBE))
            {
                prop_vis_.draw_mesh_properties_ui(mesh());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(ICON_FA_CALCULATOR))
            {
                if (mesh().n_vertices()>0 && ImGui::BeginMenu("Vertices")){
                    if (ImGui::MenuItem("Vertex Index")) {
                        add_entity_index_prop<OpenVolumeMesh::Entity::Vertex>(mesh());
                    }
                    if (ImGui::MenuItem("Vertex Valence")) {
                        add_vertex_valence_prop(mesh());
                    }
                    ImGui::EndMenu();
                }
                if (mesh().n_edges()>0 && ImGui::BeginMenu("Edges")){
                    if (ImGui::MenuItem("Edge Index")) {
                        add_entity_index_prop<OpenVolumeMesh::Entity::Edge>(mesh());
                    }
                    if (ImGui::MenuItem("Edge Valence")) {
                        add_edge_valence_prop(mesh());
                    }
                    ImGui::EndMenu();
                }
                if (mesh().n_halfedges()>0 && ImGui::BeginMenu("Halfedges")){
                    if (ImGui::MenuItem("Halfedge Index")) {
                        add_entity_index_prop<OpenVolumeMesh::Entity::HalfEdge>(mesh());
                    }
                    ImGui::EndMenu();
                }
                if (mesh().n_faces()>0 && ImGui::BeginMenu("Faces")){
                    if (ImGui::MenuItem("Face Index")) {
                        add_entity_index_prop<OpenVolumeMesh::Entity::Face>(mesh());
                    }
                    ImGui::EndMenu();
                }
                if (mesh().n_halffaces()>0 && ImGui::BeginMenu("Halffaces")){
                    if (ImGui::MenuItem("Halfface Index")) {
                        add_entity_index_prop<OpenVolumeMesh::Entity::HalfFace>(mesh());
                    }
                    if (ImGui::MenuItem("Halfface Boundary")) {
                        auto p = mesh().request_halfface_property<bool>("AxoPlotl::Boundary");
                        mesh().set_persistent(p);
                        for (OVM::HFH hfh : mesh().halffaces()) {
                            p[hfh] = mesh().is_boundary(hfh);
                        }
                    }
                    ImGui::EndMenu();
                }
                if (mesh().n_cells()>0 && ImGui::BeginMenu("Cells")) {
                    if (ImGui::MenuItem("Cell Index")) {
                        add_entity_index_prop<OpenVolumeMesh::Entity::Cell>(mesh());
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        break;
    }
    case PickMode::MINECRAFT_ADD:
    case PickMode::MINECRAFT_SUB:
    {
        auto& bits = renderers_.entity_renderer<OVM::Entity::Cell>().bits();
        ImGui::Text("%zu/%zu", bits.popcount(), bits.num_bits());
        if (pick_mode_ == PickMode::MINECRAFT_ADD && ImGui::Button("Grow k-Ring")) {
            bits.disable_buffer_write();
            int k = std::max(minecraft_state_.k_ring, 1);
            auto frontier = mesh().create_private_cell_property<bool>("", false);
            for (OVM::CH ch : mesh().cells()) {
                frontier[ch] = bits.read_bit(ch.uidx());
            }
            while (k > 0) {
                auto next_frontier = mesh().create_private_cell_property<bool>("", false);
                for (OVM::CH ch : mesh().cells()) {
                    if (frontier[ch]) {
                        for (OVM::CH ch2 : mesh().cell_cells(ch)) {
                            if (!bits.read_bit(ch2.uidx())) {
                                bits.write_bit(gpu_context.device, ch2.uidx(), true);
                                next_frontier[ch2] = true;
                            }
                        }
                    }
                }
                frontier = std::move(next_frontier);
                --k;
            }
            bits.enable_buffer_write();
            bits.write_data(gpu_context.device);
        } else if (pick_mode_ == PickMode::MINECRAFT_SUB && ImGui::Button("Shrink k-Ring")) {
            bits.disable_buffer_write();
            int k = std::clamp(minecraft_state_.k_ring, 1, 5);
            while (k > 0) {
                std::vector<OVM::CH> to_remove;
                for (OVM::CH ch : mesh().cells()) {
                    if (!bits.read_bit(ch.uidx())) {continue;} // already eroded
                    bool boundary = mesh().is_boundary(ch);
                    // Check whether any neighboring cell is eroded.
                    if (!boundary) {
                        for (OVM::CH ch2 : mesh().cell_cells(ch)) {
                            if (!bits.read_bit(ch2.uidx())) {
                                boundary = true;
                                break;
                            }
                        }
                    }
                    if (boundary) {to_remove.push_back(ch);}
                }
                // Apply removals
                for (OVM::CH ch : to_remove) {
                    bits.write_bit(gpu_context.device, ch.uidx(), false);
                }
                --k;
            }
            bits.enable_buffer_write();
            bits.write_data(gpu_context.device);
        }
        ImGui::SameLine();
        if (ImGui::InputInt("k", &minecraft_state_.k_ring)) {
            minecraft_state_.k_ring = std::max(minecraft_state_.k_ring, 1);
        }
        if (ImGui::Button("Invert")) {
            bits.invert_all_bits(gpu_context.device);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            bits.write_all_bits(gpu_context.device, true);
        }
        break;
    }
    default: break;
    }
}

void OpenVolumeMeshObject::init()
{
    const auto& gpu_context = scene()->app()->gpu_context();
    const auto& render_context = scene()->app()->render_context();
    const wgpu::Device& device = gpu_context.device;

    mesh_data_.create_buffers(device);
    renderers_.init(id(), mesh_data_, gpu_context, render_context);
}

void OpenVolumeMeshObject::cleanup()
{
    renderers_.cleanup();
}

void OpenVolumeMeshObject::render(
    wgpu::RenderPassEncoder _render_pass, const ViewSize &_view_size,
    const Mat4x4& _view,
    const Mat4x4& _proj)
{
    if (deleted() || !visible()) {return;}

    const auto& modelview = _view * transform_;
    const auto& mvp = MVP{
        .model = transform_,
        .view = _view,
        .proj = _proj
    };
    const auto& mvp_mat = _proj * modelview;

    renderers_.set_directional_light(scene()->directional_light);
    renderers_.render(_render_pass, mvp, _view_size, picked_pixel_.entity);
}

void OpenVolumeMeshObject::draw_viewport_ui()
{

    //--------------
    // Pick Popup
    //--------------
    if (request_pick_popup_) {
        ImGui::OpenPopup("PickObjectPopup");
        request_pick_popup_ = false;
    }
    if (picked_pixel_.entity.object_id == id() && ImGui::BeginPopup("PickObjectPopup")) {
        const auto& device = scene()->app()->gpu_context().device;

        if (pick_mode_ == PickMode::PROP_VIS) {
            prop_vis_.draw_picked_properties_ui(mesh(), picked_pixel_);

            // Zoom to the picked entity
            if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS)) {
                const float s = std::max<float>(bbox_.diagonal()*0.01f, 0.01f);
                BoundingBox bbox;
                bbox.expand_with_point(picked_pixel_.position-Vec3(s,s,s));
                bbox.expand_with_point(picked_pixel_.position+Vec3(s,s,s));
                scene_->zoom_to_box(bbox);
            }
        }
        ImGui::EndPopup();
    }
}

bool OpenVolumeMeshObject::visualize_property(
    const std::string& _property_name,
    std::optional<OVM::EntityType> _entity_type,
    std::optional<std::string> _type_name)
{
    const auto& gpu_context = scene_->app()->gpu_context();
    const auto& render_context = scene_->app()->render_context();
    return prop_vis_.visualize_property(
        id(), mesh_data_, gpu_context, render_context, renderers_,
        _property_name, _entity_type, _type_name);
}

void OpenVolumeMeshObject::visualize_like(const OpenVolumeMeshObject& _obj)
{
    for (const auto& prop : _obj.prop_vis_.visualized_props()) {
        visualize_property(
            prop->name(),
            prop->entity_type(),
            prop->typeNameWrapper());
    }
    renderers_.configure_like(_obj.renderers_);
}

void OpenVolumeMeshObject::recompute_bounding_box()
{
    bbox_.reset();
    for (const auto& p : mesh().vertex_positions()) {
        bbox_.expand_with_point(p);
    }
}

}

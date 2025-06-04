#include "root.hpp"
#include "app.hpp"
#include "cereal_helpers.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include "with-context.hpp"
#include <SDL_opengl.h>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>
#include <spdlog/spdlog.h>

Root::Root(App &app)
  : Root(app.getLib(), app.getUndo())
{
}

Root::Root(Lib &lib, Undo &aUndo)
  : Node(lib, aUndo, "root") {}

Root::~Root() = default;

auto Root::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Root>(*this);
}

auto Root::render(float dt, Node *hovered, Node *selected) -> void
{
  zOrder = INT_MIN;
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  Node::render(dt, hovered, selected);
}

auto Root::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("BG color");
  ImGui::TableNextColumn();
  auto oldColor = clearColor;
  if (ImGui::ColorEdit4(
        "##BG color", (float *)&clearColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
  {
    undo.get().record(
      [alive = weak_self(), newColor = clearColor]() {
        if (auto self = alive.lock())
        {
          static_cast<Root *>(self.get())->clearColor = std::move(newColor);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      [alive = weak_self(), oldColor]() {
        if (auto self = alive.lock())
        {
          static_cast<Root *>(self.get())->clearColor = std::move(oldColor);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      "##BG color");
  }
}

template <typename Archive>
auto Root::save(Archive &archive) const -> void
{
  archive(
    cereal::make_nvp("Node", cereal::virtual_base_class<Node>(this)),
    cereal::make_nvp("clear_color", clearColor));
}

template <typename Archive>
auto Root::load(Archive &archive) -> void
{
  archive(
    cereal::make_nvp("Node", cereal::virtual_base_class<Node>(this)),
    cereal::make_nvp("clear_color", clearColor));
}

template <typename Archive>
auto Root::load_and_construct(Archive &archive, cereal::construct<Root> &construct) -> void
{
  auto &real_archive = static_cast<WithContext<cereal::JSONInputArchive, App &> &>(archive);
  App &app = real_archive.getContext();

  construct(app);
}

CEREAL_REGISTER_TYPE(Root);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Node, Root)
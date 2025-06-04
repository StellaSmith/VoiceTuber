#include "bouncer.hpp"
#include "app.hpp"
#include "audio-in.hpp"
#include "node.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <cereal/types/base_class.hpp>
#include <limits>
#include <spdlog/spdlog.h>

Bouncer::Bouncer(App &app)
  : Bouncer(app.getLib(), app.getUndo(), app.getAudioIn())
{
}

Bouncer::Bouncer(Lib &lib, Undo &aUndo, class AudioIn &audioIn)
  : Node(lib, aUndo, "bouncer"),
    audioLevel(audioIn)
{
}

auto Bouncer::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Bouncer>(*this);
}

auto Bouncer::render(float dt, Node *hovered, Node *selected) -> void
{
  zOrder = INT_MIN;
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  dLoc.y += std::min(1000.f * dt / 250.f, 1.f) * (strength * audioLevel.getLevel() - dLoc.y);
  Node::render(dt, hovered, selected);
}

auto Bouncer::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("BG color");
  ImGui::TableNextColumn();
  ImGui::ColorEdit4(
    "##BG color", (float *)&clearColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
  ImGui::TableNextColumn();
  Ui::textRj("Bounce");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Bounce",
                strength,
                1.f,
                0.f,
                std::numeric_limits<float>::max(),
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp);
}

template <typename Archive>
auto Bouncer::save(Archive &archive) const -> void
{
  archive(
    cereal::make_nvp("Node", cereal::virtual_base_class<Node>(this)),
    cereal::make_nvp("strength", this->strength),
    cereal::make_nvp("clear_color", this->clearColor));
}

template <typename Archive>
auto Bouncer::load(Archive &archive) -> void
{
  archive(
    cereal::make_nvp("Node", cereal::virtual_base_class<Node>(this)),
    cereal::make_nvp("strength", this->strength),
    cereal::make_nvp("clear_color", this->clearColor));
}
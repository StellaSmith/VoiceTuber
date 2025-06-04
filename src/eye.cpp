#include "eye.hpp"
#include "mouse-tracking.hpp"
#include "ui.hpp"
#include <numbers>

Eye::Eye(MouseTracking &mouseTracking, Lib &lib, Undo &aUndo, const std::filesystem::path &path)
  : AnimSprite(lib, aUndo, path), mouseTracking(mouseTracking)
{
  mouseTracking.reg(*this);
}

Eye::~Eye()
{
  mouseTracking.get().unreg(*this);
}

auto Eye::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Eye>(*this);
}

auto Eye::render(float dt, Node *hovered, Node *selected) -> void
{
  auto clampMouse = [&]() {
    const auto mousePivot = (mouse - pivot()) * followStrength / 100.f;
    const auto distance = glm::length(mousePivot);

    if (distance > radius)
    {
      auto normalizedDirection = glm::normalize(mousePivot);
      return normalizedDirection * radius;
    }
    return mousePivot;
  }();

  dLoc.x = clampMouse.x;
  dLoc.y = clampMouse.y;

  AnimSprite::render(dt, hovered, selected);
  if (selected == this)
  {
    glBegin(GL_LINE_LOOP);
    const auto NumSegments = 100;
    for (auto i = 0; i < NumSegments; ++i)
    {
      const auto theta = 2.0f * std::numbers::pi_v<float> * i / NumSegments;
      const auto dx = radius * cosf(theta);
      const auto dy = radius * sinf(theta);
      glVertex2f(pivot().x + dx, pivot().y + dy);
    }
    glEnd();
  }
}

template <typename Archive>
auto Eye::save(Archive &archive) const -> void
{

  archive(cereal::virtual_base_class<AnimSprite>(this),
          cereal::make_nvp("radius", this->radius),
          cereal::make_nvp("follow_strength", this->followStrength),
          cereal::make_nvp("sprite", this->sprite));
}

template <typename Archive>
auto Eye::load(Archive &archive) -> void
{
  archive(cereal::virtual_base_class<AnimSprite>(this),
          cereal::make_nvp("radius", this->radius),
          cereal::make_nvp("follow_strength", this->followStrength),
          cereal::make_nvp("sprite", this->sprite));
}

auto Eye::renderUi() -> void
{
  AnimSprite::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("Radius");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Radius",
                radius,
                1,
                1,
                std::numeric_limits<float>::max(),
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Strength");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Strength",
                followStrength,
                1,
                1,
                std::numeric_limits<float>::max(),
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp);
}

auto Eye::ingest(const glm::mat4 &projMat, glm::vec2 v) -> void
{
  mouse = screenToLocal(projMat, v);
}

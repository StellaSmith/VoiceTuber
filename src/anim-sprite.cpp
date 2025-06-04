#include "anim-sprite.hpp"
#include "app.hpp"
#include "cereal_helpers.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include "with-context.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

AnimSprite::AnimSprite(App &app, const std::filesystem::path &path)
  : AnimSprite(app.getLib(), app.getUndo(), path)
{
}

AnimSprite::AnimSprite(Lib &lib, Undo &aUndo, const std::filesystem::path &path)
  : Node(lib, aUndo, path.filename().string()),
    sprite(lib, aUndo, path),
    startTime(std::chrono::high_resolution_clock::now()),
    arrowN(lib.queryTex("engine:arrow-n-circle.png", true)),
    arrowNE(lib.queryTex("engine:arrow-ne-circle.png", true)),
    arrowE(lib.queryTex("engine:arrow-e-circle.png", true)),
    arrowSE(lib.queryTex("engine:arrow-se-circle.png", true)),
    arrowS(lib.queryTex("engine:arrow-s-circle.png", true)),
    arrowSW(lib.queryTex("engine:arrow-sw-circle.png", true)),
    arrowW(lib.queryTex("engine:arrow-w-circle.png", true)),
    arrowNW(lib.queryTex("engine:arrow-nw-circle.png", true)),
    center(lib.queryTex("engine:center-circle.png", true))
{
}

static auto getProjectionMatrix() -> glm::mat4
{
  GLfloat projectionMatrixData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrixData);
  return glm::make_mat4(projectionMatrixData);
}

auto AnimSprite::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<AnimSprite>(*this);
}

auto AnimSprite::render(float dt, Node *hovered, Node *selected) -> void
{
  if (sprite.numFrames() > 0)
    sprite.frame(static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(
                                    std::chrono::high_resolution_clock::now() - startTime)
                                    .count() *
                                  fps / 1'000'000) %
                 sprite.numFrames());
  sprite.render();
  Node::render(dt, hovered, selected);
  if (dt <= 0.f)
    return;

  const auto projMat = getProjectionMatrix();
  const auto pivot4 = glm::vec4{pivot().x, pivot().y, 0.f, 1.f};
  const auto projPivot = projMat * modelViewMat * pivot4;
  const auto v = (glm::vec2{projPivot.x, projPivot.y} - lastProjPivot) / dt;
  const auto gForce = glm::vec2{0.f, .3f};
  const auto a = (v - lastProjPivotV) / dt + gForce;
  lastProjPivot = glm::vec2{projPivot.x, projPivot.y};
  lastProjPivotV = v;
  if (!physics)
    return;

  if (glm::length(end - pivot()) < 1.f)
    return;

  const auto end4 = glm::vec4{end.x, end.y, 0.f, 1.f};
  const auto projEnd = projMat * modelViewMat * end4;
  const auto pivotToEnd = glm::vec2(projEnd) - glm::vec2(projPivot);
  const auto orthogonalVec = glm::vec2{-pivotToEnd.y, pivotToEnd.x};
  const auto normalizedOrthogonalVec = glm::normalize(orthogonalVec);
  float projection = glm::dot(a, normalizedOrthogonalVec);
  animRotV += (-force * projection - dRot * springiness - animRotV * damping) * dt;
  dRot += animRotV * dt;

  if (selected != this)
    return;
  glColor4f(1.f, .7f, .0f, 1.f);
  glBegin(GL_LINES);
  glVertex2f(pivot().x, pivot().y);
  glVertex2f(end.x, end.y);
  glEnd();
}

auto AnimSprite::renderUi() -> void
{
  Node::renderUi();
  sprite.renderUi();

  ImGui::TableNextColumn();
  Ui::textRj("FPS");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo, "##FPS", fps, 1, 1, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Physics");
  ImGui::TableNextColumn();
  if (Ui::checkbox(undo, "##Physics", physics))
  {
    animRotV = {};
    dRot = {};
    dLoc = {};
    dScale = {};
  }

  ImGui::TableNextColumn();
  {
    auto physicsDisabled = Ui::Disabled{!physics};
    Ui::textRj("End");
    ImGui::TableNextColumn();
    Ui::dragFloat(undo,
                  "X##End",
                  end.x,
                  1.f,
                  -std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max(),
                  "%.1f");
    Ui::dragFloat(undo,
                  "Y##End",
                  end.y,
                  1.f,
                  -std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max(),
                  "%.1f");
    const auto sz = 2 * ImGui::GetFontSize();
    if (Ui::btnImg("nw2", *arrowNW, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{0, h()}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::SameLine();
    if (Ui::btnImg("n2", *arrowN, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{w() / 2, h()}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = newEnd;
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::SameLine();
    if (Ui::btnImg("ne2", *arrowNE, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{w(), h()}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    if (Ui::btnImg("w2", *arrowW, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{0, h() / 2}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::SameLine();
    if (Ui::btnImg("c2", *center, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{w() / 2, h() / 2}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::SameLine();
    if (Ui::btnImg("e2", *arrowE, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{w(), h() / 2}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    if (Ui::btnImg("sw2", *arrowSW, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{0, 0}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::SameLine();
    if (Ui::btnImg("s2", *arrowS, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{w() / 2, 0}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::SameLine();
    if (Ui::btnImg("se2", *arrowSE, sz, sz))
      undo.get().record(
        [newEnd = glm::vec2{w(), 0}, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(newEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [oldEnd = end, alive = weak_self()]() {
          if (auto self = alive.lock())
          {
            self->end = std::move(oldEnd);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    ImGui::TableNextColumn();
    Ui::textRj("Force");
    ImGui::TableNextColumn();
    Ui::dragFloat(undo,
                  "##Force",
                  force,
                  .1f,
                  0,
                  std::numeric_limits<float>::max(),
                  "%.1f",
                  ImGuiSliderFlags_AlwaysClamp);
    ImGui::TableNextColumn();
    Ui::textRj("Damping");
    ImGui::TableNextColumn();
    Ui::dragFloat(undo,
                  "##Damping",
                  damping,
                  .1f,
                  0,
                  std::numeric_limits<float>::max(),
                  "%.1f",
                  ImGuiSliderFlags_AlwaysClamp);
    ImGui::TableNextColumn();
    Ui::textRj("Springiness");
    ImGui::TableNextColumn();
    Ui::dragFloat(undo,
                  "##Springiness",
                  springiness,
                  .1f,
                  0,
                  std::numeric_limits<float>::max(),
                  "%.1f",
                  ImGuiSliderFlags_AlwaysClamp);
  }
}

template <typename Archive>
auto AnimSprite::save(Archive &archive) const -> void
{
  archive(
    cereal::make_nvp("Node", cereal::virtual_base_class<Node>(this)),
    cereal::make_nvp("fps", this->fps),
    cereal::make_nvp("physics", this->physics),
    cereal::make_nvp("end", this->end),
    cereal::make_nvp("damping", this->damping),
    cereal::make_nvp("force", this->force),
    cereal::make_nvp("springiness", this->springiness),

    cereal::make_nvp("sprite", this->sprite));
}

template <typename Archive>
auto AnimSprite::load(Archive &archive) -> void
{
  archive(

    cereal::make_nvp("Node", cereal::virtual_base_class<Node>(this)),
    cereal::make_nvp("fps", this->fps),
    cereal::make_nvp("physics", this->physics),
    cereal::make_nvp("end", this->end),
    cereal::make_nvp("damping", this->damping),
    cereal::make_nvp("force", this->force),
    cereal::make_nvp("springiness", this->springiness),

    cereal::make_nvp("sprite", this->sprite));
}

template <typename Archive>
auto AnimSprite::load_and_construct(Archive &archive, cereal::construct<AnimSprite> &construct) -> void
{
  auto &real_archive = static_cast<WithContext<cereal::JSONInputArchive, App &> &>(archive);
  App &app = real_archive.getContext();
  construct(app.getLib(), app.getUndo(), "${Node::name}");
}

CEREAL_REGISTER_TYPE(AnimSprite);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Node, AnimSprite);

auto AnimSprite::h() const -> float
{
  return sprite.h();
}

auto AnimSprite::isTransparent(glm::vec2 v) const -> bool
{
  return sprite.isTransparent(v);
}

auto AnimSprite::w() const -> float
{
  return sprite.w();
}

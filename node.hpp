#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <imgui/imgui.h>
#include <memory>
#include <ser/istrm.hpp>
#include <ser/macro.hpp>
#include <ser/ostrm.hpp>
#include <string>
#include <vector>

namespace Internal
{
  auto serVal(OStrm &strm, const glm::vec2 &value) noexcept -> void;
  auto deserVal(IStrm &strm, glm::vec2 &value) noexcept -> void;
  auto serVal(OStrm &strm, const ImVec4 &value) noexcept -> void;
  auto deserVal(IStrm &strm, ImVec4 &value) noexcept -> void;
} // namespace Internal
#include <ser/ser.hpp>

class Node
{
public:
#define SER_PROP_LIST       \
  SER_PROP(loc);            \
  SER_PROP(scale);          \
  SER_PROP(pivot);          \
  SER_PROP(rot);            \
  SER_PROP(uniformScaling); \
  SER_PROP(zOrder);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  using Nodes = std::vector<std::unique_ptr<Node>>;
  Node(std::string name);
  virtual ~Node() = default;

  auto addChild(std::unique_ptr<Node>) -> void;
  auto getName() const -> std::string;
  auto getNodes() const -> const Nodes &;
  auto loadAll(const class SaveFactory &, IStrm &) -> void;
  auto moveDown() -> void;
  auto moveUp() -> void;
  auto nodeUnder(const glm::mat4 &projMat, glm::vec2) -> Node *;
  auto parent() -> Node *;
  auto parentWithBellow() -> void;
  auto renderAll(float dt, Node *hovered, Node *selected) -> void;
  auto rotCancel() -> void;
  auto rotStart(glm::vec2 mouse) -> void;
  auto rotUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void;
  auto saveAll(OStrm &) const -> void;
  auto scaleCancel() -> void;
  auto scaleStart(glm::vec2 mouse) -> void;
  auto scaleUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void;
  auto translateCancel() -> void;
  auto translateStart(glm::vec2 mouse) -> void;
  auto translateUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void;
  auto unparent() -> void;
  static auto del(Node &) -> void;
  virtual auto h() const -> float;
  virtual auto isTransparent(glm::vec2) const -> bool;
  virtual auto renderUi() -> void;
  virtual auto w() const -> float;

protected:
  auto screenToLocal(const glm::mat4 &projMat, glm::vec2) const -> glm::vec2;
  virtual auto load(IStrm &) -> void;
  virtual auto render(float dt, Node *hovered, Node *selected) -> void;
  virtual auto save(OStrm &) const -> void;

  std::string name;

private:
  auto collectUnderNodes(const glm::mat4 &projMat,
                         glm::vec2 v,
                         std::vector<std::reference_wrapper<Node>> &) -> void;
  auto getAllNodesCalcModelView(std::vector<std::reference_wrapper<Node>> &) -> void;

  glm::vec2 loc = {.0f, .0f};
  glm::vec2 scale = {1.f, 1.f};

protected:
  glm::vec2 pivot = {.0f, .0f};
  float animRot = 0.f;

private:
  float rot = 0.f;
  bool uniformScaling = true;

protected:
  int zOrder = 0;

private:
  Nodes nodes;

protected:
  glm::mat4 modelViewMat;

private:
  Node *parent_ = nullptr;
  glm::vec2 startMousePos;
  glm::vec2 initLoc;
  glm::vec2 initScale;
  float initRot;
};

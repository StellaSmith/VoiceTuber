#pragma once

#include "node.hpp"
#include <cereal/access.hpp>
#include <imgui.h>

class Root final : public Node
{
public:
  static constexpr const char *className = "Root";
  Root(class App &);
  Root(class Lib &, Undo &);
  ~Root() final;

private:
  ImVec4 clearColor = ImVec4(123.f / 256.f, 164.f / 256.f, 119.f / 256.f, 1.00f);
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto do_clone() const -> std::shared_ptr<Node> final;

private:
  friend cereal::access;

  template <typename Archive>
  auto save(Archive &archive) const -> void;
  template <typename Archive>
  auto load(Archive &archive) -> void;
  template <typename Archive>
  static auto load_and_construct(Archive &archive, cereal::construct<Root> &construct) -> void;
};

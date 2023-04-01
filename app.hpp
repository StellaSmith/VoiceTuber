#pragma once
#include "anim-sprite.hpp"
#include "audio-capture.hpp"
#include "bouncer.hpp"
#include "mouth.hpp"
#include "wav-2-visemes.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

class App
{
public:
  App();
  auto render() -> void;
  auto renderUi() -> void;
  auto tick() -> void;

private:
  ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  Viseme curViseme = Viseme::sil;
  Wav2Visemes wav2Visemes;
  AudioCapture audioCapture;
  Bouncer root;
  AnimSprite face;
  Mouth mouth;
  Node *hovered = nullptr;
  Node *selected = nullptr;
};

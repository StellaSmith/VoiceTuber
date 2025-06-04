#pragma once

#include <cereal/cereal.hpp>
#include <glm/vec2.hpp>
#include <imgui.h>

namespace cereal
{
  template <typename Archive, typename T, glm::length_t L, glm::qualifier Q>
  auto serialize(Archive &archive, glm::vec<L, T, Q> &vector) -> void
  {
    cereal::size_type size = L;
    archive(cereal::make_size_tag(size));
    for (size_type i = 0; i < size; ++i)
    {
      if (i >= L)
      {
        throw cereal::Exception("glm::vec serialization error: index out of bounds");
      }
      archive(vector[i]);
    }
  }

  template <typename Archive>
  auto serialize(Archive &archive, ImVec2 &vector) -> void
  {

    cereal::size_type size = 2;
    archive(make_size_tag(size));

    for (size_type i = 0; i < size; ++i)
    {
      switch (i)
      {
      case 0:
        archive(vector.x);
        break;
      case 1:
        archive(vector.y);
        break;
      default:
        throw cereal::Exception("ImVec2 serialization error: index out of bounds");
      }
    }
  }

  template <typename Archive>
  auto serialize(Archive &archive, ImVec4 &vector) -> void
  {

    cereal::size_type size = 2;
    archive(make_size_tag(size));

    for (size_type i = 0; i < size; ++i)
    {
      switch (i)
      {
      case 0:
        archive(vector.x);
        break;
      case 1:
        archive(vector.y);
        break;
      case 2:
        archive(vector.z);
        break;
      case 3:
        archive(vector.w);
        break;
      default:
        throw cereal::Exception("ImVec4 serialization error: index out of bounds");
      }
    }
  }

} // namespace cereal

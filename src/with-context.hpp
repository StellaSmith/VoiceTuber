#pragma once

#include <utility>

template <typename T, typename Context>
class WithContext : public T
{
public:
  template <typename... Args>
  WithContext(Context context, Args &&...args)
    : T(std::forward<Args>(args)...), m_context(std::move(context))
  {
  }

  Context &getContext() { return m_context; }
  const Context &getContext() const { return m_context; }

private:
  Context m_context;
};

template <typename T, typename Context>
class WithContext<T, Context &> : public T
{
public:
  template <typename... Args>
  WithContext(Context &context, Args &&...args)
    : T(std::forward<Args>(args)...), m_context(context)
  {
  }

  Context &getContext() { return m_context; }
  const Context &getContext() const { return m_context; }

private:
  Context &m_context;
};
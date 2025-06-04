#pragma once

struct Test
{
  const char *name = nullptr;
  virtual void invoke() const = 0;

  Test *next;
  static Test *head;

  Test(const char *);
};

#define TEST_IMPLEMENTATION(id, name, ...) \
  namespace                                \
  {                                        \
    struct TestImplementation##id final    \
      : Test                               \
    {                                      \
      TestImplementation##id()             \
        : Test(name) {}                    \
      void invoke() const final;           \
    };                                     \
  }                                        \
  static TestImplementation##id test##id;  \
  void TestImplementation##id::invoke() const

#define TEST_EXPAND(id, name, ...) TEST_IMPLEMENTATION(id, name, __VA_ARGS__)
#define TEST(name, ...) TEST_EXPAND(__COUNTER__, name, __VA_ARGS__)
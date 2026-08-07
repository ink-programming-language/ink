#include <gmock/gmock.h>

#include <vector>

TEST(DemoTest, DiscoversTestsInNestedDirectories) {
  const std::vector<int> values{1, 2, 3};

  EXPECT_THAT(values, ::testing::ElementsAre(1, 2, 3));
}

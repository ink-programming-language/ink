#include <gmock/gmock.h>

#include <vector>

// Verifies that the unified test target discovers tests in nested testcase directories.
TEST(DemoTest, DiscoversTestsInNestedDirectories)
{
  const std::vector<int> Values{1, 2, 3};

  EXPECT_THAT(Values, ::testing::ElementsAre(1, 2, 3));
}

// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), n))
  {
    var emp: dynamic = cpp_array(4001);
    var nums: dynamic = cpp_uninitialized();
    var exist: dynamic = false;
    memset(emp, 0, cpp_sizeof((emp)));
    rep(i, nums.size());
    {
      if ((emp[nums[i]] >= 1000000))
      {
        exist = true;
        write(nums[i], "\n");
      }
    }
    if ((!exist))
    {
      write("NA", "\n");
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var num: dynamic = cpp_uninitialized();
      var val: dynamic = cpp_uninitialized();
      var am: dynamic = cpp_uninitialized();
      read(num, val, am);
      emp[num] += (val * am);
      if ((find(nums.begin(), nums.end(), num) == nums.end()))
      {
        nums.push_back(num);
      }
    }

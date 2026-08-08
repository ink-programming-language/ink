// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    var emp = cpp_array(4001);
    var nums: dynamic;
    var exist = false;
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var num: dynamic;
      var val: dynamic;
      var am: dynamic;
      read(num, val, am);
      emp[num] += (val * am);
      if ((find(nums.begin(), nums.end(), num) == nums.end()))
      {
        nums.push_back(num);
      }
    }

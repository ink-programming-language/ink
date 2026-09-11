// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(5005, 5005);

func get(i: dynamic, j: dynamic) -> dynamic
{
  if ((dp[i][j] != -1))
  {
    return dp[i][j];
  }
  if (cpp_binary((i >= s.size()), "or", (j >= t.size())))
  {
    return 0;
  }
  var ret: dynamic = get(i, (j + 1));
  if ((s[i] == t[j]))
  {
    ret = ((((ret + 1) + get((i + 1), (j + 1)))) % 1000000007);
  }
  return cpp_assign(dp[i][j], "=", ret);
}

func main() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 5005))
    {
      {
        var j: dynamic = 0;
        while ((j < 5005))
        {
          dp[i][j] = -1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  read(s, t);
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      ret = (((ret + get(i, 0))) % 1000000007);
      i += 1;
    }
  }
  write(ret, "\n");
}

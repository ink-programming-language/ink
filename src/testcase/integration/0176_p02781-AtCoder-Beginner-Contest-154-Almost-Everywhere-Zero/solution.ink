// Translated from solution.cpp.

var dp: dynamic = cpp_array(5, 100, 20005);

var s: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func Rec(index: dynamic, zeros: dynamic, flag: dynamic) -> dynamic
{
  if ((zeros > k))
  {
    return 0;
  }
  if ((index == n))
  {
    return (zeros == k);
  }
  if ((dp[index][zeros][flag] + 1))
  {
    return dp[index][zeros][flag];
  }
  var Res: dynamic = 0;
  var Limit: dynamic =  (flag) ? 9 : s[index];
  {
    var i: dynamic = 0;
    while ((i <= Limit))
    {
      Res += Rec((index + 1), (zeros + ((i != 0))),  (((!flag) && (i == s[index]))) ? 0 : 1);
      i += 1;
    }
  }
  return cpp_assign(dp[index][zeros][flag], "=", Res);
}

func main() -> dynamic
{
  read(s, k);
  n = s.length();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      s[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
  memset(dp, -1, cpp_sizeof((dp)));
  write(Rec(0, 0, 0));
  return 0;
}

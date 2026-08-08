// Translated from solution.cpp.

var dp = cpp_array(5, 100, 20005);

var s: dynamic;

var n: dynamic;

var k: dynamic;

func Rec(index: dynamic, zeros: dynamic, flag: dynamic)
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
  var Res = 0;
  var Limit = if (flag) 9 else s[index];
  {
    var i = 0;
    while ((i <= Limit))
    {
      Res += Rec((index + 1), (zeros + ((i != 0))), if (((!flag) && (i == s[index]))) 0 else 1);
      i += 1;
    }
  }
  return cpp_assign(dp[index][zeros][flag], "=", Res);
}

func main()
{
  read(s, k);
  n = s.length();
  {
    var i = 0;
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

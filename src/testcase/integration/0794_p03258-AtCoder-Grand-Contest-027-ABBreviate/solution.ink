// Translated from solution.cpp.

var MAXN: dynamic = (2e5 + 5);

var P: dynamic = (1e9 + 7);

func read(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  {
    while ((!isdigit(c)))
    {
      if ((c == cpp_char("-")))
      {
        f = (-f);
      }
      c = getchar();
    }
  }
  {
    while (isdigit(c))
    {
      x = (((x * 10) + c) - cpp_char("0"));
      c = getchar();
    }
  }
  x *= f;
}

var s: dynamic = cpp_array(MAXN);

var dp: dynamic = cpp_array(MAXN);

var pre: dynamic = cpp_array(MAXN);

var lst: dynamic = cpp_array(3, MAXN);

var trans: dynamic = cpp_array(3, MAXN);

func update(x: dynamic, y: dynamic) -> dynamic
{
  x += y;
  if ((x >= P))
  {
    x -= P;
  }
}

func main() -> dynamic
{
  scanf("%s", (s + 1));
  var n: dynamic = strlen((s + 1));
  memset(lst[0], -1, cpp_sizeof((lst[0])));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((s[i] == cpp_char("a")))
      {
        pre[i] = (((pre[(i - 1)] + 1)) % 3);
      } else
      {
        pre[i] = (((pre[(i - 1)] + 2)) % 3);
      }
      memcpy(lst[i], lst[(i - 1)], cpp_sizeof((lst[(i - 1)])));
      lst[i][pre[(i - 1)]] = (i - 1);
      i += 1;
    }
  }
  memset(trans[0], -1, cpp_sizeof((trans[0])));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((i != 1) && (s[(i - 1)] == s[i])))
      {
        memcpy(trans[i], lst[i], cpp_sizeof((lst[i])));
      } else
      {
        memcpy(trans[i], trans[(i - 1)], cpp_sizeof((trans[(i - 1)])));
      }
      i += 1;
    }
  }
  if ((trans[n][0] == -1))
  {
    write(1, "\n");
    return 0;
  }
  dp[n] = 1;
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      update(dp[(i - 1)], dp[i]);
      if ((s[i] == cpp_char("a")))
      {
        if ((trans[i][(((pre[i] + 1)) % 3)] != -1))
        {
          update(dp[trans[i][(((pre[i] + 1)) % 3)]], dp[i]);
        }
      } else
      {
        if ((trans[i][(((pre[i] + 2)) % 3)] != -1))
        {
          update(dp[trans[i][(((pre[i] + 2)) % 3)]], dp[i]);
        }
      }
      i -= 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      if ((pre[i] == 0))
      {
        ans = (((ans + dp[i])) % P);
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

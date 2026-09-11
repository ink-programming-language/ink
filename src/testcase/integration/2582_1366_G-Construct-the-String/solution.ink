// Translated from solution.cpp.

var INF: dynamic = int_cpp(1e9);

func setmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func main() -> dynamic
{
  var from_cpp: dynamic = cpp_uninitialized();
  var need: dynamic = cpp_uninitialized();
  read(from_cpp, need);
  var m: dynamic = int_cpp(from_cpp.size());
  var n: dynamic = int_cpp(need.size());
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      balance[i] = (( (i) ? balance[(i - 1)] : m) + ( ((from_cpp[i] == cpp_char("."))) ? -1 : 1));
      i += 1;
    }
  }
  var last_seen: dynamic = cpp_construct(((2 * m) + 1), -1);
  var nxt: dynamic = cpp_construct(m, -1);
  {
    var i: dynamic = (m - 1);
    while ((i >= 0))
    {
      if ((from_cpp[i] != cpp_char(".")))
      {
        nxt[i] = last_seen[ (i) ? balance[(i - 1)] : m];
      }
      last_seen[balance[i]] = i;
      i -= 1;
    }
  }
  var dp: dynamic = cpp_construct((m + 1), vector((n + 1), INF));
  dp[0][0] = 0;
  {
    var pref1: dynamic = 0;
    while ((pref1 <= m))
    {
      {
        var pref2: dynamic = 0;
        while ((pref2 <= n))
        {
          if ((dp[pref1][pref2] == INF))
          {
            pref2 += 1;
            continue;
          }
          if ((((pref2 != n) && (pref1 != m)) && (from_cpp[pref1] == need[pref2])))
          {
            setmin(dp[(pref1 + 1)][(pref2 + 1)], dp[pref1][pref2]);
          }
          if ((pref1 != m))
          {
            setmin(dp[(pref1 + 1)][pref2], (dp[pref1][pref2] + 1));
          }
          if (((pref1 != m) && (nxt[pref1] != -1)))
          {
            setmin(dp[(nxt[pref1] + 1)][pref2], dp[pref1][pref2]);
          }
          pref2 += 1;
        }
      }
      pref1 += 1;
    }
  }
  write(dp[m][n], cpp_char("\n"));
}

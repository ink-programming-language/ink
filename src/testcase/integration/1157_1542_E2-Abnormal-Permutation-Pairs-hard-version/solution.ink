// Translated from solution.cpp.

var alpha: dynamic = cpp_expression("#include <bits/stdc++.h>");

var B: dynamic = 130000;

var w: dynamic = cpp_array(((2 * B) + 5), 2);

var s: dynamic = cpp_array(((2 * B) + 5), 2);

var ans: dynamic = cpp_array(505);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var MOD: dynamic = cpp_uninitialized();
  read(n, MOD);
  w[0][B] = cpp_assign(s[0][B], "=", 1);
  {
    var i: dynamic = B;
    while ((i <= (2 * B)))
    {
      s[0][i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var curs: dynamic = 1;
      var I: dynamic = (i & 1);
      var J: dynamic = (I ^ 1);
      memset(w[I], 0, cpp_sizeof((w[I])));
      memset(s[I], 0, cpp_sizeof((s[I])));
      var u: dynamic = ((i * ((i - 1))) / 2);
      {
        var j: dynamic = ((-u) + B);
        while ((j <= (u + B)))
        {
          w[I][j] = curs;
          curs = ((((((((0 + curs) - s[J][j]) + s[J][(j - i)]) + s[J][(j + i)]) - s[J][j]) + (2 * MOD))) % MOD);
          j += 1;
        }
      }
      {
        var j: dynamic = (B - ((i * ((i - 1))) / 2));
        var v: dynamic = (((((i + 2)) * ((i + 1))) / 2) + B);
        while ((j <= v))
        {
          s[I][j] = (((s[I][(j - 1)] + w[I][j])) % MOD);
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j < i))
        {
          ans[i] = (((ans[i] + (((1 * (((s[J][(((((i + 1)) * i) / 2) + B)] - s[J][(j + B)]) + MOD))) % MOD) * ((i - j))))) % MOD);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      ans[i] = (((ans[i] + ((1 * i) * ans[(i - 1)]))) % MOD);
      i += 1;
    }
  }
  write(ans[n]);
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  alpha;
  freopen("input.txt", "r", stdin);
  freopen("error.txt", "w", stderr);
  freopen("output.txt", "w", stdout);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
    write(cpp_char("\n"));
  }
  write("time taken : ", (cpp_cast(clock()) / CLOCKS_PER_SEC), " secs", "\n");
  return 0;
}

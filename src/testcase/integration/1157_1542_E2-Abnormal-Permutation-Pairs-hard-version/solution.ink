// Translated from solution.cpp.

var alpha = cpp_expression("#include <bits/stdc++.h>");

var B = 130000;

var w = cpp_array(((2 * B) + 5), 2);

var s = cpp_array(((2 * B) + 5), 2);

var ans = cpp_array(505);

func solve()
{
  var n: dynamic;
  var MOD: dynamic;
  read(n, MOD);
  w[0][B] = cpp_assign(s[0][B], "=", 1);
  {
    var i = B;
    while ((i <= (2 * B)))
    {
      s[0][i] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var curs = 1;
      var I = (i & 1);
      var J = (I ^ 1);
      memset(w[I], 0, cpp_sizeof((w[I])));
      memset(s[I], 0, cpp_sizeof((s[I])));
      var u = ((i * ((i - 1))) / 2);
      {
        var j = ((-u) + B);
        while ((j <= (u + B)))
        {
          w[I][j] = curs;
          curs = ((((((((0 + curs) - s[J][j]) + s[J][(j - i)]) + s[J][(j + i)]) - s[J][j]) + (2 * MOD))) % MOD);
          j += 1;
        }
      }
      {
        var j = (B - ((i * ((i - 1))) / 2));
        var v = (((((i + 2)) * ((i + 1))) / 2) + B);
        while ((j <= v))
        {
          s[I][j] = (((s[I][(j - 1)] + w[I][j])) % MOD);
          j += 1;
        }
      }
      {
        var j = 1;
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
    var i = 2;
    while ((i <= n))
    {
      ans[i] = (((ans[i] + ((1 * i) * ans[(i - 1)]))) % MOD);
      i += 1;
    }
  }
  write(ans[n]);
}

func main(argc: dynamic, argv: dynamic)
{
  alpha;
  freopen("input.txt", "r", stdin);
  freopen("error.txt", "w", stderr);
  freopen("output.txt", "w", stdout);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
    write(cpp_char("\n"));
  }
  write("time taken : ", (cpp_cast(clock()) / CLOCKS_PER_SEC), " secs", "\n");
  return 0;
}

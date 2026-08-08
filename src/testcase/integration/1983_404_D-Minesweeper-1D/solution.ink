// Translated from solution.cpp.

var maxn = (1e6 + 10);

var mod = (1e9 + 7);

var f = cpp_array(2, 2, maxn);

var len: dynamic;

var s = cpp_array(maxn);

func dfs(pos: dynamic, pre: dynamic, next: dynamic)
{
  if ((f[pos][pre][next] != -1))
  {
    return f[pos][pre][next];
  }
  if ((pos == ((len + 1))))
  {
    return (next == 0);
  }
  var ans = 0;
  if (((next == 0) && (s[pos] == cpp_char("*"))))
  {
    return 0;
  }
  if ((((next == 1) && (s[pos] != cpp_char("*"))) && (s[pos] != cpp_char("?"))))
  {
    return 0;
  }
  {
    var i = 0;
    while ((i <= 1))
    {
      var nxt = i;
      if (((s[pos] == cpp_char("1")) && (((nxt + pre)) != 1)))
      {
        i += 1;
        continue;
      }
      if (((s[pos] == cpp_char("2")) && (((nxt + pre)) != 2)))
      {
        i += 1;
        continue;
      }
      if (((s[pos] == cpp_char("0")) && (((nxt + pre)) != 0)))
      {
        i += 1;
        continue;
      }
      ans += dfs((pos + 1), next, nxt);
      ans %= mod;
      i += 1;
    }
  }
  f[pos][pre][next] = ans;
  return ans;
}

func main()
{
  scanf("%s", (s + 1));
  len = strlen((s + 1));
  memset(f, -1, cpp_sizeof(f));
  write((((dfs(1, 0, 0) + dfs(1, 0, 1))) % mod), "\n");
  return 0;
}

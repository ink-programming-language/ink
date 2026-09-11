// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 10);

var mod: dynamic = (1e9 + 7);

var f: dynamic = cpp_array(2, 2, maxn);

var len: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(maxn);

func dfs(pos: dynamic, pre: dynamic, next: dynamic) -> dynamic
{
  if ((f[pos][pre][next] != -1))
  {
    return f[pos][pre][next];
  }
  if ((pos == ((len + 1))))
  {
    return (next == 0);
  }
  var ans: dynamic = 0;
  if (((next == 0) && (s[pos] == cpp_char("*"))))
  {
    return 0;
  }
  if ((((next == 1) && (s[pos] != cpp_char("*"))) && (s[pos] != cpp_char("?"))))
  {
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i <= 1))
    {
      var nxt: dynamic = i;
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

func main() -> dynamic
{
  scanf("%s", (s + 1));
  len = strlen((s + 1));
  memset(f, -1, cpp_sizeof(f));
  write((((dfs(1, 0, 0) + dfs(1, 0, 1))) % mod), "\n");
  return 0;
}

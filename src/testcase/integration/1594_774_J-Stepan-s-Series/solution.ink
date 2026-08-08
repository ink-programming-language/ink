// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var s: dynamic;

var cache = cpp_array(105, 101);

func dp(pos: dynamic, tk: dynamic)
{
  if ((tk > k))
  {
    return -1e18;
  }
  if ((pos == n))
  {
    return (tk == k);
  }
  var ans = (tk == k);
  if ((cache[pos][tk] != -1))
  {
    return cache[pos][tk];
  }
  if ((s[pos] != cpp_char("?")))
  {
    return cpp_assign(cache[pos][tk], "=", (ans + dp((pos + 1), if (((s[pos] == cpp_char("N")))) (tk + 1) else 0)));
  }
  return cpp_assign(cache[pos][tk], "=", (ans + max(dp((pos + 1), (tk + 1)), dp((pos + 1), 0))));
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k, s);
  memset(cache, -1, cpp_sizeof((cache)));
  write((if (((dp(0, 0) > 0))) "YES" else "NO"));
}

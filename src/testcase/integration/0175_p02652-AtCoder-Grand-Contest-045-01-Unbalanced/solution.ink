// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

func ROF(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i>=b;i--)");
}

var N: dynamic = (1e6 + 7);

var INF: dynamic = 0x3f3f3f3f;

var sgn: dynamic = [-1, 1];

var a: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(N);

var nxt: dynamic = cpp_array(N);

func solve(x: dynamic, cnt: dynamic = 0) -> dynamic
{
  var w: dynamic = 0;
  FOR(i, 1, n);
  {
    if (((a[i] == cpp_char("?")) && (((cnt + 2) + nxt[i]) <= x)))
    {
      cnt += 2;
    }
    w = min(w, (s[i] + cnt));
  }
  return (x - w);
}

func main() -> dynamic
{
  scanf("%s", (a + 1));
  n = strlen((a + 1));
  nxt[(n + 1)] = (-INF);
  FOR(i, 1, n)[i] = (s[(i - 1)] + sgn[(a[i] == cpp_char("1"))]);
  ROF(i, n, 0)[i] = max(nxt[(i + 1)], s[i]);
  write(min(solve(nxt[0]), solve((nxt[0] + 1))));
  return 0;
}

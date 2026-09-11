// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(n, k, b, t);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var sum: dynamic = 0;
  var a: dynamic = cpp_array(((2 * n) + cpp_cast(10)));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      a[i] = cpp_assign(a[(i + n)], "=", ( (((s[i] == cpp_char("w")))) ? ((b + 1)) : 1));
      sum += a[i];
      i += 1;
    }
  }
  sum -= a[0];
  var l: dynamic = 1;
  var r: dynamic = n;
  var ans: dynamic = 0;
  while ((((l <= n)) && ((r < (2 * n)))))
  {
    sum += a[r];
    r += 1;
    while (((((r - l)) > n) || (((sum + (((((r - l) - 1) + min(((r - n) - 1), (n - l)))) * k))) > t)))
    {
      sum -= a[cpp_update(l, "++")];
    }
    ans = max(ans, (r - l));
  }
  write(ans);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
}

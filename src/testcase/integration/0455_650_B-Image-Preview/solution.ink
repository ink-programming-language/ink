// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  var k: dynamic;
  var b: dynamic;
  var t: dynamic;
  read(n, k, b, t);
  var s: dynamic;
  read(s);
  var sum = 0;
  var a = cpp_array(((2 * n) + cpp_cast(10)));
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = cpp_assign(a[(i + n)], "=", (if (((s[i] == cpp_char("w")))) ((b + 1)) else 1));
      sum += a[i];
      i += 1;
    }
  }
  sum -= a[0];
  var l = 1;
  var r = n;
  var ans = 0;
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
}

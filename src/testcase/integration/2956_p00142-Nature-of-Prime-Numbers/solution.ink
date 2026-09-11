// Translated from solution.cpp.

func reps(v: dynamic, f: dynamic, l: dynamic) -> dynamic
{
  cpp_macro("for (int v = (f), v##_ = (l); v < v##_; ++v)");
}

func rep(v: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <bit");
}

func lep(v: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

func in_cpp() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  scanf("%d", (&x));
  return x;
}

func inl() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  scanf("%lld", (&x));
  return x;
}

func show(a: dynamic, del: dynamic = cpp_char("\n"), last: dynamic = cpp_char("\n")) -> dynamic
{
  ((rep(i, (a.size() - 1)) << a[i]) << del);
  write(a[(a.size() - 1)], last);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_assign(n, "=", in_cpp()))
  {
    var v: dynamic = cpp_uninitialized();
    var counter: dynamic = cpp_construct((n + 1), 0);
    lep(i, (n - 1)).push_back(((i * i) % n));
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
    rep(i, v.size());
    {
      rep(j, v.size());
      {
        if ((i == j))
        {
          continue;
        }
        var x: dynamic = (v[i] - v[j]);
        if ((x < 0))
        {
          x += n;
        }
        if ((x > (((n - 1)) / 2)))
        {
          x = (n - x);
        }
        counter[x] += 1;
      }
    }
    lep(i, (((n - 1)) / 2));
    {
      printf("%d\n", counter[i]);
    }
  }
  return 0;
}

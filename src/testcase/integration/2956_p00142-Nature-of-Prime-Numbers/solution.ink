// Translated from solution.cpp.

func reps(v: dynamic, f: dynamic, l: dynamic)
{
  cpp_macro("for (int v = (f), v##_ = (l); v < v##_; ++v)");
}

func rep(v: dynamic, n: dynamic)
{
  return cpp_expression("#include <bit");
}

func lep(v: dynamic, n: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func in_cpp()
{
  var x: dynamic;
  scanf("%d", (&x));
  return x;
}

func inl()
{
  var x: dynamic;
  scanf("%lld", (&x));
  return x;
}

func show(a: dynamic, del: dynamic = cpp_char("\n"), last: dynamic = cpp_char("\n"))
{
  ((rep(i, (a.size() - 1)) << a[i]) << del);
  write(a[(a.size() - 1)], last);
}

func main()
{
  var n: dynamic;
  while (cpp_assign(n, "=", in_cpp()))
  {
    var v: dynamic;
    var counter = cpp_construct((n + 1), 0);
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
        var x = (v[i] - v[j]);
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

// Translated from solution.cpp.

func gi()
{
  var w = 0;
  var q = 1;
  var c = getchar();
  while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    q = 0;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    w = (((w * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return if (q) w else (-w);
}

var N = (2e5 + 100);

var a = cpp_array(N);

var b = cpp_array(N);

var c = cpp_array(N);

var fa = cpp_array(N);

func find(x: dynamic)
{
  return if ((fa[x] == x)) x else cpp_assign(fa[x], "=", find(fa[x]));
}

func main()
{
  var n = gi();
  var i: dynamic;
  var m = 0;
  var tot: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      a[0] ^= cpp_assign(a[i], "=", gi());
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      b[0] ^= cpp_assign(b[i], "=", gi());
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= n))
    {
      if ((a[i] ^ b[i]))
      {
        tot[cpp_assign(c[cpp_update(m, "++")], "=", a[i])] += 1;
        tot[cpp_assign(c[cpp_update(m, "++")], "=", b[i])] -= 1;
      }
      i += 1;
    }
  }
  for (var p in tot)
  {
    if (p.second)
    {
      return cpp_comma(puts("-1"), 0);
    }
  }
  sort(c, (c + m));
  m = (unique(c, (c + m)) - c);
  {
    i = 0;
    while ((i < m))
    {
      fa[i] = i;
      i += 1;
    }
  }
  var ans = m;
  {
    i = 1;
    while ((i <= n))
    {
      if ((a[i] ^ b[i]))
      {
        a[i] = (lower_bound(c, (c + m), a[i]) - c);
        b[i] = (lower_bound(c, (c + m), b[i]) - c);
        if ((find(a[i]) == find(b[i])))
        {
          ans += 1;
        } else
        {
          fa[find(a[i])] = find(b[i]);
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      if ((c[i] == a[0]))
      {
        ans -= 1;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}

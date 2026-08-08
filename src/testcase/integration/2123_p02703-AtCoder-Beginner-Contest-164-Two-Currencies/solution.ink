// Translated from solution.cpp.

class nobe
{
  var v: dynamic;
  var a: dynamic;
  var b: dynamic;
}

var gg = cpp_array(55);

var c = cpp_array(55);

var d = cpp_array(55);

var f = cpp_array(3030, 55);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var s: dynamic;
  var i = 1;
  var j = 0;
  scanf("%d%d%d", (&n), (&m), (&s));
  s = min(s, 2500);
  while ((i <= m))
  {
    var u: dynamic;
    var v: dynamic;
    var a: dynamic;
    var b: dynamic;
    scanf("%d%d%d%d", (&u), (&v), (&a), (&b));
    gg[u].push_back([v, a, b]);
    gg[v].push_back([u, a, b]);
    i += 1;
  }
  i = 1;
  while ((i <= n))
  {
    scanf("%d%d", (&c[i]), (&d[i]));
    i += 1;
  }
  memset(f, 63, cpp_sizeof((f)));
  f[1][s] = 0;
  var qu: dynamic;
  qu.push(tt(0, 1, s));
  while ((!qu.empty()))
  {
    var t = get(qu.top());
    var u = get(qu.top());
    var w = get(qu.top());
    qu.pop();
    if ((f[u][w] > t))
    {
      continue;
    }
    i = 0;
    while ((i < gg[u].size()))
    {
      var v = gg[u][i].v;
      var a = gg[u][i].a;
      var b = gg[u][i].b;
      if ((((w >= a)) && ((f[v][(w - a)] > (t + b)))))
      {
        f[v][(w - a)] = (t + b);
        qu.push(tt((t + b), v, (w - a)));
      }
      i += 1;
    }
    if ((f[u][min((w + c[u]), 2500)] > (t + d[u])))
    {
      f[u][min((w + c[u]), 2500)] = (t + d[u]);
      qu.push(tt((t + d[u]), u, min((w + c[u]), 2500)));
    }
  }
  i = 2;
  while ((i <= n))
  {
    var ans = 999999999999999999;
    j = 0;
    while ((j <= 2500))
    {
      ans = min(ans, f[i][j]);
      j += 1;
    }
    printf("%lld\n", ans);
    i += 1;
  }
  return 0;
}

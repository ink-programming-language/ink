// Translated from solution.cpp.

class nobe
{
  var v: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

var gg: dynamic = cpp_array(55);

var c: dynamic = cpp_array(55);

var d: dynamic = cpp_array(55);

var f: dynamic = cpp_array(3030, 55);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var i: dynamic = 1;
  var j: dynamic = 0;
  scanf("%d%d%d", (&n), (&m), (&s));
  s = min(s, 2500);
  while ((i <= m))
  {
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
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
  var qu: dynamic = cpp_uninitialized();
  qu.push(tt(0, 1, s));
  while ((!qu.empty()))
  {
    var t: dynamic = get(qu.top());
    var u: dynamic = get(qu.top());
    var w: dynamic = get(qu.top());
    qu.pop();
    if ((f[u][w] > t))
    {
      continue;
    }
    i = 0;
    while ((i < gg[u].size()))
    {
      var v: dynamic = gg[u][i].v;
      var a: dynamic = gg[u][i].a;
      var b: dynamic = gg[u][i].b;
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
    var ans: dynamic = 999999999999999999;
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

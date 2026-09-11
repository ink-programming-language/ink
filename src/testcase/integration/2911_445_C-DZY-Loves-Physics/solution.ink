// Translated from solution.cpp.

var vis: dynamic = cpp_array(1000);

var q: dynamic = cpp_array(1000);

var node: dynamic = cpp_array(1000);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000, 1000);

var m: dynamic = cpp_uninitialized();

var ans: dynamic = 0;

func solve(u: dynamic) -> dynamic
{
  var l: dynamic = 0;
  var r: dynamic = 1;
  q[r] = u;
  var to: dynamic = 0;
  var maxn: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (a[i][u])
      {
        if (((cpp_cast(((node[u] + (node[i] * 1.0)))) / cpp_cast(a[i][u])) > maxn))
        {
          to = i;
          maxn = (cpp_cast(((node[u] + (node[i] * 1.0)))) / cpp_cast(a[i][u]));
        }
      }
      i += 1;
    }
  }
  if ((!to))
  {
    return;
  }
  var sumv: dynamic = (node[u] + node[to]);
  var sume: dynamic = a[u][to];
  vis[u] = 1;
  vis[to] = 1;
  while ((l < r))
  {
    var t: dynamic = q[cpp_update(l, "++")];
    var now: dynamic = 0;
    var tag: dynamic = cpp_uninitialized();
    var nowv: dynamic = cpp_uninitialized();
    var nowe: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if (((!vis[i]) && a[t][i]))
        {
          var tmpv: dynamic = (sumv + node[i]);
          var tmpe: dynamic = (sume + a[t][i]);
          {
            var j: dynamic = 1;
            while ((j <= n))
            {
              if ((vis[j] && a[i][j]))
              {
                tmpe += a[i][j];
              }
              j += 1;
            }
          }
          if (((tmpv / tmpe) > now))
          {
            now = (tmpv / tmpe);
            nowv = tmpv;
            nowe = tmpe;
            tag = i;
          }
        }
        i += 1;
      }
    }
    if ((now > ((sumv / sume) - 0.0000000001)))
    {
      sumv = nowv;
      sume = nowe;
      vis[tag] = 1;
      q[cpp_update(r, "++")] = tag;
    }
  }
  if (((sumv / sume) > ans))
  {
    ans = (sumv / sume);
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&node[i]));
      i += 1;
    }
  }
  var flg: dynamic = 0;
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%d%d", (&x), (&y), (&w));
      a[x][y] = cpp_assign(a[y][x], "=", w);
      if ((w > 0))
      {
        flg = 1;
      }
      i += 1;
    }
  }
  if ((!flg))
  {
    printf("%0.15lf\n", 0);
    return 0;
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      memset(vis, 0, cpp_sizeof(vis));
      solve(i);
      i += 1;
    }
  }
  var tans: dynamic = cpp_cast(ans);
  printf("%0.12lf\n", tans);
  return 0;
}

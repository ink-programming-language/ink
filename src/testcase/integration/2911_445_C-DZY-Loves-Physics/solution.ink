// Translated from solution.cpp.

var vis = cpp_array(1000);

var q = cpp_array(1000);

var node = cpp_array(1000);

var n: dynamic;

var a = cpp_array(1000, 1000);

var m: dynamic;

var ans = 0;

func solve(u: dynamic)
{
  var l = 0;
  var r = 1;
  q[r] = u;
  var to = 0;
  var maxn = 0;
  {
    var i = 1;
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
  var sumv = (node[u] + node[to]);
  var sume = a[u][to];
  vis[u] = 1;
  vis[to] = 1;
  while ((l < r))
  {
    var t = q[cpp_update(l, "++")];
    var now = 0;
    var tag: dynamic;
    var nowv: dynamic;
    var nowe: dynamic;
    {
      var i = 1;
      while ((i <= n))
      {
        if (((!vis[i]) && a[t][i]))
        {
          var tmpv = (sumv + node[i]);
          var tmpe = (sume + a[t][i]);
          {
            var j = 1;
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

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&node[i]));
      i += 1;
    }
  }
  var flg = 0;
  var x: dynamic;
  var y: dynamic;
  var w: dynamic;
  {
    var i = 1;
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
    var i = 1;
    while ((i <= n))
    {
      memset(vis, 0, cpp_sizeof(vis));
      solve(i);
      i += 1;
    }
  }
  var tans = cpp_cast(ans);
  printf("%0.12lf\n", tans);
  return 0;
}

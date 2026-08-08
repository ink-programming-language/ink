// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = ((((x << 3)) + ((x << 1))) + ((ch ^ 48)));
    ch = getchar();
  }
  return (x * f);
}

class arr
{
  var x: dynamic;
  var s: dynamic;
  func operator_less(A: dynamic)
  {
      return (A.s < s);
    }
}

var n: dynamic;

var m: dynamic;

var s: dynamic;

var t: dynamic;

var v = cpp_array(200005);

var w = cpp_array(200005);

var head = cpp_array(200005);

var nxt = cpp_array(200005);

var cnt: dynamic;

var dis = cpp_array(200005);

var ds = cpp_array(200005);

var dt = cpp_array(200005);

var a = cpp_array(200005);

var f = cpp_array(2005, 2005, 2);

var num = cpp_array(2005, 2005);

var sum = cpp_array(2005, 2005);

var vis = cpp_array(200005);

var sumx = cpp_array(2005, 2005);

var sumy = cpp_array(2005, 2005);

var numx = cpp_array(2005, 2005);

var numy = cpp_array(2005, 2005);

var dds: dynamic;

var ddt: dynamic;

func add(a: dynamic, b: dynamic, c: dynamic)
{
  v[cpp_update(cnt, "++")] = b;
  w[cnt] = c;
  nxt[cnt] = head[a];
  head[a] = cnt;
}

func dijkstra(S: dynamic)
{
  memset(dis, 999999, cpp_sizeof((dis)));
  memset(vis, 0, cpp_sizeof((vis)));
  dis[S] = 0;
  var q: dynamic;
  q.push([S, 0]);
  while ((!q.empty()))
  {
    var x = q.top().x;
    q.pop();
    if (vis[x])
    {
      continue;
    }
    vis[x] = 1;
    {
      var i = head[x];
      while (i)
      {
        if ((dis[v[i]] > (dis[x] + w[i])))
        {
          dis[v[i]] = (dis[x] + w[i]);
          q.push([v[i], dis[v[i]]]);
        }
        i = nxt[i];
      }
    }
  }
}

func getsumx(x: dynamic, l: dynamic, r: dynamic)
{
  return (sumx[x][r] - sumx[x][(l - 1)]);
}

func getsumy(y: dynamic, l: dynamic, r: dynamic)
{
  return (sumy[r][y] - sumy[(l - 1)][y]);
}

func getnumx(x: dynamic, l: dynamic, r: dynamic)
{
  return (sumx[x][r] - sumx[x][(l - 1)]);
}

func getnumy(y: dynamic, l: dynamic, r: dynamic)
{
  return (sumy[r][y] - sumy[(l - 1)][y]);
}

func main()
{
  n = read();
  m = read();
  s = read();
  t = read();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var x = read();
      var y = read();
      var z = read();
      add(x, y, z);
      add(y, x, z);
      i += 1;
    }
  }
  dijkstra(s);
  {
    var i = 1;
    while ((i <= n))
    {
      ds[i] = dis[i];
      dds.push_back(ds[i]);
      i += 1;
    }
  }
  dijkstra(t);
  {
    var i = 1;
    while ((i <= n))
    {
      dt[i] = dis[i];
      ddt.push_back(dt[i]);
      i += 1;
    }
  }
  sort(dds.begin(), dds.end());
  sort(ddt.begin(), ddt.end());
  var ds = (unique(dds.begin(), dds.end()) - dds.begin());
  var dt = (unique(ddt.begin(), ddt.end()) - ddt.begin());
  {
    var i = 1;
    while ((i <= n))
    {
      var xx = ((lower_bound(dds.begin(), dds.end(), ds[i]) - dds.begin()) + 1);
      var yy = ((lower_bound(ddt.begin(), ddt.end(), dt[i]) - ddt.begin()) + 1);
      num[xx][yy] += 1;
      sum[xx][yy] += a[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (dt + 1)))
    {
      {
        var j = 1;
        while ((j <= (ds + 1)))
        {
          sumx[i][j] = (sumx[i][(j - 1)] + sum[i][j]);
          sumy[i][j] = (sumy[(i - 1)][j] + sum[i][j]);
          numx[i][j] = (numx[i][(j - 1)] + num[i][j]);
          numy[i][j] = (numy[(i - 1)][j] + num[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = dt;
    while ((i >= 0))
    {
      {
        var j = ds;
        while ((j >= 0))
        {
          if ((i != dt))
          {
            var now = getnumx((i + 1), (j + 1), ds);
            var sc = getsumx((i + 1), (j + 1), ds);
            if ((!now))
            {
              f[0][i][j] = f[0][(i + 1)][j];
            } else
            {
              f[0][i][j] = (max(f[0][(i + 1)][j], f[1][(i + 1)][j]) + sc);
            }
          }
          if ((j != ds))
          {
            var now = getnumy((j + 1), (i + 1), dt);
            var sc = getsumy((j + 1), (i + 1), dt);
            if ((!now))
            {
              f[1][i][j] = f[1][i][(j + 1)];
            } else
            {
              f[1][i][j] = (min(f[0][i][(j + 1)], f[1][i][(j + 1)]) - sc);
            }
          }
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  if ((f[0][0][0] > 0))
  {
    puts("Break a heart");
  } else if ((f[0][0][0] == 0))
  {
    puts("Flowers");
  } else
  {
    puts("Cry");
  }
}

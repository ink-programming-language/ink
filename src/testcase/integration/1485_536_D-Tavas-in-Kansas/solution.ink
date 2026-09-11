// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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
  var x: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  func operator_less(A: dynamic) -> dynamic
  {
      return (A.s < s);
    }
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(200005);

var w: dynamic = cpp_array(200005);

var head: dynamic = cpp_array(200005);

var nxt: dynamic = cpp_array(200005);

var cnt: dynamic = cpp_uninitialized();

var dis: dynamic = cpp_array(200005);

var ds: dynamic = cpp_array(200005);

var dt: dynamic = cpp_array(200005);

var a: dynamic = cpp_array(200005);

var f: dynamic = cpp_array(2005, 2005, 2);

var num: dynamic = cpp_array(2005, 2005);

var sum: dynamic = cpp_array(2005, 2005);

var vis: dynamic = cpp_array(200005);

var sumx: dynamic = cpp_array(2005, 2005);

var sumy: dynamic = cpp_array(2005, 2005);

var numx: dynamic = cpp_array(2005, 2005);

var numy: dynamic = cpp_array(2005, 2005);

var dds: dynamic = cpp_uninitialized();

var ddt: dynamic = cpp_uninitialized();

func add(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  v[cpp_update(cnt, "++")] = b;
  w[cnt] = c;
  nxt[cnt] = head[a];
  head[a] = cnt;
}

func dijkstra(S: dynamic) -> dynamic
{
  memset(dis, 999999, cpp_sizeof((dis)));
  memset(vis, 0, cpp_sizeof((vis)));
  dis[S] = 0;
  var q: dynamic = cpp_uninitialized();
  q.push([S, 0]);
  while ((!q.empty()))
  {
    var x: dynamic = q.top().x;
    q.pop();
    if (vis[x])
    {
      continue;
    }
    vis[x] = 1;
    {
      var i: dynamic = head[x];
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

func getsumx(x: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  return (sumx[x][r] - sumx[x][(l - 1)]);
}

func getsumy(y: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  return (sumy[r][y] - sumy[(l - 1)][y]);
}

func getnumx(x: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  return (sumx[x][r] - sumx[x][(l - 1)]);
}

func getnumy(y: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  return (sumy[r][y] - sumy[(l - 1)][y]);
}

func main() -> dynamic
{
  n = read();
  m = read();
  s = read();
  t = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = read();
      var y: dynamic = read();
      var z: dynamic = read();
      add(x, y, z);
      add(y, x, z);
      i += 1;
    }
  }
  dijkstra(s);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ds[i] = dis[i];
      dds.push_back(ds[i]);
      i += 1;
    }
  }
  dijkstra(t);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dt[i] = dis[i];
      ddt.push_back(dt[i]);
      i += 1;
    }
  }
  sort(dds.begin(), dds.end());
  sort(ddt.begin(), ddt.end());
  var ds: dynamic = (unique(dds.begin(), dds.end()) - dds.begin());
  var dt: dynamic = (unique(ddt.begin(), ddt.end()) - ddt.begin());
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var xx: dynamic = ((lower_bound(dds.begin(), dds.end(), ds[i]) - dds.begin()) + 1);
      var yy: dynamic = ((lower_bound(ddt.begin(), ddt.end(), dt[i]) - ddt.begin()) + 1);
      num[xx][yy] += 1;
      sum[xx][yy] += a[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (dt + 1)))
    {
      {
        var j: dynamic = 1;
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
    var i: dynamic = dt;
    while ((i >= 0))
    {
      {
        var j: dynamic = ds;
        while ((j >= 0))
        {
          if ((i != dt))
          {
            var now: dynamic = getnumx((i + 1), (j + 1), ds);
            var sc: dynamic = getsumx((i + 1), (j + 1), ds);
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
            var now: dynamic = getnumy((j + 1), (i + 1), dt);
            var sc: dynamic = getsumy((j + 1), (i + 1), dt);
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

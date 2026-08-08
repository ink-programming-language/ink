// Translated from solution.cpp.

var N = 80;

var N2 = (((1 << 18)) + 1);

var cnt: dynamic;

var n: dynamic;

var m: dynamic;

var A: dynamic;

var B: dynamic;

var fa = cpp_array(N);

var sz = cpp_array(N);

var bl = cpp_array(N);

var dis = cpp_array(N2, N);

var inq = cpp_array(N2, N);

var ans = cpp_array(N);

var q: dynamic;

var g = cpp_array(N);

func find(x: dynamic)
{
  return if ((fa[x] == x)) x else cpp_assign(fa[x], "=", find(fa[x]));
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m, A, B);
  {
    var i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      sz[i] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var x: dynamic;
      var y: dynamic;
      var z: dynamic;
      read(x, y, z);
      g[x].emplace_back(y, z);
      g[y].emplace_back(x, z);
      if (((z == A) && ((cpp_assign(x, "=", find(x))) != (cpp_assign(y, "=", find(y))))))
      {
        fa[x] = y;
        sz[y] += sz[x];
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((sz[find(i)] >= 4))
      {
        if ((!bl[find(i)]))
        {
          bl[find(i)] = (1 << (cpp_update(cnt, "++")));
        }
        bl[i] = bl[find(i)];
      }
      i += 1;
    }
  }
  memset(ans, 0x3f, cpp_sizeof(ans));
  memset(dis, 0x3f, cpp_sizeof(dis));
  dis[1][bl[1]] = 0;
  q.emplace(1, bl[1]);
  while ((!q.empty()))
  {
    var x = q.front();
    q.pop();
    inq[x.first][x.second] = 0;
    var d = dis[x.first][x.second];
    ans[x.first] = min(ans[x.first], d);
    for (var i in g[x.first])
    {
      if ((i.second == A))
      {
        if ((dis[i.first][x.second] > (d + i.second)))
        {
          dis[i.first][x.second] = (d + i.second);
          if ((!inq[i.first][x.second]))
          {
            inq[i.first][x.second] = 1;
            q.emplace(i.first, x.second);
          }
        }
      } else if (((find(x.first) != find(i.first)) && ((((x.second & bl[i.first])) == 0))))
      {
        if ((dis[i.first][(x.second | bl[i.first])] > (d + i.second)))
        {
          dis[i.first][(x.second | bl[i.first])] = (d + i.second);
          if ((!inq[i.first][(x.second | bl[i.first])]))
          {
            inq[i.first][(x.second | bl[i.first])] = 1;
            q.emplace(i.first, (x.second | bl[i.first]));
          }
        }
      }
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(ans[i], " \n"[(i == n)]);
      i += 1;
    }
  }
  return 0;
}

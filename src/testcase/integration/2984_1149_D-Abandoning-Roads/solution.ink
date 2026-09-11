// Translated from solution.cpp.

var N: dynamic = 80;

var N2: dynamic = (((1 << 18)) + 1);

var cnt: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(N);

var sz: dynamic = cpp_array(N);

var bl: dynamic = cpp_array(N);

var dis: dynamic = cpp_array(N2, N);

var inq: dynamic = cpp_array(N2, N);

var ans: dynamic = cpp_array(N);

var q: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(N);

func find(x: dynamic) -> dynamic
{
  return  ((fa[x] == x)) ? x : cpp_assign(fa[x], "=", find(fa[x]));
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m, A, B);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fa[i] = i;
      sz[i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
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
    var i: dynamic = 1;
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
    var x: dynamic = q.front();
    q.pop();
    inq[x.first][x.second] = 0;
    var d: dynamic = dis[x.first][x.second];
    ans[x.first] = min(ans[x.first], d);
    for (var i: dynamic in g[x.first])
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(ans[i], " \n"[(i == n)]);
      i += 1;
    }
  }
  return 0;
}

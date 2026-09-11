// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var inq: dynamic = cpp_array(3010);

var dis: dynamic = cpp_array(3010, 3010);

var G: dynamic = cpp_array(3010);

func SPFA(s: dynamic) -> dynamic
{
  memset(inq, 0, cpp_sizeof(inq));
  var que: dynamic = cpp_uninitialized();
  dis[s][s] = 0;
  inq[s] = true;
  que.push(s);
  while ((!que.empty()))
  {
    var u: dynamic = que.front();
    que.pop();
    inq[u] = false;
    {
      var i: dynamic = 0;
      while ((i < G[u].size()))
      {
        var v: dynamic = G[u][i];
        if ((dis[s][v] > (dis[s][u] + 1)))
        {
          dis[s][v] = (dis[s][u] + 1);
          if ((!inq[v]))
          {
            inq[v] = true;
            que.push(v);
          }
        }
        i += 1;
      }
    }
  }
}

var s1: dynamic = cpp_uninitialized();

var t1: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

var t2: dynamic = cpp_uninitialized();

var l1: dynamic = cpp_uninitialized();

var l2: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  memset(dis, 0x3f, cpp_sizeof(dis));
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d", (&u), (&v));
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      SPFA(i);
      i += 1;
    }
  }
  scanf("%d%d%d", (&s1), (&t1), (&l1));
  scanf("%d%d%d", (&s2), (&t2), (&l2));
  if (((dis[s1][t1] > l1) || (dis[s2][t2] > l2)))
  {
    puts("-1");
    return 0;
  }
  var ans: dynamic = (dis[s1][t1] + dis[s2][t2]);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          if (((((dis[s1][i] + dis[i][j]) + dis[j][t1]) <= l1) && (((dis[s2][i] + dis[i][j]) + dis[j][t2]) <= l2)))
          {
            ans = min(ans, ((((dis[s1][i] + dis[s2][i]) + dis[i][j]) + dis[j][t1]) + dis[j][t2]));
          }
          if (((((dis[s1][i] + dis[i][j]) + dis[j][t1]) <= l1) && (((dis[s2][j] + dis[j][i]) + dis[i][t2]) <= l2)))
          {
            ans = min(ans, ((((dis[s1][i] + dis[i][j]) + dis[j][t1]) + dis[s2][j]) + dis[i][t2]));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", (m - ans));
  return 0;
}

// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var inq = cpp_array(3010);

var dis = cpp_array(3010, 3010);

var G = cpp_array(3010);

func SPFA(s: dynamic)
{
  memset(inq, 0, cpp_sizeof(inq));
  var que: dynamic;
  dis[s][s] = 0;
  inq[s] = true;
  que.push(s);
  while ((!que.empty()))
  {
    var u = que.front();
    que.pop();
    inq[u] = false;
    {
      var i = 0;
      while ((i < G[u].size()))
      {
        var v = G[u][i];
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

var s1: dynamic;

var t1: dynamic;

var s2: dynamic;

var t2: dynamic;

var l1: dynamic;

var l2: dynamic;

func main()
{
  memset(dis, 0x3f, cpp_sizeof(dis));
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  {
    var i = 1;
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
  var ans = (dis[s1][t1] + dis[s2][t2]);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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

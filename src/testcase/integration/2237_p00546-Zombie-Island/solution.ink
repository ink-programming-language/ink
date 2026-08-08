// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var k: dynamic;

var u: dynamic;

var p: dynamic;

var q: dynamic;

class dijkstra
{
  var V: dynamic;
  var G: dynamic;
  var d: dynamic;
  func dijkstra(nv: dynamic)
  {
      nv += 10;
      d = vector(nv);
      V = nv;
      G = vector(nv);
    }
  func run(s: dynamic)
  {
      var que: dynamic;
      fill(d.begin(), d.end(), LLONG_MAX);
      d[s] = 0;
      que.push(P(0, s));
      while ((!que.empty()))
      {
        var p = que.top();
        que.pop();
        var v = p.second;
        if ((d[v] < p.first))
        {
          continue;
        }
        {
          var i = 0;
          while ((i < G[v].size()))
          {
            var e = G[v][i];
            if ((d[e.to] > (d[v] + e.cost)))
            {
              d[e.to] = (d[v] + e.cost);
              que.push(P(d[e.to], e.to));
            }
            i += 1;
          }
        }
      }
    }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(n, m, k, u);
  read(p, q);
  var s = cpp_array(100005);
  memset(s, -1, cpp_sizeof((s)));
  var que: dynamic;
  var edge = cpp_array(100005);
  {
    var i = 0;
    while ((i < k))
    {
      var tmp: dynamic;
      read(tmp);
      tmp -= 1;
      s[tmp] = 0;
      que.push(tmp);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      edge[a].push_back(b);
      edge[b].push_back(a);
      i += 1;
    }
  }
  while (que.size())
  {
    var now = que.front();
    que.pop();
    {
      var i = 0;
      while ((i < edge[now].size()))
      {
        var next = edge[now][i];
        if ((s[next] == -1))
        {
          s[next] = (s[now] + 1);
          que.push(next);
        }
        i += 1;
      }
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < edge[i].size()))
        {
          var next = edge[i][j];
          if ((next == (n - 1)))
          {
            D.G[i].push_back([next, 0]);
          } else if (((s[next] == -1) || (s[next] > u)))
          {
            D.G[i].push_back([next, p]);
          } else if ((s[next] != 0))
          {
            D.G[i].push_back([next, q]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  D.run(0);
  write(D.d[(n - 1)], "\n");
  return 0;
}

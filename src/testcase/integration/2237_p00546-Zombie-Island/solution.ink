// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

class dijkstra
{
  var V: dynamic = cpp_uninitialized();
  var G: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  func dijkstra(nv: dynamic) -> dynamic
  {
      nv += 10;
      d = vector(nv);
      V = nv;
      G = vector(nv);
    }
  func run(s: dynamic) -> dynamic
  {
      var que: dynamic = cpp_uninitialized();
      fill(d.begin(), d.end(), LLONG_MAX);
      d[s] = 0;
      que.push(P(0, s));
      while ((!que.empty()))
      {
        var p: dynamic = que.top();
        que.pop();
        var v: dynamic = p.second;
        if ((d[v] < p.first))
        {
          continue;
        }
        {
          var i: dynamic = 0;
          while ((i < G[v].size()))
          {
            var e: dynamic = G[v][i];
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

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(n, m, k, u);
  read(p, q);
  var s: dynamic = cpp_array(100005);
  memset(s, -1, cpp_sizeof((s)));
  var que: dynamic = cpp_uninitialized();
  var edge: dynamic = cpp_array(100005);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var tmp: dynamic = cpp_uninitialized();
      read(tmp);
      tmp -= 1;
      s[tmp] = 0;
      que.push(tmp);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
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
    var now: dynamic = que.front();
    que.pop();
    {
      var i: dynamic = 0;
      while ((i < edge[now].size()))
      {
        var next: dynamic = edge[now][i];
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < edge[i].size()))
        {
          var next: dynamic = edge[i][j];
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

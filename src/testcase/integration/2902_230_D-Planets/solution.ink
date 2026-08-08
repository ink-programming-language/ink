// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a: dynamic;

var b: dynamic;

var w: dynamic;

var start = 0;

var k: dynamic;

var MAX = 1e18;

var wait = cpp_construct(100005);

var adj = cpp_construct(100005);

func binarySearch(v: dynamic, d: dynamic)
{
  if ((wait[v].size() == 0))
  {
    return;
  }
  var low = 0;
  var high = (wait[v].size() - 1);
  var mid: dynamic;
  while ((low <= high))
  {
    mid = (((low + high)) >> 1);
    if ((wait[v][mid] == d))
    {
      d += 1;
      mid += 1;
      while (cpp_binary((mid <= (wait[v].size() - 1)), "and", (wait[v][mid] == (wait[v][(mid - 1)] + 1))))
      {
        d += 1;
        mid += 1;
      }
      return;
    } else if ((wait[v][mid] > d))
    {
      high = (mid - 1);
    } else
    {
      low = (mid + 1);
    }
  }
  return;
}

func main(argument_0: dynamic)
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  {
    var i = 0;
    while ((i <= (m - 1)))
    {
      read(a, b, w);
      a -= 1;
      b -= 1;
      adj[a].push_back([b, w]);
      adj[b].push_back([a, w]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (n - 2)))
    {
      read(k);
      {
        var j = 1;
        while ((j <= k))
        {
          read(w);
          wait[i].emplace_back(w);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var s: dynamic;
  var present = cpp_construct(n, 1);
  dist[start] = 0;
  {
    var i = 0;
    while ((i <= (n - 1)))
    {
      s.insert([dist[i], i]);
      i += 1;
    }
  }
  while ((!s.empty()))
  {
    var c = (*(s.begin()));
    s.erase(c);
    var v = c.second;
    present[v] = 0;
    binarySearch(v, dist[v]);
    for (var e in adj[v])
    {
      var u = e.first;
      w = e.second;
      if (cpp_binary(present[u], "and", (dist[u] > (dist[v] + (1 * w)))))
      {
        s.erase([dist[u], u]);
        dist[u] = (dist[v] + (1 * w));
        s.insert([dist[u], u]);
      }
    }
  }
  if ((dist[(n - 1)] == MAX))
  {
    write(-1, "\n");
  } else
  {
    write(dist[(n - 1)], "\n");
  }
  return 0;
}

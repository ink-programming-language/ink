// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var start: dynamic = 0;

var k: dynamic = cpp_uninitialized();

var MAX: dynamic = 1e18;

var wait: dynamic = cpp_construct(100005);

var adj: dynamic = cpp_construct(100005);

func binarySearch(v: dynamic, d: dynamic) -> dynamic
{
  if ((wait[v].size() == 0))
  {
    return;
  }
  var low: dynamic = 0;
  var high: dynamic = (wait[v].size() - 1);
  var mid: dynamic = cpp_uninitialized();
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

func main(argument_0: dynamic) -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i <= (n - 2)))
    {
      read(k);
      {
        var j: dynamic = 1;
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
  var s: dynamic = cpp_uninitialized();
  var present: dynamic = cpp_construct(n, 1);
  dist[start] = 0;
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      s.insert([dist[i], i]);
      i += 1;
    }
  }
  while ((!s.empty()))
  {
    var c: dynamic = (*(s.begin()));
    s.erase(c);
    var v: dynamic = c.second;
    present[v] = 0;
    binarySearch(v, dist[v]);
    for (var e: dynamic in adj[v])
    {
      var u: dynamic = e.first;
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

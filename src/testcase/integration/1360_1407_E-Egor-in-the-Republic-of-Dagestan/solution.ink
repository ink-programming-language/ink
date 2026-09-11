// Translated from solution.cpp.

var N: dynamic = (5e5 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(N);

var color: dynamic = cpp_array(N);

var h: dynamic = cpp_array(N);

var d: dynamic = cpp_array(N);

func main() -> dynamic
{
  scanf("%d %d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%d %d %d", (&x), (&y), (&t));
      v[y].push_back([x, t]);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  q.push(n);
  memset(color, -1, cpp_sizeof((color)));
  d[1] = -1;
  d[n] = 0;
  h[n] = 1;
  while ((!q.empty()))
  {
    var x: dynamic = q.front();
    q.pop();
    for (var e: dynamic in v[x])
    {
      var u: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      tie(u, t) = e;
      if ((color[u] == -1))
      {
        color[u] = (!t);
      }
      if ((color[u] != t))
      {
        continue;
      }
      if ((!h[u]))
      {
        h[u] = 1;
        d[u] = (d[x] + 1);
        q.push(u);
      }
    }
  }
  printf("%d\n", d[1]);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d", max(0, color[i]));
      i += 1;
    }
  }
  puts("");
  return 0;
}

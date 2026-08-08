// Translated from solution.cpp.

var N = (5e5 + 5);

var n: dynamic;

var m: dynamic;

var v = cpp_array(N);

var color = cpp_array(N);

var h = cpp_array(N);

var d = cpp_array(N);

func main()
{
  scanf("%d %d", (&n), (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      var x: dynamic;
      var y: dynamic;
      var t: dynamic;
      scanf("%d %d %d", (&x), (&y), (&t));
      v[y].push_back([x, t]);
      i += 1;
    }
  }
  var q: dynamic;
  q.push(n);
  memset(color, -1, cpp_sizeof((color)));
  d[1] = -1;
  d[n] = 0;
  h[n] = 1;
  while ((!q.empty()))
  {
    var x = q.front();
    q.pop();
    for (var e in v[x])
    {
      var u: dynamic;
      var t: dynamic;
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
    var i = 1;
    while ((i <= n))
    {
      printf("%d", max(0, color[i]));
      i += 1;
    }
  }
  puts("");
  return 0;
}

// Translated from solution.cpp.

var N = 200005;

var x: dynamic;

var y: dynamic;

var n: dynamic;

var Max = cpp_array(N);

var Max2 = cpp_array(N);

var l = cpp_array(N);

var z = cpp_array(N);

var G = cpp_array(N);

var s = cpp_array(N);

var ans: dynamic;

func upd(x: dynamic, y: dynamic)
{
  if ((y > Max[x]))
  {
    Max2[x] = Max[x];
    Max[x] = y;
  } else if ((y > Max2[x]))
  {
    Max2[x] = y;
  }
}

func dfs1(x: dynamic, y: dynamic)
{
  l[x] = N;
  if ((s[x] == cpp_char("1")))
  {
    l[x] = 0;
    z[x] = 1;
  }
  for (var i in G[x])
  {
    if ((i != y))
    {
      dfs1(i, x);
      var d = (Max[i] + 1);
      upd(x, d);
      if (z[i])
      {
        l[x] = min(l[x], d);
        z[x] += z[i];
      }
    }
  }
}

func dfs2(x: dynamic, y: dynamic)
{
  if (y)
  {
    var d = if (((Max[x] + 1) == Max[y])) (Max2[y] + 1) else (Max[y] + 1);
    upd(x, d);
    if ((z[1] > z[x]))
    {
      l[x] = min(l[x], d);
    }
  }
  for (var i in G[x])
  {
    if ((i != y))
    {
      dfs2(i, x);
    }
  }
  var L = l[x];
  var R = min((Max2[x] + 1), (Max[x] - 1));
  if ((L <= R))
  {
    ans += ((R - L) + 1);
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i < n))
    {
      scanf("%d%d", (&x), (&y));
      G[x].push_back(y);
      G[y].push_back(x);
      i += 1;
    }
  }
  scanf("%s", (s + 1));
  dfs1(1, 0);
  dfs2(1, 0);
  printf("%lld\n", (ans + 1));
  return 0;
}

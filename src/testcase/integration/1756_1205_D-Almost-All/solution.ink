// Translated from solution.cpp.

var N = 1054;

var M = (N * 2);

var n: dynamic;

var E = 0;

var G: dynamic;

var Gm = INT_MAX;

var to = cpp_array(M);

var first = cpp_array(N);

var next = cpp_array(M);

var p = cpp_array(N);

var size = cpp_array(N);

var cnt = 0;

var nb = cpp_array(N);

var du = cpp_array(N);

var pq: dynamic;

func up(x: dynamic, y: dynamic)
{
  if ((x < y)) cpp_assign(x, "=", y) else 0;
}

func addedge(u: dynamic, v: dynamic)
{
  to[cpp_update(E, "++")] = v;
  next[E] = first[u];
  first[u] = E;
  to[cpp_update(E, "++")] = u;
  next[E] = first[v];
  first[v] = E;
}

func centr(x: dynamic, px: dynamic = 0)
{
  var i: dynamic;
  var y: dynamic;
  var max = 0;
  size[x] = 1;
  {
    i = first[x];
    while (i)
    {
      if (((cpp_assign(y, "=", to[i])) != px))
      {
        centr(y, x);
        up(max, size[y]);
        size[x] += size[y];
      }
      i = next[i];
    }
  }
  if (cpp_comma(up(max, (n - size[x])), (max <= Gm)))
  {
    Gm = max;
    G = x;
  }
}

func dfs(x: dynamic)
{
  var i: dynamic;
  var y: dynamic;
  size[x] = 1;
  {
    i = first[x];
    while (i)
    {
      if (((cpp_assign(y, "=", to[i])) != p[x]))
      {
        p[y] = x;
        dfs(y);
        size[x] += size[y];
      }
      i = next[i];
    }
  }
}

var p = cpp_array(N);

func ancestor(x: dynamic)
{
  return if ((p[x] == x)) x else (cpp_assign(p[x], "=", ancestor(p[x])));
}

func test(x: dynamic, y: dynamic, un: dynamic = false)
{
  if (((cpp_assign(x, "=", ancestor(x))) == (cpp_assign(y, "=", ancestor(y)))))
  {
    return true;
  }
  return (un && (cpp_assign(p[x], "=", false)));
}

var current: dynamic;

var step: dynamic;

func reset(step: dynamic)
{
  current = 0;
  step = step;
}

func next()
{
  return cpp_assign(current, "+=", step);
}

func work(x: dynamic)
{
  var i: dynamic;
  var y: dynamic;
  du[x] = next();
  {
    i = first[x];
    while (i)
    {
      if ((p[cpp_assign(y, "=", to[i])] == x))
      {
        work(y);
      }
      i = next[i];
    }
  }
  du[x] -= du[p[x]];
}

func main()
{
  var i: dynamic;
  var u: dynamic;
  var v: dynamic;
  var Su: dynamic;
  var Sv: dynamic;
  scanf("%d", (&n));
  {
    i = 1;
    while ((i < n))
    {
      scanf("%d%d", (&u), (&v));
      addedge(u, v);
      i += 1;
    }
  }
  centr(1);
  dfs(G);
  {
    i = first[G];
    while (i)
    {
      nb[cpp_update(cnt, "++")] = cpp_assign(v, "=", to[i]);
      pq.emplace(size[v], cpp_assign(dsu.p[v], "=", v));
      i = next[i];
    }
  }
  {
    while ((pq.size() > 2))
    {
      tie(Su, u) = pq.top();
      pq.pop();
      tie(Sv, v) = pq.top();
      pq.pop();
      dsu.test(u, v, true);
      pq.emplace((Su + Sv), v);
    }
  }
  assigner.reset(1);
  {
    i = 0;
    while ((i < cnt))
    {
      if (dsu.test((*nb), nb[i]))
      {
        assigner.work(nb[i]);
      }
      i += 1;
    }
  }
  assigner.reset(assigner.next());
  {
    i = 0;
    while ((i < cnt))
    {
      if ((!dsu.test((*nb), nb[i])))
      {
        assigner.work(nb[i]);
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if ((i != G))
      {
        printf("%d %d %d\n", i, p[i], du[i]);
      }
      i += 1;
    }
  }
  return 0;
}

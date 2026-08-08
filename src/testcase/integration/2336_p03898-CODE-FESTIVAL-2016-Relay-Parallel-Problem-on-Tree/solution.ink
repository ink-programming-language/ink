// Translated from solution.cpp.

var popcount = cpp_expression("#include <cstdio>");

var n: dynamic;

var r: dynamic;

var l: dynamic;

var g = cpp_array(100010);

var lc = cpp_array(100010);

var d = cpp_array(100010);

func dfs(x: dynamic, p: dynamic)
{
  if ((g[x].size() == 1))
  {
    lc[x] = 1;
  }
  d[x] = 1;
  for (var y in g[x])
  {
    if ((y == p))
    {
      continue;
    }
    dfs(y, x);
    lc[x] += lc[y];
  }
  for (var y in g[x])
  {
    if ((y == p))
    {
      continue;
    }
    d[x] = max(d[x], (max(1, (lc[x] - lc[y])) + d[y]));
  }
}

var ans: dynamic;

func dfs2(x: dynamic, p: dynamic)
{
  var v: dynamic;
  for (var y in g[x])
  {
    if ((y == p))
    {
      continue;
    }
    v.push_back((d[y] - lc[y]));
  }
  sort(v.begin(), v.end(), greater());
  if ((v.size() >= 2))
  {
    ans = max(ans, ((v[0] + v[1]) + l));
  }
  for (var y in g[x])
  {
    if ((y == p))
    {
      continue;
    }
    dfs2(y, x);
  }
}

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      g[a].push_back(b);
      g[b].push_back(a);
      i += 1;
    }
  }
  if ((n == 2))
  {
    write(2, "\n");
    return 0;
  }
  r = -1;
  {
    var i = 0;
    while ((i < n))
    {
      if ((g[i].size() == 1))
      {
        l += 1;
      } else if ((g[i].size() > 2))
      {
        r = i;
      }
      i += 1;
    }
  }
  if ((r == -1))
  {
    write(n, "\n");
    return 0;
  }
  dfs(r, -1);
  dfs2(r, -1);
  write(ans, "\n");
  return 0;
}

// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var root: dynamic = -1;

var g: dynamic = cpp_array(200100);

var len: dynamic = cpp_array(200100);

var viz: dynamic = cpp_array(200100, 3);

var dmax: dynamic = 0;

var ind: dynamic = cpp_uninitialized();

var stacky: dynamic = cpp_array(200100);

var l: dynamic = 0;

func dfs(x: dynamic) -> dynamic
{
  viz[0][x] = 1;
  if ((g[x].size() == 1))
  {
    len[x] = 1;
  } else
  {
    var okk: dynamic = 1;
    var kiddo: dynamic = -1;
    var kiddo2: dynamic = -1;
    for (var y: dynamic in g[x])
    {
      if (viz[0][y])
      {
        continue;
      }
      dfs(y);
      if ((x == root))
      {
        if ((len[y] == 0))
        {
          okk = 0;
        } else if ((kiddo == -1))
        {
          kiddo = len[y];
        } else if (((kiddo != len[y]) && (kiddo2 == -1)))
        {
          kiddo2 = len[y];
        } else if (((kiddo2 != len[y]) && (kiddo != len[y])))
        {
          okk = 0;
        }
      } else
      {
        if ((len[y] == 0))
        {
          okk = 0;
          continue;
        }
        if ((kiddo == -1))
        {
          kiddo = len[y];
        } else if ((kiddo != len[y]))
        {
          okk = 0;
        }
      }
    }
    if (okk)
    {
      if (((x != root) || (kiddo2 == -1)))
      {
        len[x] = (kiddo + 1);
      } else
      {
        len[x] = ((kiddo + kiddo2) + 1);
      }
    }
  }
}

func dfss(x: dynamic, t: dynamic, d: dynamic) -> dynamic
{
  stacky[cpp_update(l, "++")] = x;
  viz[t][x] = 1;
  if ((d > dmax))
  {
    ind = x;
    dmax = d;
    root = stacky[(((l + 1)) / 2)];
  }
  for (var y: dynamic in g[x])
  {
    if ((!viz[t][y]))
    {
      dfss(y, t, (d + 1));
    }
  }
  l -= 1;
}

func main() -> dynamic
{
  cin.sync_with_stdio(false);
  read(N);
  if ((N == 2))
  {
    write(1);
    return 0;
  }
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      read(x, y);
      g[x].push_back(y);
      g[y].push_back(x);
      i += 1;
    }
  }
  dfss(1, 1, 0);
  dmax = 0;
  dfss(ind, 2, 0);
  dfs(root);
  if (len[root])
  {
    var ret: dynamic = (len[root] - 1);
    while (((ret % 2) == 0))
    {
      ret /= 2;
    }
    write(ret);
    return 0;
  } else
  {
    write(-1);
    return 0;
  }
  return 0;
}

// Translated from solution.cpp.

var dp = cpp_array(83, 83, 83, 83);

var adj = cpp_array(103);

var n: dynamic;

var k: dynamic;

var m: dynamic;

var inf = 1e9;

func f(nw: dynamic, baki: dynamic, l: dynamic, r: dynamic)
{
  if ((baki == 0))
  {
    return 0;
  }
  if ((dp[nw][baki][l][r] != (-1)))
  {
    return dp[nw][baki][l][r];
  }
  var ret = inf;
  for (var x in adj[nw])
  {
    var v = x.first;
    var w = x.second;
    if (((v <= l) || (v >= r)))
    {
      continue;
    }
    if ((v == nw))
    {
      continue;
    }
    if ((v > nw))
    {
      ret = min(ret, (w + f(v, (baki - 1), nw, r)));
    } else
    {
      ret = min(ret, (w + f(v, (baki - 1), l, nw)));
    }
  }
  return cpp_assign(dp[nw][baki][l][r], "=", ret);
}

func main()
{
  memset(dp, -1, cpp_sizeof(dp));
  read(n, k, m);
  while (cpp_update(m, "--"))
  {
    var u: dynamic;
    var v: dynamic;
    var w: dynamic;
    read(u, v, w);
    adj[u].push_back(make_pair(v, w));
  }
  var ses = inf;
  {
    var i = 1;
    while ((i <= n))
    {
      ses = min(ses, f(i, (k - 1), 0, (n + 1)));
      i += 1;
    }
  }
  if ((ses >= inf))
  {
    ses = -1;
  }
  write(ses, "\n");
  return 0;
}

// Translated from solution.cpp.

var dp: dynamic = cpp_array(83, 83, 83, 83);

var adj: dynamic = cpp_array(103);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var inf: dynamic = 1e9;

func f(nw: dynamic, baki: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((baki == 0))
  {
    return 0;
  }
  if ((dp[nw][baki][l][r] != (-1)))
  {
    return dp[nw][baki][l][r];
  }
  var ret: dynamic = inf;
  for (var x: dynamic in adj[nw])
  {
    var v: dynamic = x.first;
    var w: dynamic = x.second;
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

func main() -> dynamic
{
  memset(dp, -1, cpp_sizeof(dp));
  read(n, k, m);
  while (cpp_update(m, "--"))
  {
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    read(u, v, w);
    adj[u].push_back(make_pair(v, w));
  }
  var ses: dynamic = inf;
  {
    var i: dynamic = 1;
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

// Translated from solution.cpp.

var mod: dynamic = 1000000007;

func powmod(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  a %= mod;
  assert((b >= 0));
  {
    while (b)
    {
      if ((b & 1))
      {
        res = ((res * a) % mod);
      }
      a = ((a * a) % mod);
      b >>= 1;
    }
  }
  return res;
}

var N: dynamic = 40100;

var T: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var ch: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

var st: dynamic = cpp_array(N);

var q: dynamic = cpp_uninitialized();

var ret: dynamic = cpp_array(N);

var cp: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(10);

var e: dynamic = cpp_array(N);

var Q: dynamic = cpp_array(N);

func dfs(u: dynamic, f: dynamic) -> dynamic
{
  for (var v: dynamic in e[u])
  {
    if ((v.first != f))
    {
      dfs(v.first, u);
      p[v.first] = u;
      c[v.first] = v.second;
    }
  }
}

func gao(u: dynamic, v: dynamic) -> dynamic
{
  T += 1;
  var r: dynamic = v;
  var tot: dynamic = 0;
  dep[v] = n;
  var ret: dynamic = -1;
  while ((r != u))
  {
    ch[dep[r]] = c[r];
    vis[r] = T;
    cp[r] = 1;
    dep[p[r]] = (dep[r] - 1);
    r = p[r];
    ret += 1;
  }
  vis[u] = T;
  cp[u] = 1;
  {
    var j: dynamic = 1;
    while ((j < (n + 1)))
    {
      if ((vis[j] != T))
      {
        var top: dynamic = 0;
        var r: dynamic = j;
        while ((vis[r] != T))
        {
          st[cpp_update(top, "++")] = r;
          r = p[r];
        }
        {
          var i: dynamic = (top - 1);
          while ((i >= 0))
          {
            var r: dynamic = st[i];
            dep[r] = (dep[p[r]] + 1);
            vis[r] = T;
            if ((cp[p[r]] != 1))
            {
              cp[r] = cp[p[r]];
            } else if ((ch[dep[r]] > c[r]))
            {
              cp[r] = 0;
            } else if ((ch[dep[r]] == c[r]))
            {
              cp[r] = 1;
            } else
            {
              cp[r] = 2;
            }
            ret += (((cp[r] == 0)) || (((cp[r] == 1) && (dep[r] < dep[v]))));
            i -= 1;
          }
        }
      }
      j += 1;
    }
  }
  return ret;
}

func solve(u: dynamic, f: dynamic) -> dynamic
{
  for (var v: dynamic in Q[u])
  {
    ret[v.second] = gao(u, v.first);
  }
  var pr: dynamic = c[u];
  for (var v: dynamic in e[u])
  {
    if ((v.first != f))
    {
      p[u] = v.first;
      c[u] = v.second;
      solve(v.first, u);
    }
  }
  c[u] = pr;
  p[u] = f;
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&q));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      scanf("%d%d%s", (&u), (&v), s);
      e[u].push_back(make_pair(v, s[0]));
      e[v].push_back(make_pair(u, s[0]));
      i += 1;
    }
  }
  dfs(1, 0);
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      scanf("%d%d", (&u), (&v));
      Q[u].push_back(make_pair(v, i));
      i += 1;
    }
  }
  solve(1, 0);
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      printf("%d\n", ret[i]);
      i += 1;
    }
  }
}

// Translated from solution.cpp.

var mod = 1000000007;

func powmod(a: dynamic, b: dynamic)
{
  var res = 1;
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

var N = 40100;

var T: dynamic;

var p = cpp_array(N);

var c = cpp_array(N);

var dep = cpp_array(N);

var ch = cpp_array(N);

var vis = cpp_array(N);

var st = cpp_array(N);

var q: dynamic;

var ret = cpp_array(N);

var cp = cpp_array(N);

var n: dynamic;

var u: dynamic;

var v: dynamic;

var s = cpp_array(10);

var e = cpp_array(N);

var Q = cpp_array(N);

func dfs(u: dynamic, f: dynamic)
{
  for (var v in e[u])
  {
    if ((v.first != f))
    {
      dfs(v.first, u);
      p[v.first] = u;
      c[v.first] = v.second;
    }
  }
}

func gao(u: dynamic, v: dynamic)
{
  T += 1;
  var r = v;
  var tot = 0;
  dep[v] = n;
  var ret = -1;
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
    var j = 1;
    while ((j < (n + 1)))
    {
      if ((vis[j] != T))
      {
        var top = 0;
        var r = j;
        while ((vis[r] != T))
        {
          st[cpp_update(top, "++")] = r;
          r = p[r];
        }
        {
          var i = (top - 1);
          while ((i >= 0))
          {
            var r = st[i];
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

func solve(u: dynamic, f: dynamic)
{
  for (var v in Q[u])
  {
    ret[v.second] = gao(u, v.first);
  }
  var pr = c[u];
  for (var v in e[u])
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

func main()
{
  scanf("%d%d", (&n), (&q));
  {
    var i = 1;
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
    var i = 0;
    while ((i < q))
    {
      scanf("%d%d", (&u), (&v));
      Q[u].push_back(make_pair(v, i));
      i += 1;
    }
  }
  solve(1, 0);
  {
    var i = 0;
    while ((i < q))
    {
      printf("%d\n", ret[i]);
      i += 1;
    }
  }
}

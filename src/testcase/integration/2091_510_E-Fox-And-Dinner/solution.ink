// Translated from solution.cpp.

func read()
{
  var c = getchar();
  var f = 1;
  var x = 0;
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((x * 10) + ((c - cpp_char("0"))));
    c = getchar();
  }
  return (x * f);
}

func MOD(x: dynamic)
{
  if ((x >= 1000000007))
  {
    x -= 1000000007;
  }
}

var p = cpp_array(100010);

var u = cpp_array(100010);

func pre(n: dynamic)
{
  {
    var i = 2;
    while ((i <= n))
    {
      if ((!u[i]))
      {
        p[cpp_update(p[0], "++")] = i;
      }
      {
        var j = 1;
        while (((j <= p[0]) && ((i * p[j]) <= n)))
        {
          u[(i * p[j])] = 1;
          if (((i % p[j]) == 0))
          {
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

var l = 1;

var nxt = cpp_array(100010);

var head = cpp_array(100010);

var to = cpp_array(100010);

var v = cpp_array(100010);

func ad(x: dynamic, y: dynamic, z: dynamic)
{
  l += 1;
  nxt[l] = head[x];
  head[x] = l;
  to[l] = y;
  v[l] = z;
}

func add(x: dynamic, y: dynamic, z: dynamic)
{
  ad(x, y, z);
  ad(y, x, 0);
}

var bg: dynamic;

var ed: dynamic;

var q = cpp_array(100010);

var d = cpp_array(100010);

func bfs()
{
  {
    var i = 1;
    while ((i <= ed))
    {
      d[i] = 0;
      i += 1;
    }
  }
  var l = 1;
  var r = 2;
  q[l] = bg;
  d[bg] = 1;
  while ((l < r))
  {
    var x = q[cpp_update(l, "++")];
    {
      var i = head[x];
      while (i)
      {
        var c = to[i];
        if ((d[c] || (!v[i])))
        {
          i = nxt[i];
          continue;
        }
        d[c] = (d[x] + 1);
        q[cpp_update(r, "++")] = c;
        i = nxt[i];
      }
    }
  }
  return d[ed];
}

var cur = cpp_array(100010);

func dfs(x: dynamic, f: dynamic)
{
  if (((x == ed) || (!f)))
  {
    return f;
  }
  var u = 0;
  {
    var i = cur[x];
    while (i)
    {
      var c = to[i];
      if (((d[c] != (d[x] + 1)) || (!v[i])))
      {
        i = nxt[i];
        continue;
      }
      var w = dfs(c, min((f - u), v[i]));
      u += w;
      v[i] -= w;
      v[(i ^ 1)] += w;
      if (v[i])
      {
        cur[x] = i;
      }
      if ((u == f))
      {
        return f;
      }
      i = nxt[i];
    }
  }
  if ((!u))
  {
    d[x] = 0;
  }
  return u;
}

func dinic()
{
  var ans = 0;
  while (bfs())
  {
    {
      var i = 1;
      while ((i <= ed))
      {
        cur[i] = head[i];
        i += 1;
      }
    }
    ans += dfs(bg, ((1 << 30)));
  }
  return ans;
}

var a = cpp_array(100010);

var n: dynamic;

var m: dynamic;

var vis = cpp_array(100010);

var ans = cpp_array(210, 210);

func get(x: dynamic)
{
  vis[x] = 1;
  ans[m][cpp_update(ans[m][0], "++")] = x;
  {
    var i = head[x];
    while (i)
    {
      var c = to[i];
      if ((((c > n) || vis[c]) || (!v[(i | 1)])))
      {
        i = nxt[i];
        continue;
      }
      get(c);
      return;
      i = nxt[i];
    }
  }
}

func main()
{
  pre(2e4);
  n = read();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i] & 1))
      {
        {
          var j = 1;
          while ((j <= n))
          {
            if (((i != j) && (!u[(a[i] + a[j])])))
            {
              if ((a[i] & 1))
              {
                add(i, j, 1);
              }
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  bg = (n + 1);
  ed = (bg + 1);
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i] & 1))
      {
        add(bg, i, 2);
      } else
      {
        add(i, ed, 2);
      }
      i += 1;
    }
  }
  if ((dinic() != n))
  {
    return cpp_comma(puts("Impossible"), 0);
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        m += 1;
        get(i);
      }
      i += 1;
    }
  }
  printf("%d\n", m);
  {
    var i = 1;
    while ((i <= m))
    {
      {
        var j = 0;
        while ((j <= ans[i][0]))
        {
          printf("%d ", ans[i][j]);
          j += 1;
        }
      }
      puts("");
      i += 1;
    }
  }
}

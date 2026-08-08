// Translated from solution.cpp.

var Finish_read: dynamic;

func read(x: dynamic)
{
  Finish_read = 0;
  x = 0;
  var f = 1;
  var ch = getchar();
  while ((!isdigit(ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    if ((ch == EOF))
    {
      return;
    }
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  x *= f;
  Finish_read = 1;
}

func print(x: dynamic)
{
  if (((x / 10) != 0))
  {
    print((x / 10));
  }
  putchar(((x % 10) + cpp_char("0")));
}

func writeln(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
  }
  x = abs(x);
  print(x);
  putchar(cpp_char("\n"));
}

func write(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
  }
  x = abs(x);
  print(x);
}

var maxn = 500010;

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
  var id: dynamic;
  func operator_less(rhs: dynamic)
  {
      return (w < rhs.w);
    }
}

var e = cpp_array(maxn);

var n: dynamic;

var m: dynamic;

var from_cpp = cpp_array(maxn);

var to = cpp_array(maxn);

var q: dynamic;

var k: dynamic;

var a = cpp_array(maxn);

var fa = cpp_array(maxn);

var ver: dynamic;

var edg: dynamic;

var vis = cpp_array(maxn);

var can = cpp_array(maxn);

var G = cpp_array(maxn);

func gf(x: dynamic)
{
  return if ((x == fa[x])) x else cpp_assign(fa[x], "=", gf(fa[x]));
}

func dfs(tag: dynamic, x: dynamic)
{
  if ((vis[x] == tag))
  {
    return;
  }
  vis[x] = tag;
  ver += 1;
  edg += G[x].size();
  {
    var i = 0;
    while ((i < G[x].size()))
    {
      dfs(tag, G[x][i]);
      i += 1;
    }
  }
}

func check(tag: dynamic, x: dynamic)
{
  ver = cpp_assign(edg, "=", 0);
  dfs(tag, x);
  return (edg == (2 * ((ver - 1))));
}

func main()
{
  read(n);
  read(m);
  {
    var i = 1;
    while ((i <= m))
    {
      read(e[i].u);
      read(e[i].v);
      read(e[i].w);
      e[i].id = i;
      from_cpp[i] = e[i].u;
      to[i] = e[i].v;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      i += 1;
    }
  }
  sort((e + 1), ((e + m) + 1));
  {
    var i = 1;
    while ((i <= m))
    {
      var pos = i;
      while (((e[pos].w == e[i].w) && (pos <= m)))
      {
        var u = gf(e[pos].u);
        var v = gf(e[pos].v);
        from_cpp[e[pos].id] = u;
        to[e[pos].id] = v;
        can[e[pos].id] = (u != v);
        pos += 1;
      }
      {
        while ((i < pos))
        {
          fa[gf(e[i].u)] = gf(e[i].v);
          i += 1;
        }
      }
      i -= 1;
      i += 1;
    }
  }
  read(q);
  {
    var c = 1;
    while ((c <= q))
    {
      read(k);
      var yes = 1;
      {
        var i = 1;
        while ((i <= k))
        {
          read(a[i]);
          if ((!can[a[i]]))
          {
            yes = 0;
          }
          G[from_cpp[a[i]]].clear();
          G[to[a[i]]].clear();
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= k))
        {
          G[from_cpp[a[i]]].push_back(to[a[i]]);
          G[to[a[i]]].push_back(from_cpp[a[i]]);
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= k))
        {
          var x = from_cpp[a[i]];
          var y = to[a[i]];
          if ((vis[x] != c))
          {
            yes &= check(c, x);
          }
          if ((vis[y] != c))
          {
            yes &= check(c, y);
          }
          i += 1;
        }
      }
      puts(if (yes) "YES" else "NO");
      c += 1;
    }
  }
  return 0;
}

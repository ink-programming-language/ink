// Translated from solution.cpp.

var Finish_read: dynamic = cpp_uninitialized();

func read(x: dynamic) -> dynamic
{
  Finish_read = 0;
  x = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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

func print(x: dynamic) -> dynamic
{
  if (((x / 10) != 0))
  {
    print((x / 10));
  }
  putchar(((x % 10) + cpp_char("0")));
}

func writeln(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
  }
  x = abs(x);
  print(x);
  putchar(cpp_char("\n"));
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
  }
  x = abs(x);
  print(x);
}

var maxn: dynamic = 500010;

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func operator_less(rhs: dynamic) -> dynamic
  {
      return (w < rhs.w);
    }
}

var e: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var from_cpp: dynamic = cpp_array(maxn);

var to: dynamic = cpp_array(maxn);

var q: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var fa: dynamic = cpp_array(maxn);

var ver: dynamic = cpp_uninitialized();

var edg: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(maxn);

var can: dynamic = cpp_array(maxn);

var G: dynamic = cpp_array(maxn);

func gf(x: dynamic) -> dynamic
{
  return  ((x == fa[x])) ? x : cpp_assign(fa[x], "=", gf(fa[x]));
}

func dfs(tag: dynamic, x: dynamic) -> dynamic
{
  if ((vis[x] == tag))
  {
    return;
  }
  vis[x] = tag;
  ver += 1;
  edg += G[x].size();
  {
    var i: dynamic = 0;
    while ((i < G[x].size()))
    {
      dfs(tag, G[x][i]);
      i += 1;
    }
  }
}

func check(tag: dynamic, x: dynamic) -> dynamic
{
  ver = cpp_assign(edg, "=", 0);
  dfs(tag, x);
  return (edg == (2 * ((ver - 1))));
}

func main() -> dynamic
{
  read(n);
  read(m);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      fa[i] = i;
      i += 1;
    }
  }
  sort((e + 1), ((e + m) + 1));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var pos: dynamic = i;
      while (((e[pos].w == e[i].w) && (pos <= m)))
      {
        var u: dynamic = gf(e[pos].u);
        var v: dynamic = gf(e[pos].v);
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
    var c: dynamic = 1;
    while ((c <= q))
    {
      read(k);
      var yes: dynamic = 1;
      {
        var i: dynamic = 1;
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
        var i: dynamic = 1;
        while ((i <= k))
        {
          G[from_cpp[a[i]]].push_back(to[a[i]]);
          G[to[a[i]]].push_back(from_cpp[a[i]]);
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= k))
        {
          var x: dynamic = from_cpp[a[i]];
          var y: dynamic = to[a[i]];
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
      puts( (yes) ? "YES" : "NO");
      c += 1;
    }
  }
  return 0;
}

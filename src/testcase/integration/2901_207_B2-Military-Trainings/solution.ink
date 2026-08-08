// Translated from solution.cpp.

func fpm(b: dynamic, e: dynamic, m: dynamic)
{
  var t = 1;
  {
    while (e)
    {
      if ((e & 1)) cpp_assign(t, "=", ((t * b) % m)) else 0;
      e >>= 1;
      b = ((b * b) % m);
    }
  }
  return t;
}

func chkmin(a: dynamic, b: dynamic)
{
  return if ((a > b)) cpp_comma(cpp_assign(a, "=", b), true) else false;
}

func chkmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), true) else false;
}

func sqr(x: dynamic)
{
  return (x * x);
}

class Graph
{
  var adj: dynamic;
  func Graph(n: dynamic)
  {
      adj.clear();
      adj.resize((n + 5));
    }
  func Graph()
  {
      adj.clear();
    }
  func resize(n: dynamic)
  {
      adj.resize((n + 5));
    }
  func add(s: dynamic, e: dynamic)
  {
      adj[s].push_back(e);
    }
  func del(s: dynamic, e: dynamic)
  {
      adj[s].erase(find(iter(adj[s]), e));
    }
  func deg(v: dynamic)
  {
      return adj[v].size();
    }
  func operator_index(t: dynamic)
  {
      return adj[t];
    }
}

var maxn = 524288;

var maxlevel = 20;

class seg_tree
{
  var f: dynamic = cpp_array((maxn << 1));
  func query(L: dynamic, R: dynamic)
  {
      var ret = 0x3F3F3F3F;
      {
        L += (maxn - 1);
        R += (maxn + 1);
        while (((L ^ R) ^ 1))
        {
          if (((~L) & 1))
          {
            chkmin(ret, f[(L ^ 1)]);
          }
          if ((R & 1))
          {
            chkmin(ret, f[(R ^ 1)]);
          }
          L >>= 1;
          R >>= 1;
        }
      }
      return ret;
    }
  func build(g: dynamic)
  {
      {
        var i = 1;
        while ((i < maxn))
        {
          f[(maxn + i)] = g.query(g.f[(maxn + i)], i);
          i += 1;
        }
      }
      {
        var i = (maxn - 1);
        while (i)
        {
          f[i] = min(f[(i * 2)], f[((i * 2) + 1)]);
          i -= 1;
        }
      }
    }
}

var f = cpp_array(maxlevel);

func main()
{
  ios_base.sync_with_stdio(false);
  var n: dynamic;
  var level = 0;
  read(n);
  {
    var i = 1;
    var x: dynamic;
    while ((i <= n))
    {
      read(x);
      f[0].f[(maxn + i)] = max((i - x), 1);
      f[0].f[((maxn + i) + n)] = max(((i - x) + n), 1);
      i += 1;
    }
  }
  {
    var i = (maxn - 1);
    while (i)
    {
      f[0].f[i] = min(f[0].f[(i * 2)], f[0].f[((i * 2) + 1)]);
      i -= 1;
    }
  }
  {
    var i = 1;
    while (((1 << i) <= (n * 2)))
    {
      f[i].build(f[(i - 1)]);
      level = i;
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var L = ((i + n) - 1);
      var ret = 0x3F3F3F3F;
      var now = 0;
      {
        var b = level;
        var t: dynamic;
        while ((b >= 0))
        {
          if (((cpp_assign(t, "=", f[b].query(L, ((i + n) - 1)))) <= i))
          {
            ret = (now | (1 << b));
          } else
          {
            L = t;
            now += (1 << b);
          }
          b -= 1;
        }
      }
      ans += ret;
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

// Translated from solution.cpp.

class pt
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_uninitialized();

var g: dynamic = cpp_uninitialized();

var size: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func dfs(v: dynamic, parent: dynamic = -1) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < g[v].size()))
    {
      var to: dynamic = g[v][i];
      if ((to == parent))
      {
        i += 1;
        continue;
      }
      size[v] += dfs(to, v);
      i += 1;
    }
  }
  return size[v];
}

var gx: dynamic = cpp_uninitialized();

var gy: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return ((((1 * ((a.x - gx))) * ((b.y - gy))) - ((1 * ((b.x - gx))) * ((a.y - gy)))) > 0);
}

func cmp2(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.y > b.y) || ((a.y == b.y) && (a.x < b.x)));
}

func rec(v: dynamic, p: dynamic, parent: dynamic = -1) -> dynamic
{
  gx = p.front().x;
  gy = p.front().y;
  ans[p.front().id] = v;
  p.erase(p.begin());
  sort(p.begin(), p.end(), cmp);
  var buf: dynamic = cpp_uninitialized();
  var cur: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < g[v].size()))
    {
      var to: dynamic = g[v][i];
      if ((to != parent))
      {
        buf.clear();
        {
          var x: dynamic = cur;
          while ((cur < (x + size[to])))
          {
            buf.push_back(p[cur]);
            cur += 1;
          }
        }
        rec(to, buf, v);
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read(n);
  g.resize(n);
  p.resize(n);
  ans.resize(n);
  size.assign(n, 1);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      g[cpp_update(a, "--")].push_back(cpp_update(b, "--"));
      g[b].push_back(a);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(p[i].x, p[i].y);
      p[i].id = i;
      i += 1;
    }
  }
  dfs(0);
  sort(p.begin(), p.end(), cmp2);
  rec(0, p);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(((ans[i] + 1)), " ");
      i += 1;
    }
  }
  return 0;
}

// Translated from solution.cpp.

class pt
{
  var x: dynamic;
  var y: dynamic;
  var id: dynamic;
}

var p: dynamic;

var g: dynamic;

var size: dynamic;

var ans: dynamic;

var n: dynamic;

func dfs(v: dynamic, parent: dynamic = -1)
{
  {
    var i = 0;
    while ((i < g[v].size()))
    {
      var to = g[v][i];
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

var gx: dynamic;

var gy: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  return ((((1 * ((a.x - gx))) * ((b.y - gy))) - ((1 * ((b.x - gx))) * ((a.y - gy)))) > 0);
}

func cmp2(a: dynamic, b: dynamic)
{
  return ((a.y > b.y) || ((a.y == b.y) && (a.x < b.x)));
}

func rec(v: dynamic, p: dynamic, parent: dynamic = -1)
{
  gx = p.front().x;
  gy = p.front().y;
  ans[p.front().id] = v;
  p.erase(p.begin());
  sort(p.begin(), p.end(), cmp);
  var buf: dynamic;
  var cur = 0;
  {
    var i = 0;
    while ((i < g[v].size()))
    {
      var to = g[v][i];
      if ((to != parent))
      {
        buf.clear();
        {
          var x = cur;
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

func main()
{
  read(n);
  g.resize(n);
  p.resize(n);
  ans.resize(n);
  size.assign(n, 1);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      g[cpp_update(a, "--")].push_back(cpp_update(b, "--"));
      g[b].push_back(a);
      i += 1;
    }
  }
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      write(((ans[i] + 1)), " ");
      i += 1;
    }
  }
  return 0;
}

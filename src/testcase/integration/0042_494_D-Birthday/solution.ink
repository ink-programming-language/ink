// Translated from solution.cpp.

func read()
{
  var r: dynamic;
  var sign: dynamic;
  var c: dynamic;
  r = 0;
  sign = 1;
  while (true)
  {
    c = getchar();
    if (!((((c != cpp_char("-")) && (((c < cpp_char("0")) || (c > cpp_char("9"))))))))
    {
      break;
    }
  }
  if ((c == cpp_char("-")))
  {
    sign = -1;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    r = ((r * 10) + cpp_cast(((c - cpp_char("0")))));
    c = getchar();
  }
  return (sign * r);
}

func print(a: dynamic, n: dynamic)
{
  {
    var i = 1;
    while ((i < n))
    {
      write(a[i], " ");
      i += 1;
    }
  }
  write(a[n], "\n");
}

var mod = (cpp_cast(1e9) + 7);

class hashInt
{
  var x: dynamic;
  func hashInt()
  {
    }
  func hashInt(x: dynamic)
  {
      this->x = cpp_construct(x);
    }
  func operator_add(y: dynamic)
  {
      var z = (x + y.x);
      if ((z >= mod))
      {
        z -= mod;
      }
      if ((z < 0))
      {
        z += mod;
      }
      return hashInt(z);
    }
  func operator_subtract(y: dynamic)
  {
      var z = (x - y.x);
      if ((z >= mod))
      {
        z -= mod;
      }
      if ((z < 0))
      {
        z += mod;
      }
      return hashInt(z);
    }
  func operator_multiply(y: dynamic)
  {
      return hashInt(((cpp_cast(x) * y.x) % mod));
    }
}

class edge
{
  var next: dynamic;
  var node: dynamic;
  var w: dynamic;
}

var e = cpp_array(((100100 << 1) | 1));

var head = cpp_array((100100 + 1));

var tot = 0;

func addedge(a: dynamic, b: dynamic, w: dynamic)
{
  e[cpp_update(tot, "++")].next = head[a];
  head[a] = tot;
  e[tot].node = b;
  e[tot].w = w;
}

class DP
{
  var sum: dynamic;
  var sqr: dynamic;
  func DP()
  {
    }
  func DP(s: dynamic, q: dynamic)
  {
      this->sum = cpp_construct(s);
      this->sqr = cpp_construct(q);
    }
  func DP(s: dynamic, q: dynamic)
  {
      this->sum = cpp_construct(s);
      this->sqr = cpp_construct(q);
    }
}

var up = cpp_array((100100 + 1));

var down = cpp_array((100100 + 1));

var downex = cpp_array((100100 + 1));

var n: dynamic;

var size = cpp_array((100100 + 1));

func delta(x: dynamic, n: dynamic, b: dynamic)
{
  return ((x.sqr + ((b * b) * n)) + ((b * 2) * x.sum));
}

func preUp(x: dynamic, f: dynamic)
{
  size[x] = 1;
  up[x].sum = cpp_assign(up[x].sqr, "=", 0);
  {
    var i = head[x];
    while (i)
    {
      var node = e[i].node;
      if ((node == f))
      {
        i = e[i].next;
        continue;
      }
      preUp(node, x);
      size[x] += size[node];
      up[x].sum = ((up[x].sum + up[node].sum) + (e[i].w * size[node]));
      up[x].sqr = (up[x].sqr + delta(up[node], size[node], e[i].w));
      i = e[i].next;
    }
  }
}

func preDown(x: dynamic, f: dynamic)
{
  {
    var i = head[x];
    while (i)
    {
      var node = e[i].node;
      if ((node == f))
      {
        i = e[i].next;
        continue;
      }
      var cur: dynamic;
      cur.sum = (((down[x].sum + up[x].sum) - up[node].sum) - (e[i].w * size[node]));
      cur.sqr = ((down[x].sqr + up[x].sqr) - delta(up[node], size[node], e[i].w));
      downex[node] = cur;
      down[node].sum = (cur.sum + (e[i].w * ((n - size[node]))));
      down[node].sqr = delta(cur, (n - size[node]), e[i].w);
      preDown(node, x);
      i = e[i].next;
    }
  }
}

var p = cpp_array((100100 + 1), (18 + 1));

var logn: dynamic;

var dep = cpp_array((100100 + 1));

var dis = cpp_array((100100 + 1));

var dissum = cpp_array((100100 + 1));

var dissqr = cpp_array((100100 + 1));

func preDA(x: dynamic, f: dynamic)
{
  p[0][x] = f;
  {
    var i = head[x];
    while (i)
    {
      var node = e[i].node;
      if ((node == f))
      {
        i = e[i].next;
        continue;
      }
      dep[node] = (dep[x] + 1);
      dis[node] = (dis[x] + e[i].w);
      dissum[node] = (dissum[x] + dis[node]);
      dissqr[node] = (dissqr[x] + (dis[node] * dis[node]));
      preDA(node, x);
      i = e[i].next;
    }
  }
}

func LCA(x: dynamic, y: dynamic)
{
  if ((dep[x] > dep[y]))
  {
    swap(x, y);
  }
  {
    var i = logn;
    while ((i >= 0))
    {
      if ((dep[x] <= dep[p[i][y]]))
      {
        y = p[i][y];
      }
      i -= 1;
    }
  }
  if ((x == y))
  {
    return x;
  }
  {
    var i = logn;
    while ((i >= 0))
    {
      if ((p[i][x] != p[i][y]))
      {
        x = p[i][x];
        y = p[i][y];
      }
      i -= 1;
    }
  }
  return p[0][x];
}

func LCA2(x: dynamic, y: dynamic)
{
  {
    var i = logn;
    while ((i >= 0))
    {
      if ((dep[x] < dep[p[i][y]]))
      {
        y = p[i][y];
      }
      i -= 1;
    }
  }
  return y;
}

var Q = cpp_array((100100 + 1));

var q: dynamic;

var ans = cpp_array((100100 + 1));

var seq = cpp_array((100100 + 1));

var cnt = 0;

var pre = cpp_array((100100 + 1));

var suf = cpp_array((100100 + 1));

class BIT
{
  var a: dynamic = cpp_array((100100 + 1));
  func modify(x: dynamic, w: dynamic)
  {
      {
        while ((x <= n))
        {
          a[x] = (a[x] + w);
          x += (x & (-x));
        }
      }
    }
  func query(x: dynamic)
  {
      var r = 0;
      {
        while ((x > 0))
        {
          r = (r + a[x]);
          x -= (x & (-x));
        }
      }
      return r;
    }
}

var sufsum: dynamic;

var sufsqr: dynamic;

func dfs(x: dynamic, f: dynamic)
{
  {
    var i = 0;
    while ((i < Q[x].size()))
    {
      var node = Q[x][i].first;
      var id = Q[x][i].second;
      var p = LCA(x, node);
      var b = (dis[node] - dis[p]);
      b = (b * b);
      i += 1;
    }
  }
  {
    var i = head[x];
    while (i)
    {
      var node = e[i].node;
      if ((node == f))
      {
        i = e[i].next;
        continue;
      }
      seq[cpp_update(cnt, "++")] = x;
      pre[cnt].sum = (pre[(cnt - 1)].sum + (e[i].w * ((cnt - 1))));
      pre[cnt].sqr = delta(pre[(cnt - 1)], (cnt - 1), e[i].w);
      dfs(node, x);
      cnt -= 1;
      i = e[i].next;
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      var w: dynamic;
      scanf("%d%d%d", (&x), (&y), (&w));
      addedge(x, y, w);
      addedge(y, x, w);
      i += 1;
    }
  }
  preUp(1, 0);
  preDown(1, 0);
  dep[1] = 1;
  preDA(1, 0);
  logn = cpp_cast(((log(cpp_cast(n)) / log(2.0))));
  {
    var i = 1;
    while ((i <= logn))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          p[i][j] = p[(i - 1)][p[(i - 1)][j]];
          j += 1;
        }
      }
      i += 1;
    }
  }
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var x: dynamic;
    var y: dynamic;
    scanf("%d%d", (&x), (&y));
    var p = LCA(x, y);
    var d = ((dis[x] + dis[y]) - (dis[p] * 2));
    var ans = 0;
    if ((p == y))
    {
      ans = delta(down[y], (n - size[y]), d);
      ans = ((up[x].sqr + down[x].sqr) - (ans * 2));
    } else
    {
      ans = delta(up[y], size[y], d);
      ans = (((ans * 2) - up[x].sqr) - down[x].sqr);
    }
    printf("%d\n", ans.x);
  }
  fclose(stdin);
  fclose(stdout);
  return 0;
}

// Translated from solution.cpp.

var maxn = 150001;

class BIT
{
  var bit: dynamic = cpp_array(maxn);
  func BIT()
  {
      memset(bit, 0, cpp_sizeof((bit)));
    }
  func add(x: dynamic, v: dynamic)
  {
      {
        x += 1;
        while ((x < maxn))
        {
          bit[x] += v;
          x += (x & (-x));
        }
      }
    }
  func add(a: dynamic, b: dynamic, v: dynamic)
  {
      add(a, 1);
      add((b + 1), -1);
    }
  func qry(x: dynamic)
  {
      var ret = 0;
      {
        x += 1;
        while (x)
        {
          ret += bit[x];
          x -= (x & (-x));
        }
      }
      return ret;
    }
}

class segTree
{
  var l: dynamic;
  var r: dynamic;
  var left: dynamic;
  var right: dynamic;
  var val: dynamic;
  func segTree(a: dynamic, b: dynamic)
  {
      this->l = cpp_construct(a);
      this->r = cpp_construct(b);
    }
  func add(x: dynamic, v: dynamic)
  {
      if ((l == r))
      {
        val += v;
        return;
      }
      var mid = (((l + r)) / 2);
      if ((x <= mid))
      {
        if ((!left))
        {
          left = cpp_new(l, mid);
        }
        left->add(x, v);
      } else
      {
        if ((!right))
        {
          right = cpp_new((mid + 1), r);
        }
        right->add(x, v);
      }
      val = ((if (left) left->val else 0) + (if (right) right->val else 0));
    }
  func mrg(tre: dynamic)
  {
      if ((!tre))
      {
        return;
      }
      if ((!left))
      {
        left = tre->left;
      } else if (tre->left)
      {
        left->mrg(tre->left);
      }
      if ((!right))
      {
        right = tre->right;
      } else if (tre->right)
      {
        right->mrg(tre->right);
      }
      val += tre->val;
      cpp_delete(tre);
    }
  func qry(a: dynamic, b: dynamic)
  {
      if (((a <= l) && (r <= b)))
      {
        return val;
      }
      var ret = 0;
      var mid = (((l + r)) / 2);
      if ((((a <= mid) && (b >= l)) && left))
      {
        ret += left->qry(a, b);
      }
      if ((((b > mid) && (a <= r)) && right))
      {
        ret += right->qry(a, b);
      }
      return ret;
    }
  func clear()
  {
      if (left)
      {
        left->clear();
      }
      if (right)
      {
        right->clear();
      }
      cpp_delete(this);
    }
}

var w = 18;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var q = cpp_array(2, maxn);

var p = cpp_array(maxn, w);

var d = cpp_array(maxn);

var sz = cpp_array(maxn);

var h = cpp_array(maxn);

var l = cpp_array(maxn);

var r = cpp_array(maxn);

var graph = cpp_array(maxn);

var v = cpp_array(maxn);

var v2 = cpp_array(maxn);

var bit: dynamic;

var tre: dynamic;

func dfsh(c: dynamic)
{
  sz[c] = 1;
  h[c] = -1;
  {
    var i = 1;
    while ((i < w))
    {
      p[i][c] = if ((~p[(i - 1)][c])) p[(i - 1)][p[(i - 1)][c]] else -1;
      i += 1;
    }
  }
  for (var i in graph[c])
  {
    if ((i == p[0][c]))
    {
      continue;
    }
    p[0][i] = c;
    d[i] = (d[c] + 1);
    sz[c] += dfsh(i);
    if (((!(~h[c])) || (sz[i] > sz[h[c]])))
    {
      h[c] = i;
    }
  }
  return sz[c];
}

func dfsh2(c: dynamic)
{
  r[c] = l[c];
  for (var i in graph[c])
  {
    if (((i == p[0][c]) || (i == h[c])))
    {
      continue;
    }
    l[i] = (r[c] + 1);
    r[c] = dfsh2(i);
  }
  if ((~h[c]))
  {
    l[h[c]] = (r[c] + 1);
    r[c] = dfsh2(h[c]);
  }
  return r[c];
}

func lft(c: dynamic, x: dynamic)
{
  {
    var i = 0;
    while ((i < w))
    {
      if ((((((x >> i)) & 1)) && (~c)))
      {
        c = p[i][c];
      }
      i += 1;
    }
  }
  return c;
}

func lca(x: dynamic, y: dynamic)
{
  if ((d[x] < d[y]))
  {
    swap(x, y);
  }
  x = lft(x, (d[x] - d[y]));
  {
    var i = (w - 1);
    while ((~i))
    {
      if ((p[i][x] != p[i][y]))
      {
        x = p[i][x];
        y = p[i][y];
      }
      i -= 1;
    }
  }
  return if ((x == y)) x else p[0][x];
}

func dfs2(c: dynamic, rt: dynamic)
{
  var ret = 0;
  tre[c] = cpp_new(0, (n - 1));
  for (var i in v2[c])
  {
    var dd = max(0, ((k + d[rt]) - d[q[i][0]]));
    var j = q[i][1];
    if (((d[j] - d[rt]) >= dd))
    {
      j = lft(j, ((d[j] - d[rt]) - dd));
      ret += tre[c]->qry(l[j], r[j]);
    }
    tre[c]->add(l[q[i][1]], 1);
  }
  for (var i in graph[c])
  {
    if (((i == p[0][c]) || (((c == rt) && (i == h[c])))))
    {
      continue;
    }
    ret += dfs2(i, rt);
    if ((v2[c].size() < v2[i].size()))
    {
      swap(v2[c], v2[i]);
      swap(tre[c], tre[i]);
    }
    for (var it in v2[i])
    {
      var dd = max(0, ((k + d[rt]) - d[c]));
      var j = q[it][1];
      if (((d[j] - d[rt]) >= dd))
      {
        j = lft(j, ((d[j] - d[rt]) - dd));
        ret += tre[c]->qry(l[j], r[j]);
      }
      v2[c].push_back(it);
    }
    v2[i].clear();
    tre[c]->mrg(tre[i]);
  }
  return ret;
}

func dfs(c: dynamic)
{
  var ret = 0;
  for (var i in graph[c])
  {
    if ((i != p[0][c]))
    {
      ret += dfs(i);
    }
  }
  {
    var t = 0;
    while ((t < 2))
    {
      for (var i in v[c])
      {
        ret += bit.qry(l[q[i][t]]);
      }
      for (var i in v[c])
      {
        var j = q[i][t];
        if (((d[j] - d[c]) >= k))
        {
          j = lft(j, ((d[j] - d[c]) - k));
          bit.add(l[j], r[j], 1);
        }
        if ((!t))
        {
          v2[q[i][t]].push_back(i);
        }
      }
      t += 1;
    }
  }
  ret += dfs2(c, c);
  v2[c].clear();
  tre[c]->clear();
  return ret;
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m, k);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      u -= 1;
      v -= 1;
      graph[u].push_back(v);
      graph[v].push_back(u);
      i += 1;
    }
  }
  p[0][0] = -1;
  dfsh(0);
  dfsh2(0);
  {
    var i = 0;
    while ((i < m))
    {
      read(q[i][0], q[i][1]);
      q[i][0] -= 1;
      q[i][1] -= 1;
      if ((l[q[i][0]] > l[q[i][1]]))
      {
        swap(q[i][0], q[i][1]);
      }
      v[lca(q[i][0], q[i][1])].push_back(i);
      i += 1;
    }
  }
  write(dfs(0), cpp_char("\n"));
  return 0;
}

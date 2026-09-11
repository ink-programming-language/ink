// Translated from solution.cpp.

var maxn: dynamic = 150001;

class BIT
{
  var bit: dynamic = cpp_array(maxn);
  func BIT() -> dynamic
  {
      memset(bit, 0, cpp_sizeof((bit)));
    }
  func add(x: dynamic, v: dynamic) -> dynamic
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
  func add(a: dynamic, b: dynamic, v: dynamic) -> dynamic
  {
      add(a, 1);
      add((b + 1), -1);
    }
  func qry(x: dynamic) -> dynamic
  {
      var ret: dynamic = 0;
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
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var left: dynamic = cpp_uninitialized();
  var right: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
  func segTree(a: dynamic, b: dynamic) -> dynamic
  {
      self->l = cpp_construct(a);
      self->r = cpp_construct(b);
    }
  func add(x: dynamic, v: dynamic) -> dynamic
  {
      if ((l == r))
      {
        val += v;
        return;
      }
      var mid: dynamic = (((l + r)) / 2);
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
      val = (( (left) ? left->val : 0) + ( (right) ? right->val : 0));
    }
  func mrg(tre: dynamic) -> dynamic
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
  func qry(a: dynamic, b: dynamic) -> dynamic
  {
      if (((a <= l) && (r <= b)))
      {
        return val;
      }
      var ret: dynamic = 0;
      var mid: dynamic = (((l + r)) / 2);
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
  func clear() -> dynamic
  {
      if (left)
      {
        left->clear();
      }
      if (right)
      {
        right->clear();
      }
      cpp_delete(self);
    }
}

var w: dynamic = 18;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array(2, maxn);

var p: dynamic = cpp_array(maxn, w);

var d: dynamic = cpp_array(maxn);

var sz: dynamic = cpp_array(maxn);

var h: dynamic = cpp_array(maxn);

var l: dynamic = cpp_array(maxn);

var r: dynamic = cpp_array(maxn);

var graph: dynamic = cpp_array(maxn);

var v: dynamic = cpp_array(maxn);

var v2: dynamic = cpp_array(maxn);

var bit: dynamic = cpp_uninitialized();

var tre: dynamic = cpp_uninitialized();

func dfsh(c: dynamic) -> dynamic
{
  sz[c] = 1;
  h[c] = -1;
  {
    var i: dynamic = 1;
    while ((i < w))
    {
      p[i][c] =  ((~p[(i - 1)][c])) ? p[(i - 1)][p[(i - 1)][c]] : -1;
      i += 1;
    }
  }
  for (var i: dynamic in graph[c])
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

func dfsh2(c: dynamic) -> dynamic
{
  r[c] = l[c];
  for (var i: dynamic in graph[c])
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

func lft(c: dynamic, x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
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

func lca(x: dynamic, y: dynamic) -> dynamic
{
  if ((d[x] < d[y]))
  {
    swap(x, y);
  }
  x = lft(x, (d[x] - d[y]));
  {
    var i: dynamic = (w - 1);
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
  return  ((x == y)) ? x : p[0][x];
}

func dfs2(c: dynamic, rt: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  tre[c] = cpp_new(0, (n - 1));
  for (var i: dynamic in v2[c])
  {
    var dd: dynamic = max(0, ((k + d[rt]) - d[q[i][0]]));
    var j: dynamic = q[i][1];
    if (((d[j] - d[rt]) >= dd))
    {
      j = lft(j, ((d[j] - d[rt]) - dd));
      ret += tre[c]->qry(l[j], r[j]);
    }
    tre[c]->add(l[q[i][1]], 1);
  }
  for (var i: dynamic in graph[c])
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
    for (var it: dynamic in v2[i])
    {
      var dd: dynamic = max(0, ((k + d[rt]) - d[c]));
      var j: dynamic = q[it][1];
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

func dfs(c: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  for (var i: dynamic in graph[c])
  {
    if ((i != p[0][c]))
    {
      ret += dfs(i);
    }
  }
  {
    var t: dynamic = 0;
    while ((t < 2))
    {
      for (var i: dynamic in v[c])
      {
        ret += bit.qry(l[q[i][t]]);
      }
      for (var i: dynamic in v[c])
      {
        var j: dynamic = q[i][t];
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

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m, k);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
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

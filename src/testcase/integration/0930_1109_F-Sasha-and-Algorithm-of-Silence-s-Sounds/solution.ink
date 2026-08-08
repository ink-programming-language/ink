// Translated from solution.cpp.

var N = 1000005;

class TNode
{
  var ch: dynamic = cpp_array(2);
  var f: dynamic;
  var r: dynamic;
}

var t = cpp_array(N);

func isroot(pos: dynamic)
{
  return ((t[t[pos].f].ch[0] != pos) && (t[t[pos].f].ch[1] != pos));
}

func pushdown(pos: dynamic)
{
  if (t[pos].r)
  {
    t[pos].r = 0;
    t[t[pos].ch[0]].r ^= 1;
    t[t[pos].ch[1]].r ^= 1;
    swap(t[pos].ch[0], t[pos].ch[1]);
  }
}

func rotate(pos: dynamic)
{
  var y = t[pos].f;
  var z = t[y].f;
  var k = (t[y].ch[1] == pos);
  var w = t[pos].ch[(k ^ 1)];
  if ((!isroot(y)))
  {
    t[z].ch[(t[z].ch[1] == y)] = pos;
  }
  t[pos].f = z;
  t[pos].ch[(!k)] = y;
  t[y].ch[k] = w;
  t[w].f = y;
  t[y].f = pos;
}

var stk = cpp_array(N);

func splay(pos: dynamic)
{
  var ptr = 0;
  var tmp = pos;
  stk[cpp_update(ptr, "++")] = tmp;
  while ((!isroot(tmp)))
  {
    stk[cpp_update(ptr, "++")] = cpp_assign(tmp, "=", t[tmp].f);
  }
  while (cpp_update(ptr, "--"))
  {
    pushdown(stk[ptr]);
  }
  while ((!isroot(pos)))
  {
    var y = t[pos].f;
    var z = t[y].f;
    if ((!isroot(y)))
    {
      rotate(if ((((t[y].ch[0] == pos)) ^ ((t[z].ch[0] == y)))) pos else y);
    }
    rotate(pos);
  }
}

func access(pos: dynamic)
{
  {
    var last = 0;
    while (pos)
    {
      splay(pos);
      t[pos].ch[1] = last;
      pos = t[cpp_assign(last, "=", pos)].f;
    }
  }
}

func makeroot(pos: dynamic)
{
  access(pos);
  splay(pos);
  t[pos].r ^= 1;
}

func findroot(pos: dynamic)
{
  access(pos);
  splay(pos);
  pushdown(pos);
  while (t[pos].ch[0])
  {
    pushdown(cpp_assign(pos, "=", t[pos].ch[0]));
  }
  splay(pos);
  return pos;
}

func split(x: dynamic, y: dynamic)
{
  makeroot(x);
  access(y);
  splay(y);
}

func link(x: dynamic, y: dynamic)
{
  makeroot(x);
  if ((findroot(y) != x))
  {
    t[x].f = y;
    return 1;
  } else
  {
    return 0;
  }
}

func cut(x: dynamic, y: dynamic)
{
  makeroot(x);
  if ((((findroot(y) == x) && (t[y].f == x)) && (!t[y].ch[0])))
  {
    t[x].ch[1] = cpp_assign(t[y].f, "=", 0);
  }
}

class SegTree
{
  var t: dynamic = cpp_array((N << 2));
  var lazy: dynamic = cpp_array((N << 2));
  func lc(pos: dynamic)
  {
      return (pos << 1);
    }
  func rc(pos: dynamic)
  {
      return ((pos << 1) | 1);
    }
  func pushdown(pos: dynamic)
  {
      if (lazy[pos])
      {
        lazy[lc(pos)] += lazy[pos];
        lazy[rc(pos)] += lazy[pos];
        t[lc(pos)].mn += lazy[pos];
        t[rc(pos)].mn += lazy[pos];
        lazy[pos] = 0;
      }
    }
  func pushup(pos: dynamic)
  {
      t[pos].mn = min(t[lc(pos)].mn, t[rc(pos)].mn);
      t[pos].v = ((t[lc(pos)].v * ((t[pos].mn == t[lc(pos)].mn))) + (t[rc(pos)].v * ((t[pos].mn == t[rc(pos)].mn))));
    }
  func build(pos: dynamic, l: dynamic, r: dynamic)
  {
      t[pos].l = l;
      t[pos].r = r;
      if ((l == r))
      {
        t[pos].v = 1;
        return;
      }
      var mid = (((l + r)) >> 1);
      build(lc(pos), l, mid);
      build(rc(pos), (mid + 1), r);
      pushup(pos);
    }
  func modify(pos: dynamic, l: dynamic, r: dynamic, v: dynamic)
  {
      if (((t[pos].l == l) && (t[pos].r == r)))
      {
        lazy[pos] += v;
        t[pos].mn += v;
        return;
      }
      pushdown(pos);
      var mid = (((t[pos].l + t[pos].r)) >> 1);
      if ((r <= mid))
      {
        modify(lc(pos), l, r, v);
      } else if ((l > mid))
      {
        modify(rc(pos), l, r, v);
      } else
      {
        modify(lc(pos), l, mid, v);
        modify(rc(pos), (mid + 1), r, v);
      }
      pushup(pos);
    }
  func query(pos: dynamic, l: dynamic, r: dynamic)
  {
      if (((t[pos].l == l) && (t[pos].r == r)))
      {
        return (((t[pos].mn == 1)) * t[pos].v);
      }
      pushdown(pos);
      var mid = (((t[pos].l + t[pos].r)) >> 1);
      if ((r <= mid))
      {
        return query(lc(pos), l, r);
      } else if ((l > mid))
      {
        return query(rc(pos), l, r);
      } else
      {
        return (query(lc(pos), l, mid) + query(rc(pos), (mid + 1), r));
      }
    }
}

var t: dynamic;

var v = cpp_array(N);

var M = 3005;

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

var n: dynamic;

var m: dynamic;

var w = cpp_array(M, M);

var ans: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          read(w[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          {
            var k = 0;
            while ((k < 4))
            {
              var x = (i + dx[k]);
              var y = (j + dy[k]);
              if ((((((x < 1) || (y < 1)) || (x > n)) || (y > m)) || (w[i][j] > w[x][y])))
              {
                k += 1;
                continue;
              }
              v[w[x][y]].push_back(w[i][j]);
              v[w[i][j]].push_back(w[x][y]);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var r = 0;
  var tt = (n * m);
  t.build(1, 1, tt);
  {
    var i = 1;
    while ((i <= tt))
    {
      {
        var j = (r + 1);
        while ((j <= tt))
        {
          var fl = 0;
          for (var d in v[j])
          {
            if ((((d < j) && (d >= i)) && (!LCT.link(d, j))))
            {
              fl = 1;
              break;
            }
          }
          for (var d in v[j])
          {
            LCT.cut(j, d);
          }
          if ((fl == 1))
          {
            break;
          }
          r = j;
          var cc = 0;
          for (var d in v[j])
          {
            if (((d < j) && (d >= i)))
            {
              LCT.link(d, j);
              cc += 1;
            }
          }
          t.modify(1, r, r, ((r - i) + 1));
          t.modify(1, r, tt, (-cc));
          j += 1;
        }
      }
      ans += t.query(1, i, r);
      for (var d in v[i])
      {
        if (((d <= r) && (d > i)))
        {
          LCT.cut(d, i);
          t.modify(1, d, tt, 1);
        }
      }
      t.modify(1, i, r, -1);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

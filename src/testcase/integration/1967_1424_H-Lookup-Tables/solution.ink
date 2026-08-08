// Translated from solution.cpp.

class segtree
{
  func unite(a: dynamic, b: dynamic)
  {
      var res: dynamic;
      res.val = (a.val | b.val);
      return res;
    }
  func push(x: dynamic, l: dynamic, r: dynamic)
  {
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      if ((tree[x].put != 0))
      {
        tree[(x + 1)].apply(l, y, tree[x].put);
        tree[z].apply((y + 1), r, tree[x].put);
        tree[x].put = 0;
      }
    }
  func pull(x: dynamic, z: dynamic)
  {
      tree[x] = unite(tree[(x + 1)], tree[z]);
    }
  var n: dynamic;
  var tree: dynamic;
  func build(x: dynamic, l: dynamic, r: dynamic)
  {
      if ((l == r))
      {
        return;
      }
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      build((x + 1), l, y);
      build(z, (y + 1), r);
      pull(x, z);
    }
  func build(x: dynamic, l: dynamic, r: dynamic, v: dynamic)
  {
      if ((l == r))
      {
        tree[x].apply(l, r, v[l]);
        return;
      }
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      build((x + 1), l, y, v);
      build(z, (y + 1), r, v);
      pull(x, z);
    }
  func get(x: dynamic, l: dynamic, r: dynamic, ll: dynamic, rr: dynamic)
  {
      if (((ll <= l) && (r <= rr)))
      {
        return tree[x];
      }
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      push(x, l, r);
      var res = [];
      if ((rr <= y))
      {
        res = get((x + 1), l, y, ll, rr);
      } else
      {
        if ((ll > y))
        {
          res = get(z, (y + 1), r, ll, rr);
        } else
        {
          res = unite(get((x + 1), l, y, ll, rr), get(z, (y + 1), r, ll, rr));
        }
      }
      pull(x, z);
      return res;
    }
  func modify(x: dynamic, l: dynamic, r: dynamic, ll: dynamic, rr: dynamic, v: dynamic...)
  {
      if (((ll <= l) && (r <= rr)))
      {
        tree[x].apply(l, r, cpp_expand(v));
        return;
      }
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      push(x, l, r);
      if ((ll <= y))
      {
        modify((x + 1), l, y, ll, rr, cpp_expand(v));
      }
      if ((rr > y))
      {
        modify(z, (y + 1), r, ll, rr, cpp_expand(v));
      }
      pull(x, z);
    }
  func find_first_knowingly(x: dynamic, l: dynamic, r: dynamic, f: dynamic)
  {
      if ((l == r))
      {
        return l;
      }
      push(x, l, r);
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      var res: dynamic;
      if (f(tree[(x + 1)]))
      {
        res = find_first_knowingly((x + 1), l, y, f);
      } else
      {
        res = find_first_knowingly(z, (y + 1), r, f);
      }
      pull(x, z);
      return res;
    }
  func find_first(x: dynamic, l: dynamic, r: dynamic, ll: dynamic, rr: dynamic, f: dynamic)
  {
      if (((ll <= l) && (r <= rr)))
      {
        if ((!f(tree[x])))
        {
          return -1;
        }
        return find_first_knowingly(x, l, r, f);
      }
      push(x, l, r);
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      var res = -1;
      if ((ll <= y))
      {
        res = find_first((x + 1), l, y, ll, rr, f);
      }
      if (((rr > y) && (res == -1)))
      {
        res = find_first(z, (y + 1), r, ll, rr, f);
      }
      pull(x, z);
      return res;
    }
  func find_last_knowingly(x: dynamic, l: dynamic, r: dynamic, f: dynamic)
  {
      if ((l == r))
      {
        return l;
      }
      push(x, l, r);
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      var res: dynamic;
      if (f(tree[z]))
      {
        res = find_last_knowingly(z, (y + 1), r, f);
      } else
      {
        res = find_last_knowingly((x + 1), l, y, f);
      }
      pull(x, z);
      return res;
    }
  func find_last(x: dynamic, l: dynamic, r: dynamic, ll: dynamic, rr: dynamic, f: dynamic)
  {
      if (((ll <= l) && (r <= rr)))
      {
        if ((!f(tree[x])))
        {
          return -1;
        }
        return find_last_knowingly(x, l, r, f);
      }
      push(x, l, r);
      var y = (((l + r)) >> 1);
      var z = (x + (((((y - l) + 1)) << 1)));
      var res = -1;
      if ((rr > y))
      {
        res = find_last(z, (y + 1), r, ll, rr, f);
      }
      if (((ll <= y) && (res == -1)))
      {
        res = find_last((x + 1), l, y, ll, rr, f);
      }
      pull(x, z);
      return res;
    }
  func segtree(n: dynamic)
  {
      this->n = cpp_construct(n);
      assert((n > 0));
      tree.resize(((2 * n) - 1));
      build(0, 0, (n - 1));
    }
  func segtree(v: dynamic)
  {
      n = v.size();
      assert((n > 0));
      tree.resize(((2 * n) - 1));
      build(0, 0, (n - 1), v);
    }
  func get(ll: dynamic, rr: dynamic)
  {
      assert((((0 <= ll) && (ll <= rr)) && (rr <= (n - 1))));
      return get(0, 0, (n - 1), ll, rr);
    }
  func get(p: dynamic)
  {
      assert(((0 <= p) && (p <= (n - 1))));
      return get(0, 0, (n - 1), p, p);
    }
  func modify(ll: dynamic, rr: dynamic, v: dynamic...)
  {
      assert((((0 <= ll) && (ll <= rr)) && (rr <= (n - 1))));
      modify(0, 0, (n - 1), ll, rr, cpp_expand(v));
    }
  func find_first(ll: dynamic, rr: dynamic, f: dynamic)
  {
      assert((((0 <= ll) && (ll <= rr)) && (rr <= (n - 1))));
      return find_first(0, 0, (n - 1), ll, rr, f);
    }
  func find_last(ll: dynamic, rr: dynamic, f: dynamic)
  {
      assert((((0 <= ll) && (ll <= rr)) && (rr <= (n - 1))));
      return find_last(0, 0, (n - 1), ll, rr, f);
    }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var k: dynamic;
  var q: dynamic;
  read(k, q);
  {
    var i = 0;
    while ((i < q))
    {
      read(l[i], r[i], v[i]);
      i += 1;
    }
  }
  var msb = cpp_construct((1 << k));
  var lsb = cpp_construct((1 << k));
  {
    var i = 0;
    while ((i < q))
    {
      {
        var from_cpp = (l[i] >> k);
        var to = (r[i] >> k);
        msb.modify(from_cpp, to, v[i]);
      }
      {
        if ((((r[i] - l[i]) + 1) >= ((1 << k))))
        {
          lsb.modify(0, (((1 << k)) - 1), v[i]);
        } else
        {
          var from_cpp = (l[i] & ((((1 << k)) - 1)));
          var to = (r[i] & ((((1 << k)) - 1)));
          if ((from_cpp <= to))
          {
            lsb.modify(from_cpp, to, v[i]);
          } else
          {
            lsb.modify(from_cpp, (((1 << k)) - 1), v[i]);
            lsb.modify(0, to, v[i]);
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      var x = l[i];
      var y = r[i];
      while ((x <= y))
      {
        if (((((x & ((((1 << k)) - 1)))) == 0) && (((y - x) + 1) >= ((1 << k)))))
        {
          var full = ((((y - x) + 1)) >> k);
          var a = msb.get((x >> k), ((((x >> k)) + full) - 1)).val;
          var b = lsb.get(0, (((1 << k)) - 1)).val;
          if ((((a & b)) != v[i]))
          {
            write("impossible", cpp_char("\n"));
            return 0;
          }
          x += ((cpp_cast(full)) << k);
          continue;
        }
        var z = min(y, (x | ((((1 << k)) - 1))));
        var a = msb.get((x >> k), (x >> k)).val;
        var b = lsb.get((x & ((((1 << k)) - 1))), (z & ((((1 << k)) - 1)))).val;
        if ((((a & b)) != v[i]))
        {
          write("impossible", cpp_char("\n"));
          return 0;
        }
        x = (z + 1);
      }
      i += 1;
    }
  }
  write("possible", cpp_char("\n"));
  {
    var i = 0;
    while ((i < ((1 << k))))
    {
      write(lsb.get(i, i).val, cpp_char("\n"));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < ((1 << k))))
    {
      write(msb.get(i, i).val, cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}

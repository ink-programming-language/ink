// Translated from solution.cpp.

var INF = 1e18;

class SegmentTree
{
  func merge(a: dynamic, b: dynamic)
  {
      var ans: dynamic;
      ans.x = max(a.x, b.x);
      return ans;
    }
  func merge(a: dynamic, x: dynamic, n: dynamic)
  {
      var ans = a;
      ans.x += x;
      return ans;
    }
  var n: dynamic;
  var tree: dynamic;
  var lazy: dynamic;
  var undefined: dynamic;
  func unite(v: dynamic, a: dynamic, b: dynamic)
  {
      tree[v] = merge(tree[a], tree[b]);
    }
  func build(v: dynamic, l: dynamic, r: dynamic, A: dynamic)
  {
      if ((l == r))
      {
        tree[v].set(A[l]);
        return;
      }
      var left = (v << 1);
      var right = ((v << 1) | 1);
      var md = (((l + r)) >> 1);
      build(left, l, md, A);
      build(right, (md + 1), r, A);
      unite(v, left, right);
    }
  func build(v: dynamic, l: dynamic, r: dynamic)
  {
      if ((l == r))
      {
        return;
      }
      var left = (v << 1);
      var right = ((v << 1) | 1);
      var md = (((l + r)) >> 1);
      build(left, l, md);
      build(right, (md + 1), r);
      unite(v, left, right);
    }
  func SegmentTree(n: dynamic)
  {
      this->n = cpp_construct(n);
      assert((n > 0));
      tree.resize((4 * n));
      lazy.assign((4 * n), undefined);
      build(1, 0, (n - 1));
    }
  func SegmentTree(A: dynamic)
  {
      n = A.size();
      assert((n > 0));
      tree.resize((4 * n));
      lazy.assign((4 * n), undefined);
      build(1, 0, (n - 1), A);
    }
  func get(v: dynamic, ll: dynamic, rr: dynamic, l: dynamic, r: dynamic)
  {
      if ((l > r))
      {
        return node();
      }
      if (((l == ll) && (r == rr)))
      {
        return tree[v];
      }
      push(v, ll, rr);
      var left = (v << 1);
      var right = ((v << 1) | 1);
      var md = (((ll + rr)) >> 1);
      var a = get(left, ll, md, l, min(md, r));
      var b = get(right, (md + 1), rr, max((md + 1), l), r);
      return merge(a, b);
    }
  func uplazy(v: dynamic, ll: dynamic, rr: dynamic, x: dynamic)
  {
      tree[v] = merge(tree[v], x, ((rr - ll) + 1));
      lazy[v] += x;
    }
  func push(v: dynamic, ll: dynamic, rr: dynamic)
  {
      if ((lazy[v] != undefined))
      {
        if ((ll != rr))
        {
          var left = (v << 1);
          var right = ((v << 1) | 1);
          var md = (((ll + rr)) >> 1);
          uplazy(left, ll, md, lazy[v]);
          uplazy(right, (md + 1), rr, lazy[v]);
        }
        lazy[v] = undefined;
      }
    }
  func update(v: dynamic, ll: dynamic, rr: dynamic, l: dynamic, r: dynamic, x: dynamic)
  {
      if ((l > r))
      {
        return;
      }
      if (((ll == l) && (rr == r)))
      {
        uplazy(v, ll, rr, x);
      } else
      {
        push(v, ll, rr);
        var left = (v << 1);
        var right = ((v << 1) | 1);
        var md = (((ll + rr)) >> 1);
        update(left, ll, md, l, min(md, r), x);
        update(right, (md + 1), rr, max((md + 1), l), r, x);
        unite(v, left, right);
      }
    }
  func get(i: dynamic)
  {
      assert(((i >= 0) && (i < n)));
      var ans = get(1, 0, (n - 1), i, i);
      return ans;
    }
  func get(l: dynamic, r: dynamic)
  {
      r = min((n - 1), r);
      l = max(0, l);
      assert((l <= r));
      assert(((l >= 0) && (r < n)));
      var ans = get(1, 0, (n - 1), l, r);
      return ans;
    }
  func update(p: dynamic, x: dynamic)
  {
      assert(((p >= 0) && (p < n)));
      update(1, 0, (n - 1), p, p, x);
    }
  func update(l: dynamic, r: dynamic, x: dynamic)
  {
      r = min((n - 1), r);
      l = max(0, l);
      assert((l <= r));
      assert(((l >= 0) && (r < n)));
      update(1, 0, (n - 1), l, r, x);
    }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  var A = cpp_construct((n + 2), vector(m));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(A[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      var res = 0;
      {
        var j = 0;
        while ((j < (k - 1)))
        {
          sg.update(((j - k) + 1), j, (-A[(i + 1)][j]));
          res += (A[i][j] + A[(i + 1)][j]);
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j < m))
        {
          if (((j + k) <= m))
          {
            res += (A[i][((j + k) - 1)] + A[(i + 1)][((j + k) - 1)]);
            sg.update(j, ((j + k) - 1), (-A[(i + 1)][((j + k) - 1)]));
          }
          var tr = sg.get(0, (m - 1)).x;
          tr = max(tr, 0);
          dp2[j] = (res + tr);
          sg.update(((j - k) + 1), j, A[(i + 1)][j]);
          res -= (A[i][j] + A[(i + 1)][j]);
          j += 1;
        }
      }
      dp2.swap(dp);
      i -= 1;
    }
  }
  write((*max_element(dp.begin(), dp.end())), cpp_char("\n"));
  return 0;
}

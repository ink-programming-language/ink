// Translated from solution.cpp.

var INF = numeric_limits.max();

var MINF = numeric_limits.min();

var INFF = (((INF / 2) - 1));

var EPS = 1e-9;

var dy8 = [-1, -1, 0, 1, 1, 1, 0, -1];

var dx8 = [0, 1, 1, 1, 0, -1, -1, -1];

var dy4 = [-1, 0, 1, 0];

var dx4 = [0, 1, 0, -1];

var pi = 3.1415926535897932384626;

class SegmentTree
{
  var tree: dynamic;
  var data: dynamic;
  var left: dynamic;
  var right: dynamic;
  func build(node: dynamic, l: dynamic, r: dynamic)
  {
      if ((l == r))
      {
        tree[node] = data[l];
      } else
      {
        var mid = (((l + r)) / 2);
        var left_child = ((node * 2) + 1);
        var right_child = (left_child + 1);
        build(left_child, l, mid);
        build(right_child, (mid + 1), r);
        tree[node] = merge(tree[left_child], tree[right_child]);
      }
    }
  func query(node: dynamic, l: dynamic, r: dynamic, i: dynamic, j: dynamic)
  {
      if (((l == i) && (r == j)))
      {
        return tree[node];
      } else
      {
        var mid = (((l + r)) / 2);
        var left_child = ((node * 2) + 1);
        var right_child = (left_child + 1);
        if ((j <= mid))
        {
          return query(left_child, l, mid, i, j);
        }
        if ((i > mid))
        {
          return query(right_child, (mid + 1), r, i, j);
        }
        return merge(query(left_child, l, mid, i, mid), query(right_child, (mid + 1), r, (mid + 1), j));
      }
    }
  func update(node: dynamic, l: dynamic, r: dynamic, idx: dynamic, val: dynamic)
  {
      if ((l == r))
      {
        tree[node] += val;
      } else
      {
        var mid = (((l + r)) / 2);
        var left_child = ((node * 2) + 1);
        var right_child = (left_child + 1);
        if ((idx <= mid))
        {
          update(left_child, l, mid, idx, val);
        } else
        {
          update(right_child, (mid + 1), r, idx, val);
        }
        tree[node] = merge(tree[left_child], tree[right_child]);
      }
    }
  func SegmentTree()
  {
    }
  func SegmentTree(d: dynamic, n: dynamic)
  {
      this->left = cpp_construct(0);
      this->right = cpp_construct((n - 1));
      this->data = cpp_construct(d);
      tree = vector((n * 4));
      build(0, left, right);
    }
  func SegmentTree(d: dynamic)
  {
      this->left = cpp_construct(0);
      this->right = cpp_construct((d.size() - 1));
      this->data = cpp_construct(d.data());
      tree = vector((d.size() * 4));
      build(0, left, right);
    }
  func query(i: dynamic, j: dynamic)
  {
      return query(0, left, right, i, j);
    }
  func update(idx: dynamic, val: dynamic)
  {
      update(0, left, right, idx, val);
    }
  func set(idx: dynamic, val: dynamic)
  {
      var tmp = query(idx, idx);
      update(idx, ((-tmp) + val));
    }
}

class Interval
{
  var bestL: dynamic;
  var bestR: dynamic;
  var bestLR: dynamic;
}

func mergeInterval(a: dynamic, b: dynamic)
{
  var res: dynamic;
  res.bestL = max(a.bestL, b.bestL);
  res.bestR = max(a.bestR, b.bestR);
  res.bestLR = max(a.bestLR, b.bestLR);
  res.bestLR = max(res.bestLR, (a.bestL + b.bestR));
  return res;
}

class TaskC_516
{
  var h: dynamic = cpp_array(200002);
  var d: dynamic = cpp_array(200002);
  var sum: dynamic = cpp_array(200002);
  var v: dynamic = cpp_array(200002);
  func solve(in_cpp: dynamic, out: dynamic)
  {
      memset(h, 0, cpp_sizeof(h));
      memset(d, 0, cpp_sizeof(d));
      memset(sum, 0, cpp_sizeof(sum));
      memset(v, 0, cpp_sizeof(v));
      var n: dynamic;
      var m: dynamic;
      ((in_cpp >> n) >> m);
      {
        var i = 0;
        while ((i < n))
        {
          (in_cpp >> d[i]);
          d[(n + i)] = d[i];
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < n))
        {
          (in_cpp >> h[i]);
          h[(n + i)] = h[i];
          i += 1;
        }
      }
      v[0].bestL = cpp_assign(v[0].bestR, "=", (2 * h[0]));
      v[0].bestLR = numeric_limits.min();
      {
        var i = 1;
        while ((i <= (n << 1)))
        {
          sum[i] = (sum[(i - 1)] + d[(i - 1)]);
          v[i].bestL = ((cpp_cast(2) * h[i]) - sum[i]);
          v[i].bestR = ((cpp_cast(2) * h[i]) + sum[i]);
          v[i].bestLR = numeric_limits.min();
          i += 1;
        }
      }
      var seg = cpp_construct(v, (n << 1));
      while (cpp_update(m, "--"))
      {
        var a: dynamic;
        var b: dynamic;
        ((in_cpp >> a) >> b);
        if ((b < a))
        {
          b += n;
        }
        var length = (n - (((b - a) + 1)));
        ((out << seg.query(b, ((b + length) - 1)).bestLR) << endl);
      }
    }
}

var solver: dynamic;

func main()
{
  cin.sync_with_stdio(false);
  cin.tie(0);
  var in_cpp: dynamic;
  var out: dynamic;
  SHelper.solver.solve(in_cpp, out);
  return 0;
}

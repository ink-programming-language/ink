// Translated from solution.cpp.

var LINF = 4e18;

var mxN = (2e5 + 10);

var INF = 2e9;

var mod = (if (1) (1e9 + 7) else 998244353);

var p = cpp_array(mxN);

var is_query = (-((1 << 62)));

class line
{
  var m: dynamic;
  var b: dynamic;
  var succ: dynamic;
  func operator_less(rhs: dynamic)
  {
      if ((rhs.b != is_query))
      {
        return (m < rhs.m);
      }
      var s = succ();
      if ((!s))
      {
        return 0;
      }
      var x = rhs.m;
      return ((b - s->b) < (((s->m - m)) * x));
    }
}

class dynamic_hull
{
  var inf: dynamic;
  func bad(y: dynamic)
  {
      var z = next(y);
      if ((y == begin()))
      {
        if ((z == end()))
        {
          return 0;
        }
        return ((y->m == z->m) && (y->b <= z->b));
      }
      var x = prev(y);
      if ((z == end()))
      {
        return ((y->m == x->m) && (y->b <= x->b));
      }
      var v1 = ((x->b - y->b));
      if ((y->m == x->m))
      {
        v1 = if ((x->b > y->b)) inf else (-inf);
      } else
      {
        v1 /= ((y->m - x->m));
      }
      var v2 = ((y->b - z->b));
      if ((z->m == y->m))
      {
        v2 = if ((y->b > z->b)) inf else (-inf);
      } else
      {
        v2 /= ((z->m - y->m));
      }
      return (v1 >= v2);
    }
  func insert_line(m: dynamic, b: dynamic)
  {
      var y = insert([m, b]);
      y->succ = __cpp_lambda_1;
      if (bad(y))
      {
        erase(y);
        return;
      }
      while (((next(y) != end()) && bad(next(y))))
      {
        erase(next(y));
      }
      while (((y != begin()) && bad(prev(y))))
      {
        erase(prev(y));
      }
    }
  func eval(x: dynamic)
  {
      var l = (*lower_bound([x, is_query]));
      return ((l.m * x) + l.b);
    }
}

class segtree
{
  var n: dynamic;
  var tree: dynamic;
  func init(s: dynamic, arr: dynamic)
  {
      n = s;
      tree = vector((4 * s));
      init(1, 0, (n - 1), arr);
    }
  func init(s: dynamic, l: dynamic, r: dynamic, arr: dynamic)
  {
      {
        var i = l;
        while ((i <= r))
        {
          tree[s].insert_line((-arr[i]), (-(((i * arr[i]) - p[i]))));
          i += 1;
        }
      }
      if ((l == r))
      {
        return;
      }
      var m = (((l + r)) / 2);
      init((2 * s), l, m, arr);
      init(((2 * s) + 1), (m + 1), r, arr);
    }
  func query(l: dynamic, r: dynamic, x: dynamic)
  {
      return query(1, 0, (n - 1), l, r, x);
    }
  func query(s: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic, x: dynamic)
  {
      if ((((l > r) || (l > b)) || (r < a)))
      {
        return LINF;
      }
      if (((l >= a) && (r <= b)))
      {
        return (-tree[s].eval(x));
      }
      var m = (((l + r)) / 2);
      var q1 = query((2 * s), l, m, a, b, x);
      var q2 = query(((2 * s) + 1), (m + 1), r, a, b, x);
      return min(q1, q2);
    }
}

var n: dynamic;

var q: dynamic;

var a = cpp_array(mxN);

var st: dynamic;

func Solve()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  p[0] = a[0];
  {
    var i = 1;
    while ((i < n))
    {
      p[i] = (p[(i - 1)] + a[i]);
      i += 1;
    }
  }
  st.init(n, a);
  read(q);
  while (cpp_update(q, "--"))
  {
    var i: dynamic;
    var j: dynamic;
    read(i, j);
    j -= 1;
    write((p[j] + st.query(((j - i) + 1), j, (i - j))), cpp_char("\n"));
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  write(setprecision(12), fixed);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    Solve();
  }
  return 0;
}

func __cpp_lambda_1()
{
  return if ((next(y) == end())) 0 else (&(*next(y)));
}

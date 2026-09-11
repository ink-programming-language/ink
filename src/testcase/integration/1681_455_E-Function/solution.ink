// Translated from solution.cpp.

var LINF: dynamic = 4e18;

var mxN: dynamic = (2e5 + 10);

var INF: dynamic = 2e9;

var mod: dynamic = ( (1) ? (1e9 + 7) : 998244353);

var p: dynamic = cpp_array(mxN);

var is_query: dynamic = (-((1 << 62)));

class line
{
  var m: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var succ: dynamic = cpp_uninitialized();
  func operator_less(rhs: dynamic) -> dynamic
  {
      if ((rhs.b != is_query))
      {
        return (m < rhs.m);
      }
      var s: dynamic = succ();
      if ((!s))
      {
        return 0;
      }
      var x: dynamic = rhs.m;
      return ((b - s->b) < (((s->m - m)) * x));
    }
}

class dynamic_hull
{
  var inf: dynamic = cpp_uninitialized();
  func bad(y: dynamic) -> dynamic
  {
      var z: dynamic = next(y);
      if ((y == begin()))
      {
        if ((z == end()))
        {
          return 0;
        }
        return ((y->m == z->m) && (y->b <= z->b));
      }
      var x: dynamic = prev(y);
      if ((z == end()))
      {
        return ((y->m == x->m) && (y->b <= x->b));
      }
      var v1: dynamic = ((x->b - y->b));
      if ((y->m == x->m))
      {
        v1 =  ((x->b > y->b)) ? inf : (-inf);
      } else
      {
        v1 /= ((y->m - x->m));
      }
      var v2: dynamic = ((y->b - z->b));
      if ((z->m == y->m))
      {
        v2 =  ((y->b > z->b)) ? inf : (-inf);
      } else
      {
        v2 /= ((z->m - y->m));
      }
      return (v1 >= v2);
    }
  func insert_line(m: dynamic, b: dynamic) -> dynamic
  {
      var y: dynamic = insert([m, b]);
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
  func eval(x: dynamic) -> dynamic
  {
      var l: dynamic = (*lower_bound([x, is_query]));
      return ((l.m * x) + l.b);
    }
}

class segtree
{
  var n: dynamic = cpp_uninitialized();
  var tree: dynamic = cpp_uninitialized();
  func init(s: dynamic, arr: dynamic) -> dynamic
  {
      n = s;
      tree = vector((4 * s));
      init(1, 0, (n - 1), arr);
    }
  func init(s: dynamic, l: dynamic, r: dynamic, arr: dynamic) -> dynamic
  {
      {
        var i: dynamic = l;
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
      var m: dynamic = (((l + r)) / 2);
      init((2 * s), l, m, arr);
      init(((2 * s) + 1), (m + 1), r, arr);
    }
  func query(l: dynamic, r: dynamic, x: dynamic) -> dynamic
  {
      return query(1, 0, (n - 1), l, r, x);
    }
  func query(s: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic, x: dynamic) -> dynamic
  {
      if ((((l > r) || (l > b)) || (r < a)))
      {
        return LINF;
      }
      if (((l >= a) && (r <= b)))
      {
        return (-tree[s].eval(x));
      }
      var m: dynamic = (((l + r)) / 2);
      var q1: dynamic = query((2 * s), l, m, a, b, x);
      var q2: dynamic = query(((2 * s) + 1), (m + 1), r, a, b, x);
      return min(q1, q2);
    }
}

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(mxN);

var st: dynamic = cpp_uninitialized();

func Solve() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  p[0] = a[0];
  {
    var i: dynamic = 1;
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
    var i: dynamic = cpp_uninitialized();
    var j: dynamic = cpp_uninitialized();
    read(i, j);
    j -= 1;
    write((p[j] + st.query(((j - i) + 1), j, (i - j))), cpp_char("\n"));
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  write(setprecision(12), fixed);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    Solve();
  }
  return 0;
}

func __cpp_lambda_1() -> dynamic
{
  return  ((next(y) == end())) ? 0 : (&(*next(y)));
}

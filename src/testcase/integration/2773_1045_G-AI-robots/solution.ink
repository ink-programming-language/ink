// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var maxn = (3e5 + 10);

class Point
{
  var p: dynamic;
  var l: dynamic;
  var r: dynamic;
  var len: dynamic;
  var q: dynamic;
}

var a = cpp_array(maxn);

var b = cpp_array(maxn);

var c = cpp_array(maxn);

func low(x: dynamic)
{
  return (x & (-x));
}

func add(x: dynamic, y: dynamic)
{
  {
    while ((x <= n))
    {
      c[x] += y;
      x += low(x);
    }
  }
}

func qry(x: dynamic)
{
  var ans = 0;
  {
    while (x)
    {
      ans += c[x];
      x ^= low(x);
    }
  }
  return ans;
}

var ans = 0;

func solve(l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    return;
  }
  var mid = ((l + r) >> 1);
  solve(l, mid);
  solve((mid + 1), r);
  var L = l;
  var R = (l - 1);
  {
    var i = ((mid + 1));
    while ((i <= (r)))
    {
      while (((L <= mid) && ((a[i].q - a[L].q) > k)))
      {
        add(a[cpp_update(L, "++")].p, -1);
      }
      while (((R < mid) && ((a[(R + 1)].q - a[i].q) <= k)))
      {
        add(a[cpp_update(R, "++")].p, 1);
      }
      ans += (qry(a[i].r) - qry((a[i].l - 1)));
      i += 1;
    }
  }
  {
    var i = (L);
    while ((i <= (R)))
    {
      add(a[i].p, -1);
      i += 1;
    }
  }
  sort((a + l), ((a + r) + 1), __cpp_lambda_1);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, k);
  {
    var i = (1);
    while ((i <= (n)))
    {
      read(a[i].p, a[i].len, a[i].q);
      i += 1;
    }
  }
  var len = 0;
  {
    var i = (1);
    while ((i <= (n)))
    {
      b[cpp_update(len, "++")] = a[i].p;
      i += 1;
    }
  }
  sort((b + 1), ((b + len) + 1));
  len = ((unique((b + 1), ((b + len) + 1)) - b) - 1);
  {
    var i = (1);
    while ((i <= (n)))
    {
      a[i].l = (lower_bound((b + 1), ((b + len) + 1), (a[i].p - a[i].len)) - b);
      a[i].r = ((upper_bound((b + 1), ((b + len) + 1), (a[i].p + a[i].len)) - b) - 1);
      a[i].p = (lower_bound((b + 1), ((b + len) + 1), a[i].p) - b);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), __cpp_lambda_2);
  solve(1, n);
  write(ans, cpp_char("\n"));
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (a.q < b.q);
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return (a.len > b.len);
}

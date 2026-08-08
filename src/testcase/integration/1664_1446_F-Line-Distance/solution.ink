// Translated from solution.cpp.

func read(n: dynamic)
{
  var w = 1;
  n = 0;
  var ch = getchar();
  while (((!isdigit(ch)) && (ch != EOF)))
  {
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
    ch = getchar();
  }
  while ((isdigit(ch) && (ch != EOF)))
  {
    n = ((((n << 3)) + ((n << 1))) + ((ch & 15)));
    ch = getchar();
  }
  n *= w;
}

func write(x: dynamic)
{
  var l = 0;
  var y = 0;
  if ((x < 0))
  {
    x = (-x);
    putchar(cpp_char("-"));
  }
  if ((!x))
  {
    putchar(48);
    return;
  }
  while (x)
  {
    y = ((y * 10) + (x % 10));
    x /= 10;
    l += 1;
  }
  while (l)
  {
    putchar(((y % 10) + 48));
    y /= 10;
    l -= 1;
  }
}

func writes(x: dynamic)
{
  write(x);
  putchar(cpp_char(" "));
}

func writeln(x: dynamic)
{
  write(x);
  puts("");
}

func checkmax(a: dynamic, b: dynamic)
{
  a = if ((a > b)) a else b;
}

func checkmin(a: dynamic, b: dynamic)
{
  a = if ((a < b)) a else b;
}

var N = (2e5 + 10);

var eps = 1e-10;

var pi = acos(-1);

var n: dynamic;

var sum: dynamic;

var it: dynamic;

var pp: dynamic;

var k: dynamic;

class Point
{
  var x: dynamic;
  var y: dynamic;
}

var p = cpp_array(N);

var yd: dynamic;

func dist(x: dynamic, y: dynamic)
{
  return sqrt(((((x.x - y.x)) * ((x.x - y.x))) + (((x.y - y.y)) * ((x.y - y.y)))));
}

func doit(x: dynamic)
{
  x -= ((int_cpp((x / ((2 * pi)))) * 2) * pi);
  if ((x < (-eps)))
  {
    x += (2 * pi);
  }
}

class yy
{
  var l: dynamic;
  var r: dynamic;
}

var q = cpp_array(N);

func gaota(l: dynamic, r: dynamic)
{
  doit(l);
  doit(r);
  if ((l > r))
  {
    swap(l, r);
  }
  q[cpp_update(sum, "++")] = [l, r];
}

var t = cpp_array((N * 2));

func cmp1(x: dynamic, y: dynamic)
{
  return (x.r < y.r);
}

func cmp2(x: dynamic, y: dynamic)
{
  return ((*x) < (*y));
}

func lowbit(x: dynamic)
{
  return (x & ((-x)));
}

func insert(x: dynamic)
{
  {
    while ((x <= it))
    {
      t[x] += 1;
      x += lowbit(x);
    }
  }
}

func qry(x: dynamic)
{
  var ss = 0;
  {
    while (x)
    {
      ss += t[x];
      x -= lowbit(x);
    }
  }
  return ss;
}

func check(x: dynamic)
{
  sum = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var dis = dist(p[i], yd);
      if ((dis >= x))
      {
        var jd1 = atan2(p[i].y, p[i].x);
        var jd2 = acos((x / dis));
        gaota((jd1 - jd2), (jd1 + jd2));
      }
      i += 1;
    }
  }
  var res = (((1 * sum) * ((n - sum))) + (((1 * ((n - sum))) * (((n - sum) - 1))) / 2));
  sort((q + 1), ((q + sum) + 1), cmp1);
  {
    var i = 1;
    while ((i <= sum))
    {
      pp[i] = (&q[i].l);
      pp[(sum + i)] = (&q[i].r);
      i += 1;
    }
  }
  sort((pp + 1), ((pp + (2 * sum)) + 1), cmp2);
  var las = 14913233;
  it = 0;
  {
    var i = 1;
    while ((i <= (2 * sum)))
    {
      if (((*pp[i]) != las))
      {
        las = (*pp[i]);
        it += 1;
      }
      (*pp[i]) = it;
      i += 1;
    }
  }
  memset(t, 0, (cpp_sizeof(dynamic) * ((it + 1))));
  {
    var i = sum;
    while ((i >= 1))
    {
      res += ((qry(int_cpp(q[i].l)) + ((sum - i))) - qry(int_cpp(q[i].r)));
      insert(int_cpp(q[i].l));
      i -= 1;
    }
  }
  return (res >= k);
}

func main()
{
  read(n);
  read(k);
  yd = [0, 0];
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lf%lf", (&p[i].x), (&p[i].y));
      i += 1;
    }
  }
  var l = 0;
  var r = 2e8;
  while ((((r - l)) >= eps))
  {
    var mid = (((l + r)) / 2);
    if (check(mid))
    {
      r = mid;
    } else
    {
      l = mid;
    }
  }
  printf("%.8lf\n", l);
  return 0;
}

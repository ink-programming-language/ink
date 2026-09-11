// Translated from solution.cpp.

func read(n: dynamic) -> dynamic
{
  var w: dynamic = 1;
  n = 0;
  var ch: dynamic = getchar();
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

func write(x: dynamic) -> dynamic
{
  var l: dynamic = 0;
  var y: dynamic = 0;
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

func writes(x: dynamic) -> dynamic
{
  write(x);
  putchar(cpp_char(" "));
}

func writeln(x: dynamic) -> dynamic
{
  write(x);
  puts("");
}

func checkmax(a: dynamic, b: dynamic) -> dynamic
{
  a =  ((a > b)) ? a : b;
}

func checkmin(a: dynamic, b: dynamic) -> dynamic
{
  a =  ((a < b)) ? a : b;
}

var N: dynamic = (2e5 + 10);

var eps: dynamic = 1e-10;

var pi: dynamic = acos(-1);

var n: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var it: dynamic = cpp_uninitialized();

var pp: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(N);

var yd: dynamic = cpp_uninitialized();

func dist(x: dynamic, y: dynamic) -> dynamic
{
  return sqrt(((((x.x - y.x)) * ((x.x - y.x))) + (((x.y - y.y)) * ((x.y - y.y)))));
}

func doit(x: dynamic) -> dynamic
{
  x -= ((int_cpp((x / ((2 * pi)))) * 2) * pi);
  if ((x < (-eps)))
  {
    x += (2 * pi);
  }
}

class yy
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
}

var q: dynamic = cpp_array(N);

func gaota(l: dynamic, r: dynamic) -> dynamic
{
  doit(l);
  doit(r);
  if ((l > r))
  {
    swap(l, r);
  }
  q[cpp_update(sum, "++")] = [l, r];
}

var t: dynamic = cpp_array((N * 2));

func cmp1(x: dynamic, y: dynamic) -> dynamic
{
  return (x.r < y.r);
}

func cmp2(x: dynamic, y: dynamic) -> dynamic
{
  return ((*x) < (*y));
}

func lowbit(x: dynamic) -> dynamic
{
  return (x & ((-x)));
}

func insert(x: dynamic) -> dynamic
{
  {
    while ((x <= it))
    {
      t[x] += 1;
      x += lowbit(x);
    }
  }
}

func qry(x: dynamic) -> dynamic
{
  var ss: dynamic = 0;
  {
    while (x)
    {
      ss += t[x];
      x -= lowbit(x);
    }
  }
  return ss;
}

func check(x: dynamic) -> dynamic
{
  sum = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var dis: dynamic = dist(p[i], yd);
      if ((dis >= x))
      {
        var jd1: dynamic = atan2(p[i].y, p[i].x);
        var jd2: dynamic = acos((x / dis));
        gaota((jd1 - jd2), (jd1 + jd2));
      }
      i += 1;
    }
  }
  var res: dynamic = (((1 * sum) * ((n - sum))) + (((1 * ((n - sum))) * (((n - sum) - 1))) / 2));
  sort((q + 1), ((q + sum) + 1), cmp1);
  {
    var i: dynamic = 1;
    while ((i <= sum))
    {
      pp[i] = (&q[i].l);
      pp[(sum + i)] = (&q[i].r);
      i += 1;
    }
  }
  sort((pp + 1), ((pp + (2 * sum)) + 1), cmp2);
  var las: dynamic = 14913233;
  it = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = sum;
    while ((i >= 1))
    {
      res += ((qry(int_cpp(q[i].l)) + ((sum - i))) - qry(int_cpp(q[i].r)));
      insert(int_cpp(q[i].l));
      i -= 1;
    }
  }
  return (res >= k);
}

func main() -> dynamic
{
  read(n);
  read(k);
  yd = [0, 0];
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lf%lf", (&p[i].x), (&p[i].y));
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = 2e8;
  while ((((r - l)) >= eps))
  {
    var mid: dynamic = (((l + r)) / 2);
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

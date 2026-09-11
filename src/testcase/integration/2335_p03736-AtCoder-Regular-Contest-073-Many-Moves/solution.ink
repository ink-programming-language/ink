// Translated from solution.cpp.

func getint() -> dynamic
{
  var ch: dynamic = cpp_uninitialized();
  while ((!isdigit(cpp_assign(ch, "=", getchar()))))
  {
  }
  var x: dynamic = (ch ^ cpp_char("0"));
  while (isdigit(cpp_assign(ch, "=", getchar())))
  {
    x = (((((((x << 2)) + x)) << 1)) + ((ch ^ cpp_char("0"))));
  }
  return x;
}

func min(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? a : b;
}

var inf: dynamic = 0x7ffffffffffffff;

var N: dynamic = 200001;

var n: dynamic = cpp_uninitialized();

class FenwickTree
{
  var val: dynamic = cpp_array(N);
  func lowbit(x: dynamic) -> dynamic
  {
      return (x & (-x));
    }
  func FenwickTree() -> dynamic
  {
      fill((&val[0]), (&val[N]), inf);
    }
  func modify(p: dynamic, x: dynamic) -> dynamic
  {
      while ((p <= n))
      {
        val[p] = min(val[p], x);
        p += lowbit(p);
      }
    }
  func query(p: dynamic) -> dynamic
  {
      var ret: dynamic = inf;
      while (p)
      {
        ret = min(ret, val[p]);
        p -= lowbit(p);
      }
      return ret;
    }
}

var ta: dynamic = cpp_uninitialized();

class RevFenwickTree
{
  var val: dynamic = cpp_array(N);
  func lowbit(x: dynamic) -> dynamic
  {
      return (x & (-x));
    }
  func RevFenwickTree() -> dynamic
  {
      fill((&val[0]), (&val[N]), inf);
    }
  func modify(p: dynamic, x: dynamic) -> dynamic
  {
      while (p)
      {
        val[p] = min(val[p], x);
        p -= lowbit(p);
      }
    }
  func query(p: dynamic) -> dynamic
  {
      var ret: dynamic = inf;
      while ((p <= n))
      {
        ret = min(ret, val[p]);
        p += lowbit(p);
      }
      return ret;
    }
}

var tb: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

func modify(p: dynamic, x: dynamic) -> dynamic
{
  if ((x < f[p]))
  {
    f[p] = x;
    ta.modify(p, (x - p));
    tb.modify(p, (x + p));
  }
}

func main() -> dynamic
{
  n = getint();
  var q: dynamic = getint();
  var a: dynamic = getint();
  var b: dynamic = getint();
  fill((&f[0]), (&f[N]), inf);
  modify(a, 0);
  var sum: dynamic = 0;
  while (cpp_update(q, "--"))
  {
    a = b;
    b = getint();
    sum += abs((a - b));
    var t1: dynamic = (ta.query(b) + b);
    var t2: dynamic = (tb.query(b) - b);
    modify(a, (min(t1, t2) - abs((a - b))));
  }
  var tmp: dynamic = inf;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      tmp = min(tmp, f[i]);
      i += 1;
    }
  }
  printf("%lld\n", (tmp + sum));
  return 0;
}

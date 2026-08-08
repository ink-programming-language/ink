// Translated from solution.cpp.

func getint()
{
  var ch: dynamic;
  while ((!isdigit(cpp_assign(ch, "=", getchar()))))
  {
  }
  var x = (ch ^ cpp_char("0"));
  while (isdigit(cpp_assign(ch, "=", getchar())))
  {
    x = (((((((x << 2)) + x)) << 1)) + ((ch ^ cpp_char("0"))));
  }
  return x;
}

func min(a: dynamic, b: dynamic)
{
  return if ((a < b)) a else b;
}

var inf = 0x7ffffffffffffff;

var N = 200001;

var n: dynamic;

class FenwickTree
{
  var val: dynamic = cpp_array(N);
  func lowbit(x: dynamic)
  {
      return (x & (-x));
    }
  func FenwickTree()
  {
      fill((&val[0]), (&val[N]), inf);
    }
  func modify(p: dynamic, x: dynamic)
  {
      while ((p <= n))
      {
        val[p] = min(val[p], x);
        p += lowbit(p);
      }
    }
  func query(p: dynamic)
  {
      var ret = inf;
      while (p)
      {
        ret = min(ret, val[p]);
        p -= lowbit(p);
      }
      return ret;
    }
}

var ta: dynamic;

class RevFenwickTree
{
  var val: dynamic = cpp_array(N);
  func lowbit(x: dynamic)
  {
      return (x & (-x));
    }
  func RevFenwickTree()
  {
      fill((&val[0]), (&val[N]), inf);
    }
  func modify(p: dynamic, x: dynamic)
  {
      while (p)
      {
        val[p] = min(val[p], x);
        p -= lowbit(p);
      }
    }
  func query(p: dynamic)
  {
      var ret = inf;
      while ((p <= n))
      {
        ret = min(ret, val[p]);
        p += lowbit(p);
      }
      return ret;
    }
}

var tb: dynamic;

var f = cpp_array(N);

func modify(p: dynamic, x: dynamic)
{
  if ((x < f[p]))
  {
    f[p] = x;
    ta.modify(p, (x - p));
    tb.modify(p, (x + p));
  }
}

func main()
{
  n = getint();
  var q = getint();
  var a = getint();
  var b = getint();
  fill((&f[0]), (&f[N]), inf);
  modify(a, 0);
  var sum = 0;
  while (cpp_update(q, "--"))
  {
    a = b;
    b = getint();
    sum += abs((a - b));
    var t1 = (ta.query(b) + b);
    var t2 = (tb.query(b) - b);
    modify(a, (min(t1, t2) - abs((a - b))));
  }
  var tmp = inf;
  {
    var i = 1;
    while ((i <= n))
    {
      tmp = min(tmp, f[i]);
      i += 1;
    }
  }
  printf("%lld\n", (tmp + sum));
  return 0;
}

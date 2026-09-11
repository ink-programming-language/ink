// Translated from solution.cpp.

var pi: dynamic = acos(-1);

var eps: dynamic = 1e-8;

var maxn: dynamic = (4e5 + 5);

var maxm: dynamic = (4e4 + 5);

var mod: dynamic = (1e9 + 7);

var inf: dynamic = 0x3f3f3f3f;

var inf: dynamic = (-1e9 + 7);

func scan() -> dynamic
{
  var m: dynamic = 0;
  var c: dynamic = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    m = (((m * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return m;
}

var N: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

class node
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func node() -> dynamic
  {
    }
  func node(a: dynamic, b: dynamic) -> dynamic
  {
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
    }
  func operator_less(t: dynamic) -> dynamic
  {
      return ((a * t.b) < (b * t.a));
    }
  func operator_equal(t: dynamic) -> dynamic
  {
      return ((a * t.b) == (b * t.a));
    }
  func inv() -> dynamic
  {
      return node((-(self->a)), (-(self->b)));
    }
}

var lk: dynamic = cpp_array(maxn, 4);

var lkcnt: dynamic = cpp_array(4);

var lkcnt: dynamic = cpp_array(4);

func work(i: dynamic, j: dynamic) -> dynamic
{
  if (((i == 0) && (j == 1)))
  {
    return (lkcnt[0] - lkcnt[0]);
  }
  if (((i == 0) && (j == 2)))
  {
    return (lkcnt[0] - lkcnt[1]);
  }
  if (((i == 1) && (j == 2)))
  {
    return (lkcnt[1] - lkcnt[2]);
  }
  if (((i == 3) && (j == 1)))
  {
    return (lkcnt[3] - lkcnt[3]);
  }
  return lkcnt[i];
}

func main() -> dynamic
{
  read(N, C, D);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      a -= C;
      b -= D;
      if (((a >= 0) && (b >= 0)))
      {
        lk[0][cpp_update(lkcnt[0], "++")] = node(a, b);
        if ((a == 0))
        {
          lkcnt[0] += 1;
        }
        if ((b == 0))
        {
          lkcnt[1] += 1;
        }
      } else if (((a < 0) && (b >= 0)))
      {
        lk[1][cpp_update(lkcnt[1], "++")] = node(a, b);
        if ((b == 0))
        {
          lkcnt[2] += 1;
        }
      } else if (((a < 0) && (b < 0)))
      {
        lk[2][cpp_update(lkcnt[2], "++")] = node(a, b);
      } else
      {
        lk[3][cpp_update(lkcnt[3], "++")] = node(a, b);
        if ((a == 0))
        {
          lkcnt[3] += 1;
        }
      }
      i += 1;
    }
  }
  sort(lk[0], (lk[0] + lkcnt[0]));
  sort(lk[1], (lk[1] + lkcnt[1]));
  sort(lk[2], (lk[2] + lkcnt[2]));
  sort(lk[3], (lk[3] + lkcnt[3]));
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      var i2: dynamic = (((i + 2)) % 4);
      var i1: dynamic = (((i + 1)) % 4);
      {
        var j: dynamic = 0;
        while ((j < lkcnt[i]))
        {
          var l: dynamic = (lower_bound(lk[i2], (lk[i2] + lkcnt[i2]), lk[i][j].inv()) - lk[i2]);
          var r: dynamic = (upper_bound(lk[i2], (lk[i2] + lkcnt[i2]), lk[i][j].inv()) - lk[i2]);
          if (((r < lkcnt[i2]) && (lk[i][j].inv() == lk[i2][r])))
          {
            r += 1;
          }
          sum += ((1 * l) * ((lkcnt[i2] - r)));
          var x: dynamic = lk[i][j].a;
          var y: dynamic = lk[i][j].b;
          sum += ((1 * l) * lkcnt[i1]);
          if (((i == 0) && (y == 0)))
          {
            sum -= ((1 * l) * lkcnt[2]);
          }
          if (((i == 0) && (x == 0)))
          {
            sum -= ((1 * ((lkcnt[2] - r))) * lkcnt[3]);
          }
          if (((i == 3) && (x == 0)))
          {
            sum -= ((1 * l) * lkcnt[0]);
          }
          if (((i == 1) && (y == 0)))
          {
            sum -= ((1 * ((lkcnt[3] - r))) * lkcnt[1]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(sum, "\n");
  return 0;
}

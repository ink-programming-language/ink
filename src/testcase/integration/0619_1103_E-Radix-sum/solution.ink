// Translated from solution.cpp.

var N = 100000;

var f = cpp_array(11111);

var g = cpp_array(11111);

class Ring
{
  var a: dynamic = cpp_array(5);
  func Ring()
  {
    }
  func clear()
  {
      memset(a, 0, cpp_sizeof(a));
    }
  func operator_add(r: dynamic)
  {
      var R = r;
      {
        var i = 0;
        while ((i < 5))
        {
          R.a[i] += a[i];
          i += 1;
        }
      }
      return R;
    }
  func operator_add_assign(r: dynamic)
  {
      ((*this)) = (((*this)) + r);
    }
  func operator_multiply(r: dynamic)
  {
      var R: dynamic;
      R.clear();
      {
        var i = 0;
        while ((i < 5))
        {
          {
            var j = 0;
            while ((j < 5))
            {
              R.a[f[(i + j)]] += ((a[i] * r.a[j]) * g[(i + j)]);
              j += 1;
            }
          }
          i += 1;
        }
      }
      return R;
    }
  func operator(r: dynamic)
  {
      ((*this)) = (((*this)) * r);
    }
  func operator_shift_left(k: dynamic)
  {
      var R: dynamic;
      {
        var i = 0;
        while ((i < 5))
        {
          R.a[f[(i + k)]] = (a[i] * g[(i + k)]);
          i += 1;
        }
      }
      return R;
    }
  func real()
  {
      return (a[0] + a[1]);
    }
}

var x = cpp_array(N);

var tmp = cpp_array(10);

func power(a: dynamic, n: dynamic, ans: dynamic = 1)
{
  {
    while (n)
    {
      if ((n & 1))
      {
        ans *= a;
      }
      n >>= 1;
      a *= a;
    }
  }
  return ans;
}

func power(a: dynamic, n: dynamic)
{
  var ans: dynamic;
  ans.clear();
  ans.a[0] = 1;
  {
    while (n)
    {
      if ((n & 1))
      {
        ans *= a;
      }
      n >>= 1;
      a *= a;
    }
  }
  return ans;
}

func DFT(P: dynamic, op: dynamic)
{
  {
    var i = 1;
    while ((i < N))
    {
      {
        var p = (i * 10);
        var j = 0;
        while ((j < N))
        {
          {
            var k = 0;
            while ((k < i))
            {
              {
                var x = 0;
                while ((x < 10))
                {
                  tmp[x] = P[((j + k) + (x * i))];
                  x += 1;
                }
              }
              {
                var x = 0;
                var t = 0;
                while ((x < 10))
                {
                  var r = P[((j + k) + (x * i))];
                  r.clear();
                  {
                    var y = 0;
                    var d = 0;
                    while ((y < 10))
                    {
                      r += (tmp[y] << d);
                      y += 1;
                      d += t;
                    }
                  }
                  x += 1;
                  t += op;
                }
              }
              k += 1;
            }
          }
          j += p;
        }
      }
      i *= 10;
    }
  }
}

var n: dynamic;

func main()
{
  memset(x, 0, cpp_sizeof(x));
  scanf("%d", (&n));
  {
    var i = 0;
    var k: dynamic;
    while ((i < n))
    {
      scanf("%d", (&k));
      x[k].a[0] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 1111))
    {
      f[i] = (i % 5);
      g[i] = if (((i % 10) < 5)) 1 else -1;
      i += 1;
    }
  }
  DFT(x, 1);
  {
    var i = 0;
    while ((i < N))
    {
      x[i] = power(x[i], n);
      i += 1;
    }
  }
  DFT(x, 9);
  var inv = power(5, (((1 << 63)) - 5));
  {
    var i = 0;
    while ((i < n))
    {
      printf("%I64d\n", ((((x[i].real() >> 5)) * inv) & ((((1 << 58)) - 1))));
      i += 1;
    }
  }
}

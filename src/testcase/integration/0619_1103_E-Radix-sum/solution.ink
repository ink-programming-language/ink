// Translated from solution.cpp.

var N: dynamic = 100000;

var f: dynamic = cpp_array(11111);

var g: dynamic = cpp_array(11111);

class Ring
{
  var a: dynamic = cpp_array(5);
  func Ring() -> dynamic
  {
    }
  func clear() -> dynamic
  {
      memset(a, 0, cpp_sizeof(a));
    }
  func operator_add(r: dynamic) -> dynamic
  {
      var R: dynamic = r;
      {
        var i: dynamic = 0;
        while ((i < 5))
        {
          R.a[i] += a[i];
          i += 1;
        }
      }
      return R;
    }
  func operator_add_assign(r: dynamic) -> dynamic
  {
      ((*self)) = (((*self)) + r);
    }
  func operator_multiply(r: dynamic) -> dynamic
  {
      var R: dynamic = cpp_uninitialized();
      R.clear();
      {
        var i: dynamic = 0;
        while ((i < 5))
        {
          {
            var j: dynamic = 0;
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
  func operator(r: dynamic) -> dynamic
  {
      ((*self)) = (((*self)) * r);
    }
  func operator_shift_left(k: dynamic) -> dynamic
  {
      var R: dynamic = cpp_uninitialized();
      {
        var i: dynamic = 0;
        while ((i < 5))
        {
          R.a[f[(i + k)]] = (a[i] * g[(i + k)]);
          i += 1;
        }
      }
      return R;
    }
  func real() -> dynamic
  {
      return (a[0] + a[1]);
    }
}

var x: dynamic = cpp_array(N);

var tmp: dynamic = cpp_array(10);

func power(a: dynamic, n: dynamic, ans: dynamic = 1) -> dynamic
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

func power(a: dynamic, n: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
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

func DFT(P: dynamic, op: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      {
        var p: dynamic = (i * 10);
        var j: dynamic = 0;
        while ((j < N))
        {
          {
            var k: dynamic = 0;
            while ((k < i))
            {
              {
                var x: dynamic = 0;
                while ((x < 10))
                {
                  tmp[x] = P[((j + k) + (x * i))];
                  x += 1;
                }
              }
              {
                var x: dynamic = 0;
                var t: dynamic = 0;
                while ((x < 10))
                {
                  var r: dynamic = P[((j + k) + (x * i))];
                  r.clear();
                  {
                    var y: dynamic = 0;
                    var d: dynamic = 0;
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

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  memset(x, 0, cpp_sizeof(x));
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    var k: dynamic = cpp_uninitialized();
    while ((i < n))
    {
      scanf("%d", (&k));
      x[k].a[0] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 1111))
    {
      f[i] = (i % 5);
      g[i] =  (((i % 10) < 5)) ? 1 : -1;
      i += 1;
    }
  }
  DFT(x, 1);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      x[i] = power(x[i], n);
      i += 1;
    }
  }
  DFT(x, 9);
  var inv: dynamic = power(5, (((1 << 63)) - 5));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      printf("%I64d\n", ((((x[i].real() >> 5)) * inv) & ((((1 << 58)) - 1))));
      i += 1;
    }
  }
}

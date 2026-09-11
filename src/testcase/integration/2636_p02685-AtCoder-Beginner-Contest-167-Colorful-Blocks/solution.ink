// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = 0;

var p: dynamic = cpp_array(200001);

var q: dynamic = cpp_array(200001);

var r: dynamic = cpp_array(200001);

var M: dynamic = 998244353;

func pow(a: dynamic, b: dynamic) -> dynamic
{
  var w: dynamic = 1;
  var x: dynamic = a;
  while (b)
  {
    if ((b & 1))
    {
      w = ((w * x) % M);
    }
    x = ((x * x) % M);
    b >>= 1;
  }
  return w;
}

func main() -> dynamic
{
  read(n, m, k);
  p[0] = 1;
  q[0] = 1;
  r[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      p[i] = ((p[(i - 1)] * i) % M);
      q[i] = pow(p[i], (M - 2));
      r[i] = ((r[(i - 1)] * ((m - 1))) % M);
      i += 1;
    }
  }
  {
    var j: dynamic = 0;
    while ((j <= k))
    {
      var s: dynamic = m;
      s = (((((((((((s * p[(n - 1)]) % M)) * q[j]) % M)) * q[((n - j) - 1)]) % M)) * r[((n - j) - 1)]) % M);
      t = (((t + s)) % M);
      j += 1;
    }
  }
  write(t, "\n");
}

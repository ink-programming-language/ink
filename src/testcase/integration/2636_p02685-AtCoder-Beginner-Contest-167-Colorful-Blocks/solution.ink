// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var k: dynamic;

var t = 0;

var p = cpp_array(200001);

var q = cpp_array(200001);

var r = cpp_array(200001);

var M = 998244353;

func pow(a: dynamic, b: dynamic)
{
  var w = 1;
  var x = a;
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

func main()
{
  read(n, m, k);
  p[0] = 1;
  q[0] = 1;
  r[0] = 1;
  {
    var i = 1;
    while ((i < n))
    {
      p[i] = ((p[(i - 1)] * i) % M);
      q[i] = pow(p[i], (M - 2));
      r[i] = ((r[(i - 1)] * ((m - 1))) % M);
      i += 1;
    }
  }
  {
    var j = 0;
    while ((j <= k))
    {
      var s = m;
      s = (((((((((((s * p[(n - 1)]) % M)) * q[j]) % M)) * q[((n - j) - 1)]) % M)) * r[((n - j) - 1)]) % M);
      t = (((t + s)) % M);
      j += 1;
    }
  }
  write(t, "\n");
}

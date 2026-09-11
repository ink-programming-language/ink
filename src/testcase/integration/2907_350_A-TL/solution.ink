// Translated from solution.cpp.

var d: dynamic = cpp_array(10);

var e: dynamic = cpp_array(10);

func po(n: dynamic, exp: dynamic) -> dynamic
{
  var prod: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < exp))
    {
      prod = ((prod * n));
      i += 1;
    }
  }
  return prod;
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = 0;
  var t: dynamic = cpp_uninitialized();
  read(n, m);
  var a: dynamic = cpp_array(n);
  var b: dynamic = cpp_array(m);
  var c: dynamic = cpp_array(n);
  var f: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(b[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  sort(b, (b + m));
  k = max((2 * a[0]), a[(n - 1)]);
  t = b[0];
  if ((k >= t))
  {
    write("-1");
  } else
  {
    write(k);
  }
}

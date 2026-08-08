// Translated from solution.cpp.

var d = cpp_array(10);

var e = cpp_array(10);

func po(n: dynamic, exp: dynamic)
{
  var prod = 1;
  {
    var i = 0;
    while ((i < exp))
    {
      prod = ((prod * n));
      i += 1;
    }
  }
  return prod;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var n: dynamic;
  var m: dynamic;
  var s = 0;
  var t: dynamic;
  read(n, m);
  var a = cpp_array(n);
  var b = cpp_array(m);
  var c = cpp_array(n);
  var f: dynamic;
  var g: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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

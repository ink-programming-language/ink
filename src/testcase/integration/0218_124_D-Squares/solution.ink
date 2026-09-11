// Translated from solution.cpp.

func f(s: dynamic) -> dynamic
{
  var k: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      k = (((10 * k) + int_cpp(s[i])) - 48);
      i += 1;
    }
  }
  return k;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var x3: dynamic = cpp_uninitialized();
  var y3: dynamic = cpp_uninitialized();
  var x4: dynamic = cpp_uninitialized();
  var y4: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var x1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  read(a, b, x1, y1, x2, y2);
  if (((x1 + y1) >= 0))
  {
    x3 = (((x1 + y1)) / ((2 * a)));
  } else
  {
    x3 = ((((x1 + y1)) / ((2 * a))) - 1);
  }
  if (((x1 - y1) >= 0))
  {
    y3 = (((x1 - y1)) / ((2 * b)));
  } else
  {
    y3 = ((((x1 - y1)) / ((2 * b))) - 1);
  }
  if (((x2 + y2) >= 0))
  {
    x4 = (((x2 + y2)) / ((2 * a)));
  } else
  {
    x4 = ((((x2 + y2)) / ((2 * a))) - 1);
  }
  if (((x2 - y2) >= 0))
  {
    y4 = (((x2 - y2)) / ((2 * b)));
  } else
  {
    y4 = ((((x2 - y2)) / ((2 * b))) - 1);
  }
  k = max(abs((x3 - x4)), abs((y3 - y4)));
  write(k);
  return 0;
}

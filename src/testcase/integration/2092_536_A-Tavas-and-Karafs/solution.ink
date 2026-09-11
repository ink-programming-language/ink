// Translated from solution.cpp.

var INF: dynamic = 1000000000;

var PI: dynamic = 3.1415926535897932384626433832795028841971;

var sum: dynamic = cpp_array(10000010);

var s: dynamic = cpp_array(10000010);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  read(a, b, n);
  {
    i = 1;
    while ((i <= 10000000))
    {
      s[i] = (a + (((i - 1)) * b));
      sum[i] = (sum[(i - 1)] + s[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      read(l, t, m);
      var left: dynamic = l;
      var right: dynamic = 10000000;
      var middle: dynamic = cpp_uninitialized();
      while (((left + 1) != right))
      {
        middle = (((left + right)) / 2);
        if ((middle < l))
        {
          left = middle;
          break;
        }
        if (((s[middle] <= t) && ((sum[middle] - sum[(l - 1)]) <= (t * m))))
        {
          left = middle;
        } else
        {
          right = middle;
        }
      }
      if ((!(((s[left] <= t) && ((sum[left] - sum[(l - 1)]) <= (t * m))))))
      {
        left = -1;
      }
      if ((left < l))
      {
        left = -1;
      }
      write(left, "\n");
      i += 1;
    }
  }
}

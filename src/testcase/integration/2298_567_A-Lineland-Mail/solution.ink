// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var p: dynamic = cpp_array(n);
  var q: dynamic = cpp_array(n);
  p[0] = abs((a[(n - 1)] - a[0]));
  q[0] = abs((a[1] - a[0]));
  p[(n - 1)] = p[0];
  q[(n - 1)] = abs((a[(n - 1)] - a[(n - 2)]));
  write(q[0], " ", p[0], "\n");
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      p[i] = max(abs((a[i] - a[0])), abs((a[i] - a[(n - 1)])));
      q[i] = min(abs((a[i] - a[(i - 1)])), abs((a[i] - a[(i + 1)])));
      write(q[i], " ", p[i], "\n");
      i += 1;
    }
  }
  write(q[(n - 1)], " ", p[(n - 1)], "\n");
  return 0;
}

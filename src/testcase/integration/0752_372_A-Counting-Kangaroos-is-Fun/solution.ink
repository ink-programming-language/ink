// Translated from solution.cpp.

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var c: dynamic = 0;
  read(n);
  var x: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      v.push_back(x);
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var r: dynamic = (n - 1);
  {
    var i: dynamic = ((n / 2) - 1);
    while ((i >= 0))
    {
      if (((2 * v[i]) <= v[r]))
      {
        c += 1;
        r -= 1;
      }
      i -= 1;
    }
  }
  write((n - c));
  return 0;
}

// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var cnt: dynamic = 2;
  var ans: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = 0;
  var pi: dynamic = 3.1415926536;
  read(n);
  var a: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < n))
    {
      read(p);
      a.push_back(p);
      i += 1;
    }
  }
  a.push_back(0);
  sort(a.begin(), a.end());
  {
    i = n;
    while ((i > 0))
    {
      ans += (((a[i] * a[i])) - ((a[(i - 1)] * a[(i - 1)])));
      i -= 2;
    }
  }
  write((ans * pi), setprecision(9));
}

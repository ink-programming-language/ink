// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = cpp_array(103);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(s[i]);
      i += 1;
    }
  }
  sort((s + 1), ((s + 1) + n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(s[i]);
      i += 1;
    }
  }
}

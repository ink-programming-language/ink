// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n);
  var R: dynamic = n;
  var tmp: dynamic = n;
  var k: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var r: dynamic = i;
      k = cpp_cast(sqrt(((R * R) - (r * r))));
      ans += ( ((tmp == k)) ? 1 : (tmp - k));
      tmp = k;
      i += 1;
    }
  }
  if ((n == 0))
  {
    write(1, "\n");
  } else
  {
    write((ans * 4), "\n");
  }
  return 0;
}

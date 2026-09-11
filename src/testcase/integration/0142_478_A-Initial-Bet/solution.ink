// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(5);
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      read(a[i]);
      sum += a[i];
      i += 1;
    }
  }
  n = (sum / 5);
  if ((sum == 0))
  {
    write(-1);
  } else if (((n * 5) == sum))
  {
    write(n);
  } else
  {
    write(-1);
  }
  return 0;
}

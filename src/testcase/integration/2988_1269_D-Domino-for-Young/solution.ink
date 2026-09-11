// Translated from solution.cpp.

var a: dynamic = cpp_array(300005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var black: dynamic = 0;
  var wite: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i % 2))
      {
        black += 1;
        black += (((a[i] - 1)) / 2);
        wite += (a[i] / 2);
      } else
      {
        wite += 1;
        wite += (((a[i] - 1)) / 2);
        black += (a[i] / 2);
      }
      i += 1;
    }
  }
  write(min(wite, black), "\n");
}

// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = 0;
  var d: dynamic = 0;
  var beg: dynamic = 0;
  var end: dynamic = 0;
  read(n);
  var a: dynamic = cpp_array(n);
  end = (n - 1);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((i % 2) == 0))
      {
        if ((a[beg] < a[end]))
        {
          s += a[end];
          end -= 1;
        } else
        {
          s += a[beg];
          beg += 1;
        }
      } else
      {
        if ((a[beg] < a[end]))
        {
          d += a[end];
          end -= 1;
        } else
        {
          d += a[beg];
          beg += 1;
        }
      }
      i += 1;
    }
  }
  write(s, " ", d);
}

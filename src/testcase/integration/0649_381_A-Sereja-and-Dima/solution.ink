// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var s = 0;
  var d = 0;
  var beg = 0;
  var end = 0;
  read(n);
  var a = cpp_array(n);
  end = (n - 1);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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

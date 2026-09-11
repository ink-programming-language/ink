// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var max: dynamic = cpp_uninitialized();
  var count: dynamic = 0;
  var flag: dynamic = 0;
  read(n);
  var a: dynamic = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  read(m);
  var b: dynamic = cpp_array(m);
  {
    j = 0;
    while ((j < m))
    {
      read(b[j]);
      {
        i = 0;
        while ((i < n))
        {
          if (((b[j] % a[i]) == 0))
          {
            if ((flag == 0))
            {
              max = (b[j] / a[i]);
              count += 1;
              flag = 1;
            } else if (((b[j] / a[i]) == max))
            {
              count += 1;
            } else if (((b[j] / a[i]) > max))
            {
              max = (b[j] / a[i]);
              count = 1;
            }
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  write(count);
  return 0;
}

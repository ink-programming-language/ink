// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var max: dynamic;
  var count = 0;
  var flag = 0;
  read(n);
  var a = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  read(m);
  var b = cpp_array(m);
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

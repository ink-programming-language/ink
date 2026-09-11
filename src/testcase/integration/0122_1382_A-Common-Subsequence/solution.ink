// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    var arr: dynamic = cpp_array(n);
    var arr1: dynamic = cpp_array(m);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        read(arr1[i]);
        i += 1;
      }
    }
    var num: dynamic = -1;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            if ((arr[i] == arr1[j]))
            {
              num = arr[i];
              break;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    if ((num != -1))
    {
      write("YES", "\n");
      write("1 ", num, "\n");
    } else
    {
      write("NO", "\n");
    }
  }
}

// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var arr = cpp_array(n);
    var arr1 = cpp_array(m);
    {
      var i = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < m))
      {
        read(arr1[i]);
        i += 1;
      }
    }
    var num = -1;
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
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

// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var even = 0;
    var odd = 0;
    {
      var i = 0;
      while ((i < n))
      {
        read(arr[i]);
        if (((arr[i] % 2) == 0))
        {
          even += 1;
        } else
        {
          odd += 1;
        }
        i += 1;
      }
    }
    if ((((even % 2) == 0) && ((odd % 2) == 0)))
    {
      write("YES", "\n");
    } else
    {
      sort(arr.begin(), arr.end());
      {
        var i = 0;
        while ((i < (n - 1)))
        {
          if ((abs((arr[i] - arr[(i + 1)])) == 1))
          {
            even -= 1;
            odd -= 1;
            i += 2;
            if ((((even % 2) == 0) && ((odd % 2) == 0)))
            {
              break;
            }
          } else
          {
            i += 1;
          }
        }
      }
      if (((even % 2) == 0))
      {
        even = 0;
      }
      if (((odd % 2) == 0))
      {
        odd = 0;
      }
      if (((even != 0) || (odd != 0)))
      {
        write("NO", "\n");
      } else
      {
        write("YES", "\n");
      }
    }
  }
  return 0;
}

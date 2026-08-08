// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = (s[i] - cpp_char("0"));
      i += 1;
    }
  }
  var sum = 0;
  {
    var k = 0;
    while (((k + 1) < n))
    {
      sum += a[k];
      var c_sum = 0;
      {
        var i = (k + 1);
        while ((i < n))
        {
          if ((a[i] == 0))
          {
            i += 1;
            continue;
          }
          if ((c_sum == sum))
          {
            c_sum = 0;
          }
          c_sum += a[i];
          if ((c_sum > sum))
          {
            break;
          }
          i += 1;
        }
      }
      if ((c_sum == sum))
      {
        write("YES");
        return 0;
      }
      k += 1;
    }
  }
  write("NO");
}

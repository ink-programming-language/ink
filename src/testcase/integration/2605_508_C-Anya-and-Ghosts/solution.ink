// Translated from solution.cpp.

var cnt = cpp_array(1005);

var arr = cpp_array(1005);

func main()
{
  var m: dynamic;
  var t: dynamic;
  var r: dynamic;
  var a: dynamic;
  var ans = 0;
  read(m, t, r);
  {
    var i = 0;
    while ((i < m))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var k = 0;
      {
        var j = ((arr[i] - 1) + 305);
        while ((j > (((arr[i] - t) - 1) + 305)))
        {
          if ((cnt[j] == 1))
          {
            k += 1;
          }
          j -= 1;
        }
      }
      if ((k < r))
      {
        {
          var j = ((arr[i] - 1) + 305);
          while ((j > (((arr[i] - t) - 1) + 305)))
          {
            if ((cnt[j] == 0))
            {
              ans += 1;
              cnt[j] = 1;
              k += 1;
            }
            if ((k == r))
            {
              break;
            }
            j -= 1;
          }
        }
        if ((k < r))
        {
          write(-1, "\n");
          return 0;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

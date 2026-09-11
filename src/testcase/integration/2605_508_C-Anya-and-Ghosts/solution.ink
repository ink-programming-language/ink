// Translated from solution.cpp.

var cnt: dynamic = cpp_array(1005);

var arr: dynamic = cpp_array(1005);

func main() -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(m, t, r);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(arr[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var k: dynamic = 0;
      {
        var j: dynamic = ((arr[i] - 1) + 305);
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
          var j: dynamic = ((arr[i] - 1) + 305);
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

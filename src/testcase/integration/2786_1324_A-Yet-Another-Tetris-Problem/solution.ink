// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  read(t);
  var m: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < t))
    {
      read(m);
      var arr: dynamic = cpp_array(m);
      {
        j = 0;
        while ((j < m))
        {
          read(arr[j]);
          j += 1;
        }
      }
      sort(arr, (arr + n));
      var flag: dynamic = 0;
      {
        j = 0;
        while ((j < (m - 1)))
        {
          if (((abs((arr[j] - arr[(j + 1)])) % 2) != 0))
          {
            flag = 1;
            break;
          }
          j += 1;
        }
      }
      if ((flag == 1))
      {
        write("NO\n");
      } else
      {
        write("YES\n");
      }
      i += 1;
    }
  }
  return 0;
}

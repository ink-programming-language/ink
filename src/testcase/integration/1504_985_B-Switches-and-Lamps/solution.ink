// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s);
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((s[j] == cpp_char("1")))
          {
            vec[i][j] = 1;
          } else
          {
            vec[i][j] = 0;
          }
          sum[j] += vec[i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  var count: dynamic = cpp_uninitialized();
  var flag: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      count = 0;
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          sum[j] -= vec[i][j];
          if ((sum[j] >= 1))
          {
            count += 1;
          }
          sum[j] += vec[i][j];
          j += 1;
        }
      }
      if ((count == m))
      {
        write("Yes\n");
        flag = 1;
        break;
      }
      i += 1;
    }
  }
  if ((flag == 0))
  {
    write("No\n");
  }
}

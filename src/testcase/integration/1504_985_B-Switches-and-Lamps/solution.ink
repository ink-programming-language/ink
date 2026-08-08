// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var s: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(s);
      {
        var j = 0;
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
  var count: dynamic;
  var flag = 0;
  {
    var i = 0;
    while ((i < n))
    {
      count = 0;
      {
        var j = 0;
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

// Translated from solution.cpp.

var arr = cpp_array(1001, 1001);

func main()
{
  var row = [0];
  var col = [0];
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(arr[i][j]);
          if ((arr[i][j] == cpp_char("*")))
          {
            row[i] += 1;
            col[j] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          if ((arr[i][j] == cpp_char("*")))
          {
            ans += (((row[i] - 1)) * ((col[j] - 1)));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

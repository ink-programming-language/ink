// Translated from solution.cpp.

var arr: dynamic = cpp_array(1001, 1001);

func main() -> dynamic
{
  var row: dynamic = [0];
  var col: dynamic = [0];
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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

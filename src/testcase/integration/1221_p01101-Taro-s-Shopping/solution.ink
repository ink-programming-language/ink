// Translated from solution.cpp.

var int_cpp = dynamic;

var double = dynamic;

var INF = cpp_expression("#inc");

var a = cpp_array(1000);

func main()
{
  var N: dynamic;
  var M: dynamic;
  while (cpp_comma(((cin >> N) >> M), ((N + M) != 0)))
  {
    {
      var i = 0;
      while ((i < N))
      {
        read(a[i]);
        i += 1;
      }
    }
    var ans = 0;
    {
      var i = 0;
      while ((i < N))
      {
        {
          var j = 0;
          while ((j < N))
          {
            if (((i == j) || ((a[i] + a[j]) > M)))
            {
              j += 1;
              continue;
            }
            ans = max((a[i] + a[j]), ans);
            j += 1;
          }
        }
        i += 1;
      }
    }
    if ((ans == 0))
    {
      write("NONE", "\n");
    } else
    {
      write(ans, "\n");
    }
  }
}

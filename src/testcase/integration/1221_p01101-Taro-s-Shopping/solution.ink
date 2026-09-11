// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var cpp_double: dynamic = dynamic;

var INF: dynamic = cpp_expression("#inc");

var a: dynamic = cpp_array(1000);

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> N) >> M), ((N + M) != 0)))
  {
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        read(a[i]);
        i += 1;
      }
    }
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        {
          var j: dynamic = 0;
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

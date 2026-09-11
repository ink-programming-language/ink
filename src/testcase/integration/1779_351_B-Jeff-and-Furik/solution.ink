// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(4000);

var memo: dynamic = cpp_array(9000010);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(num[i]);
      i += 1;
    }
  }
  var cant: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          if ((num[i] > num[j]))
          {
            cant += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  memo[0] = 0.0;
  memo[1] = 1.0;
  {
    var i: dynamic = 2;
    while ((i <= cant))
    {
      memo[i] = (4 + memo[(i - 2)]);
      i += 1;
    }
  }
  printf("%.6lf\n", memo[cant]);
  return 0;
}

// Translated from solution.cpp.

var n: dynamic;

var num = cpp_array(4000);

var memo = cpp_array(9000010);

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(num[i]);
      i += 1;
    }
  }
  var cant = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
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
    var i = 2;
    while ((i <= cant))
    {
      memo[i] = (4 + memo[(i - 2)]);
      i += 1;
    }
  }
  printf("%.6lf\n", memo[cant]);
  return 0;
}

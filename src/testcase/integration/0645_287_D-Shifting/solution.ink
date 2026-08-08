// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var N = (4e6 + 5);

var a = cpp_array(N);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = (i + 1);
      i += 1;
    }
  }
  var L = 0;
  var R = (n - 1);
  {
    var i = 2;
    while ((i <= n))
    {
      {
        var k = (((n - 1)) / i);
        while ((k > 0))
        {
          swap(a[(L + (((k - 1)) * i))], a[(L + (k * i))]);
          k -= 1;
        }
      }
      R += 1;
      a[R] = a[L];
      L += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      printf("%d%s", a[(L + i)], if ((i == (n - 1))) "\n" else " ");
      i += 1;
    }
  }
  return 0;
}

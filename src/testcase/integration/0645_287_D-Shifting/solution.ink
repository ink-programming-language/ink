// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var N: dynamic = (4e6 + 5);

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      a[i] = (i + 1);
      i += 1;
    }
  }
  var L: dynamic = 0;
  var R: dynamic = (n - 1);
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      {
        var k: dynamic = (((n - 1)) / i);
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
    var i: dynamic = 0;
    while ((i < n))
    {
      printf("%d%s", a[(L + i)],  ((i == (n - 1))) ? "\n" : " ");
      i += 1;
    }
  }
  return 0;
}

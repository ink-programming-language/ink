// Translated from solution.cpp.

var N = ((200 * 1000) + 555);

var n: dynamic;

var k: dynamic;

var a = cpp_array(N);

func main()
{
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  a[cpp_update(n, "++")] = int_cpp(2e9);
  var ans = 0;
  var u = 0;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      while (((u < n) && (a[i] == a[u])))
      {
        u += 1;
      }
      if (((a[u] - a[i]) > k))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

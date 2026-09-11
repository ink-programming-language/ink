// Translated from solution.cpp.

var INF: dynamic = 1000000007;

var N: dynamic = 100100;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var used: dynamic = cpp_array(N);

func main() -> dynamic
{
  read(n, k);
  var last: dynamic = (1 + k);
  {
    var i: dynamic = 1;
    while ((i < k))
    {
      if (used[i])
      {
        i += 1;
        continue;
      }
      printf("%d ", i);
      used[i] = true;
      if (used[((last - i) + 1)])
      {
        i += 1;
        continue;
      }
      printf("%d ", ((last - i) + 1));
      used[((last - i) + 1)] = true;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!used[i]))
      {
        printf("%d ", i);
      }
      i += 1;
    }
  }
}

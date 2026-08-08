// Translated from solution.cpp.

var INF = 1000000007;

var N = 100100;

var n: dynamic;

var k: dynamic;

var used = cpp_array(N);

func main()
{
  read(n, k);
  var last = (1 + k);
  {
    var i = 1;
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
    var i = 1;
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

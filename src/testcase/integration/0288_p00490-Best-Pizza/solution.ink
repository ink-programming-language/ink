// Translated from solution.cpp.

var N: dynamic;

var A: dynamic;

var B: dynamic;

var C: dynamic;

var D = cpp_array(100);

func main()
{
  read(N, A, B, C);
  {
    var i = 0;
    while ((i != N))
    {
      read(D[i]);
      i += 1;
    }
  }
  sort(D, (D + N), greater());
  var sum = 0;
  var ans = 0;
  {
    var i = 0;
    while ((i != N))
    {
      sum += D[i];
      ans = max(ans, (((C + sum)) / ((A + (((i + 1)) * B)))));
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

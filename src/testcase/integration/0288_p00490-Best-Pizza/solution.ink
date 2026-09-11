// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var D: dynamic = cpp_array(100);

func main() -> dynamic
{
  read(N, A, B, C);
  {
    var i: dynamic = 0;
    while ((i != N))
    {
      read(D[i]);
      i += 1;
    }
  }
  sort(D, (D + N), greater());
  var sum: dynamic = 0;
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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

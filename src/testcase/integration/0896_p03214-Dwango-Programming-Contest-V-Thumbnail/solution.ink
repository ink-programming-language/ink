// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array((1 << 18));

var S: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(N);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(A[i]);
      S += A[i];
      A[i] *= N;
      i += 1;
    }
  }
  var maxn: dynamic = ((1 << 30));
  var r: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      if ((maxn > abs((S - A[i]))))
      {
        maxn = abs((S - A[i]));
        r = i;
      }
      i += 1;
    }
  }
  write((r - 1), "\n");
  return 0;
}

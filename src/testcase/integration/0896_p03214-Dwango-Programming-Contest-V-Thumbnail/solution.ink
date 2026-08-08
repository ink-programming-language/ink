// Translated from solution.cpp.

var N: dynamic;

var A = cpp_array((1 << 18));

var S: dynamic;

func main()
{
  read(N);
  {
    var i = 1;
    while ((i <= N))
    {
      read(A[i]);
      S += A[i];
      A[i] *= N;
      i += 1;
    }
  }
  var maxn = ((1 << 30));
  var r = 0;
  {
    var i = 1;
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

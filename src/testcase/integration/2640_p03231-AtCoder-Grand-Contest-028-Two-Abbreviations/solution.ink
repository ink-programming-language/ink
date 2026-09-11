// Translated from solution.cpp.

var Q: dynamic = 1000000007;

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  read(N, M);
  var S: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  read(S, T);
  var g: dynamic = 1;
  var l: dynamic = cpp_uninitialized();
  while (true)
  {
    l = (N * g);
    if (((l % M) == 0))
    {
      break;
    }
    g += 1;
  }
  g = ((N * M) / l);
  {
    var i: dynamic = 0;
    while ((i < g))
    {
      if ((S[((i * l) / M)] != T[((i * l) / N)]))
      {
        write(-1, "\n");
        return 0;
      }
      i += 1;
    }
  }
  write(l, "\n");
}

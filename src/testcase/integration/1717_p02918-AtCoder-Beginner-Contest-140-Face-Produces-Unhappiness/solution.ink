// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  read(N, K);
  read(S);
  var cnt: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (S.size() - 1)))
    {
      if ((S[i] == S[(i + 1)]))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  write(min((cnt + (2 * K)), (N - 1)), "\n");
  return 0;
}

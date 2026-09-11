// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  read(N, S);
  var cnt1: dynamic = 0;
  var cnt2: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((S[i] == cpp_char(".")))
      {
        cnt2 += 1;
      }
      i += 1;
    }
  }
  var ans: dynamic = cnt2;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((S[i] == cpp_char("#")))
      {
        cnt1 += 1;
      } else
      {
        cnt2 -= 1;
      }
      ans = min(ans, (cnt1 + cnt2));
      i += 1;
    }
  }
  write(ans, "\n");
}

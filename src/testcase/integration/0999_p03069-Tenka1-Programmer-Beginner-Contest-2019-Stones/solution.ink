// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var S: dynamic;
  read(N, S);
  var cnt1 = 0;
  var cnt2 = 0;
  {
    var i = 0;
    while ((i < N))
    {
      if ((S[i] == cpp_char(".")))
      {
        cnt2 += 1;
      }
      i += 1;
    }
  }
  var ans = cnt2;
  {
    var i = 0;
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

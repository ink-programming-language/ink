// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var K: dynamic;
  var S: dynamic;
  read(N, K);
  read(S);
  var cnt = 0;
  {
    var i = 0;
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

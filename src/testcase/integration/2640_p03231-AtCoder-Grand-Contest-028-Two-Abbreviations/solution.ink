// Translated from solution.cpp.

var Q = 1000000007;

func main()
{
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  var S: dynamic;
  var T: dynamic;
  read(S, T);
  var g = 1;
  var l: dynamic;
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
    var i = 0;
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

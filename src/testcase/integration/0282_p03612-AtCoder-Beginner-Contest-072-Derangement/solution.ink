// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  var S = 0;
  var seq = 0;
  var p: dynamic;
  {
    var i = 1;
    while ((i <= N))
    {
      read(p);
      if ((i != p))
      {
        S += ((seq / 2) + (seq % 2));
        seq = 0;
      }
      if ((i == p))
      {
        seq += 1;
      }
      i += 1;
    }
  }
  S += ((seq / 2) + (seq % 2));
  write(S, "\n");
  return 0;
}

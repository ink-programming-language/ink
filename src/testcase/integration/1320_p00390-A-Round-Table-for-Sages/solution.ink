// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func FOR(i: dynamic, s: dynamic, e: dynamic)
{
  cpp_macro("for(int i=s;i<e;i++)");
}

func main(argument_0: dynamic)
{
  var N: dynamic;
  var MINR = 1001;
  var MINL = 1001;
  read(N);
  REP(i, N);
  read(a[i]);
  REP(i, N);
  read(w[i]);
  if (((MINL == 1001) || (MINR == 1001)))
  {
    write(0, "\n");
  } else
  {
    write((MINL + MINR), "\n");
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    if (((a[i] == 0) && (w[i] < MINR)))
    {
      MINR = w[i];
    } else if (((a[i] == 1) && (w[i] < MINL)))
    {
      MINL = w[i];
    }
  }

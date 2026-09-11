// Translated from solution.cpp.

func REP(i: dynamic, a: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i = ((ll) a); i < ((ll) n); i++)");
}

func main(argument_0: dynamic) -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  REP(i, 0, N);
  read(D[i]);
  var ok: dynamic = cpp_construct(((N * N) + 1), true);
  REP(i, 0, N);
  {
    var x: dynamic = 0;
  }
  REP(i, 0, ((N * N) + 1));
  {
    if (ok[i])
    {
      write(i, "\n");
      break;
    }
  }
}

func REP(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic) -> dynamic
{
      x = ((x * 10) + D[j]);
      if ((x > (N * N)))
      {
        break;
      }
      ok[x] = false;
    }
